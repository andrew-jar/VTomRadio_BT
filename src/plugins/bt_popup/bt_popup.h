#pragma once
#include <Arduino.h>
#include "core/display.h"
#include "core/config.h"

#ifndef WROOM_SERIAL
#define WROOM_SERIAL Serial2
#endif

extern String btPopupName;
extern String btPopupMac;
extern volatile bool btPopupActive;
extern volatile unsigned long btPopupUntil;
extern bool btConnected;

void btPopupInit();
void btPopupLoop();
void btPopupShow(String name, String mac);
void btPopupCancel();
void btPopupForceClose();
void btPopupHandleWroomLine(String line);
void btPopupSnapshot(char* name, size_t nameSize, char* mac, size_t macSize, unsigned long* untilMs, bool* active);
