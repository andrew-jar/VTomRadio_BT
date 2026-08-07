#include "bt_popup.h"
#include "core/display.h"
#include <Arduino.h>

String btPopupName = "";
String btPopupMac = "";
volatile bool btPopupActive = false;
volatile unsigned long btPopupUntil = 0;
bool btConnected = false;

static portMUX_TYPE g_btPopupMux = portMUX_INITIALIZER_UNLOCKED;
static char g_btPopupNameBuf[96] = "";
static char g_btPopupMacBuf[32] = "";

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
    portENTER_CRITICAL(&g_btPopupMux);
    btPopupName = name;
    btPopupMac = mac;
    strlcpy(g_btPopupNameBuf, name.c_str(), sizeof(g_btPopupNameBuf));
    strlcpy(g_btPopupMacBuf, mac.c_str(), sizeof(g_btPopupMacBuf));
    btPopupUntil = millis() + 4000;
    btPopupActive = true;
    btConnected = true;
    portEXIT_CRITICAL(&g_btPopupMux);
    Serial.printf("[BT] SHOW %s\n", name.c_str());
}

void btPopupInit() {
    portENTER_CRITICAL(&g_btPopupMux);
    btPopupActive = false;
    btPopupUntil = 0;
    g_btPopupNameBuf[0] = '\0';
    g_btPopupMacBuf[0] = '\0';
    portEXIT_CRITICAL(&g_btPopupMux);
}

void btPopupCancel() {
    portENTER_CRITICAL(&g_btPopupMux);
    btPopupActive = false;
    btPopupName = "";
    btPopupMac = "";
    g_btPopupNameBuf[0] = '\0';
    g_btPopupMacBuf[0] = '\0';
    portEXIT_CRITICAL(&g_btPopupMux);
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

void btPopupSnapshot(char* name, size_t nameSize, char* mac, size_t macSize, unsigned long* untilMs, bool* active) {
    if (name && nameSize > 0) name[0] = '\0';
    if (mac && macSize > 0) mac[0] = '\0';
    if (untilMs) *untilMs = 0;
    if (active) *active = false;

    portENTER_CRITICAL(&g_btPopupMux);
    if (name && nameSize > 0) strlcpy(name, g_btPopupNameBuf, nameSize);
    if (mac && macSize > 0) strlcpy(mac, g_btPopupMacBuf, macSize);
    if (untilMs) *untilMs = btPopupUntil;
    if (active) *active = btPopupActive;
    portEXIT_CRITICAL(&g_btPopupMux);
}
