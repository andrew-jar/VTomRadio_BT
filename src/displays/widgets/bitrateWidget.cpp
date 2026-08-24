#include "../../core/options.h"
#if DSP_MODEL != DSP_DUMMY
#    include "../../core/config.h"
#    include "../display_select.h"
#    include "widgets.h"
#    include "../../core/fonts.h"

BitrateWidget::~BitrateWidget() {
    if (_spr) {
        _spr->deleteSprite();
        delete _spr;
        _spr = nullptr;
    }
}

void BitrateWidget::init(BitrateBoxConfig boxconf, uint16_t fgcolor, uint16_t bgcolor) {
    Widget::init({boxconf.left, boxconf.top, 0, boxconf.align}, fgcolor, bgcolor);

    _box = boxconf;

    _bitrate = 0;
    _format = BF_UNKNOWN;
    _sampleRateHz = 0;
    _bitDepth = 0;

    memset(_buf, 0, sizeof(_buf));

    // sprite NULLÁZÁS BIZTOSRA
    if (_spr) {
        _spr->deleteSprite();
        delete _spr;
        _spr = nullptr;
    }
}

void BitrateWidget::_ensureSprite() {
    bool flat = _isFlat();

    uint16_t w = flat ? _box.dimension * 2 : _box.dimension;
    uint16_t h = flat ? _box.dimension : _box.dimension;

    if (_spr && (_spr->width() == w) && (_spr->height() == h)) return; // ha van srite és mérete is jó, nem csinál semmit

    if (_spr) {
        _spr->deleteSprite();
    } else {
        _spr = new LGFX_Sprite(&dsp);
        _spr->setColorDepth(16);
        _spr->setPsram(true);
    }

    _spr->createSprite(w, h);
}

bool BitrateWidget::_applyFont(uint8_t size) {
    if (size == 0) size = _box.textsize;

    _usingVlw = false;
    if (size == 12 && font_vlw_12) {
        _spr->loadFont(font_vlw_12);
        _spr->setTextSize(1);
        _usingVlw = true;
    } else if (size == 14 && font_vlw_14) {
        _spr->loadFont(font_vlw_14);
        _spr->setTextSize(1);
        _usingVlw = true;
    } else if (size == 16 && font_vlw_16) {
        _spr->loadFont(font_vlw_16);
        _spr->setTextSize(1);
        _usingVlw = true;
    } else if (size == 18 && font_vlw_18) {
        _spr->loadFont(font_vlw_18);
        _spr->setTextSize(1);
        _usingVlw = true;
    } else if (size == 20 && font_vlw_20) {
        _spr->loadFont(font_vlw_20);
        _spr->setTextSize(1);
        _usingVlw = true;
    } else if (size == 22 && font_vlw_22) {
        _spr->loadFont(font_vlw_22);
        _spr->setTextSize(1);
        _usingVlw = true;
    } else {
        _spr->unloadFont();
        _spr->setFont(nullptr);
        _spr->setTextSize(2);
    }

    return _usingVlw;
}

void BitrateWidget::setBitrate(uint32_t bitrate) {
    _bitrate = bitrate;

    if (_bitrate > 20000) { _bitrate /= 1000; }

    _draw();
}

void BitrateWidget::setFormat(BitrateFormat format) {
    _format = format;
    _draw();
}

void BitrateWidget::setSampleRate(uint32_t sampleRateHz) {
    _sampleRateHz = sampleRateHz;
    _draw();
}

void BitrateWidget::setBitDepth(uint8_t bitDepth) {
    _bitDepth = bitDepth;
    _draw();
}

void BitrateWidget::_draw() {
    if (!_active) return;

    // Theme can change at runtime from the web editor. Keep widget colors in sync.
    _fgcolor = config.theme.bitrate;
    _bgcolor = config.theme.background;

    if (_format == BF_UNKNOWN || _bitrate == 0) {
        _clear();
        // return;
    }

    _ensureSprite(); // Sprite létrehozása vagy újraméretezése, ha szükséges
    _applyFont();    // Font beállítása (VLW vagy default) a textsize alapján

    _spr->fillSprite(_bgcolor);

    bool flat = _isFlat();

    int w = _spr->width();
    int h = _spr->height();

    // =====================
    // BITRATE STRING
    // =====================

    if (_bitrate == 0) {
        snprintf(_buf, sizeof(_buf), "--- kbps");
    } else {
        snprintf(_buf, sizeof(_buf), "%u kbps", static_cast<unsigned>(_bitrate));
    }

    const char* fmt = "";
    switch (_format) {
        case BF_MP3: fmt = "MP3"; break;
        case BF_AAC: fmt = "AAC"; break;
        case BF_FLAC: fmt = "FLAC"; break;
        case BF_OGG: fmt = "OGG"; break;
        case BF_WAV: fmt = "WAV"; break;
        case BF_VOR: fmt = "VOR"; break;
        case BF_OPU: fmt = "OPU"; break;
        default: fmt = "---"; break;
    }

    /****** ELRENDEZÉS ******/
    if (!flat) {
        _spr->setTextDatum(middle_center);
        // 🔲
        // Ha nem flat, akkor négyzetes elrendezés: bitrate fent, formátum lent
        _spr->setTextColor(_fgcolor, _bgcolor);
        char boxBr[16];
        if (_bitrate == 0) {
            snprintf(boxBr, sizeof(boxBr), "---");
        } else if (_bitrate < 1000) {
            snprintf(boxBr, sizeof(boxBr), "%u", static_cast<unsigned>(_bitrate));
        } else {
            const float br = static_cast<float>(_bitrate) / 1000.0f;
            snprintf(boxBr, sizeof(boxBr), "%.1f", br);
        }
        _spr->drawString(boxBr, w / 2, h / 4);
        int halfH = h / 2;
        if (_box.radius > 0) {
            _spr->fillRoundRect(0, halfH, w, halfH, _box.radius, _fgcolor);
            _spr->fillRect(0, halfH, w, _box.radius, _fgcolor);
        } else {
            _spr->fillRect(0, halfH, w, halfH, _fgcolor);
        }
        _spr->setTextColor(_bgcolor, _fgcolor);
        _spr->drawString(fmt, w / 2, (h * 3) / 4);
    } else {
        // Flat 2-line layout: codec + bitrate on line 1, sample format on line 2.
        const uint16_t infoColor = lgfx::color565(143, 216, 184);

        _spr->setTextDatum(lgfx::top_left);

        uint8_t line1Size = 14;

        const int16_t line1Y = 4;
        const int16_t line2Y = 26;
        const int16_t contentX = 0;

        _applyFont(line1Size);
        int32_t codecW = _spr->textWidth(fmt);
        int32_t brW = _spr->textWidth(_buf);
        const int32_t gap = 4;
        const int32_t maxLine1W = w - (contentX * 2);
        if (codecW + gap + brW > maxLine1W) {
            line1Size = 12;
            _applyFont(line1Size);
            codecW = _spr->textWidth(fmt);
            brW = _spr->textWidth(_buf);
        }

        _spr->setTextColor(infoColor, _bgcolor);
        _spr->drawString(fmt, contentX, line1Y);

        const int32_t bitrateX = contentX + codecW + gap;
        _spr->setTextColor(TFT_WHITE, _bgcolor);
        _spr->drawString(_buf, bitrateX, line1Y);

        char infoBuf[32];
        if (_sampleRateHz > 0) {
            const uint32_t khzX10 = _sampleRateHz / 100;
            const unsigned long srWhole = static_cast<unsigned long>(khzX10 / 10);
            const unsigned long srFrac = static_cast<unsigned long>(khzX10 % 10);
            const bool hasSourceBitDepth = (_format == BF_FLAC || _format == BF_WAV) && _bitDepth > 0;
            if (hasSourceBitDepth) {
                snprintf(infoBuf, sizeof(infoBuf), "%lu.%lu KHz • %ubit", srWhole, srFrac, static_cast<unsigned>(_bitDepth));
            } else {
                snprintf(infoBuf, sizeof(infoBuf), "%lu.%lu KHz", srWhole, srFrac);
            }
        } else {
            snprintf(infoBuf, sizeof(infoBuf), "--kHz");
        }

        if (!_applyFont(line1Size)) {
            // Keep second line readable when VLW fallback is unavailable.
            _spr->setTextSize(1);
        }
        _spr->setTextColor(infoColor, _bgcolor);
        _spr->drawString(infoBuf, contentX, line2Y);
    }
    _spr->pushSprite(_config.left, _config.top);
    /*
    //#if // OLED DISPLAY
    // Serial.printf(
    // "widget.cpp--> BITRATE-- left: %d, top: %d, dimension: %d _bitrate: %d, textsize: %d \n ", _config.left, _config.top, _box.dimension, _bitrate, _config.textsize
    // );
    // felső: üres keret (bitrate szám)
    dsp.drawRect(_config.left, _config.top, _box.dimension, _box.dimension, GRAY_5);
    // alsó: kitöltött (formátum)
    dsp.fillRect(_config.left, _config.top + _box.dimension / 2, _box.dimension, _box.dimension / 2, GRAY_5);
    // -------- bitrate szám --------
    dsp.setFont(nullptr);
    dsp.setTextSize(_config.textsize);
    dsp.setTextColor(GRAY_2);

    if (_bitrate < 999) {
        snprintf(_buf, 6, "%d", _bitrate); // Módisítás "bitrate"
    } else {
        float _br = (float)_bitrate / 1000;
        snprintf(_buf, 6, "%.1f", _br);
    }
    uint8_t cw = CHARWIDTH * _config.textsize;
    uint8_t ch = CHARHEIGHT * _config.textsize;
    dsp.setCursor((_config.left + (_box.dimension - strlen(_buf) * cw) / 2) + 1, _config.top + (_box.dimension / 4) - ch / 2 + 1);
    dsp.print(_buf);
    // -------- formátum szöveg --------
    dsp.setTextColor(BLACK);
    const char* fmt = "";
    switch (_format) {
        case BF_MP3: fmt = "MP3"; break;
        case BF_AAC: fmt = "AAC"; break;
        case BF_FLAC: fmt = "FLC"; break;
        case BF_OGG: fmt = "OGG"; break;
        case BF_WAV: fmt = "WAV"; break;
        case BF_VOR: fmt = "VOR"; break;
        case BF_OPU: fmt = "OPU"; break;
        default: return;
    }
    dsp.setCursor((_config.left + (_box.dimension - strlen(fmt) * cw) / 2) + 1, _config.top + (3 * _box.dimension / 4) - ch / 2);
    dsp.print(fmt);
    //#endif
    */
}

inline bool BitrateWidget::_isFlat() {
    // Keep the FLAT bitrate layout stable regardless of Nameday UI toggle.
    return (DSP_MODEL != DSP_SSD1322);
}

void BitrateWidget::_clear() {
    if (_isFlat()) {
        dsp.fillRect(_config.left, _config.top, _box.dimension * 2, _box.dimension, config.theme.background); // _bgcolor  próba szürke 0x7BEF
    } else {
        dsp.fillRect(_config.left, _config.top, _box.dimension, _box.dimension, config.theme.background); // _bgcolor  próba szürke 0x7BEF
    }
}

/* Törli mindkét bitratewidget területét és a namedayt is */
void BitrateWidget::clearAll() {
    dsp.fillRect(_config.left, _config.top, _box.dimension * 2, _box.dimension, config.theme.background); // _bgcolor zöld próba 0xAEE5
}

void BitrateWidget::refresh() {
    _clear();
    if (_spr) {
        _spr->deleteSprite();
        delete _spr;
        _spr = nullptr;
    }
    _draw();
}

#endif // DSP_MODEL != DSP_DUMMY
