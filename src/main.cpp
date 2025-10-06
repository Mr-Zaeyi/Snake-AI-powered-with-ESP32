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
   int snakebodyx1 = snakeheadx-L;
   int snakebodyx2 = snakeheadx-2*L;
   int snakebodyx3 = snakeheadx-3*L;
   int snakebodyy1 = snakeheady;
   int snakebodyy2 = snakeheady;
   int snakebodyy3 = snakeheady;
   int deplacement = (rand() % 4)+1;
   

void setup()
{
  Serial.begin(115200);
  Serial.println("Boot");


  // Initialisation du port série pour debug
  Serial.begin(115200);
  delay(200);
  Serial.println("Démarrage test ILI9341...");
 
  // Initialisation de l'écran
  tft.begin();
 
  // Orientation de l'écran 
  tft.setRotation(1);  // Paysage = 0 ; Portrait = 1 , Paysage inversé = 2 etc....important pour le dx et le dy !!!!
 
  // Effacer l'écran en noir
  tft.fillScreen(ILI9341_BLACK);
 
  Serial.println("Écran initialisé !");
 
  // Fond coloré
  tft.fillScreen(ILI9341_BLUE);
  delay(200);
 
  // Rectangle rouge
  tft.fillRect(50, 50, 100, 80, ILI9341_RED);
  delay(200);
 
  // Texte blanc
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.setCursor(30, 100);
  tft.println("Il marche !");

   tft.fillRect(snakeheadx, snakeheady, L,L, ILI9341_GREEN);
  tft.fillRect(snakebodyx1,snakebodyy1,L,L,ILI9341_YELLOW);
  tft.fillRect(snakebodyx2,snakebodyy2,L,L,ILI9341_BLUE);
  tft.fillRect(snakebodyx3,snakebodyy3,L,L,ILI9341_RED);
 
  Serial.println("Test terminé !");

}

void loop()
{
  digitalToggle(LED_VERTE);
  

   // Animation simple : cercle qui bouge
  // static int x = 160;
  // static int y = 120;

 
  // Effacer l'ancien cercle
  tft.fillRect(snakeheadx, snakeheady,L,L, ILI9341_BLACK);
  tft.fillRect(snakebodyx1,snakebodyy1,L,L,ILI9341_BLACK);
  tft.fillRect(snakebodyx2,snakebodyy2,L,L,ILI9341_BLACK);
  tft.fillRect(snakebodyx3,snakebodyy3,L,L,ILI9341_BLACK);
  
  snakebodyx3=snakebodyx2;
  snakebodyx2=snakebodyx1;
  snakebodyx1=snakeheadx;
  snakebodyy3=snakebodyy2;
  snakebodyy2=snakebodyy1;
  snakebodyy1=snakeheady;

 

 

 if (deplacement==1){
    tft.fillRect(snakebodyx3,snakebodyy3,L,L,ILI9341_YELLOW);
    tft.fillRect(snakebodyx2,snakebodyy2,L,L,ILI9341_BLUE);
    tft.fillRect(snakebodyx1,snakebodyy1,L,L,ILI9341_RED);
    snakeheadx+=L;
    tft.fillRect(snakeheadx, snakeheady,L,L, ILI9341_GREEN);
 }
 else if (deplacement==2)
 {
    tft.fillRect(snakebodyx3,snakebodyy3,L,L,ILI9341_RED);
    tft.fillRect(snakebodyx2,snakebodyy2,L,L,ILI9341_BLUE);
    tft.fillRect(snakebodyx1,snakebodyy1,L,L,ILI9341_YELLOW);
    snakeheadx-=L;
    tft.fillRect(snakeheadx, snakeheady,L,L, ILI9341_GREEN);
 }
 else if (deplacement==3)
 {
  tft.fillRect(snakebodyx3,snakebodyy3,L,L,ILI9341_RED);
    tft.fillRect(snakebodyx2,snakebodyy2,L,L,ILI9341_BLUE);
    tft.fillRect(snakebodyx1,snakebodyy1,L,L,ILI9341_YELLOW);
    snakeheady+=L;
    tft.fillRect(snakeheadx, snakeheady,L,L, ILI9341_GREEN);
 }
 else{
    tft.fillRect(snakebodyx3,snakebodyy3,L,L,ILI9341_RED);
    tft.fillRect(snakebodyx2,snakebodyy2,L,L,ILI9341_BLUE);
    tft.fillRect(snakebodyx1,snakebodyy1,L,L,ILI9341_YELLOW);
    snakeheady-=L;
    tft.fillRect(snakeheadx, snakeheady,L,L, ILI9341_GREEN);
 }
 
  // Déplacer
  //snakeheadx += dx;
  //snakeheady += dy;
 
  // Rebondir sur les bords
  //if (snakeheadx <= 5 || snakeheadx >= 315) dx = -dx;
  //if (snakeheadx <= 5 || snakeheady >= 235) dy = -dy;
 
  // Dessiner le nouveau snake
  delay(500);
}
