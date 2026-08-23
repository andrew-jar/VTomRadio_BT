
VTomRadio_BT (bluetooth) - Mod. Andrzej Jaroszuk.

Moja modyfikacja oparta na projekcie [VTomRadio](https://github.com/VaraiTamas/VTomRadio) kolegi VaraiTamas.

---------------------------------------------------------------------

## Credits

- Projekt bazowy: [VaraiTamas - VTomRadio](https://github.com/VaraiTamas/VTomRadio)
- ADD st7789 : [MaSo-CZ Marek Zettik](https://github.com/MaSo-CZ)

---------------------------------------------------------------------

## Czym rozni sie ten mod od oryginalu?

Krotko: to nie jest oryginal z paroma dodatkami. W katalogu `src/` jest okolo
47 zmienionych plikow i ponad 2000 linii wlasnego kodu.

Nowe moduly, ktorych oryginal nie ma:

- `src/core/btbridge.cpp` - most UART do zewnetrznego nadajnika BT (ESP32 WROOM + A2DP).
- `src/core/serial_littlefs.cpp` - tryb serwisowy po porcie szeregowym z oknem startowym.
- `src/plugins/bt_popup/bt_popup.cpp` - okno na TFT po polaczeniu z urzadzeniem BT.

Najmocniej przerobione pliki wspolne:

- `netserver.cpp` - obsluga BT z panelu WWW, presety, krzywa glosnosci, bezpieczne formatowanie buforow.
- `display.cpp` - ikona i status BT w stopce, popup BT, poprawki artefaktow rysowania.
- `controls.cpp` - rozbudowana obsluga sterowania.
- `main.cpp` - okno serwisowe przy starcie, nieblokujace rozjasnianie podswietlenia.
- `options.h` / `config.h` - wlasne opcje BT, powiekszony EEPROM (1024 -> 2048),
  wiecej kodow IR (20 -> 24 pozycje, fix przyciskow RED/GREEN/YELLOW/BLUE)
  oraz opozniony zapis EEPROM (mniejsze zuzycie flash).
- `audioI2S/Audio.*` - strojenie VU i spektrum, wymuszone 48 kHz dla mostu BT.

Dodatkowo caly pakiet poprawek stabilnosci widgetow (wycieki pamieci, podwojne
zwolnienia, inicjalizacja pol) opisany w `CHANGELOG_2026-08-07_P0_P1_P2.md`.

Uwaga przy aktualizacjach: z powodu skali zmian nie da sie zrobic prostego
`git merge` z oryginalem. Nowosci z upstream trzeba przenosic wybiorczo.

---------------------------------------------------------------------

## Wersja hybrydowa (0.1.14-mod.hybrid)

Wersja firmware to `0.1.14-mod.hybrid`. "Hybryda" oznacza, ze z oryginalu 0.1.14
przeniesiony zostal tylko menedzer plikow po WiFi, bez rezygnacji z wlasnego
trybu serwisowego po porcie szeregowym. Dzieki temu dzialaja oba naraz.

Menedzer plikow po WiFi (`src/core/fs_api_http.cpp`, kod z oryginalu):

- `GET /api/fs/ping` - sprawdzenie, czy system plikow odpowiada.
- `GET /api/fs/list` - lista plikow i katalogow.
- `GET /api/fs/info` - rozmiar, zajete i wolne miejsce.
- `GET /api/fs/read` - odczyt pliku.
- `POST /api/fs/mkdir`, `/api/fs/rmdir`, `/api/fs/delete` - operacje na plikach.
- `POST /api/reboot` - restart urzadzenia.

Wlasne dodatki do tego mechanizmu:

- Menedzer mozna wlaczyc i wylaczyc z panelu WWW - przelacznik `HTTP FS manager (WiFi)`
  w ustawieniach, obok `Serial LittleFS maintenance mode`.
- Tryb serwisowy po porcie szeregowym (`serialLittlefsEnabled`) zostal zachowany
  i dziala niezaleznie - okno na wejscie otwiera sie przez 4 sekundy po starcie.

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

Jak to dziala: ESP32-S3 wysyla dzwiek do WROOM przewodem I2S w formacie stereo 32-bit.
Nastepnie WROOM zmniejsza format do 16-bit i dopiero wtedy wysyla dzwiek przez Bluetooth.
Czyli 32-bit dotyczy polaczenia miedzy plytkami, a 16-bit dotyczy transmisji Bluetooth - oba ustawienia sa prawidlowe.

WROOM pracuje jako odbiornik I2S (I2S Slave), sam rozpoznaje dzwiek 44,1 kHz lub 48 kHz
i w razie potrzeby dopasowuje go do 44,1 kHz przed wyslaniem jako nadajnik Bluetooth A2DP (TX).

----------------------------------------------


<img width="650" height="366" alt="foto" src="https://github.com/user-attachments/assets/3312710f-0300-4406-8c3c-5c7fc1efbfd1" />

Schemat podłączenia wroom do esp32s3: 
<img width="1147" height="806" alt="schemat" src="https://github.com/user-attachments/assets/7cf0fc9e-f626-4a39-ba9b-1448d093fd7e" />

Schemat zasilania ESP32-S3 i WROOM (filtrowanie, kondensatory odsprzęgające):
<img width="1271" height="845" alt="schemat_zasilania" src="https://github.com/user-attachments/assets/5da29161-37b0-4239-9faf-ba5b462d1adc" />

--------------------------------------------

## Najwazniejsze zmiany BT

- Inteligentny reconnect tylko do ostatniego poprawnego urzadzenia.
    Radio po utracie linku wraca do ostatniego dzialajacego glosnika bez losowych prob laczenia.
- Lepsze skanowanie i lista urzadzen.
    Lista jest sortowana po RSSI, ukrywa smieciowe wpisy i przyspiesza wybor sluchawek lub glosnika.
- Bezpieczny CONNECT flow (anty-zawieszka).
    Gdy jest `CONNECTED`, ale przez kilka sekund nie ma `AUDIO STARTED`, firmware robi kontrolowany reconnect zamiast zostawic martwe polaczenie bez dzwieku.

--------------------------------------------

## Aktualizacja BT panel (bt.html)

- Ograniczono zakres glosnosci w panelu BT: `VOL 0..85`.
- Ograniczono zakres podbicia: `BOOST 100..200`.
- Wysylka komend z przyciskow `SET VOL` i `SET BOOST` ma dodatkowy clamp po stronie JS,
  wiec panel nie wysyla wartosci poza bezpieczny zakres.
- `DISCONNECT` oraz `BT OFF` czysci liste `Detected Devices` w UI,
  ze starego wyniku skanowania.


### bt_popup (popup polaczenia BT)

- Popup uruchamia sie po zdarzeniu `EVT A2DP_CONN CONNECTED` i ponownie przy `EVT A2DP_AUDIO STARTED`.
- Wyswietla nazwe urzadzenia i adres MAC pobrany z WROOM.
- Popup jest tymczasowy: znika automatycznie po ok. 4 sekundach.
- W trakcie aktywnego popupu nawigacja UI (zmiana ekranu/listy) zamyka popup,
  aby nie blokowac normalnej obslugi.
- Po zamknieciu wykonywany jest powrot do widoku PLAYER (redraw),
  zeby ekran nie zostawal z artefaktem okna popup.

--------------------------------------------

## presets.cpp

- Usunalem falszywy partial refresh.
- Wlaczylem realny partial update i przenioslem bufor do PSRAM.
Dzieki temu UI działa szybciej i zuzywa mniej RAM.

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

----------------------------------------------------------------------
##  Export/ Import - Presets/FAV

Dodano funkcję eksportu i importu presetów (lista FAV), dostępną z poziomu panelu UI.
Plik z presetami jest zapisywany w katalogu Data/data jako presets.csv.

Funkcja Export/Import presets.csv jest widoczna, gdy w konfiguracji włączony jest co najmniej jeden interfejs sterowania: IR lub ekran dotykowy (Touch).

