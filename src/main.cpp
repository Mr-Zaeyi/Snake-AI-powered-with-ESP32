#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <Wire.h>
#include <Adafruit_MPR121.h>

// Définition des pins
#define TFT_CS PB6  // D10
#define TFT_DC PC7  // D9
#define TFT_RST PA9 // D8
#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

uint16_t lasttouched = 0;
uint16_t currtouched = 0;

// Création de l'objet écran
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
// Création de l'objet clavier
Adafruit_MPR121 cap = Adafruit_MPR121();

int dx = 5;
int dy = 5;
int snakeheadx = 160;
int snakeheady = 120;
int L = 10;
const int MAX_TAIL = 100; // Longueur max du corps
int snakeBodyX[MAX_TAIL];
int snakeBodyY[MAX_TAIL];
int tailLength = 3; // Longueur actuelle du corps (initialement 3)
int longueur = 280;
int largeur = 200;
int pommex;
int pommey;
int deplacement;

void genererPomme()
{
  int minX = 10;
  int maxX = longueur - 10 - L;
  int minY = 10;
  int maxY = largeur - 10 - L;

  bool enCollision;

  do
  {
    // Coordonnées multiples de L, dans la zone noire (grille)
    pommex = (rand() % ((maxX - minX) / L + 1)) * L + minX;
    pommey = (rand() % ((maxY - minY) / L + 1)) * L + minY;

    enCollision = false;

    // Si la pomme est sur la tête
    if (pommex == snakeheadx && pommey == snakeheady)
    {
      enCollision = true;
    }

    // Si la pomme est sur un segment du corps
    for (int i = 0; i < tailLength; i++)
    {
      if (pommex == snakeBodyX[i] && pommey == snakeBodyY[i])
      {
        enCollision = true;
        break;
      }
    }

  } while (enCollision);

  tft.fillRect(pommex, pommey, L, L, ILI9341_RED);
  Serial.println("Adafruit MPR121 Capacitive Touch sensor test");

  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A))
  {
    Serial.println("MPR121 not found, check wiring?");
    while (1)
      ;
  }
  Serial.println("MPR121 found!");
}

bool collision()
{
  for (int i = 1; i < tailLength; i++)
  {
    if (snakeheadx == snakeBodyX[i] && snakeheady == snakeBodyY[i])
    {
      return true;
    }
  }
  return false;
}
bool collisionAvecArene(int x, int y, int longueur, int largeur, int L)
{

  // Vérifie si la tête est en dehors du rectangle noir (terrain de jeu)
  if (x < 10 || y < 10)
    return true; // Trop à gauche ou en haut
  if (x + L > (longueur - 10))
    return true; // Trop à droite
  if (y + L > (largeur - 10))
    return true; // Trop en bas

  return false; // Pas de collision
}

void setup()
{

  Serial.begin(115200);
  delay(200);
  Wire.begin();
  while (!Serial); // Leonardo: wait for serial monitor
  Serial.println("\nI2C Scanner");

  // Initialisation de l'écran
  tft.begin();

  // Orientation de l'écran
  tft.setRotation(1); // Paysage = 0 ; Portrait = 1 , Paysage inversé = 2

  tft.fillScreen(ILI9341_BLACK);
  tft.fillRect(0, 0, longueur, largeur, ILI9341_BLUE);
  tft.fillRect(10, 10, longueur - 20, largeur - 20, ILI9341_BLACK);
  snakeBodyX[0] = snakeheadx - L;
  snakeBodyX[1] = snakeheadx - 2 * L;
  snakeBodyX[2] = snakeheadx - 3 * L;

  snakeBodyY[0] = snakeheady;
  snakeBodyY[1] = snakeheady;
  snakeBodyY[2] = snakeheady;

  tft.fillRect(snakeheadx, snakeheady, L, L, ILI9341_GREEN); // Tête

  for (int i = 0; i < tailLength; i++)
  {
    tft.fillRect(snakeBodyX[i], snakeBodyY[i], L, L, ILI9341_YELLOW); // Corps
  }

  genererPomme();

  Serial.println("Adafruit MPR121 Capacitive Touch sensor test");

  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A))
  {
    Serial.println("MPR121 not found, check wiring?");
    while (1)
      ;
  }
  Serial.println("MPR121 found!");
}

void loop()
{ 
  
  currtouched = cap.touched();
  if (cap.filteredData(0)<=12)
  {
    deplacement=2;
  }
  else if (cap.filteredData(4)<=12)
  {
    deplacement=3;
  }
  else if (cap.filteredData(5)<=12)
  {
    deplacement=4;
  }
  else if (cap.filteredData(8)<=12)
  {
    deplacement=1;
  }
  
  


  
  // Game over si collision
  if (collision() || collisionAvecArene(snakeheadx, snakeheady, longueur, largeur, L))
  {
    tft.setCursor(50, 120);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(3);
    tft.print("GAME OVER");
    while (true) {

    }
  }

  // Effacer ancien corps
  for (int i = 0; i < tailLength; i++)
  {
    tft.fillRect(snakeBodyX[i], snakeBodyY[i], L, L, ILI9341_BLACK);
  }
  tft.fillRect(snakeheadx, snakeheady, L, L, ILI9341_BLACK); // Tête
  // 7 premiers déplacement choisis de façon à faire un test puis déplacement aléatoire
  /*static int t = 0;
  if (t <= 4)
    deplacement = 2;
  else if (t > 4 && t <= 7)
    deplacement = 4;
  else if (t > 7)
    deplacement = (rand() % 4) + 1;
  t++;*/

  // Éviter de revenir sur le segment précédent
  if (tailLength > 0)
  {
    int dx = snakeheadx - snakeBodyX[0];
    int dy = snakeheady - snakeBodyY[0];

    if (dx == L && deplacement == 2)
      deplacement = 1 ; // va pas à gauche
    if (dx == -L && deplacement == 1)
      deplacement = 2 ; // va pas à droite
    if (dy == L && deplacement == 4)
      deplacement = 3; // va pas en haut
    if (dy == -L && deplacement == 3)
      deplacement = 4; // va pas en bas
  }

  // Décaler les segments du corps
  for (int i = tailLength - 1; i > 0; i--)
  {
    snakeBodyX[i] = snakeBodyX[i - 1];
    snakeBodyY[i] = snakeBodyY[i - 1];
  }
  // Le 1er segment prend la place de l'ancienne tête
  if (tailLength > 0)
  {
    snakeBodyX[0] = snakeheadx;
    snakeBodyY[0] = snakeheady;
  }

  // Mise à jour de la tête
  if (deplacement == 1)
    snakeheadx += L; // droite
  else if (deplacement == 2)
    snakeheadx -= L; // gauche
  else if (deplacement == 3)
    snakeheady += L; // bas
  else if (deplacement == 4)
    snakeheady -= L; // haut

  // Dessiner le nouveau corps
  for (int i = 0; i < tailLength; i++)
  {
    tft.fillRect(snakeBodyX[i], snakeBodyY[i], L, L, ILI9341_YELLOW);
  }

  // Dessiner la tête
  tft.fillRect(snakeheadx, snakeheady, L, L, ILI9341_GREEN);

  // Agrandir le serpent quand il mange la pomme & faire réapparaitre une pomme
  if (snakeheadx == pommex && snakeheady == pommey)
  {
    tailLength++;
    genererPomme();
  }
  /*byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++)
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");*/
  // Get the currently touched pads
  currtouched = cap.touched();

  for (uint8_t i = 0; i < 12; i++)
  {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((currtouched & _BV(i)) )
    {
      Serial.print(i);
      Serial.println(" touched");
    }
    // if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)))
    {
      Serial.print(i);
      Serial.println(" released");
    }
  }

  // reset our state
  lasttouched = currtouched;

  // comment out this line for detailed data from the sensor!
  //return;

  // debugging info, what
  Serial.print("\t\t\t\t\t\t\t\t\t\t\t\t\t 0x");
  Serial.println(cap.touched(), HEX);
  Serial.print("Filt: ");
  for (uint8_t i = 0; i < 12; i++)
  {
    Serial.print(cap.filteredData(i));
    Serial.print("\t");
  }
  Serial.println();
  Serial.print("Base: ");
  for (uint8_t i = 0; i < 12; i++)
  {
    Serial.print(cap.baselineData(i));
    Serial.print("\t");
  }
  Serial.println();

  // put a delay so it isn't overwhelming

  delay(250);
}
