#pragma once

#include "options.h"

// Kompiluj tylko wtedy, gdy zdefiniowano NAMEDAYS_FILE
#ifdef NAMEDAYS_FILE

#include <Arduino.h>

// Deklaracja tablicy
extern const char* namedays[];

// Deklaracja etykiety
extern const char* nameday_label;

// Zmienne rotacji imienin
extern uint32_t namedayLastRotation;      // czas ostatniej zmiany
extern uint8_t  namedayCurrentIndex;      // aktualny indeks imienia
extern char     currentNamedayBuffer[50]; // bufor dla aktualnego imienia
extern int      lastNamedayDay;           // ostatni dzien do resetu rotacji
extern int      lastNamedayMonth;         // ostatni miesiac do resetu rotacji

// Funkcja zwraca aktualne imie dla danego dnia roku, zmieniane co 4 sekundy
const char *getNameDay(int month, int day);

#endif // NAMEDAYS_FILE

