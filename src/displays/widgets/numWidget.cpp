#include "../../core/config.h"
#include "../display_select.h"
#include "widgets.h"
#include "../../core/fonts.h"
//   NUM WIDGET (hangerő stb.)

NumWidget::~NumWidget() {
    if (_spr) {
        _spr->deleteSprite();
        delete _spr;
        _spr = nullptr;
    }
}

void NumWidget::init(WidgetConfig wconf, uint16_t buffsize, bool uppercase, uint16_t fgcolor, uint16_t bgcolor) {
    if (_spr) {
        _spr->deleteSprite();
        delete _spr;
        _spr = nullptr;
    }
    if (_text) {
        free(_text);
        _text = nullptr;
    }
    if (_oldtext) {
        free(_oldtext);
        _oldtext = nullptr;
    }

    Widget::init(wconf, fgcolor, bgcolor);
    _buffsize = buffsize;
    _text = (char*)malloc(_buffsize);
    _oldtext = (char*)malloc(_buffsize);
    if (_text) memset(_text, 0, _buffsize);
    if (_oldtext) memset(_oldtext, 0, _buffsize);
    _uppercase = uppercase;

    _spr = new LGFX_Sprite(&dsp);
    if (!_spr) return;
    _spr->setColorDepth(16);
    _spr->setPsram(true);
    if (font_vlw_clock) {
        _spr->loadFont(font_vlw_clock);
        _spr->setTextSize(1);
    }
    uint16_t fontHeight = _spr->fontHeight();
    uint16_t spriteWidth = font_textWidth("8888") + 20;
    if (spriteWidth < 150) { spriteWidth = 150; }

    _spr->createSprite(spriteWidth, fontHeight);
}

void NumWidget::setText(const char* txt) {
    strlcpy(_text, txt, _buffsize);
    if (strcmp(_oldtext, _text) == 0) return;
    if (_active) { _draw(); }
    strlcpy(_oldtext, _text, _buffsize);
}

void NumWidget::setText(int val, const char* format) {
    char buf[_buffsize];
    snprintf(buf, _buffsize, format, val);
    setText(buf);
}

void NumWidget::setColors(uint16_t fg, uint16_t bg) {
    _fgcolor = fg;
    _bgcolor = bg;
    if (_active) { _draw(); }
}

void NumWidget::_getBounds() {
    _textwidth = font_textWidth(_text);
}

void NumWidget::_draw() {
    if (!_active || !_spr) return;
    _spr->fillSprite(config.theme.background); // szürke 0x8410
    if (font_vlw_clock) { _spr->loadFont(font_vlw_clock); }
    _spr->setTextSize(1);
    _spr->setTextColor(_fgcolor, _bgcolor);
    _spr->setTextDatum(middle_center);
    _spr->drawString(_text, _spr->width() / 2, _spr->height() / 2);
    _spr->pushSprite(_realLeft() - _spr->width() / 2, _config.top - _spr->height() / 2);
    _spr->unloadFont();
}