#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_MPR121.h>
#include <WiFi.h>
#include <WebServer.h>


// Pins (quand je mets const uint8_t ça bug quelquefois???)

#define TFT_CS    21
#define TFT_DC    19
#define TFT_RST   20
#define TFT_MISO  22
#define TFT_MOSI  23
#define TFT_SCK   18
#define SDA_PIN   5
#define SCL_PIN   4


// WIFI
const char* ssid = "Projet_IE3_Snake";
const char* password = "Palantir03";
WebServer server(80);


// VARIABLES SNAKE

int snakeheadx = 160;
int snakeheady = 120;
const int L = 10;
const int MAX_TAIL = 100;
int snakeBodyX[MAX_TAIL];
int snakeBodyY[MAX_TAIL];
int tailLength = 3;
const int longueur = 280;
const int largeur = 200;
int pommex, pommey;
int deplacement = 1;
int score = 0;

unsigned long lastMoveTime = 0;
unsigned long moveInterval = 200;
unsigned long lastButtonPress = 0; 
const unsigned long DEBOUNCE_DELAY = 300;

enum GameState {
  MENU,
  PLAYING_SOLO,
  PLAYING_IA,
  GAME_OVER
};
GameState gameState = MENU;
GameState previousGameMode = MENU;

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
Adafruit_MPR121 cap = Adafruit_MPR121();


// Page HTML

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Progression de notre Snake trop magnifique</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            background: rgba(255, 255, 255, 0.1);
            backdrop-filter: blur(10px);
            border-radius: 20px;
            padding: 40px;
            max-width: 500px;
            width: 100%;
            box-shadow: 0 8px 32px 0 rgba(31, 38, 135, 0.37);
            border: 1px solid rgba(255, 255, 255, 0.18);
        }
        h1 {
            text-align: center;
            font-size: 2.5em;
            margin-bottom: 30px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        .stats {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
            margin-bottom: 30px;
        }
        .stat-card {
            background: rgba(255, 255, 255, 0.2);
            padding: 20px;
            border-radius: 15px;
            text-align: center;
            transition: transform 0.3s;
        }
        .stat-card:hover {
            transform: translateY(-5px);
        }
        .stat-label {
            font-size: 0.9em;
            opacity: 0.8;
            margin-bottom: 10px;
        }
        .stat-value {
            font-size: 2.5em;
            font-weight: bold;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        .status {
            text-align: center;
            padding: 15px;
            background: rgba(255, 255, 255, 0.2);
            border-radius: 15px;
            font-size: 1.2em;
            margin-bottom: 20px;
        }
        .status.alive { background: rgba(34, 197, 94, 0.3); }
        .status.dead { background: rgba(239, 68, 68, 0.3); }
        .mode {
            text-align: center;
            font-size: 1.1em;
            opacity: 0.9;
            padding: 10px;
            background: rgba(255, 255, 255, 0.15);
            border-radius: 10px;
            margin-bottom: 20px;
        }
        .update-time {
            text-align: center;
            margin-top: 20px;
            font-size: 0.85em;
            opacity: 0.7;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>📊 PROGRESSION SCORE</h1>
        
        <div class="mode" id="mode">Mode: Chargement...</div>
        
        <div class="status alive" id="status">⚡ EN JEU</div>
        
        <div class="stats">
            <div class="stat-card">
                <div class="stat-label">Score</div>
                <div class="stat-value" id="score">-</div>
            </div>
            <div class="stat-card">
                <div class="stat-label">Longueur</div>
                <div class="stat-value" id="length">-</div>
            </div>
        </div>
        
        <div class="update-time">Mise à jour : <span id="updateTime">-</span></div>
    </div>

    <script>
        async function updateStats() {
            try {
                const response = await fetch('/stats');
                const data = await response.json();
                
                document.getElementById('score').textContent = data.score;
                document.getElementById('length').textContent = data.length;
                document.getElementById('mode').textContent = 'Mode: ' + data.mode;
                
                const statusEl = document.getElementById('status');
                if (data.status === 'alive') {
                    statusEl.textContent = '⚡ EN JEU';
                    statusEl.className = 'status alive';
                } else {
                    statusEl.textContent = '💀 GAME OVER';
                    statusEl.className = 'status dead';
                }
                
                const now = new Date();
                document.getElementById('updateTime').textContent = 
                    now.toLocaleTimeString('fr-FR');
                    
            } catch (error) {
                console.error('Erreur:', error);
            }
        }
        
        updateStats();
        setInterval(updateStats, 1000);
    </script>
</body>
</html>
)rawliteral";


// Handlers web

void handleRoot() {
    server.send(200, "text/html", INDEX_HTML);
}

void handleStats() {
    String mode = "MENU";
    if (gameState == PLAYING_SOLO) mode = "SOLO";
    else if (gameState == PLAYING_IA) mode = "IA";
    else if (gameState == GAME_OVER) mode = "GAME OVER";
    
    String status = (gameState == PLAYING_SOLO || gameState == PLAYING_IA) ? "alive" : "dead";
    
    String json = "{";
    json += "\"score\":" + String(score) + ",";
    json += "\"length\":" + String(tailLength) + ",";
    json += "\"mode\":\"" + mode + "\",";
    json += "\"status\":\"" + status + "\"";
    json += "}";
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", json);
}


// Fonctions

void reinitJeu() {
    snakeheadx = 160;
    snakeheady = 120;
    deplacement = 1;
    tailLength = 3;
    score = 0;

    snakeBodyX[0] = snakeheadx - L;
    snakeBodyX[1] = snakeheadx - 2 * L;
    snakeBodyX[2] = snakeheadx - 3 * L;
    snakeBodyY[0] = snakeheady;
    snakeBodyY[1] = snakeheady;
    snakeBodyY[2] = snakeheady;

    tft.fillScreen(ILI9341_BLACK);
    tft.fillRect(0, 0, longueur, largeur, ILI9341_BLUE);
    tft.fillRect(10, 10, longueur - 20, largeur - 20, ILI9341_BLACK);

    tft.fillRect(snakeheadx, snakeheady, L, L, ILI9341_GREEN);
    for (int i = 0; i < tailLength; i++) {
        tft.fillRect(snakeBodyX[i], snakeBodyY[i], L, L, ILI9341_YELLOW);
    }
}

void genererPomme() {
    int minX = 10;
    int maxX = longueur - 10 - L;
    int minY = 10;
    int maxY = largeur - 10 - L;
    bool enCollision;

    do {
        pommex = (random((maxX - minX) / L + 1)) * L + minX;
        pommey = (random((maxY - minY) / L + 1)) * L + minY;

        enCollision = false;
        if (pommex == snakeheadx && pommey == snakeheady) enCollision = true;

        for (int i = 0; i < tailLength; i++) {
            if (pommex == snakeBodyX[i] && pommey == snakeBodyY[i]) {
                enCollision = true;
                break;
            }
        }
    } while (enCollision);

    tft.fillRect(pommex, pommey, L, L, ILI9341_RED);
}

bool collisionCorps() {
    for (int i = 1; i < tailLength; i++) {
        if (snakeheadx == snakeBodyX[i] && snakeheady == snakeBodyY[i]) return true;
    }
    return false;
}

bool collisionMur() {
    if (snakeheadx < 10 || snakeheady < 10) return true;
    if (snakeheadx + L > (longueur - 10)) return true;
    if (snakeheady + L > (largeur - 10)) return true;
    return false;
}

bool positionDangereuse(int x, int y) {
    if (x < 10 || y < 10) return true;
    if (x + L > (longueur - 10)) return true;
    if (y + L > (largeur - 10)) return true;

    for (int i = 0; i < tailLength; i++) {
        if (x == snakeBodyX[i] && y == snakeBodyY[i]) return true;
    }
    return false;
}

int calculerDirectionIA() {
    int dx = pommex - snakeheadx;
    int dy = pommey - snakeheady;

    int directions[4] = {0, 0, 0, 0};

    if (dx > 0) directions[0] += 10;
    if (dx < 0) directions[1] += 10;
    if (dy > 0) directions[2] += 10;
    if (dy < 0) directions[3] += 10;

    int newX, newY;

    newX = snakeheadx + L;
    newY = snakeheady;
    if (!positionDangereuse(newX, newY)) directions[0] += 5;
    else directions[0] = -100;

    newX = snakeheadx - L;
    newY = snakeheady;
    if (!positionDangereuse(newX, newY)) directions[1] += 5;
    else directions[1] = -100;

    newX = snakeheadx;
    newY = snakeheady + L;
    if (!positionDangereuse(newX, newY)) directions[2] += 5;
    else directions[2] = -100;

    newX = snakeheadx;
    newY = snakeheady - L;
    if (!positionDangereuse(newX, newY)) directions[3] += 5;
    else directions[3] = -100;

    if (tailLength > 0) {
        int dxBody = snakeheadx - snakeBodyX[0];
        int dyBody = snakeheady - snakeBodyY[0];

        if (dxBody == L) directions[1] = -100;
        if (dxBody == -L) directions[0] = -100;
        if (dyBody == L) directions[3] = -100;
        if (dyBody == -L) directions[2] = -100;
    }

    int meilleurScore = -1000;
    int meilleureDir = 1;

    for (int i = 0; i < 4; i++) {
        if (directions[i] > meilleurScore) {
            meilleurScore = directions[i];
            meilleureDir = i + 1;
        }
    }

    return meilleureDir;
}

void lireClavier() {
    if (cap.filteredData(0) <= 12) deplacement = 2;
    else if (cap.filteredData(4) <= 12) deplacement = 3;
    else if (cap.filteredData(5) <= 12) deplacement = 4;
    else if (cap.filteredData(8) <= 12) deplacement = 1;
}

void eviterRetourArriere() {
    if (tailLength > 0) {
        int dx = snakeheadx - snakeBodyX[0];
        int dy = snakeheady - snakeBodyY[0];

        if (dx == L && deplacement == 2) deplacement = 1;
        if (dx == -L && deplacement == 1) deplacement = 2;
        if (dy == L && deplacement == 4) deplacement = 3;
        if (dy == -L && deplacement == 3) deplacement = 4;
    }
}

void deplacerSerpent() {
    for (int i = 0; i < tailLength; i++) {
        tft.fillRect(snakeBodyX[i], snakeBodyY[i], L, L, ILI9341_BLACK);
    }
    tft.fillRect(snakeheadx, snakeheady, L, L, ILI9341_BLACK);

    for (int i = tailLength - 1; i > 0; i--) {
        snakeBodyX[i] = snakeBodyX[i - 1];
        snakeBodyY[i] = snakeBodyY[i - 1];
    }

    if (tailLength > 0) {
        snakeBodyX[0] = snakeheadx;
        snakeBodyY[0] = snakeheady;
    }

    if (deplacement == 1) snakeheadx += L;
    else if (deplacement == 2) snakeheadx -= L;
    else if (deplacement == 3) snakeheady += L;
    else if (deplacement == 4) snakeheady -= L;

    for (int i = 0; i < tailLength; i++) {
        tft.fillRect(snakeBodyX[i], snakeBodyY[i], L, L, ILI9341_YELLOW);
    }
    tft.fillRect(snakeheadx, snakeheady, L, L, ILI9341_GREEN);
}

void verifierPomme() {
    if (snakeheadx == pommex && snakeheady == pommey) {
        tailLength++;
        score++;
        genererPomme();
    }
}

void afficherMenu() {
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(60, 30);
    tft.setTextColor(ILI9341_GREEN);
    tft.setTextSize(4);
    tft.print("SNAKE");

    tft.setCursor(20, 80);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.print("[0] MODE SOLO");

    tft.setCursor(20, 110);
    tft.setTextColor(ILI9341_CYAN);
    tft.print("[4] MODE IA");

    tft.setCursor(30, 160);
    tft.setTextColor(ILI9341_YELLOW);
    tft.setTextSize(1);
    tft.print("Selectionne un mode...");
}

void afficherGameOver() {
    tft.setCursor(40, 80);
    tft.setTextColor(ILI9341_RED);
    tft.setTextSize(3);
    tft.print("GAME OVER");

    tft.setCursor(60, 120);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.print("Score: ");
    tft.print(score);

    tft.setCursor(20, 150);
    tft.setTextColor(ILI9341_GREEN);
    tft.setTextSize(1);
    tft.print("[0] Rejouer");
    
    tft.setCursor(20, 170);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print("[8] Menu");
}


// SETUP

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== SNAKE ESP32-C6 avec WiFi ===");

    // Init écran
    SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);
    tft.begin();
    tft.setRotation(1);

    // Init clavier
    Wire.begin(SDA_PIN, SCL_PIN);
    if (!cap.begin(0x5A)) {
        Serial.println("ERREUR: MPR121");
        tft.fillScreen(ILI9341_BLACK);
        tft.setCursor(10, 10);
        tft.setTextColor(ILI9341_RED);
        tft.setTextSize(2);
        tft.print("ERREUR CLAVIER");
        while(1);
    }

    // Init WiFi en Point d'Accès
    Serial.println("\n--- Configuration WiFi ---");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("Réseau WiFi: ");
    Serial.println(ssid);
    Serial.print("Mot de passe: ");
    Serial.println(password);
    Serial.print("Adresse IP: ");
    Serial.println(IP);
    Serial.println("Ouvrez http://" + IP.toString() + " dans votre navigateur\n");

    // Init serveur web
    server.on("/", handleRoot);
    server.on("/stats", handleStats);
    server.begin();
    Serial.println("Serveur web démarré !\n");

    randomSeed(analogRead(0));
    afficherMenu();
    Serial.println("Menu affiché - Prêt à jouer !");
}


// LOOP

void loop() {
    unsigned long currentTime = millis();
    
    // Traiter requêtes web
    server.handleClient();

    if (gameState == MENU) {
        if (cap.filteredData(0) <= 12) {
            Serial.println("Mode SOLO sélectionné");
            gameState = PLAYING_SOLO;
            moveInterval = 200;
            reinitJeu();
            genererPomme();
            previousGameMode = PLAYING_SOLO;
        }
        else if (cap.filteredData(4) <= 12) {
            Serial.println("Mode IA sélectionné");
            gameState = PLAYING_IA;
            moveInterval = 150;
            reinitJeu();
            genererPomme();
            previousGameMode = PLAYING_IA;
        }
        return;
    }

    if (gameState == GAME_OVER) {
        if (cap.filteredData(8) <= 12) {
            Serial.println("Retour au menu");
            gameState = MENU;
            afficherMenu();
        }
        else if (cap.filteredData(0) <= 12) {
            Serial.println("Rejouer en mode précédent");
            gameState = previousGameMode;
            reinitJeu();
            genererPomme();
        }
        return;
    }

    if (gameState == PLAYING_SOLO) {
        lireClavier();
    }

    if (currentTime - lastMoveTime >= moveInterval) {
        lastMoveTime = currentTime;

        if (gameState == PLAYING_IA) {
            deplacement = calculerDirectionIA();
        }

        if (gameState == PLAYING_SOLO) {
            eviterRetourArriere();
        }

        deplacerSerpent();
        verifierPomme();

        if (collisionCorps() || collisionMur()) {
            gameState = GAME_OVER;
            afficherGameOver();
            Serial.print("Game Over - Score: ");
            Serial.println(score);
        }
    }
}