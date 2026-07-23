# VTomRadio_BT - wymagane srodowisko kompilacji (VS Code + PlatformIO)

Ten projekt jest przygotowany pod PlatformIO i framework `pioarduino` dla ESP32-S3.
Najwazniejsze: zgodnosc wersji PlatformIO Core i platformy ESP32.

## 1) Wymagane narzedzia

- VS Code: stabilna wersja (zalecana aktualna)
- Rozszerzenie VS Code: `PlatformIO IDE` (publisher: `platformio`)
- PlatformIO Core: `6.1.19`
- Python (dla PlatformIO): 3.9+ (zalecane 3.10/3.11)

## 2) Wersje wymagane przez ten projekt

W pliku `platformio.ini` ustawiona jest platforma:

- `platform = https://github.com/pioarduino/platform-espressif32/releases/download/55.03.37/platform-espressif32.zip`

To oznacza:

- platforma ESP32 (pioarduino): `55.03.37`
- Arduino framework dla ESP32: `3.3.7`
- framework-arduinoespressif32-libs: `5.5.0+sha.87912cd291`
- standard C++: `gnu++20` (wymagane przez kod, m.in. `std::span`/`requires`/`std::ranges`)

## 3) Szybka weryfikacja po stronie osoby, ktorej przekazujesz projekt

W terminalu w katalogu projektu:

```powershell
pio --version
pio pkg update -e esp32-s3-devkitc1-n16r8
pio run -e esp32-s3-devkitc1-n16r8
```

Oczekiwane:

- `PlatformIO Core, version 6.1.19`
- build przechodzi bez `fatal error`
- ostrzezenia z `WiFi.cpp` typu `missing initializer...` sa dopuszczalne (to warningi frameworka)

## 4) Czysta instalacja (gdy "u mnie dziala, u kogos nie")

Jesli ktos ma problemy z paczkami lub wersjami, zrobic:

1. Zamknac VS Code.
2. Usunac cache PlatformIO (folder usera):
   - `.platformio/platforms/espressif32`
   - `.platformio/packages/framework-arduinoespressif32`
   - `.platformio/packages/framework-arduinoespressif32-libs`
   - `.platformio/packages/toolchain-xtensa-esp-elf`
3. Otworzyc terminal i wykonac:

```powershell
pio upgrade
pio --version
```

4. Wrocic do projektu i wykonac:

```powershell
pio run -t clean -e esp32-s3-devkitc1-n16r8
pio run -e esp32-s3-devkitc1-n16r8
```

## 5) Uwaga dot. Arduino IDE

Ten projekt nie jest docelowo przygotowany pod Arduino IDE.
Wymaga C++20 i konfiguracji z `platformio.ini`, dlatego zalecane jest budowanie w VS Code + PlatformIO.

## 6) Uwaga po flashowaniu (czesto mylona z bledem kompilacji)

Brak polaczenia z domowym Wi-Fi po flashu nie musi oznaczac bledu builda.
Po czystym flashu urzadzenie moze wejsc w tryb AP (konfiguracyjny) i czekac na dane Wi-Fi (`/data/wifi.csv`).

---

# VTomRadio_BT - required build environment (VS Code + PlatformIO)

This project is prepared for PlatformIO and the `pioarduino` framework for ESP32-S3.
Most important: keep PlatformIO Core and ESP32 platform versions aligned.

## 1) Required tools

- VS Code: stable release (latest recommended)
- VS Code extension: `PlatformIO IDE` (publisher: `platformio`)
- PlatformIO Core: `6.1.19`
- Python (for PlatformIO): 3.9+ (3.10/3.11 recommended)

## 2) Versions required by this project

In `platformio.ini`, the platform is pinned to:

- `platform = https://github.com/pioarduino/platform-espressif32/releases/download/55.03.37/platform-espressif32.zip`

This means:

- ESP32 platform (pioarduino): `55.03.37`
- Arduino framework for ESP32: `3.3.7`
- framework-arduinoespressif32-libs: `5.5.0+sha.87912cd291`
- C++ standard: `gnu++20` (required by code, e.g. `std::span`/`requires`/`std::ranges`)

## 3) Quick verification for the recipient

Run in terminal from project root:

```powershell
pio --version
pio pkg update -e esp32-s3-devkitc1-n16r8
pio run -e esp32-s3-devkitc1-n16r8
```

Expected:

- `PlatformIO Core, version 6.1.19`
- build finishes without `fatal error`
- `WiFi.cpp` warnings such as `missing initializer...` are acceptable (framework warnings)

## 4) Clean reinstall (when "works on my machine" but not on another PC)

If package/version issues appear, do this:

1. Close VS Code.
2. Remove PlatformIO cache (user folder):
   - `.platformio/platforms/espressif32`
   - `.platformio/packages/framework-arduinoespressif32`
   - `.platformio/packages/framework-arduinoespressif32-libs`
   - `.platformio/packages/toolchain-xtensa-esp-elf`
3. Open terminal and run:

```powershell
pio upgrade
pio --version
```

4. Return to project and run:

```powershell
pio run -t clean -e esp32-s3-devkitc1-n16r8
pio run -e esp32-s3-devkitc1-n16r8
```

## 5) Note about Arduino IDE

This project is not primarily prepared for Arduino IDE.
It requires C++20 and the `platformio.ini` setup, so VS Code + PlatformIO is recommended.

## 6) Note after flashing (often confused with a build failure)

No connection to home Wi-Fi after flashing does not always mean a build error.
After a clean flash, the device may enter AP (configuration) mode and wait for Wi-Fi credentials (`/data/wifi.csv`).
