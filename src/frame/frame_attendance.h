#ifndef _FRAME_ATTENDANCE_H_
#define _FRAME_ATTENDANCE_H_

#include "frame_base.h"
#include "../epdgui/epdgui.h"

class Frame_Attendance : public Frame_Base {
   public:
    Frame_Attendance();
    ~Frame_Attendance();
    int init(epdgui_args_vector_t &args);
    void exit();
    void init_names();
    bool init_wifi();

   private:
    M5EPD_Canvas *_names;
    EPDGUI_Button *_key_clock_in;
    EPDGUI_Button *_key_clock_out;
    EPDGUI_Button *_key_start_break;
    EPDGUI_Button *_key_end_break;
    EPDGUI_Button *_key_setting;
    String _session_id;
    String _authenticity_token;
};

#endif //_FRAME_ATTENDANCE_H_