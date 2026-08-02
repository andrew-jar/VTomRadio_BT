WERSJA 1 - POPUP BT 4s - INSTRUKCJA INSTALACJI do VTomRadio_BT

1. Skopiuj folder bt_popup do src/plugins/bt_popup/
   Struktura:
   src/plugins/bt_popup/bt_popup.h
   src/plugins/bt_popup/bt_popup.cpp

2. W src/pluginsManager/pluginsManager.h dodaj:
   #include "bt_popup/bt_popup.h"

3. W src/pluginsManager/pluginsManager.cpp w funkcji init() dodaj:
   btPopupInit();

4. W src/main.cpp w loop() dodaj przed player.loop():
   btPopupLoop();

5. W src/core/display.cpp - znajdź funkcję gdzie rysujesz PLAYER
   Dodaj na początku drawPlayer() lub w putRequest:

   extern bool btPopupActive;
   extern String btPopupName;
   extern String btPopupMac;
   extern unsigned long btPopupUntil;

   W funkcji display::loop() lub w miejscu gdzie rysujesz ekrany:

   if (btPopupActive) {
       tft.fillScreen(TFT_BLACK);
       tft.setTextColor(TFT_CYAN, TFT_BLACK);
       tft.setTextSize(2);
       tft.setCursor(20, 60);
       tft.print("Połączono BT");

       tft.setTextColor(TFT_WHITE, TFT_BLACK);
       tft.setTextSize(2);
       tft.setCursor(10, 100);
       tft.print(btPopupName.substring(0, 20));

       tft.setTextSize(1);
       tft.setCursor(10, 140);
       tft.print(btPopupMac);

       tft.setCursor(10, 180);
       tft.setTextColor(TFT_GREEN, TFT_BLACK);
       int sec = (btPopupUntil - millis()) / 1000 + 1;
       if (sec <0) sec=0;
       tft.printf("Powrót za %ds...", sec);

       // ikonka BT - prosty kwadrat z BT
       tft.fillRoundRect(200, 60, 30, 30, 5, TFT_BLUE);
       tft.setCursor(208, 68);
       tft.setTextColor(TFT_WHITE);
       tft.print("BT");

       return; // nie rysuj reszty playera
   }

6. Sprawdź w src/core/config.h który Serial to WROOM:
   U Ciebie schemat: S3 TX -> WROOM RX, S3 RX -> WROOM TX
   Zazwyczaj: Serial2 na GPIO 16/17 lub 43/44
   W bt_popup.h zmień jeśli trzeba: #define WROOM_SERIAL Serial2

KOMPILACJA:
PlatformIO -> Build

TEST:
- Włącz radio, połącz JBL
- Na TFT powinien pojawić się na 4s napis "Połączono BT / JBL Tune 520BT"

Jeśli nie działa - w Serial Monitor zobaczysz [BT_POPUP] Connected...
