#include <HTTPClient.h>
#include <regex.h>
#include "frame_attendance.h"
#include "frame_attendance_setting.h"
#include "frame_wifiscan.h"

#define BTN_W 184
#define BTN_H 184
#define MGN_X 60  // margin
#define MGN_Y 150 // margin
#define BTN_MGN_X 42
#define BTN_MGN_Y 42

#define SUCCESS 1
#define FAILED 2

#define JST_OFFSET 3600 * 9

static const char *URL = "https://attendance.moneyforward.com/my_page";
static const char *UA = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/104.0.0.0 Safari/537.36";
static const char *COOKIE = "_session_id=%s";
static const char *FORM_DATA = "authenticity_token=%s&web_time_recorder_form[event]=%s&web_time_recorder_form[date]=%s&web_time_recorder_form[user_time]=%s&web_time_recorder_form[office_location_id]=21558";

static const char *REGEX_STR[] =
    { 
        "_session_id=(.*); path",
        "<meta name=\"csrf-token\" content=\"(.*)\" />",
        "attendance-notification is-warning",
        "attendance-notification is-success"
    };

static const char* ROOT_CA = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIEMjCCAxqgAwIBAgIBATANBgkqhkiG9w0BAQUFADB7MQswCQYDVQQGEwJHQjEb\n" \
"MBkGA1UECAwSR3JlYXRlciBNYW5jaGVzdGVyMRAwDgYDVQQHDAdTYWxmb3JkMRow\n" \
"GAYDVQQKDBFDb21vZG8gQ0EgTGltaXRlZDEhMB8GA1UEAwwYQUFBIENlcnRpZmlj\n" \
"YXRlIFNlcnZpY2VzMB4XDTA0MDEwMTAwMDAwMFoXDTI4MTIzMTIzNTk1OVowezEL\n" \
"MAkGA1UEBhMCR0IxGzAZBgNVBAgMEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UE\n" \
"BwwHU2FsZm9yZDEaMBgGA1UECgwRQ29tb2RvIENBIExpbWl0ZWQxITAfBgNVBAMM\n" \
"GEFBQSBDZXJ0aWZpY2F0ZSBTZXJ2aWNlczCCASIwDQYJKoZIhvcNAQEBBQADggEP\n" \
"ADCCAQoCggEBAL5AnfRu4ep2hxxNRUSOvkbIgwadwSr+GB+O5AL686tdUIoWMQua\n" \
"BtDFcCLNSS1UY8y2bmhGC1Pqy0wkwLxyTurxFa70VJoSCsN6sjNg4tqJVfMiWPPe\n" \
"3M/vg4aijJRPn2jymJBGhCfHdr/jzDUsi14HZGWCwEiwqJH5YZ92IFCokcdmtet4\n" \
"YgNW8IoaE+oxox6gmf049vYnMlhvB/VruPsUK6+3qszWY19zjNoFmag4qMsXeDZR\n" \
"rOme9Hg6jc8P2ULimAyrL58OAd7vn5lJ8S3frHRNG5i1R8XlKdH5kBjHYpy+g8cm\n" \
"ez6KJcfA3Z3mNWgQIJ2P2N7Sw4ScDV7oL8kCAwEAAaOBwDCBvTAdBgNVHQ4EFgQU\n" \
"oBEKIz6W8Qfs4q8p74Klf9AwpLQwDgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB/wQF\n" \
"MAMBAf8wewYDVR0fBHQwcjA4oDagNIYyaHR0cDovL2NybC5jb21vZG9jYS5jb20v\n" \
"QUFBQ2VydGlmaWNhdGVTZXJ2aWNlcy5jcmwwNqA0oDKGMGh0dHA6Ly9jcmwuY29t\n" \
"b2RvLm5ldC9BQUFDZXJ0aWZpY2F0ZVNlcnZpY2VzLmNybDANBgkqhkiG9w0BAQUF\n" \
"AAOCAQEACFb8AvCb6P+k+tZ7xkSAzk/ExfYAWMymtrwUSWgEdujm7l3sAg9g1o1Q\n" \
"GE8mTgHj5rCl7r+8dFRBv/38ErjHT1r0iWAFf2C3BUrz9vHCv8S5dIa2LX1rzNLz\n" \
"Rt0vxuBqw8M0Ayx9lt1awg6nCpnBBYurDC/zXDrPbDdVCYfeU0BsWO/8tqtlbgT2\n" \
"G9w84FoVxp7Z8VlIMCFlA2zs6SFz7JsDoeA3raAVGI/6ugLOpyypEBMs1OUIJqsi\n" \
"l2D4kF501KKaU73yqWjgom7C12yxow+ev+to51byrvLjKzg6CYG1a4XXvi3tPxq3\n" \
"smPi9WIsgtRqAEFQ8TmDn5XpNpaYbg==\n" \
"-----END CERTIFICATE-----\n";

void set_current_time(char* date, char* datetime) {
    rtc_time_t time_struct;
    rtc_date_t date_struct;
    M5.RTC.getTime(&time_struct);
    M5.RTC.getDate(&date_struct);

    struct tm tm = { time_struct.sec, time_struct.min, time_struct.hour, date_struct.day, date_struct.mon-1, date_struct.year-1900 };
    time_t timer = mktime(&tm) - JST_OFFSET;
    tm = *localtime(&timer);
    
    sprintf(date, "%d/%d/%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    sprintf(datetime, "%d-%02d-%02dT%02d:%02d:%02d.000Z", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

int post_event(String event, String *session_id, String *authenticity_token) {
    char date[16];
    char datetime[32];
    set_current_time(date, datetime);

    HTTPClient client;

    char url[128];
    char cookie[128];
    sprintf(url, "%s/%s", URL, "web_time_recorder");
    sprintf(cookie, COOKIE, session_id->c_str());

    client.begin(url, ROOT_CA);
    client.addHeader("Cookie", cookie);
    client.addHeader("User-Agent", UA);
    client.addHeader("Content-Type", "application/x-www-form-urlencoded");

    char form_data[512];
    sprintf(form_data, FORM_DATA, authenticity_token->c_str(), event, date, datetime);

    int status_code = client.POST(form_data);
    return status_code == 302 ? SUCCESS : FAILED;
}

void free_regex(regex_t *regex, int n) {
    for (int i=0; i<n; ++i) {
        regfree(&regex[i]);
    }
}

int renew_session(String *session_id, String *authenticity_token) {
    HTTPClient client;
    
    client.begin(URL, ROOT_CA);
    client.addHeader("Cookie", "_session_id=" + *session_id);
    client.addHeader("User-Agent", UA);
    
    const char *keys[] = {"Set-Cookie"};
    client.collectHeaders(keys, 1);
    
    int status_code = client.GET();
    
    if (status_code != HTTP_CODE_OK) {
        client.end();
        return FAILED;
    }

    String header = client.header("Set-Cookie");

    char buffer[1024];
    regex_t regex[4];
    for (int i = 0; i < 4; ++i){
        int ret = regcomp(&regex[i], REGEX_STR[i], REG_EXTENDED);
        if (regcomp(&regex[i], REGEX_STR[i], REG_EXTENDED)) {
            regerror(ret, &regex[i], buffer, 1024);
            log_e("error: %s", buffer);

            free_regex(regex, 4);
            client.end();
            return FAILED;
        }
    }

    regmatch_t match[2];
    if(regexec(&regex[0], header.c_str(), 2, match, 0) == 0) {
        int len = match[1].rm_eo - match[1].rm_so;
        strncpy(buffer, header.c_str()+match[1].rm_so, len);
        buffer[len] = '\0';

        (*session_id) = String(buffer);
    } else {
        free_regex(regex, 4);
        client.end();
        return FAILED;
    }

    int result = 0;
    WiFiClient *stream = client.getStreamPtr();
    while(client.connected()) {
        size_t size = stream->available();
        if(size == 0) {
            continue;
            delay(1);
        }

        int c = stream->readBytesUntil('>', buffer, 1024);
        buffer[c] = '>';
        buffer[c+1] = '\0';

        if(strcmp(buffer, "</html>") == 0) {
            break;
        }

        if (regexec(&regex[1], buffer, 2, match, 0) == 0) {
            int len = match[1].rm_eo - match[1].rm_so;
            strncpy(buffer, buffer+match[1].rm_so, len);
            buffer[len] = '\0';
            (*authenticity_token) = String(buffer);
        }
        else if (regexec(&regex[2], buffer, 2, match, 0) == 0) {
            result = FAILED;
        }
        else if (regexec(&regex[3], buffer, 2, match, 0) == 0) {
            result = SUCCESS;
        }

        delay(1);
    }

    client.end();
    free_regex(regex, 4);

    return result;
}

void show_result(const char* type, int result) {
    M5EPD_Canvas canvas(&M5.EPD);
    canvas.createCanvas(540, 60);
    canvas.setTextDatum(CC_DATUM);

    canvas.setTextSize(20);
    canvas.fillCanvas(0);

    char buffer[128];
    switch(result)
    {
        case SUCCESS:
        sprintf(buffer, "[%s] SUCCESS", type);
        break;
        case FAILED:
        sprintf(buffer, "[%s] FAILED", type);
        break;
    }
    
    canvas.drawString(buffer,  270, 20);
    canvas.pushCanvas(0, 90, UPDATE_MODE_NONE);
}

bool init_wifi() {
    if (!isWiFiConfiged()) {
        return false;
    }

    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }

    WiFi.begin(GetWifiSSID().c_str(), GetWifiPassword().c_str());
    uint32_t t = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - t > 8000) {
            return false;
        }
    }

    return true;
}

void key_clock_in_cb(epdgui_args_vector_t &args) {
    String *session_id = (String *)(args[1]);
    String *authenticity_token = (String *)(args[2]);
    
    if(init_wifi() == false) {
        show_result("Clock In", FAILED);
        return;
    };
   
    post_event("clock_in", session_id, authenticity_token);
    int result = renew_session(session_id, authenticity_token);
    show_result("Clock In", result);
}

void key_clock_out_cb(epdgui_args_vector_t &args) {
    String *session_id = (String *)(args[1]);
    String *authenticity_token = (String *)(args[2]);
    
    if(init_wifi() == false) {
        show_result("Clock Out", FAILED);
        return;
    };
    
    post_event("clock_out", session_id, authenticity_token);
    int result = renew_session(session_id, authenticity_token);
    show_result("Clock Out", result);
}

void key_start_break_cb(epdgui_args_vector_t &args) {
    String *session_id = (String *)(args[1]);
    String *authenticity_token = (String *)(args[2]);
    
    if(init_wifi() == false) {
        show_result("Start Break", FAILED);
        return;
    };

    post_event("start_break", session_id, authenticity_token);
    int result = renew_session(session_id, authenticity_token);
    show_result("Start Break", result);
}

void key_end_break_cb(epdgui_args_vector_t &args) {
    String *session_id = (String *)(args[1]);
    String *authenticity_token = (String *)(args[2]);
    
    if(init_wifi() == false) {
        show_result("End Break", FAILED);
        return;
    };
    
    post_event("end_break", session_id, authenticity_token);
    int result = renew_session(session_id, authenticity_token);
    show_result("End Break", result);
}

void key_attendance_setting_cb(epdgui_args_vector_t &args) {
    Frame_Base *frame = EPDGUI_GetFrame("Frame_Setting_Language");
    if (frame == NULL) {
        frame = new Frame_Attendance_Setting();
        EPDGUI_AddFrame("Frame_Attendance_Setting", frame);
    }
    EPDGUI_PushFrame(frame);
    *((int *)(args[0])) = 0;
}

void init_button(EPDGUI_Button *button, int *is_run, String *session_id, String *authenticity_token,
    void (*func_cb)(epdgui_args_vector_t&), const uint8_t *data) {
    button->CanvasNormal()->pushImage(2, 2, 180, 180, data);
    button->CanvasPressed()->ReverseColor();
    button->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, (void *)(is_run));
    button->AddArgs(EPDGUI_Button::EVENT_RELEASED, 1, (void *)(session_id));
    button->AddArgs(EPDGUI_Button::EVENT_RELEASED, 2, (void *)(authenticity_token));
    button->Bind(EPDGUI_Button::EVENT_RELEASED, func_cb);
}

Frame_Attendance::Frame_Attendance(void) {
    _frame_name = "Frame_Attendance";
    _canvas_title->drawString("Attendance", 270, 34);

    _names = new M5EPD_Canvas(&M5.EPD);
    _names->createCanvas(540, 32);
    _names->setTextDatum(CC_DATUM);    

    _key_clock_in    = new EPDGUI_Button("CI", MGN_X, MGN_Y, BTN_W, BTN_H);
    _key_clock_out   = new EPDGUI_Button("CO", MGN_X + BTN_W + BTN_MGN_X,  MGN_Y, BTN_W, BTN_H);
    _key_start_break = new EPDGUI_Button("SB", MGN_X, MGN_Y + BTN_H + BTN_MGN_Y, BTN_W, BTN_H);
    _key_end_break   = new EPDGUI_Button("EB", MGN_X + BTN_W + BTN_MGN_X, MGN_Y + BTN_H + BTN_MGN_Y, BTN_W, BTN_H);

    init_button(_key_clock_in, &_is_run, &_session_id, &_authenticity_token, key_clock_in_cb, ImageResource_clock_180x180);
    init_button(_key_clock_out, &_is_run, &_session_id, &_authenticity_token, key_clock_out_cb, ImageResource_walking_180x180);
    init_button(_key_start_break, &_is_run, &_session_id, &_authenticity_token, key_start_break_cb, ImageResource_coffee_180x180);
    init_button(_key_end_break, &_is_run, &_session_id, &_authenticity_token, key_end_break_cb, ImageResource_circleleft_180x180);

    _key_setting = new EPDGUI_Button("Setting", MGN_X, MGN_Y + (BTN_MGN_Y + BTN_H) * 2, BTN_W, BTN_H); 
    
     _key_setting->CanvasNormal()->pushImage(2, 2, 180, 180, ImageResource_cog_180x180);
    _key_setting->CanvasPressed()->ReverseColor();
    _key_setting->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, (void *)(&_is_run));
    _key_setting->Bind(EPDGUI_Button::EVENT_RELEASED, key_attendance_setting_cb);
     
    exitbtn("Home"); // _key_exit is created
    _key_exit->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, (void*)(&_is_run));
    _key_exit->Bind(EPDGUI_Button::EVENT_RELEASED, &Frame_Base::exit_cb);
}

void Frame_Attendance::init_names(void) {
    if (!_names->isRenderExist(20)) {
        _names->createRender(20, 26);
    }
    _names->setTextSize(20);

    _names->fillCanvas(0);
    _names->drawString("Clock In",  MGN_X + BTN_W/2, 16);
    _names->drawString("Clock Out", MGN_X + BTN_W + BTN_MGN_X + BTN_W/2, 16);
    _names->pushCanvas(0, MGN_Y + BTN_H + 4, UPDATE_MODE_NONE);

    _names->fillCanvas(0);
    _names->drawString("Start Break", MGN_X + BTN_W/2, 16);
    _names->drawString("End Break",   MGN_X + BTN_W + BTN_MGN_X + BTN_W/2, 16);
    _names->pushCanvas(0, MGN_Y + BTN_H * 2 + BTN_MGN_Y + 4, UPDATE_MODE_NONE);
    
    _names->fillCanvas(0);
    _names->drawString("Setting", MGN_X + BTN_W/2,  16);
    _names->pushCanvas(0, MGN_Y + (BTN_H + BTN_MGN_Y) * 2 + BTN_H + 4, UPDATE_MODE_NONE);
}

void Frame_Attendance::exit(void) {
    SetSessionId(_session_id);
}

Frame_Attendance::~Frame_Attendance(void) {
    delete _key_clock_in;
    delete _key_clock_out;
    delete _key_start_break;
    delete _key_end_break;
    delete _key_setting;
    delete _names;
}

int Frame_Attendance::init(epdgui_args_vector_t &args) {
    _is_run = 1;
    M5.EPD.Clear();
    _canvas_title->pushCanvas(0, 8, UPDATE_MODE_NONE);

    EPDGUI_AddObject(_key_exit);
    EPDGUI_AddObject(_key_clock_in);
    EPDGUI_AddObject(_key_clock_out);
    EPDGUI_AddObject(_key_start_break);
    EPDGUI_AddObject(_key_end_break);
    EPDGUI_AddObject(_key_setting);

    init_names();
    init_wifi();

    _session_id = GetSessionId();
    renew_session(&_session_id, &_authenticity_token);

    return 3;
}