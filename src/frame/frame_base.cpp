#include "frame_base.h"
#include "../epdgui/epdgui.h"

Frame_Base::Frame_Base(bool _has_title) {
    if (_has_title) {
        _canvas_title = new M5EPD_Canvas(&M5.EPD);
        _canvas_title->createCanvas(540, 64);
        _canvas_title->drawFastHLine(0, 64, 540, 15);
        _canvas_title->drawFastHLine(0, 63, 540, 15);
        _canvas_title->drawFastHLine(0, 62, 540, 15);
        _canvas_title->setTextSize(26);
        _canvas_title->setTextDatum(CC_DATUM);
    }
    _frame_id   = 0;
    _frame_name = "Frame_Base";
    EPDGUI_UpdateGlobalLastActiveTime();
}

Frame_Base::~Frame_Base() {
    if (_key_exit != NULL) delete _key_exit;
    if (_canvas_title != NULL) delete _canvas_title;
    if (_canvas_footer != NULL) {
        delete _canvas_footer;
    }
}

void Frame_Base::exitbtn(String title, uint16_t width) {
    _key_exit = new EPDGUI_Button(8, 12, width, 48);
    _key_exit->CanvasNormal()->fillCanvas(0);
    _key_exit->CanvasNormal()->setTextSize(26);
    _key_exit->CanvasNormal()->setTextDatum(CL_DATUM);
    _key_exit->CanvasNormal()->setTextColor(15);
    _key_exit->CanvasNormal()->drawString(title, 47 + 13, 28);
    _key_exit->CanvasNormal()->pushImage(15, 8, 32, 32,
                                         ImageResource_item_icon_arrow_l_32x32);
    *(_key_exit->CanvasPressed()) = *(_key_exit->CanvasNormal());
    _key_exit->CanvasPressed()->ReverseColor();
}

void Frame_Base::CheckAutoPowerSave() {
    unsigned long now = millis();
    int footer_height = 28;
    int margin_bottom = 10;
    if (now - g_last_active_time_millis > TIME_BEFORE_SHUTDOWN_MS) {
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, LOW); // TOUCH_INT
        esp_light_sleep_start();
    }
}

int Frame_Base::run(void) {
    if (ENABLE_AUTO_POWER_SAVE) {
        CheckAutoPowerSave();
    }
    return _is_run;
}

void Frame_Base::exit(void) {
}

void Frame_Base::exit_cb(epdgui_args_vector_t &args) {
    EPDGUI_PopFrame();
    *((int *)(args[0])) = 0;
}