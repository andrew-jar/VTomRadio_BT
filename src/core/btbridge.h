#pragma once

#include <Arduino.h>

class BtBridge {
  public:
    void begin();
    void loop();

    bool configured() const;
    bool ready() const;
    bool connected() const;

    bool sendLine(const char* line, uint8_t clientId = 0);
    bool requestStatus(uint8_t clientId = 0);

  private:
    void flushRxLine(uint8_t clientId = 0);
    void wsLine(const char* line, uint8_t clientId = 0);
};

extern BtBridge btBridge;
