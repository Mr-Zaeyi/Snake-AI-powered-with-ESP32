#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_MPR121.h>
#include <stdlib.h>
#include <time.h>

// --- Broches pour ESP32-C6 ---
#define TFT_CS    21  // Chip select SPI
#define TFT_DC    19  // Data/Command
#define TFT_RST   20  // Reset
#define TFT_MISO  22  // MISO
#define TFT_MOSI  23  // MOSI
#define TFT_SCK   18  // SCK
#define SDA_PIN   5   // I2C SDA
#define SCL_PIN   4   // I2C SCL

// --- Variables Snake ---
int snakeheadx = 160;
int snakeheady = 120;
int L = 10;                     // Taille d'un segment
const int MAX_TAIL = 100;       
int snakeBodyX[MAX_TAIL];
int snakeBodyY[MAX_TAIL];
int tailLength = 3;
int dx = 5, dy = 5;
int longueur = 280;
int largeur = 200;
int pommex, pommey;
int deplacement;

// --- Capteurs et écran ---
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
Adafruit_MPR121 cap = Adafruit_MPR121();
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

// --- Fonctions ---
void genererPomme() {
    int minX = 10, maxX = longueur - 10 - L;
    int minY = 10, maxY = largeur - 10 - L;
    bool enCollision;

    do {
        pommex = (rand() % ((maxX - minX) / L + 1)) * L + minX;
        pommey = (rand() % ((maxY - minY) / L + 1)) * L + minY;

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

bool collision() {
    for (int i = 1; i < tailLength; i++) {
        if (snakeheadx == snakeBodyX[i] && snakeheady == snakeBodyY[i]) return true;
    }
    return false;
}

bool collisionAvecArene(int x, int y) {
    if (x < 10 || y < 10) return true;
    if (x + L > (longueur - 10)) return true;
    if (y + L > (largeur - 10)) return true;
    return false;
}

// --- Setup ---
void setup() {
    Serial.begin(115200);
    delay(200);

    // SPI écran
    SPI.begin(18, 22, 23, 21); // SCK, MISO, MOSI, CS
    tft.begin();
    tft.setRotation(1);  
    tft.fillScreen(ILI9341_BLACK);

    // I2C MPR121
    Wire.begin(SDA_PIN, SCL_PIN);
    if (!cap.begin(0x5A)) {
        Serial.println("MPR121 not found, check wiring?");
        while (1);
    }

    Serial.println("Setup OK, MPR121 found!");
    srand(time(NULL));
}

// --- Loop principal ---
void loop() {
    currtouched = cap.touched();

    // Menu principal
    static int menu = 0;
    while (menu == 0) {
        tft.setCursor(10, 10);
        tft.setTextColor(ILI9341_WHITE);
        tft.setTextSize(3);
        tft.print("SNAKE");
        tft.setCursor(10, 60);
        tft.print("0 : JOUER");

        if (cap.filteredData(0) <= 10) {
            menu = 1;

            // Initialisation jeu
            tft.fillRect(0, 0, longueur, largeur, ILI9341_BLUE);
            tft.fillRect(10, 10, longueur - 20, largeur - 20, ILI9341_BLACK);

            snakeBodyX[0] = snakeheadx - L;
            snakeBodyX[1] = snakeheadx - 2 * L;
            snakeBodyX[2] = snakeheadx - 3 * L;

            snakeBodyY[0] = snakeheady;
            snakeBodyY[1] = snakeheady;
            snakeBodyY[2] = snakeheady;

            tft.fillRect(snakeheadx, snakeheady, L, L, ILI9341_GREEN);
            for (int i = 0; i < tailLength; i++) {
                tft.fillRect(snakeBodyX[i], snakeBodyY[i], L, L, ILI9341_YELLOW);
            }

            genererPomme();
        }
    }

    // Déplacement avec MPR121
    if (cap.filteredData(0) <= 10) deplacement = 2; // gauche
    else if (cap.filteredData(4) <= 10) deplacement = 3; // bas
    else if (cap.filteredData(5) <= 10) deplacement = 4; // haut
    else if (cap.filteredData(8) <= 10) deplacement = 1; // droite

    // Effacer ancien corps
    for (int i = 0; i < tailLength; i++) tft.fillRect(snakeBodyX[i], snakeBodyY[i], L, L, ILI9341_BLACK);
    tft.fillRect(snakeheadx, snakeheady, L, L, ILI9341_BLACK);

    // Décaler les segments
    for (int i = tailLength - 1; i > 0; i--) {
        snakeBodyX[i] = snakeBodyX[i - 1];
        snakeBodyY[i] = snakeBodyY[i - 1];
    }
    if (tailLength > 0) {
        snakeBodyX[0] = snakeheadx;
        snakeBodyY[0] = snakeheady;
    }

    // Mise à jour tête
    if (deplacement == 1) snakeheadx += L;
    else if (deplacement == 2) snakeheadx -= L;
    else if (deplacement == 3) snakeheady += L;
    else if (deplacement == 4) snakeheady -= L;

    // Dessiner corps + tête
    for (int i = 0; i < tailLength; i++) tft.fillRect(snakeBodyX[i], snakeBodyY[i], L, L, ILI9341_YELLOW);
    tft.fillRect(snakeheadx, snakeheady, L, L, ILI9341_GREEN);

    // Manger pomme
    if (snakeheadx == pommex && snakeheady == pommey) {
        tailLength++;
        genererPomme();
    }

    // Game over
    if (collision() || collisionAvecArene(snakeheadx, snakeheady)) {
        tft.setCursor(10, 20);
        tft.setTextColor(ILI9341_WHITE);
        tft.setTextSize(3);
        tft.print("GAME OVER");
        while (true);
    }

    lasttouched = currtouched;
    delay(200);
}
