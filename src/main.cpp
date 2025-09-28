#include <Arduino.h>

void setup()
{
  Serial.begin(115200);
  Serial.println("Boot");

  pinMode(LED_VERTE, OUTPUT);
}

void loop()
{
  digitalToggle(LED_VERTE);
  delay(1000);
}
