// clang-format off
/* https://trip5.github.io/ehRadio_myoptions/generator.html
   https://github.com/VaraiTamas/yoRadio.git
    Przeczytaj przed użyciem!!! - Read the before use !!!
   https://github.com/VaraiTamas/yoRadio/blob/main/README.md !!!

   - Znak // na początku linii dezaktywuje polecenie, dlatego kompilator je ignoruje!
      W ten sposób możesz ustawić konfigurację odpowiednią dla swojego sprzętu.
  - The // sign at the beginning of the line makes the command inactive, so the compiler ignores it! 
    This allows you to set the appropriate configuration for your hardware.
*/

#pragma once

#ifndef ARDUINO_ESP32S3_DEV
    #define ARDUINO_ESP32S3_DEV
#endif

// #define HEAP_DBG

/* Tutaj możesz ustawić język programu
   You can set the program language here.
   Supported languages: HU NL PL RU EN GR SK DE UA ES. */
#define LANGUAGE PL

/* -- Wyświetlanie imienin -- Display name days --
Supported languages: HU, PL, NL, GR, DE (UA Local/namedays/namedays_UA.h is not filled in.) */
#define NAMEDAYS_FILE PL

#define USE_BUILTIN_LED false /* The RGB LED does not turn on.. */

/* Arduino OTA Support */
 #define USE_OTA true                    /* Enable OTA updates from Arduino IDE */
// #define OTA_PASS "myotapassword12345"   /* OTA password for secure updates */

/* HTTP Authentication */
// #define HTTP_USER ""               /* HTTP basic authentication username */
// #define HTTP_PASS ""               /* HTTP basic authentication password */

/*----- LCD DISPLAY -----*/
#define DSP_MODEL DSP_ILI9488
//#define DSP_MODEL DSP_ST7796

/*----- DISPLAY PIN SETS -----*/
#define TFT_DC         9
#define TFT_CS         10
#define TFT_RST        -1
#define BRIGHTNESS_PIN 14
/*
   GPIO 11 - MOSI
   GPIO 12 - CLK
   GPIO 13 - MISO  // Nie podłączaj do wyświetlacza LCD!!! - Do not connect to the LCD display!!!
*/

/*----- Touch ISP -----*/
//#define TS_MODEL TS_MODEL_XPT2046
//#define TS_CS    3

/*----- Touch I2C -----*/
// #define TS_MODEL TS_MODEL_FT6X36
// #define TS_MODEL TS_MODEL_AXS15231B
// #define TS_SCL     7
// #define TS_SDA     8
// #define TS_INT    17 
// #define TS_RST     1

/*----- NEXTION DISPLAY serial port -----*/
// #define NEXTION_RX			15
// #define NEXTION_TX			16

/*----- PCM5102A  DAC -----*/
#define I2S_DOUT 4
#define I2S_BCLK 5
#define I2S_LRC  6
// #define I2S_MCLK 0  /* CS4344 DAC: MCLK pin (dla PCM5102A nie jest wymagany / not needed for PCM5102A) */
//Wolne piny odpowiednie dla MCLK: GPIO 0, 7, 8, 15, 16, 17, 18, 45, 46

/*----- ENCODER 1 ------*/
//#define ENC_BTNR 47 // S2 41
//#define ENC_BTNL 42 // S1 40 
//#define ENC_BTNB 21 // KEY 39
// #define ENC_INTERNALPULLUP	true

/*----- ENCODER 2 -----*/
#define ENC2_BTNR 41 // S2 47
#define ENC2_BTNL 40 // S1 48-RGB LED!
#define ENC2_BTNB 39 // KEY 21
#define ENC2_INTERNALPULLUP	true

/*----- CLOCK MODUL RTC DS3132 -----*/
 #define RTC_SCL			     7
 #define RTC_SDA			     8
 #define RTC_MODULE DS3231

/*----- REMOTE CONTROL INFRARED RECEIVER -----*/
/*----- Aby wybudzać ze snu, trzeba użyć GPIO 2, ponieważ GPIO 38 nie jest pinem RTC. Należy wykonać połączenie na PCB! -----*/
/*----- To wake from sleep, you must use GPIO 2, because GPIO 38 is not an RTC pin. It must be connected via the PCB! -----*/
#define IR_PIN 2  //38
#define IR_NEC_ONLY  // Build only NEC decoder sources from IRremoteESP8266 (faster/smaller build)

/*----- SD CARD -----*/
// #define SDC_CS     18
// #define SDSPISPEED 4000000 /* 4MHz - Slower speed to prevent display flicker on shared SPI bus */

/*----- Przy tym ustawieniu nie ma przewijania paska pogody. -----*/
/*----- With this setting there is no scrolling on the weather bar. -----*/
#define WEATHER_FMT_SHORT

/*----- Przy tym ustawieniu wyświetlany jest pełny raport pogodowy. -----*/
/*----- With this setting, the full weather report is displayed. -----*/
// #define EXT_WEATHER  true

/*----- Przy tym ustawieniu prędkość wiatru będzie w km/h. -----*/
/*----- With this setting, the wind speed will be km/h. -----*/
// #define WIND_SPEED_IN_KMH

/*----- Pin ustawiony tutaj może sterować zasilaniem wzmacniacza audio. Podczas odtwarzania muzyki pin ma stan HIGH (wysoki), co załącza
przekaźnik zasilania wzmacniacza. Gdy muzyka nie jest odtwarzana (STOP lub głośność 0), pin ma stan LOW (niski).
Ta zmiana następuje, gdy włącza się wygaszacz ekranu w trybie "while not playing".
This pin controls the amplifier's power supply. When music is playing, the pin is set to HIGH to control the relay.
When music is not playing (stopped or volume is 0), the pin is set to LOW. This change occurs when the screensaver is running. -----*/
// #define PWR_AMP 2

/*----- Jeśli to jest zdefiniowane, przy uruchomieniu radia zawsze zostanie ustawiony pierwszy kanał. -----*/
/*----- If this is defined at radio startup, the first channel will always be set. -----*/
//#define ALWAYS_START_FROM_FIRST

/*----- Sleep functions -----*/
/*----- Zamiast WAKE_PIN można teraz ustawić dwa piny do wybudzania: WAKE_PIN1 i WAKE_PIN2 -----*/
/*----- Dzięki temu urządzenie można wybudzić pilotem i dodatkowym przyciskiem. -----*/
/*----- Instead of WAKE_PIN, you can now set two pins for wake-up: WAKE_PIN1 and WAKE_PIN2 -----*/
/*----- This way, you can wake up the device with a remote control and another button. -----*/
#define BTN_MODE IR_PIN
#define WAKE_PIN1 IR_PIN
// #define WAKE_PIN2 ENC2_BTNB

/*----- by Zsolt Simon -----*/
/*----- Tested on Synology NAS ----- */
// #define USE_DLNA
// #define dlnaHost "192.168.1.200"
// #define dlnaIDX  21

#define POWER_LED 38      // Button LED pin (will be turned on when player is on)
