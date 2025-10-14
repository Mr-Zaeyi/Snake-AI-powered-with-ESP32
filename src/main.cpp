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

// Variables globales pour notre SNaaaaaaaake 

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
int deplacement = 1; // Droite par défaut
int score = 0;

// Compteur 

unsigned long lastMoveTime = 0;
const unsigned long moveInterval = 200; // Vitesse du serpent (ms)

// Etapes - Peut-être un switch-case plus tard

enum GameState {
  PLAYING, // = 0
  GAME_OVER
};
GameState gameState = PLAYING;

// Objets de Bibliothèque 

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
Adafruit_MPR121 cap = Adafruit_MPR121();

// Les applessssssss 
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

        if (pommex == snakeheadx && pommey == snakeheady) {
            enCollision = true;
        }

        for (int i = 0; i < tailLength; i++) {
            if (pommex == snakeBodyX[i] && pommey == snakeBodyY[i]) {
                enCollision = true;
                break;
            }
        }
    } while (enCollision);

    tft.fillRect(pommex, pommey, L, L, ILI9341_RED);
}

// Qu'il se retourne pas 

bool collisionCorps() {
    for (int i = 1; i < tailLength; i++) {
        if (snakeheadx == snakeBodyX[i] && snakeheady == snakeBodyY[i]) {
            return true;
        }
    }
    return false;
}

// COllision 

bool collisionMur() {
    if (snakeheadx < 10 || snakeheady < 10) return true;
    if (snakeheadx + L > (longueur - 10)) return true;
    if (snakeheady + L > (largeur - 10)) return true;
    return false;
}

// Lecture clavier

void lireClavier() {
    // Détection simple par seuil
    if (cap.filteredData(0) <= 12) {
        deplacement = 2; // Gauche
    } else if (cap.filteredData(4) <= 12) {
        deplacement = 3; // Bas
    } else if (cap.filteredData(5) <= 12) {
        deplacement = 4; // Haut
    } else if (cap.filteredData(8) <= 12) {
        deplacement = 1; // Droite
    }
}


// ÉVITER RETOUR ARRIÈRE

void eviterRetourArriere() {
    if (tailLength > 0) {
        int dx = snakeheadx - snakeBodyX[0];
        int dy = snakeheady - snakeBodyY[0];

        if (dx == L && deplacement == 2) deplacement = 1;   // Pas à gauche
        if (dx == -L && deplacement == 1) deplacement = 2;  // Pas à droite
        if (dy == L && deplacement == 4) deplacement = 3;   // Pas en haut
        if (dy == -L && deplacement == 3) deplacement = 4;  // Pas en bas
    }
}

// ============================================
// DÉPLACER LE SERPENT
// ============================================
void deplacerSerpent() {
    // Effacer ancien corps
    for (int i = 0; i < tailLength; i++) {
        tft.fillRect(snakeBodyX[i], snakeBodyY[i], L, L, ILI9341_BLACK);
    }
    tft.fillRect(snakeheadx, snakeheady, L, L, ILI9341_BLACK);

    // Décaler segments
    for (int i = tailLength - 1; i > 0; i--) {
        snakeBodyX[i] = snakeBodyX[i - 1];
        snakeBodyY[i] = snakeBodyY[i - 1];
    }

    // Le 1er segment prend la place de l'ancienne tête
    if (tailLength > 0) {
        snakeBodyX[0] = snakeheadx;
        snakeBodyY[0] = snakeheady;
    }

    // Mise à jour tête selon direction
    if (deplacement == 1) snakeheadx += L;      // Droite
    else if (deplacement == 2) snakeheadx -= L; // Gauche
    else if (deplacement == 3) snakeheady += L; // Bas
    else if (deplacement == 4) snakeheady -= L; // Haut

    // Dessiner nouveau corps
    for (int i = 0; i < tailLength; i++) {
        tft.fillRect(snakeBodyX[i], snakeBodyY[i], L, L, ILI9341_YELLOW);
    }

    // Dessiner nouvelle tête
    tft.fillRect(snakeheadx, snakeheady, L, L, ILI9341_GREEN);
}

// ============================================
// VÉRIFIER SI POMME MANGÉE
// ============================================
void verifierPomme() {
    if (snakeheadx == pommex && snakeheady == pommey) {
        tailLength++;
        score++;
        genererPomme();
        Serial.print("Score: ");
        Serial.println(score);
    }
}

// ============================================
// AFFICHER GAME OVER (non-bloquant)
// ============================================
void afficherGameOver() {
    tft.setCursor(60, 100);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(3);
    tft.print("GAME OVER");
    
    tft.setCursor(80, 140);
    tft.setTextSize(2);
    tft.print("Score: ");
    tft.print(score);

    Serial.print("Partie terminée - Score final: ");
    Serial.println(score);
}


// SETUP

void setup() {
    Serial.begin(115200);
    Serial.println("=== SNAKE ESP32-C6 - MODE SOLO ===");

    // Init SPI écran
    SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);
    tft.begin();
    tft.setRotation(1);

    // Dessiner arène
    tft.fillScreen(ILI9341_BLACK);
    tft.fillRect(0, 0, longueur, largeur, ILI9341_BLUE);
    tft.fillRect(10, 10, longueur - 20, largeur - 20, ILI9341_BLACK);

    // Init I2C clavier
    Wire.begin(SDA_PIN, SCL_PIN);
    if (!cap.begin(0x5A)) {
        Serial.println("ERREUR: MPR121 non détecté");
        tft.setCursor(10, 10);
        tft.setTextColor(ILI9341_RED);
        tft.setTextSize(2);
        tft.print("ERREUR CLAVIER");
        gameState = GAME_OVER; // Pas de while(1), on bloque juste le jeu
        return;
    }
    Serial.println("MPR121 OK");

    // Init corps serpent
    snakeBodyX[0] = snakeheadx - L;
    snakeBodyX[1] = snakeheadx - 2 * L;
    snakeBodyX[2] = snakeheadx - 3 * L;
    snakeBodyY[0] = snakeheady;
    snakeBodyY[1] = snakeheady;
    snakeBodyY[2] = snakeheady;

    // Dessiner serpent initial
    tft.fillRect(snakeheadx, snakeheady, L, L, ILI9341_GREEN);
    for (int i = 0; i < tailLength; i++) {
        tft.fillRect(snakeBodyX[i], snakeBodyY[i], L, L, ILI9341_YELLOW);
    }

    // Générer pomme
    randomSeed(analogRead(0));
    genererPomme();

    lastMoveTime = millis();
    Serial.println("Jeu démarré !");
}

// LOOP - 100% NON-BLOQUANT

void loop() {
    // Si game over, ne rien faire (mais pas de while!)
    if (gameState == GAME_OVER) {
        return;
    }

    
    lireClavier();

    // Déplacer uniquement après l'intervalle de temps
    unsigned long currentTime = millis();
    if (currentTime - lastMoveTime >= moveInterval) {
        lastMoveTime = currentTime;

        // Éviter retour arrière
        eviterRetourArriere();

        // Déplacer serpent
        deplacerSerpent();

        // Vérifier pomme
        verifierPomme();

        // Vérifier collisions
        if (collisionCorps() || collisionMur()) {
            gameState = GAME_OVER;
            afficherGameOver();
        }
    }
}