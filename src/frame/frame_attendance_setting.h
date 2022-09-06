#ifndef _FRAME_ATTENDANCE_SETTING_
#define _FRMAE_ATTENDANCE_SETTING_

#include "frame_base.h"
#include "../epdgui/epdgui.h"

class Frame_Attendance_Setting: public Frame_Base {
   public:
    Frame_Attendance_Setting();
    ~Frame_Attendance_Setting();
    int run();
    int init(epdgui_args_vector_t &args);

   private:
    EPDGUI_Textbox *_inputbox;
    EPDGUI_Keyboard *_keyboard;
    EPDGUI_Button *_key_textclear;
};

#endif //_FRAME_ATTENDANCE_SETTING_