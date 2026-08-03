#include "btbridge.h"

#include "netserver.h"
#include "options.h"
#include "config.h"
#include <Preferences.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "../plugins/bt_popup/bt_popup.h"

BtBridge btBridge;

namespace {

bool     g_ready = false;
bool     g_connected = false;
bool     g_btEnabled = false;
bool     g_startupVolApplied = false;
bool     g_recentAudioStarted = false;
uint32_t g_lastAudioStartedMs = 0;
bool     g_seenConnectEvent = false;
uint32_t g_restartGuardUntilMs = 0;
bool     g_connectAttemptActive = false;
bool     g_connectRecoveryDone = false;
bool     g_waitingAudioNotified = false;
uint32_t g_connectAttemptStartedMs = 0;
uint32_t g_reconnectAtMs = 0;
bool     g_reconnectFromLastGood = false;
bool     g_internalRecoveryDisconnect = false;
bool     g_internalRecoveryConnect = false;
uint32_t g_autoReconnectBlockedUntilMs = 0;
uint32_t g_lastStatusTxMs = 0;
uint32_t g_lastRbProcessMs = 0;
String   g_connectArg;
String   g_lastGoodTarget;
String   g_lastConnectedMac;
String   g_lastConnectedName;
String   g_lastPersistedTarget;
Preferences g_btPrefs;
bool     g_btPrefsReady = false;
SemaphoreHandle_t g_btUartMutex = nullptr;
char     g_rxBuf[1024];
size_t   g_rxLen = 0;
bool     g_rxOverflow = false;
uint32_t g_lastStateQueryMs = 0;

constexpr uint32_t kWaitingAudioNotifyMs = 3000;
constexpr uint32_t kRecoverNoAudioMs = 15000;
constexpr uint32_t kRbProcessGuardMs = 6000;

static bool isMacToken(const String& value) {
    if (value.length() != 17) return false;
    for (int i = 0; i < 17; ++i) {
        const char c = value[i];
        if ((i % 3) == 2) {
            if (c != ':') return false;
            continue;
        }
        const bool isDigit = (c >= '0' && c <= '9');
        const bool isHexUpper = (c >= 'A' && c <= 'F');
        const bool isHexLower = (c >= 'a' && c <= 'f');
        if (!isDigit && !isHexUpper && !isHexLower) return false;
    }
    return true;
}

static String normalizeMacToken(const String& value) {
    String mac = value;
    mac.trim();
    mac.toUpperCase();
    if (!isMacToken(mac)) return String();
    return mac;
}

static void persistLastGoodTarget(const String& target) {
    if (!g_btPrefsReady) return;
    if (!target.length()) return;
    if (g_lastPersistedTarget == target) return;
    g_btPrefs.putString("last_mac", target);
    g_lastPersistedTarget = target;
}

static bool lockBtUart(TickType_t timeoutTicks) {
    if (!g_btUartMutex) return true;
    return xSemaphoreTake(g_btUartMutex, timeoutTicks) == pdTRUE;
}

static void unlockBtUart() {
    if (g_btUartMutex) {
        xSemaphoreGive(g_btUartMutex);
    }
}

static String jsonEscape(const char* src) {
    String out;
    if (!src) return out;

    const size_t len = strlen(src);
    out.reserve(len + 16);

    for (size_t i = 0; i < len; ++i) {
        const char c = src[i];
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) >= 0x20) {
                    out += c;
                }
                break;
        }
    }

    return out;
}

static void replaceTokenValue(String& line, const char* key, const char* newValue) {
    const int keyPos = line.indexOf(key);
    if (keyPos < 0) return;
    const int valStart = keyPos + static_cast<int>(strlen(key));
    int valEnd = line.indexOf(' ', valStart);
    if (valEnd < 0) valEnd = line.length();
    line = line.substring(0, valStart) + String(newValue) + line.substring(valEnd);
}

static bool sanitizeStateForClient(const char* line, String& out) {
    if (!line || strncmp(line, "STATE", 5) != 0) return false;

    const bool restartGuardActive =
        g_restartGuardUntilMs != 0 && static_cast<int32_t>(millis() - g_restartGuardUntilMs) < 0;
    if (!restartGuardActive || g_seenConnectEvent || g_recentAudioStarted) return false;

    if (strstr(line, "CONN=1") == nullptr) return false;

    out = line;
    replaceTokenValue(out, "CONN=", "0");
    replaceTokenValue(out, "MAC=", "None");
    replaceTokenValue(out, "NAME=", "\"-\"");
    return true;
}

} // namespace

bool BtBridge::configured() const {
#ifdef USE_BT_BRIDGE
    return true;
#else
    return false;
#endif
}

bool BtBridge::ready() const {
    return g_ready;
}

bool BtBridge::connected() const {
    return g_ready && g_btEnabled && g_connected;
}

static void parseStateLine(const char* line) {
    if (!line || !line[0]) return;

    if (strncmp(line, "READY ", 6) == 0 && strstr(line, "BT-TX") != nullptr) {
        g_connected = false;
        g_recentAudioStarted = false;
        g_lastAudioStartedMs = 0;
        g_seenConnectEvent = false;
        g_connectAttemptActive = false;
        g_connectRecoveryDone = false;
        g_waitingAudioNotified = false;
        g_reconnectAtMs = 0;
        g_reconnectFromLastGood = false;
        g_internalRecoveryDisconnect = false;
        g_internalRecoveryConnect = false;
        g_connectArg = "";
        g_lastConnectedMac = "";
        g_lastConnectedName = "";
        g_lastRbProcessMs = 0;
        g_restartGuardUntilMs = millis() + 3000;
        return;
    }

    const bool isState = (strncmp(line, "STATE", 5) == 0);
    if (!isState && strstr(line, "A2DP_CONN") == nullptr && strstr(line, "A2DP_AUDIO") == nullptr) {
        return;
    }

    if (isState && strstr(line, "RB=") != nullptr && strstr(line, "[PROCESS]") != nullptr) {
        // Extend PROCESS guard only when callback flow is active.
        const char* cbPtr = strstr(line, " CB=");
        const int cb = cbPtr ? atoi(cbPtr + 4) : 0;
        if (cb > 0) {
            g_lastRbProcessMs = millis();
        }
    }

    // Prioritize explicit events from the BT stack over unreliable CONN=0 state snapshots.
    if (strstr(line, "EVT A2DP_CONN DISCONNECTED") != nullptr || strstr(line, "EVT A2DP_CONN DISCONNECTING") != nullptr) {
        g_connected = false;
        btConnected = false;
        g_recentAudioStarted = false;
        g_lastAudioStartedMs = 0;
        const bool reconnectBlocked =
            g_autoReconnectBlockedUntilMs != 0 && static_cast<int32_t>(millis() - g_autoReconnectBlockedUntilMs) < 0;
        if (!reconnectBlocked && g_btEnabled && !g_connectAttemptActive && !g_internalRecoveryDisconnect
            && !g_internalRecoveryConnect && g_reconnectAtMs == 0 && g_lastGoodTarget.length() > 0) {
            g_reconnectAtMs = millis() + 2200;
            g_reconnectFromLastGood = true;
        }
        return;
    }

    if (strstr(line, "EVT A2DP_CONN CONNECTED") != nullptr) {
        g_seenConnectEvent = true;
        g_connected = true;
        btConnected = true;

        String mac = "";
        String name = "Bluetooth";
        const char* macPos = strstr(line, "MAC=");
        if (macPos) {
            macPos += 4;
            const char* macEnd = strchr(macPos, ' ');
            if (macEnd) {
                mac = String(macPos).substring(0, macEnd - macPos);
            } else {
                mac = String(macPos);
            }
            mac.trim();
        }
        const char* namePos = strstr(line, "NAME=");
        if (namePos) {
            namePos += 5;
            if (*namePos == '"') {
                ++namePos;
                const char* nameEnd = strchr(namePos, '"');
                if (nameEnd && nameEnd > namePos) {
                    name = String(namePos).substring(0, nameEnd - namePos);
                }
            } else {
                const char* nameEnd = strchr(namePos, ' ');
                if (nameEnd && nameEnd > namePos) {
                    name = String(namePos).substring(0, nameEnd - namePos);
                } else {
                    name = String(namePos);
                }
            }
            name.trim();
        }
        if (name.length() == 0) name = "Bluetooth";
        btPopupShow(name, mac);
        g_lastConnectedMac = normalizeMacToken(mac);
        g_lastConnectedName = name;

        int v = config.store.volume;
        if (v < 0) v = 0;
        if (v > 100) v = 100;
        char cmd[16];
        snprintf(cmd, sizeof(cmd), "VOL %d", v);
        btBridge.sendLine(cmd);
    }

    if (strstr(line, "EVT A2DP_AUDIO STARTED") != nullptr) {
        g_seenConnectEvent = true;
        g_recentAudioStarted = true;
        g_lastAudioStartedMs = millis();
        g_connected = true;
        g_connectAttemptActive = false;
        g_connectRecoveryDone = false;
        g_waitingAudioNotified = false;
        g_reconnectAtMs = 0;
        g_internalRecoveryDisconnect = false;
        g_internalRecoveryConnect = false;

        String popupMac = g_lastConnectedMac;
        if (popupMac.length() == 0) {
            popupMac = normalizeMacToken(g_connectArg);
        }
        String popupName = g_lastConnectedName;
        popupName.trim();
        if (popupName.length() == 0) popupName = "Bluetooth";
        btPopupShow(popupName, popupMac);

        if (g_lastConnectedMac.length() > 0) {
            g_lastGoodTarget = g_lastConnectedMac;
        } else {
            const String connectMac = normalizeMacToken(g_connectArg);
            if (connectMac.length() > 0) {
                g_lastGoodTarget = connectMac;
            }
        }
        if (g_lastGoodTarget.length() > 0) {
            persistLastGoodTarget(g_lastGoodTarget);
        }
    }

    if (strstr(line, "EVT A2DP_AUDIO STOPPED") != nullptr) {
        g_recentAudioStarted = false;
        g_lastAudioStartedMs = 0;
    }

    const char* btPtr = strstr(line, "BT=");
    if (btPtr) {
        btPtr += 3;
        g_btEnabled = (strncmp(btPtr, "ON", 2) == 0 || strncmp(btPtr, "1", 1) == 0 || strncmp(btPtr, "TRUE", 4) == 0);
        if (!g_btEnabled) {
            g_connected = false;
            g_recentAudioStarted = false;
            g_lastAudioStartedMs = 0;
            g_connectAttemptActive = false;
            g_connectRecoveryDone = false;
            g_waitingAudioNotified = false;
            g_reconnectAtMs = 0;
            g_reconnectFromLastGood = false;
            g_internalRecoveryDisconnect = false;
            g_internalRecoveryConnect = false;
            g_autoReconnectBlockedUntilMs = 0;
            g_connectArg = "";
            g_lastConnectedMac = "";
            g_lastConnectedName = "";
            g_lastRbProcessMs = 0;
            return;
        }
    }

    const char* connPtr = strstr(line, "CONN=");
    const char* macPtr = strstr(line, "MAC=");
    bool hasValidMac = false;
    if (macPtr) {
        macPtr += 4;
        hasValidMac = (strncmp(macPtr, "None", 4) != 0 && strncmp(macPtr, "NONE", 4) != 0 && strncmp(macPtr, "00:00:00:00:00:00", 17) != 0);
    }

    if (connPtr) {
        connPtr += 5;
        const int connVal = atoi(connPtr);
        const bool restartGuardActive =
            g_restartGuardUntilMs != 0 && static_cast<int32_t>(millis() - g_restartGuardUntilMs) < 0;
        if (connVal == 0) {
            const bool audioStartedRecently =
                g_recentAudioStarted && g_lastAudioStartedMs != 0 && static_cast<int32_t>(millis() - g_lastAudioStartedMs) < 3000;
            if (!audioStartedRecently) {
                g_connected = false;
                g_recentAudioStarted = false;
                g_lastAudioStartedMs = 0;
            }
        } else if (hasValidMac || g_recentAudioStarted) {
            if (!restartGuardActive || g_seenConnectEvent || g_recentAudioStarted) {
                g_connected = true;
            }
        }
    }
}

void BtBridge::begin() {
#ifdef USE_BT_BRIDGE
    if (g_ready) return;

    Serial2.begin(BT_BRIDGE_BAUD, SERIAL_8N1, BT_BRIDGE_RX, BT_BRIDGE_TX);
    if (!g_btUartMutex) {
        g_btUartMutex = xSemaphoreCreateMutex();
    }
    g_rxLen = 0;
    g_rxOverflow = false;
    g_lastStateQueryMs = 0;
    g_lastStatusTxMs = 0;
    g_connected = false;
    g_btEnabled = false;
    g_recentAudioStarted = false;
    g_lastAudioStartedMs = 0;
    g_seenConnectEvent = false;
    g_restartGuardUntilMs = 0;
    g_waitingAudioNotified = false;
    g_startupVolApplied = false;
    g_reconnectFromLastGood = false;
    g_autoReconnectBlockedUntilMs = 0;
    if (!g_btPrefsReady) {
        g_btPrefsReady = g_btPrefs.begin("btbridge", false);
    }
    g_lastGoodTarget = "";
    g_lastPersistedTarget = "";
    if (g_btPrefsReady) {
        g_lastGoodTarget = normalizeMacToken(g_btPrefs.getString("last_mac", ""));
        g_lastPersistedTarget = g_lastGoodTarget;
    }
    g_lastConnectedMac = "";
    g_lastConnectedName = "";
    g_lastRbProcessMs = 0;
    g_ready = true;
    wsLine("STATE BTBRIDGE=READY");
#endif
}

void BtBridge::wsLine(const char* line, uint8_t clientId) {
    if (!line || !line[0]) return;
    if (websocket.count() == 0) return;

    String payload;
    payload.reserve(strlen(line) + 20);
    payload = "{\"btline\":\"";
    payload += jsonEscape(line);
    payload += "\"}";

    if (clientId == 0) {
        websocket.textAll(payload);
    } else {
        websocket.text(clientId, payload);
    }
}

void BtBridge::flushRxLine(uint8_t clientId) {
    if (g_rxOverflow) {
        g_rxLen = 0;
        g_rxOverflow = false;
        return;
    }

    if (g_rxLen == 0) return;
    g_rxBuf[g_rxLen] = '\0';
    parseStateLine(g_rxBuf);

    String clientLine;
    const char* lineToSend = g_rxBuf;
    if (sanitizeStateForClient(g_rxBuf, clientLine)) {
        lineToSend = clientLine.c_str();
    }
    wsLine(lineToSend, clientId);
    g_rxLen = 0;
}

bool BtBridge::sendLine(const char* line, uint8_t clientId) {
#ifdef USE_BT_BRIDGE
    if (!line) return false;
    if (!g_ready) begin();
    if (!g_ready) return false;

    String cmd = line;
    cmd.trim();
    if (!cmd.length()) return false;

    String cmdUpper = cmd;
    cmdUpper.toUpperCase();
    if (cmdUpper == "STATUS?") {
        // Throttle bursty STATUS requests (UI poll + backend poll + fast refresh bursts).
        if (g_lastStatusTxMs != 0 && static_cast<int32_t>(millis() - g_lastStatusTxMs) < 600) {
            return true;
        }
        g_lastStatusTxMs = millis();
    }

    if (cmdUpper.startsWith("CONNECT ")) {
        g_connectAttemptActive = true;
        if (!g_internalRecoveryConnect) {
            g_connectRecoveryDone = false;
        }
        g_connectAttemptStartedMs = millis();
        g_waitingAudioNotified = false;
        g_reconnectAtMs = 0;
        g_reconnectFromLastGood = false;
        g_internalRecoveryDisconnect = false;
        g_connectArg = cmd.substring(8);
        g_connectArg.trim();
        g_lastConnectedMac = "";
        g_lastConnectedName = "";
        g_lastRbProcessMs = 0;
        g_internalRecoveryConnect = false;
        g_autoReconnectBlockedUntilMs = 0;
    } else if (cmdUpper == "DISCONNECT" || cmdUpper == "BT OFF" || cmdUpper == "MODE OFF") {
        g_connectAttemptActive = false;
        g_waitingAudioNotified = false;

        const bool internalRecoveryDisconnect = (g_internalRecoveryDisconnect && cmdUpper == "DISCONNECT");
        if (internalRecoveryDisconnect) {
            // Internal recovery flow keeps reconnect schedule/target and one-shot recovery guard.
            g_internalRecoveryDisconnect = false;
        } else {
            g_connectRecoveryDone = false;
            g_autoReconnectBlockedUntilMs = millis() + 9000;
            g_reconnectAtMs = 0;
            g_reconnectFromLastGood = false;
            g_connectArg = "";
            g_internalRecoveryDisconnect = false;
            g_internalRecoveryConnect = false;
            g_lastRbProcessMs = 0;
        }
    } else if (cmdUpper == "SCAN") {
        g_autoReconnectBlockedUntilMs = millis() + 12000;
    }

    // BT queue timeout: 200ms (from bt_source pattern)
    if (!lockBtUart(pdMS_TO_TICKS(200))) {
        wsLine("ERR BTBRIDGE BUSY", clientId);
        return false;
    }
    Serial2.print(cmd);
    Serial2.print("\r\n");
    unlockBtUart();

    String tx = String("> ") + cmd;
    wsLine(tx.c_str(), clientId);
    return true;
#else
    (void)line;
    (void)clientId;
    return false;
#endif
}

bool BtBridge::requestStatus(uint8_t clientId) {
#ifdef USE_BT_BRIDGE
    return sendLine("STATUS?", clientId);
#else
    (void)clientId;
    return false;
#endif
}

void BtBridge::loop() {
#ifdef USE_BT_BRIDGE
    if (!g_ready) return;

#ifdef BT_STARTUP_VOL
    if (!g_startupVolApplied) {
        int v = BT_STARTUP_VOL;
        if (v < 0) v = 0;
        if (v > 100) v = 100;
        char cmd[16];
        snprintf(cmd, sizeof(cmd), "VOL %d", v);
        sendLine(cmd);
        g_startupVolApplied = true;
    }
#endif

    // UART RX queue timeout: 10ms (from bt_source pattern)
    if (lockBtUart(pdMS_TO_TICKS(10))) {
        size_t budget = 128;
        while (budget-- > 0 && Serial2.available() > 0) {
            const int ch = Serial2.read();
            if (ch < 0) break;

            if (ch == '\n') {
                flushRxLine();
                continue;
            }

            if (ch == '\r') {
                continue;
            }

            if (g_rxOverflow) {
                continue;
            }

            if (g_rxLen < sizeof(g_rxBuf) - 1) {
                g_rxBuf[g_rxLen++] = static_cast<char>(ch);
            } else {
                // Drop overlong UART line instead of emitting truncated/fragmented entries.
                g_rxOverflow = true;
            }
        }
        unlockBtUart();
    }

    // Keep backend self-polling only when no WS clients are active.
    if (websocket.count() == 0 && (g_lastStateQueryMs == 0 || static_cast<int32_t>(millis() - g_lastStateQueryMs) >= 15000)) {
        g_lastStateQueryMs = millis();
        sendLine("STATUS?");
    }

    // CONNECT can be established before headset audio path becomes ready.
    // If no audio starts within a grace window, run one controlled reconnect.
    if (g_connectAttemptActive && g_connected && !g_recentAudioStarted && !g_connectRecoveryDone
        && static_cast<int32_t>(millis() - g_connectAttemptStartedMs) >= static_cast<int32_t>(kWaitingAudioNotifyMs)) {
        if (!g_waitingAudioNotified) {
            wsLine("STATE BTBRIDGE=WAITING_AUDIO");
            g_waitingAudioNotified = true;
        }
    }

    const bool rbProcessSeenRecently =
        g_lastRbProcessMs != 0 && static_cast<int32_t>(millis() - g_lastRbProcessMs) < static_cast<int32_t>(kRbProcessGuardMs);
    if (g_connectAttemptActive && g_connected && !g_recentAudioStarted && !g_connectRecoveryDone
        && !rbProcessSeenRecently
        && static_cast<int32_t>(millis() - g_connectAttemptStartedMs) >= static_cast<int32_t>(kRecoverNoAudioMs)) {
        g_connectRecoveryDone = true;
        if (g_connectArg.length() > 0) {
            wsLine("STATE BTBRIDGE=RECOVER CONNECT_NO_AUDIO");
            g_internalRecoveryDisconnect = true;
            sendLine("DISCONNECT");
            g_reconnectAtMs = millis() + 1300;
            g_reconnectFromLastGood = false;
        } else {
            wsLine("STATE BTBRIDGE=RECOVER NO_TARGET");
        }
    }

    if (g_reconnectAtMs != 0 && static_cast<int32_t>(millis() - g_reconnectAtMs) >= 0) {
        const bool reconnectBlocked =
            g_autoReconnectBlockedUntilMs != 0 && static_cast<int32_t>(millis() - g_autoReconnectBlockedUntilMs) < 0;
        if (!reconnectBlocked && g_btEnabled && !g_connectAttemptActive) {
            String target = g_reconnectFromLastGood ? g_lastGoodTarget : g_connectArg;
            target.trim();
            if (target.length() > 0) {
                String cmd = String("CONNECT ") + target;
                g_internalRecoveryConnect = true;
                if (g_reconnectFromLastGood) {
                    wsLine("STATE BTBRIDGE=RECONNECT LAST_OK");
                }
                sendLine(cmd.c_str());
            }
        }
        g_reconnectAtMs = 0;
        g_reconnectFromLastGood = false;
    }
#endif
}
