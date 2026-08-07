#include <Arduino.h>

String currentArtist = "";
String currentTitle = "";
uint16_t currentStationId = static_cast<uint16_t>(-1);
bool metaOff = false;
bool g_forcePlaylistStationName = false;
String g_forcedPlaylistStationName = "";
