#include "frame_attendance_setting.h"

void key_session_id_clear_cb(epdgui_args_vector_t &args) {
    ((EPDGUI_Textbox *)(args[0]))->SetText("");
}

Frame_Attendance_Setting::Frame_Attendance_Setting() : Frame_Base() {
    _frame_name      = "Frame_Attendance_Setting";
    
    _inputbox      = new EPDGUI_Textbox(4, 100, 532, 60);
    _key_textclear = new EPDGUI_Button("CLR", 4, 176, 260, 52);

    _inputbox->SetTextMargin(8, 15, 8, 8);
    _inputbox->SetState(EPDGUI_Textbox::EVENT_PRESSED);

    _keyboard = new EPDGUI_Keyboard(false, EPDGUI_Keyboard::STYLE_INPUTMODE_NEEDCONFIRM);

    _key_textclear->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, (void *)_inputbox);
    _key_textclear->Bind(EPDGUI_Button::EVENT_RELEASED, key_session_id_clear_cb);

    _canvas_title->drawString("Session Id", 270, 34);

    exitbtn("Attendance");
    
    _key_exit->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, (void *)(&_is_run));
    _key_exit->Bind(EPDGUI_Button::EVENT_RELEASED, &Frame_Base::exit_cb);
}

Frame_Attendance_Setting::~Frame_Attendance_Setting() {
    delete _inputbox;
    delete _keyboard;
    delete _key_textclear;
}

int Frame_Attendance_Setting::init(epdgui_args_vector_t &args) {
    _is_run = 1;
    M5.EPD.Clear();
    _canvas_title->pushCanvas(0, 8, UPDATE_MODE_NONE);
    EPDGUI_AddObject(_inputbox);
    EPDGUI_AddObject(_keyboard);
    EPDGUI_AddObject(_key_exit);
    EPDGUI_AddObject(_key_textclear);
    return 6;
}

int Frame_Attendance_Setting::run(void) {
    Frame_Base::run();
    String data = _keyboard->getData();
    if (data.indexOf("\n") >= 0) {
        String sesseion_id = _inputbox->GetText();
        SetSessionId(sesseion_id);
        _inputbox->SetText("");
        EPDGUI_PopFrame();
        _is_run = 0;
        return 0;
    }
    _inputbox->AddText(data);
    return 1;
}