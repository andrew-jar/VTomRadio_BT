
VTomRadio_BT (bluetooth) - Mod. Andrzej Jaroszuk.

Moja modyfikacja oparta na projekcie VTomRadio kolegi VaraiTamas.
---------------------------------------------------------------------

## Credits

- ADD st7789 : [MaSo-CZ Marek Zettik](https://github.com/MaSo-CZ)

---------------------------------------------------------------------

Wymagane srodowisko kompilacji (VS Code + PlatformIO)

Ten projekt jest przygotowany pod PlatformIO i framework `pioarduino` dla ESP32-S3.
Najwazniejsze: zgodnosc wersji PlatformIO Core i platformy ESP32.

Wymagane narzedzia

- VS Code: stabilna wersja (zalecana aktualna)
- Rozszerzenie VS Code: `PlatformIO IDE` (publisher: `platformio`)
- PlatformIO Core: `6.1.19`
- Python (dla PlatformIO): 3.9+ (zalecane 3.10/3.13.7)

Do pełnego szczęścia musisz wgrać jeszcze z repo : "WROOM_A2DP"  na dodatkowy esp32 Wroom

https://github.com/andrew-jar/WROOM_A2DP

----------------------------------------------


<img width="650" height="366" alt="foto" src="https://github.com/user-attachments/assets/3312710f-0300-4406-8c3c-5c7fc1efbfd1" />

Schemat podłączenia wroom do esp32s3: 
<img width="1147" height="806" alt="schemat" src="https://github.com/user-attachments/assets/7cf0fc9e-f626-4a39-ba9b-1448d093fd7e" />

Schemat zasilania ESP32-S3 i WROOM (filtrowanie, kondensatory odsprzęgające):
<img width="1271" height="845" alt="schemat_zasilania" src="https://github.com/user-attachments/assets/5da29161-37b0-4239-9faf-ba5b462d1adc" />

--------------------------------------------

## Aktualizacja BT panel (bt.html)

- Ograniczono zakres glosnosci w panelu BT: `VOL 0..85`.
- Ograniczono zakres podbicia: `BOOST 80..200`.
- Wysylka komend z przyciskow `SET VOL` i `SET BOOST` ma dodatkowy clamp po stronie JS,
  wiec panel nie wysyla wartosci poza bezpieczny zakres.
- `DISCONNECT` oraz `BT OFF` czysci liste `Detected Devices` w UI,
  ze starego wyniku skanowania.


### bt_popup (popup polaczenia BT)

- Popup uruchamia sie po zdarzeniu `EVT A2DP_CONN CONNECTED`.
- Wyswietla nazwe urzadzenia i adres MAC pobrany z WROOM.
- Popup jest tymczasowy: znika automatycznie po ok. 4 sekundach.
- W trakcie aktywnego popupu nawigacja UI (zmiana ekranu/listy) zamyka popup,
  aby nie blokowac normalnej obslugi.
- Po zamknieciu wykonywany jest powrot do widoku PLAYER (redraw),
  zeby ekran nie zostawal z artefaktem okna popup.

--------------------------------------------

## presets.cpp

- Usunąłem fałszywy partial refresh.
- Włączyłem realny partial update i przeniosłem bufor do PSRAM.
- Dzięki temu UI działa szybciej i zużywa mniej RAM.

--------------------------------------------

## Pilot IR - Presets/FAV

---------------------------------------------

Oryginalny VTomRadio pozwala na obsluge ulubionych FAV tylko na ekranie dotykowym!!!

W tym mod. dodalem pelna obsluge FAV na pilocie IR, wiec dziala rowniez na zwyklych wyswietlaczach bez panelu dotykowego (np. ST7789, ILI9341) jak i z panelem dotykowym.

---------------------------------------------

Wejście/wyjście PRESETS:

- `RED` -> wejscie / wyjscie PRESETS

Nawigacja w PRESETS:

- `VOL-` / `VOL+` -> zmiana banku `FAV1..FAV5`
- `PREV` / `NEXT` -> zmiana slotu
- `GREEN` -> odtworzenie wybranego presetu
- `YELLOW` -> zapis aktualnie granej stacji do wybranego slotu
- `BLUE` -> wyczyszczenie wybranego slotu
- `BACK` -> powrot do ekranu PLAYER

Foto IR Recorder (kolorowe przyciski PRESETS/FAV):

RED / GREEN / YELLOW / BLUE dodane do mapowania IR i nauki kodow pilota.

<img width="500" height="523" alt="IR" src="https://github.com/user-attachments/assets/4555c16c-fd7e-4d3e-903b-a7384f46a0ad" />

Dodatkowo:

- `LIST` -> zwykla lista stacji `PLAYER <-> STATIONS`
- w `IR Recorder` dodano kolorowe przyciski `RED`, `GREEN`, `YELLOW`, `BLUE` - nalezy je nauczyc tak samo jak pozostale przyciski pilota.

Zegar:


- godziny poranne wyswietlane sa z zerem wiodacym, np. `09:07`.

