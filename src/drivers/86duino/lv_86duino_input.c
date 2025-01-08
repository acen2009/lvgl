#include "lv_86duino.h"

#if LV_USE_86DUINO

#define ALLEGRO_HAVE_STDINT_H
#include <allegro.h> // Need it when we use the USB mouse/keyboard

#include "v86board.h"
#include "../src/drivers/86duino/touchpanel.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct _tp {
    double DAX;
    double DBX;
    double DDX;
    double DAY;
    double DBY;
    double DDY;
    bool TP_ENABLED;
    bool MOUSE_ENABLED;
} TPD;

static void setInputData(lv_indev_t * indev, int inputType) {
    int i = 0, j, k;
    char c;
    int data1 = 0, data2 = 0;
    char floatchr[128] = {'\0'};
    
    TPD* tpd = (TPD*)malloc(sizeof(TPD));
    
    if (inputType == LV_86DUINO_INPUT_ONLY_TOUCHPAD || inputType == LV_86DUINO_INPUT_TOUCHPAD_KBMS) {
        // Default value
        tpd->DAX = -0.424629;
        tpd->DBX = -0.002177;
        tpd->DDX = 834.603571;
        tpd->DAY = -0.000115;
        tpd->DBY = -0.292238;
        tpd->DDY = 537.392779;
        tpd->TP_ENABLED = true;
        
        // Update value from QEC 
        FILE* fp = fopen("B:\\TP_CAL3.txt", "r");
        if (fp != NULL) {
            for (i=0; i<128 && (c = fgetc(fp)) != ','; i++)
                floatchr[i] = c;
            floatchr[i] = '\0';
            tpd->DAX = atof(floatchr);
            for (int i=0; i<128 && (c = fgetc(fp)) != ','; i++)
                floatchr[i] = c;
            floatchr[i] = '\0';
            tpd->DBX = atof(floatchr);
            for (int i=0; i<128 && (c = fgetc(fp)) != '\n'; i++)
                floatchr[i] = c;
            floatchr[i] = '\0';
            tpd->DDX = atof(floatchr);
            for (i=0; i<128 && (c = fgetc(fp)) != ','; i++)
                floatchr[i] = c;
            floatchr[i] = '\0';
            tpd->DAY = atof(floatchr);
            for (int i=0; i<128 && (c = fgetc(fp)) != ','; i++)
                floatchr[i] = c;
            floatchr[i] = '\0';
            tpd->DBY = atof(floatchr);
            for (int i=0; i<128 && (c = fgetc(fp)) != '\n'; i++)
                floatchr[i] = c;
            floatchr[i] = '\0';
            tpd->DDY = atof(floatchr);
            
            fclose(fp);
        }
        
        touchpanel_init(); // QEC LCD touch init
        
    } else
        tpd->TP_ENABLED = false;
    
    
    if (inputType == LV_86DUINO_INPUT_ONLY_KBMS || inputType == LV_86DUINO_INPUT_TOUCHPAD_KBMS)
        tpd->MOUSE_ENABLED = true;
    else
        tpd->MOUSE_ENABLED = false;
    
    lv_indev_set_user_data(indev, (void*)tpd);
}

static void touch_mouse_event(lv_indev_t * indev, lv_indev_data_t * data)
{
    static int pre_dx = 0, pre_dy = 0;
    int dx, dy, msdx = 0, msdy = 0;
    double newX = 0.0, newY = 0.0;
    bool isPress = false;
    
    lv_display_t * disp = lv_indev_get_driver_data(indev);
    int32_t hor_res = lv_display_get_horizontal_resolution(disp);
    int32_t ver_res = lv_display_get_vertical_resolution(disp);
    
    TPD* tpd = (TPD*)lv_indev_get_user_data(indev);
    
    if (tpd->TP_ENABLED) {
        isPress = get_touchXY(&dx, &dy);
        
        if (isPress) {
            newX = dx * tpd->DAX + dy * tpd->DBX + tpd->DDX;
            newY = dx * tpd->DAY + dy * tpd->DBY + tpd->DDY;
            
            if (newX < 0) newX = 0;
            else if (newX > hor_res) newX = hor_res;
            
            if (newY < 0) newY = 0;
            else if (newY > ver_res) newY = ver_res;
            
            data->point.x = pre_dx = newX;
            data->point.y = pre_dy = newY;
        } else {
            data->point.x = pre_dx;
            data->point.y = pre_dy;
        }
    }
    
    if (tpd->MOUSE_ENABLED) {
        
        if (mouse_needs_poll()) poll_mouse();
        
        get_mouse_mickeys(&msdx, &msdy);
        
        pre_dx += msdx;
        pre_dy += msdy;
        
        if (pre_dx < 0) pre_dx = 0;
        else if (pre_dx > hor_res) pre_dx = hor_res;
        
        if (pre_dy < 0) pre_dy = 0;
        else if (pre_dy > ver_res) pre_dy = ver_res;
        
        data->point.x = pre_dx;
        data->point.y = pre_dy;
    }
    
    if (isPress || (tpd->MOUSE_ENABLED && (mouse_b & 0x01) == 0x01))
        data->state = LV_INDEV_STATE_PRESSED;
    else
        data->state = LV_INDEV_STATE_RELEASED;
}

static void keyboard_event(lv_indev_t * indev, lv_indev_data_t * data)
{
    if (keyboard_needs_poll()) poll_keyboard();
    
    data->state = keypressed() ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;

    if (keypressed()) {
        if (key[KEY_UP]) data->key = LV_KEY_UP;
        else if (key[KEY_DOWN]) data->key = LV_KEY_DOWN;
        else if (key[KEY_RIGHT]) data->key = LV_KEY_RIGHT;
        else if (key[KEY_LEFT]) data->key = LV_KEY_LEFT;
        else if (key[KEY_ESC]) data->key = LV_KEY_ESC;
        else if (key[KEY_DEL]) data->key = LV_KEY_DEL;
        else if (key[KEY_BACKSPACE]) data->key = LV_KEY_BACKSPACE;
        else if (key[KEY_ENTER]) data->key = LV_KEY_ENTER;
        else if (key[KEY_HOME]) data->key = LV_KEY_HOME;
        else if (key[KEY_END]) data->key = LV_KEY_END;
        else {
            if (key_shifts & KB_SHIFT_FLAG) {
                if (key[KEY_TAB]) data->key = LV_KEY_PREV;
                else data->key = readkey() & 0xFF;
            } else {
                if (key[KEY_TAB]) data->key = LV_KEY_NEXT;
                else data->key = readkey() & 0xFF;
            }
        }
    }
}

void lv_86duino_inputs_create(lv_display_t * disp, int inputType, lv_image_dsc_t const * mouse_img)
{
    lv_group_t* _inp_group;
    bool kbms_enabled = false;
    
    if (disp == NULL) return; // if we selected __86DUINO_QEC_M2 board
    
    lv_indev_t * tp_ms = lv_indev_create();
    lv_indev_set_type(tp_ms, LV_INDEV_TYPE_POINTER); /*Touchpad or Mouse should have POINTER type*/
    lv_indev_set_read_cb(tp_ms, touch_mouse_event);
    lv_indev_set_driver_data(tp_ms, disp);

    if (inputType == LV_86DUINO_INPUT_AUTO) {
        #if defined (__86DUINO_QEC_M02) || defined (__86DUINO_ONE) || defined (__86DUINO_DUO) // Only screen and USB
        setInputData(tp_ms, LV_86DUINO_INPUT_ONLY_KBMS);
        kbms_enabled = true;
        #elif defined (__86DUINO_QEC)
        setInputData(tp_ms, LV_86DUINO_INPUT_ONLY_TOUCHPAD);
        #else // Touchpad and USB
        setInputData(tp_ms, LV_86DUINO_INPUT_TOUCHPAD_KBMS);
        kbms_enabled = true;
        #endif
    } else {
        setInputData(tp_ms, inputType);
        if (inputType == LV_86DUINO_INPUT_ONLY_KBMS || inputType == LV_86DUINO_INPUT_TOUCHPAD_KBMS)
            kbms_enabled = true;
    }

    _inp_group = lv_group_create();
    lv_group_set_default(_inp_group);
    lv_indev_set_group(tp_ms, _inp_group);
    
    // If use USB keyboard/mouse
    if (kbms_enabled) {
        lv_obj_t * mouse_cursor =  lv_image_create(lv_screen_active());
        if (mouse_img == NULL) {
            extern const lv_image_dsc_t mouse_cursor_icon;
            lv_image_set_src(mouse_cursor, &mouse_cursor_icon);
        } else
            lv_image_set_src(mouse_cursor, mouse_img);
        lv_indev_set_cursor(tp_ms, mouse_cursor);
        
        // Initialize the Allegro Mouse Driver
        allegro_init();
        install_mouse();
        install_keyboard();
        set_mouse_range(0, 0, 0x3FFF-1, 0x3FFF-1);
        
        // Initialize KeyBoard
        lv_indev_t * kb = lv_indev_create();
        lv_indev_set_type(kb, LV_INDEV_TYPE_KEYPAD);
        lv_indev_set_read_cb(kb, keyboard_event);
        lv_indev_set_group(kb, _inp_group);
    }
}

#endif /*LV_USE_VORTEX86*/