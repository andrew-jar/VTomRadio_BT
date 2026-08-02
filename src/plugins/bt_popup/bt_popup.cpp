#include "bt_popup.h"
#include "core/display.h"
#include <Arduino.h>

String btPopupName = "";
String btPopupMac = "";
volatile bool btPopupActive = false;
volatile unsigned long btPopupUntil = 0;
bool btConnected = false;

// Parser dla linii z WROOM: EVT A2DP_CONN CONNECTED MAC=... NAME="..."
void btPopupHandleWroomLine(String line) {
    line.trim();
    if (line.length() == 0) return;

    // Przykład: EVT A2DP_CONN CONNECTED MAC=E4:61:F4:3E:C3:35 NAME="JBL Tune 520BT"
    if (line.indexOf("CONNECTED") >= 0 && line.indexOf("MAC=") >= 0) {
        String mac = "";
        String name = "";
        int macIdx = line.indexOf("MAC=");
        if (macIdx >= 0) {
            mac = line.substring(macIdx + 4);
            int spaceIdx = mac.indexOf(' ');
            if (spaceIdx >= 0) mac = mac.substring(0, spaceIdx);
            mac.trim();
        }
        int nameIdx = line.indexOf("NAME=");
        if (nameIdx >= 0) {
            name = line.substring(nameIdx + 5);
            if (name.startsWith("\"")) {
                name = name.substring(1);
                int endQuote = name.indexOf('"');
                if (endQuote >= 0) {
                    name = name.substring(0, endQuote);
                }
            } else {
                int spaceIdx = name.indexOf(' ');
                if (spaceIdx >= 0) name = name.substring(0, spaceIdx);
            }
            name.trim();
        }
        if (name.length() == 0) name = "Bluetooth";
        btPopupShow(name, mac);
    }
    if (line.indexOf("DISCONNECTED") >= 0) {
        btConnected = false;
    }
}

void btPopupShow(String name, String mac) {
    btPopupName = name;
    btPopupMac = mac;
    btPopupUntil = millis() + 4000;
    btPopupActive = true;
    btConnected = true;
    Serial.printf("[BT] SHOW %s\n", name.c_str());
}

void btPopupInit() {
    btPopupActive = false;
    btPopupUntil = 0;
}

void btPopupCancel() {
    btPopupActive = false;
    btPopupName = "";
    btPopupMac = "";
}

void btPopupForceClose() {
    if (!btPopupActive) return;
    btPopupCancel();
    Serial.println("[BT] CLOSE -> PLAYER");
    display.putRequest(NEWMODE, PLAYER);
}

void btPopupLoop() {
    if (!btPopupActive) return;

    if (static_cast<int32_t>(millis() - btPopupUntil) >= 0) {
        Serial.printf("[BT] TIMEOUT (until=%lu now=%lu)\n", btPopupUntil, millis());
        btPopupCancel();
    }
}
