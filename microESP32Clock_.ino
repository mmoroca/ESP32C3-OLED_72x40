/*

  microESP32Clock.ino
  
  Internet clock for the ESP32-C3 with 72x40 oled display (@mmoroca 20250311)

  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2016, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list 
    of conditions and the following disclaimer.
    
  * Redistributions in binary form must reproduce the above copyright notice, this 
    list of conditions and the following disclaimer in the documentation and/or other 
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  

*/

#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#include <WiFi.h>
#include "time.h"

const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_PASS";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

int LED = 8;

bool switchEnc;
uint8_t m = 24;

int width = 72; 
int height = 40; 
int xOffset = 30; // = (132-w)/2 
int yOffset = 12; // = (64-h)/2 

// U8g2 Contructor
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 6, 5); 

void setup(void) {
  u8g2.begin(); // init led display 
  u8g2.setContrast(255); // set contrast to maximum 
  u8g2.setBusClock(400000); //400kHz I2C 
  u8g2.setFont(u8g2_font_5x7_tr); // set the interface font

  pinMode(LED, OUTPUT); // set the pin for the integrated led
  
  Serial.begin(115200);

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  u8g2.firstPage();
  do {
    u8g2.drawStr(xOffset+3,yOffset+15,"CONECTANDO...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("Connected!");
    u8g2.drawStr(xOffset+8,yOffset+25,"¡CONECTADO!");
  } while ( u8g2.nextPage() );

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  // set the clock front face font
  u8g2.setFont(u8g2_font_ncenB10_tr); 
}

void printLocalTime(){
  struct tm timeinfo;
  char reloj[20];
  //u8g2.clearBuffer(); // Update screen method A: more memory
  u8g2.firstPage(); // Update screeen method B: less memory 
  do {
    u8g2.drawFrame(xOffset+0, yOffset+0, width, height); //draw a frame around the border 

    // Get local time from the time server
    if(!getLocalTime(&timeinfo)){
      Serial.println("Failed to obtain time");
      u8g2.drawStr(xOffset+20,yOffset+25,"¡ERROR!");
      return;
    }
    strftime(reloj, sizeof(reloj), "%H:%M:%S", &timeinfo); // Format time string
    u8g2.drawStr(xOffset+8, yOffset+25, reloj); // Show time string
  
  } while ( u8g2.nextPage() ); // Update screeen method B: flush data page 
  //u8g2.sendBuffer(); // Update screeen method A: transfer internal memory to the display 

  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void loop(void) {

  // Get Internet time
  printLocalTime();

  // Blink integrated led every second 
  delay(1000);
  digitalWrite(LED, switchEnc);
  switchEnc=!switchEnc;
}
