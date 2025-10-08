#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>





  // Définition des pins
  #define TFT_CS   PB6  // D10
  #define TFT_DC   PC7  // D9
  #define TFT_RST  PA9  // D8
  // Création de l'objet écran
  Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
  
   int dx = 5;
   int dy = 5;
   int snakeheadx = 160;
   int snakeheady = 120;
   int L = 10;
   const int MAX_TAIL = 100; // Longueur max du corps
   int snakeBodyX[MAX_TAIL];
   int snakeBodyY[MAX_TAIL];
   int tailLength = 3; // Longueur actuelle du corps (initialement 3)
   int longueur= 280;
   int largeur= 200;
   int pommex;
   int pommey;

 void genererPomme() {
  int minX = 10;
  int maxX = longueur - 10 - L;
  int minY = 10;
  int maxY = largeur - 10 - L;

  bool enCollision;

  do {
    // Coordonnées multiples de L, dans la zone noire (grille)
    pommex = (rand() % ((maxX - minX) / L + 1)) * L + minX;
    pommey = (rand() % ((maxY - minY) / L + 1)) * L + minY;

    enCollision = false;

    // Si la pomme est sur la tête
    if (pommex == snakeheadx && pommey == snakeheady) {
      enCollision = true;
    }

    // Si la pomme est sur un segment du corps
    for (int i = 0; i < tailLength; i++) {
      if (pommex == snakeBodyX[i] && pommey == snakeBodyY[i]) {
        enCollision = true;
        break;
      }
    }

  } while (enCollision);

  tft.fillRect(pommex,pommey,L,L, ILI9341_RED);
}







bool collision() {
   for (int i =1; i < tailLength; i++){
      if (snakeheadx == snakeBodyX[i] && snakeheady == snakeBodyY[i]){
         return true;
      }
   }
   return false;
}
bool collisionAvecArene(int x, int y, int longueur, int largeur, int L) {
    
    
    // Vérifie si la tête est en dehors du rectangle noir (terrain de jeu)
    if (x < 10 || y < 10) return true;                      // Trop à gauche ou en haut
    if (x + L > (longueur - 10)) return true;              // Trop à droite
    if (y + L > (largeur - 10)) return true;               // Trop en bas
    
    return false; // Pas de collision
}


void setup()
{

  

 
  Serial.begin(115200);
  delay(200);

 
  // Initialisation de l'écran
  tft.begin();
 
  // Orientation de l'écran 
  tft.setRotation(1);  // Paysage = 0 ; Portrait = 1 , Paysage inversé = 2 
 
 
  
 
  
 
  tft.fillScreen(ILI9341_BLACK);
  tft.fillRect(0, 0, longueur, largeur, ILI9341_BLUE);
  tft.fillRect(10,10,longueur-20,largeur-20, ILI9341_BLACK);
  snakeBodyX[0] = snakeheadx - L;
  snakeBodyX[1] = snakeheadx - 2*L;
  snakeBodyX[2] = snakeheadx - 3*L;

  snakeBodyY[0] = snakeheady;
  snakeBodyY[1] = snakeheady;
  snakeBodyY[2] = snakeheady;

  tft.fillRect(snakeheadx, snakeheady, L, L, ILI9341_GREEN); // Tête

  for (int i = 0; i < tailLength; i++) {
    tft.fillRect(snakeBodyX[i], snakeBodyY[i], L, L, ILI9341_YELLOW); // Corps
  }

  genererPomme();
  

 


  

}


void loop()
{
   int deplacement;
   // Game over si collision
 if (collision()|| collisionAvecArene(snakeheadx, snakeheady, longueur, largeur, L)) {
  tft.setCursor(50, 120);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.print("GAME OVER");
  while (true) {
    
  }

  }


  // Effacer ancien corps
  for (int i = 0; i < tailLength; i++) {
    tft.fillRect(snakeBodyX[i], snakeBodyY[i], L, L, ILI9341_BLACK);
  }
  tft.fillRect(snakeheadx, snakeheady, L, L, ILI9341_BLACK); // Tête
  //7 premiers déplacement choisis de façon à faire un test puis déplacement aléatoire
  static int t= 0;
  if (t <= 4) deplacement = 2;       
  else if (t>4 && t<=7) deplacement = 4;  
  else if (t>7) deplacement = (rand() % 4) + 1;
  t++;
  
  


  
  

  // Éviter de revenir sur le segment précédent
  if (tailLength > 0) {
    int dx = snakeheadx - snakeBodyX[0];
    int dy = snakeheady - snakeBodyY[0];

    if (dx == L && deplacement == 2) deplacement = 1 + (rand() % 3); // va pas à gauche
    if (dx == -L && deplacement == 1) deplacement = 2 + (rand() % 3); // va pas à droite
    if (dy == L && deplacement == 4) deplacement = 1 + (rand() % 3); // va pas en haut
    if (dy == -L && deplacement == 3) deplacement = 1 + (rand() % 3); // va pas en bas
  }

  // Décaler les segments du corps
  for (int i = tailLength - 1; i > 0; i--) {
    snakeBodyX[i] = snakeBodyX[i - 1];
    snakeBodyY[i] = snakeBodyY[i - 1];
  }
  // Le 1er segment prend la place de l'ancienne tête
  if (tailLength > 0) {
    snakeBodyX[0] = snakeheadx;
    snakeBodyY[0] = snakeheady;
  }
  
  // Mise à jour de la tête
  if (deplacement == 1) snakeheadx += L;       // droite
  else if (deplacement == 2) snakeheadx -= L;  // gauche
  else if (deplacement == 3) snakeheady += L;  // bas
  else if (deplacement == 4) snakeheady -= L;  // haut
 

  // Dessiner le nouveau corps 
  for (int i = 0; i < tailLength; i++) {
    tft.fillRect(snakeBodyX[i], snakeBodyY[i], L, L, ILI9341_YELLOW);
  }

  // Dessiner la tête 
  tft.fillRect(snakeheadx, snakeheady, L, L, ILI9341_GREEN);


  //Agrandir le serpent quand il mange la pomme & faire réapparaitre une pomme
  if (snakeheadx == pommex && snakeheady == pommey) {
  tailLength++;
  genererPomme();
}

  delay(500);

}
