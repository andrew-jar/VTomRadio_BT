#include "namedays.h"

#ifdef NAMEDAYS_FILE     // plik z imieninami
#if NAMEDAYS_FILE == HU
  #include "../../locale/namedays/namedays_HU.h"
#elif NAMEDAYS_FILE == PL
  #include "../../locale/namedays/namedays_PL.h"
//#elif NAMEDAYS_FILE == RU
//  #include "../../locale/namedays/namedays_RU.h"
#elif NAMEDAYS_FILE == GR
  #include "../../locale/namedays/namedays_GR.h"
#elif NAMEDAYS_FILE == NL
  #include "../../locale/namedays/namedays_NL.h"
#elif NAMEDAYS_FILE == UA
  #include "../../locale/namedays/namedays_UA.h"  
#elif NAMEDAYS_FILE == DE
  #include "../../locale/namedays/namedays_DE.h"    
#elif NAMEDAYS_FILE == CZ
  #include "../../locale/namedays/namedays_CZ.h"
#elif NAMEDAYS_FILE == SK
  #include "../../locale/namedays/namedays_SK.h"
#else
  #error "Unsupported NAMEDAYS_FILE"
#endif

// --- Zmienne rotacji ---
uint32_t namedayLastRotation = 0;  // czas ostatniej zmiany
uint8_t  namedayCurrentIndex = 0;  // aktualny indeks imienia
char     currentNamedayBuffer[50]; // bufor dla aktualnego imienia
int      lastNamedayDay = -1;      // ostatni dzien do resetu rotacji
int      lastNamedayMonth = -1;    // ostatni miesiac do resetu rotacji

// Funkcja zwraca aktualne imie dla danego dnia roku, zmieniane co 4 sekundy
const char *getNameDay(int month, int day) {
  const int daysInMonth[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int       dayOfYear = day - 1; // dzien miesiaca (indeks od 0)

  for (int i = 0; i < month - 1; i++) {
    dayOfYear += daysInMonth[i];
  }

  if (dayOfYear < 0 || dayOfYear >= 366) {
    return ""; // zwroc pusty napis, jesli dzien jest nieprawidlowy
  }

  // Sprawdz, czy zmienil sie dzien; jesli tak, zresetuj rotacje
  if (lastNamedayDay != day || lastNamedayMonth != month) {
    lastNamedayDay = day;
    lastNamedayMonth = month;
    namedayCurrentIndex = 0;
    namedayLastRotation = millis();
    memset(currentNamedayBuffer, 0, sizeof(currentNamedayBuffer)); // wyczysc bufor przy zmianie dnia
  }

  // Pobranie lancucha z imionami dla danego dnia
  char tempBuffer[80];
  //strcpy_P(tempBuffer, (const char *)pgm_read_ptr(&namedays[dayOfYear]));
  strcpy(tempBuffer, namedays[dayOfYear]);

  //  Serial.printf("displayILI9488.cpp -> Wczytane imieniny: %s%\n", tempBuffer);
  // Policz liczbe imion w lancuchu (oddzielonych przecinkiem)
  uint8_t nameCount = 1;
  for (int i = 0; tempBuffer[i] != '\0'; i++) {
    if (tempBuffer[i] == ',')
      nameCount++;
  }

  // Jesli jest tylko jedno imie, nie rotuj
  if (nameCount == 1) {
    memset(currentNamedayBuffer, 0, sizeof(currentNamedayBuffer)); // wyczysc bufor
    strlcpy(currentNamedayBuffer, tempBuffer, sizeof(currentNamedayBuffer));
    return currentNamedayBuffer;
  }

  // Sprawdz, czy minely 4 sekundy od ostatniej zmiany
  uint32_t currentTime = millis();
  if (currentTime - namedayLastRotation >= 4000) { // 4 sekundy
    namedayLastRotation = currentTime;
    namedayCurrentIndex++;

    // Reset indeksu, jesli przekroczono liczbe imion
    if (namedayCurrentIndex >= nameCount) {
      namedayCurrentIndex = 0;
    }
  }

  // Znajdz odpowiednie imie na liscie
  //strcpy_P(tempBuffer, (const char *)pgm_read_ptr(&namedays[dayOfYear])); // Skopiuj ponownie, bo strtok modyfikuje bufor
 strcpy(tempBuffer, namedays[dayOfYear]);

  char *token = strtok(tempBuffer, ",");
  for (uint8_t i = 0; i < namedayCurrentIndex && token != NULL; i++) {
    token = strtok(NULL, ",");
  }

  if (token) {
    // Usun biale znaki z poczatku
    while (*token == ' ' || *token == '\t')
      token++;

    memset(currentNamedayBuffer, 0, sizeof(currentNamedayBuffer)); // wyczysc bufor przed zapisem
    strlcpy(currentNamedayBuffer, token, sizeof(currentNamedayBuffer));
    return currentNamedayBuffer;
  }

  return "";
}

#endif // NAMEDAY_ENABLED
