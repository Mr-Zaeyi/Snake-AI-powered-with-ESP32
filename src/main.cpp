#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_MPR121.h>

#define TFT_CS    21
#define TFT_DC    19
#define TFT_RST   20
#define TFT_MISO  22
#define TFT_MOSI  23
#define TFT_SCK   18
#define SDA_PIN   5
#define SCL_PIN   4

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

// Minuteur 

unsigned long lastMoveTime = 0;
unsigned long moveInterval = 200; // Vitesse (IA plus rapide)

// Machine à états

enum GameState {
  MENU,
  PLAYING_SOLO,
  PLAYING_IA,
  GAME_OVER
};
GameState gameState = MENU;

// Objets matériels 

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
Adafruit_MPR121 cap = Adafruit_MPR121();

// Réinitialiser 

void reinitJeu() {
    // Position initiale
    snakeheadx = 160;
    snakeheady = 120;
    deplacement = 1;
    tailLength = 3;
    score = 0;
    
    // Corps initial
    snakeBodyX[0] = snakeheadx - L;
    snakeBodyX[1] = snakeheadx - 2 * L;
    snakeBodyX[2] = snakeheadx - 3 * L;
    snakeBodyY[0] = snakeheady;
    snakeBodyY[1] = snakeheady;
    snakeBodyY[2] = snakeheady;
    
    // Redessiner arène
    tft.fillScreen(ILI9341_BLACK);
    tft.fillRect(0, 0, longueur, largeur, ILI9341_BLUE);
    tft.fillRect(10, 10, longueur - 20, largeur - 20, ILI9341_BLACK);
    
    // Dessiner serpent
    tft.fillRect(snakeheadx, snakeheady, L, L, ILI9341_GREEN);
    for (int i = 0; i < tailLength; i++) {
        tft.fillRect(snakeBodyX[i], snakeBodyY[i], L, L, ILI9341_YELLOW);
    }
}

// Le programme pomme de Théo 

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

// BOING !! 

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

// Position compliquée pour le serpent 

bool positionDangereuse(int x, int y) {
    // Hors limites
    if (x < 10 || y < 10) return true;
    if (x + L > (longueur - 10)) return true;
    if (y + L > (largeur - 10)) return true;
    
    // Sur le corps
    for (int i = 0; i < tailLength; i++) {
        if (x == snakeBodyX[i] && y == snakeBodyY[i]) return true;
    }
    return false;
}

// Algorithme de Manhattan 

int calculerDirectionIA() {

    // Calcul distance Manhattan vers pomme

    int dx = pommex - snakeheadx;
    int dy = pommey - snakeheady;
    
    // Directions possibles : 1=droite, 2=gauche, 3=bas, 4=haut

    int directions[4] = {0, 0, 0, 0}; // Scores pour chaque direction
    
    // Direction vers pomme (priorité haute)

    if (dx > 0) directions[0] += 10; // Aller à droite
    if (dx < 0) directions[1] += 10; // Aller à gauche
    if (dy > 0) directions[2] += 10; // Aller en bas
    if (dy < 0) directions[3] += 10; // Aller en haut
    
    // Vérifier sécurité de chaque direction


    int newX, newY;
    
    // Droite
    newX = snakeheadx + L;
    newY = snakeheady;
    if (!positionDangereuse(newX, newY)) directions[0] += 5;
    else directions[0] = -100; // Pour qu'il aille pas ( j'espère ) 
    
    // Gauche
    newX = snakeheadx - L;
    newY = snakeheady;
    if (!positionDangereuse(newX, newY)) directions[1] += 5;
    else directions[1] = -100;
    
    // Bas
    newX = snakeheadx;
    newY = snakeheady + L;
    if (!positionDangereuse(newX, newY)) directions[2] += 5;
    else directions[2] = -100;
    
    // Haut
    newX = snakeheadx;
    newY = snakeheady - L;
    if (!positionDangereuse(newX, newY)) directions[3] += 5;
    else directions[3] = -100;
    
 
    if (tailLength > 0) {
        int dxBody = snakeheadx - snakeBodyX[0];
        int dyBody = snakeheady - snakeBodyY[0];
        
        if (dxBody == L) directions[1] = -100;   // Pas à gauche
        if (dxBody == -L) directions[0] = -100;  // Pas à droite
        if (dyBody == L) directions[3] = -100;   // Pas en haut
        if (dyBody == -L) directions[2] = -100;  // Pas en bas
    }
    
    // Points pour direction 

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

// EN mode Solo

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

// Déplacement 

void deplacerSerpent() {
    // Effacer ancien
    for (int i = 0; i < tailLength; i++) {
        tft.fillRect(snakeBodyX[i], snakeBodyY[i], L, L, ILI9341_BLACK);
    }
    tft.fillRect(snakeheadx, snakeheady, L, L, ILI9341_BLACK);

    // Décaler segments
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

    // Dessiner nouveau
    for (int i = 0; i < tailLength; i++) {
        tft.fillRect(snakeBodyX[i], snakeBodyY[i], L, L, ILI9341_YELLOW);
    }
    tft.fillRect(snakeheadx, snakeheady, L, L, ILI9341_GREEN);
}


// VÉRIFIER POMME

void verifierPomme() {
    if (snakeheadx == pommex && snakeheady == pommey) {
        tailLength++;
        score++;
        genererPomme();
    }
}


// AFFICHER MENU

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

// ============================================
// AFFICHER GAME OVER
// ============================================
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
    
    tft.setCursor(20, 160);
    tft.setTextColor(ILI9341_YELLOW);
    tft.setTextSize(1);
    tft.print("[8] Retour au menu");
}

// ============================================
// SETUP
// ============================================
void setup() {
    Serial.begin(115200);
    Serial.println("=== SNAKE ESP32-C6 ===");

    // Init SPI
    SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);
    tft.begin();
    tft.setRotation(1);

    // Init I2C
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

    randomSeed(analogRead(0));
    afficherMenu();
    Serial.println("Menu affiché");
}

// On va utiliser une machine à états pour gérer les différents écrans et modes du jeu

void loop() {
    unsigned long currentTime = millis();
    
    // Le Menu

    if (gameState == MENU) {
        // Lecture touches menu
        if (cap.filteredData(0) <= 12) {
            Serial.println("Mode SOLO sélectionné");
            gameState = PLAYING_SOLO;
            moveInterval = 200; // Vitesse normale
            reinitJeu();
            genererPomme();
            delay(300); // Debounce simple
        }
        else if (cap.filteredData(4) <= 12) {
            Serial.println("Mode IA sélectionné");
            gameState = PLAYING_IA;
            moveInterval = 150; // IA plus rapide
            reinitJeu();
            genererPomme();
            delay(300);
        }
        return;
    }
    
    // Etat

    if (gameState == GAME_OVER) {
        // Touche 8 pour retour menu
        if (cap.filteredData(8) <= 12) {
            Serial.println("Retour au menu");
            gameState = MENU;
            afficherMenu();
            delay(300);
        }
        return;
    }
    
    // ========== ÉTAT : En SOlo ou Machine
    
    // Lecture clavier en mode SOLO uniquement
    if (gameState == PLAYING_SOLO) {
        lireClavier();
    }
    
    // Déplacement selon intervalle
    if (currentTime - lastMoveTime >= moveInterval) {
        lastMoveTime = currentTime;
        
        // IA calcule sa direction
        if (gameState == PLAYING_IA) {
            deplacement = calculerDirectionIA();
        }
        
        // Éviter retour arrière (solo uniquement, IA gère déjà)
        if (gameState == PLAYING_SOLO) {
            eviterRetourArriere();
        }
        
        // Déplacer
        deplacerSerpent();
        verifierPomme();
        
        // Vérifier collisions
        if (collisionCorps() || collisionMur()) {
            gameState = GAME_OVER;
            afficherGameOver();
            Serial.print("Game Over - Score: ");
            Serial.println(score);
        }
    }
}