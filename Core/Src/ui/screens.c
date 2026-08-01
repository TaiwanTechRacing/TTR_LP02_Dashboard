#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;
lv_obj_t *tick_value_change_obj;
uint32_t active_theme_index = 0;

void create_screen_welcome() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.welcome = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 272);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // tt
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.tt = obj;
            lv_obj_set_pos(obj, 128, 106);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_ttr_logo_2);
            lv_image_set_inner_align(obj, LV_IMAGE_ALIGN_DEFAULT);
        }
        {
            // tt_1
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.tt_1 = obj;
            lv_obj_set_pos(obj, 145, 194);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_partner_logo);
            lv_image_set_inner_align(obj, LV_IMAGE_ALIGN_DEFAULT);
        }
    }
    
    tick_screen_welcome();
}

void tick_screen_welcome() {
}

void create_screen_speed() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.speed = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 272);
    lv_obj_set_style_bg_image_src(obj, &img_speed_page, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // speed_pure
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.speed_pure = obj;
            lv_obj_set_pos(obj, 126, 122);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_speed_150, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffe1ff00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "000");
        }
        {
            // RTD_pure
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.rtd_pure = obj;
            lv_obj_set_pos(obj, 195, 70);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffff0000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "P");
        }
        {
            // DriveMode_pure
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.drive_mode_pure = obj;
            lv_obj_set_pos(obj, 250, 79);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "OFF");
        }
        {
            // speed_unit
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.speed_unit = obj;
            lv_obj_set_pos(obj, 354, 203);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "km/h");
        }
        {
            // ERROR_pure
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.error_pure = obj;
            lv_obj_set_pos(obj, 0, 20);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffff0000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 100, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "ERR - MCU1 MCU2 MCU3 MCU4 AMS");
        }
        {
            // ERROR_pure_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.error_pure_1 = obj;
            lv_obj_set_pos(obj, 0, 49);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xfffff000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 100, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "WARN - TEBPPC CELL_OVT");
        }
        {
            // Lap_time
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.lap_time = obj;
            lv_obj_set_pos(obj, 224, 231);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "00:00");
        }
        {
            // TEBPPC_warn
            lv_obj_t *obj = lv_msgbox_create(parent_obj);
            objects.tebppc_warn = obj;
            lv_obj_set_pos(obj, 0, 71);
            lv_obj_set_size(obj, 480, 147);
            lv_obj_set_style_align(obj, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xfffff000), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_speed();
}

void tick_screen_speed() {
}

void create_screen_racing() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.racing = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 272);
    lv_obj_set_style_bg_image_src(obj, &img_0622_rbg2, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Speed_Process
            lv_obj_t *obj = lv_arc_create(parent_obj);
            objects.speed_process = obj;
            lv_obj_set_pos(obj, 106, 17);
            lv_obj_set_size(obj, 270, 270);
            lv_arc_set_value(obj, 50);
            lv_arc_set_bg_start_angle(obj, 120);
            lv_arc_set_bg_end_angle(obj, 240);
            lv_obj_set_style_arc_color(obj, lv_color_hex(0xfff39b21), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_width(obj, 5, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_width(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
        }
        {
            // SteeringWheel_Dir
            lv_obj_t *obj = lv_arc_create(parent_obj);
            objects.steering_wheel_dir = obj;
            lv_obj_set_pos(obj, 116, 8);
            lv_obj_set_size(obj, 250, 250);
            lv_arc_set_range(obj, -180, 180);
            lv_arc_set_value(obj, 0);
            lv_arc_set_bg_start_angle(obj, 240);
            lv_arc_set_bg_end_angle(obj, 300);
            lv_arc_set_mode(obj, LV_ARC_MODE_SYMMETRICAL);
            lv_obj_set_style_arc_rounded(obj, true, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_color(obj, lv_color_hex(0xff21d7f3), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_width(obj, 10, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_width(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
        }
        {
            // Acc_Process
            lv_obj_t *obj = lv_arc_create(parent_obj);
            objects.acc_process = obj;
            lv_obj_set_pos(obj, 90, 2);
            lv_obj_set_size(obj, 300, 300);
            lv_arc_set_value(obj, 50);
            lv_arc_set_bg_start_angle(obj, 330);
            lv_arc_set_bg_end_angle(obj, 30);
            lv_arc_set_mode(obj, LV_ARC_MODE_REVERSE);
            lv_obj_set_style_arc_rounded(obj, true, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_color(obj, lv_color_hex(0xff60f321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_width(obj, 10, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_width(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
        }
        {
            // brake_process
            lv_obj_t *obj = lv_arc_create(parent_obj);
            objects.brake_process = obj;
            lv_obj_set_pos(obj, 90, 2);
            lv_obj_set_size(obj, 300, 300);
            lv_arc_set_value(obj, 50);
            lv_arc_set_bg_start_angle(obj, 150);
            lv_arc_set_bg_end_angle(obj, 210);
            lv_obj_set_style_arc_rounded(obj, true, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_color(obj, lv_color_hex(0xfff32121), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_width(obj, 10, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_width(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
        }
        {
            // pressure_Indicator
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.pressure_indicator = obj;
            lv_obj_set_pos(obj, 34, 101);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_pressure);
        }
        {
            // fans_Indicator
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.fans_indicator = obj;
            lv_obj_set_pos(obj, 34, 69);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_power);
            lv_obj_set_style_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // temp_Indicator
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.temp_indicator = obj;
            lv_obj_set_pos(obj, 34, 133);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_temp);
        }
        {
            // Speed_R
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.speed_r = obj;
            lv_obj_set_pos(obj, 169, 106);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, 61);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_speed_90, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "000");
        }
        {
            // Text
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.text = obj;
            lv_obj_set_pos(obj, 307, 150);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "km/h");
        }
        {
            // RTD
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.rtd = obj;
            lv_obj_set_pos(obj, 205, 45);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffff0000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "P");
        }
        {
            // Text_10
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.text_10 = obj;
            lv_obj_set_pos(obj, 20, 35);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "STATUS");
        }
        {
            // power_Indicator
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.power_indicator = obj;
            lv_obj_set_pos(obj, 34, 165);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_pwr_v2);
        }
        {
            // HV_Volt_R
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.hv_volt_r = obj;
            lv_obj_set_pos(obj, 196, 205);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "400.0");
        }
        {
            // GLV_Volt_R
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.glv_volt_r = obj;
            lv_obj_set_pos(obj, 245, 205);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "10.0");
        }
        {
            // HV_SOC_R
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.hv_soc_r = obj;
            lv_obj_set_pos(obj, 217, 221);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "100");
        }
        {
            // GLV_SOC_R
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.glv_soc_r = obj;
            lv_obj_set_pos(obj, 245, 221);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "100");
        }
        {
            // IOS_0
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.ios_0 = obj;
            lv_obj_set_pos(obj, 439, 43);
            lv_obj_set_size(obj, 10, 3);
            lv_led_set_color(obj, lv_color_hex(0xff71ff00));
            lv_led_set_brightness(obj, 255);
        }
        {
            // IOS__1
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.ios__1 = obj;
            lv_obj_set_pos(obj, 439, 57);
            lv_obj_set_size(obj, 10, 3);
            lv_led_set_color(obj, lv_color_hex(0xff71ff00));
            lv_led_set_brightness(obj, 255);
        }
        {
            // IOS__2
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.ios__2 = obj;
            lv_obj_set_pos(obj, 439, 71);
            lv_obj_set_size(obj, 10, 3);
            lv_led_set_color(obj, lv_color_hex(0xff71ff00));
            lv_led_set_brightness(obj, 255);
        }
        {
            // IOS__3
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.ios__3 = obj;
            lv_obj_set_pos(obj, 439, 85);
            lv_obj_set_size(obj, 10, 3);
            lv_led_set_color(obj, lv_color_hex(0xff71ff00));
            lv_led_set_brightness(obj, 255);
        }
        {
            // IOS__4
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.ios__4 = obj;
            lv_obj_set_pos(obj, 439, 99);
            lv_obj_set_size(obj, 10, 3);
            lv_led_set_color(obj, lv_color_hex(0xff71ff00));
            lv_led_set_brightness(obj, 255);
        }
        {
            // IOS__5
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.ios__5 = obj;
            lv_obj_set_pos(obj, 439, 113);
            lv_obj_set_size(obj, 10, 3);
            lv_led_set_color(obj, lv_color_hex(0xff71ff00));
            lv_led_set_brightness(obj, 255);
        }
        {
            // IOS__6
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.ios__6 = obj;
            lv_obj_set_pos(obj, 439, 127);
            lv_obj_set_size(obj, 10, 3);
            lv_led_set_color(obj, lv_color_hex(0xff71ff00));
            lv_led_set_brightness(obj, 255);
        }
        {
            // IOS__7
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.ios__7 = obj;
            lv_obj_set_pos(obj, 439, 142);
            lv_obj_set_size(obj, 10, 3);
            lv_led_set_color(obj, lv_color_hex(0xff71ff00));
            lv_led_set_brightness(obj, 255);
        }
        {
            // IOS__8
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.ios__8 = obj;
            lv_obj_set_pos(obj, 439, 156);
            lv_obj_set_size(obj, 10, 3);
            lv_led_set_color(obj, lv_color_hex(0xff71ff00));
            lv_led_set_brightness(obj, 255);
        }
        {
            // IOS__9
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.ios__9 = obj;
            lv_obj_set_pos(obj, 439, 170);
            lv_obj_set_size(obj, 10, 3);
            lv_led_set_color(obj, lv_color_hex(0xff71ff00));
            lv_led_set_brightness(obj, 255);
        }
        {
            // IOS__10
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.ios__10 = obj;
            lv_obj_set_pos(obj, 439, 184);
            lv_obj_set_size(obj, 10, 3);
            lv_led_set_color(obj, lv_color_hex(0xff71ff00));
            lv_led_set_brightness(obj, 255);
        }
        {
            // IOS__11
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.ios__11 = obj;
            lv_obj_set_pos(obj, 439, 198);
            lv_obj_set_size(obj, 10, 3);
            lv_led_set_color(obj, lv_color_hex(0xff71ff00));
            lv_led_set_brightness(obj, 255);
        }
        {
            // DriveMode
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.drive_mode = obj;
            lv_obj_set_pos(obj, 245, 57);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "OFF");
        }
    }
    
    tick_screen_racing();
}

void tick_screen_racing() {
}

void create_screen_factory_bat_sum() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.factory_bat_sum = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 272);
    lv_obj_set_style_bg_image_src(obj, &img_new_bg, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Text_16
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.text_16 = obj;
            lv_obj_set_pos(obj, 181, 11);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "FACTORY BAT");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 134, 122);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Accumalator SOC:");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 97, 100);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Accumulator Voltage:");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 110, 56);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "GLV Battery Voltage:");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 143, 78);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "GLV Battery SOC:");
        }
        {
            // GLV_V
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.glv_v = obj;
            lv_obj_set_pos(obj, 328, 56);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "00.0");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 389, 56);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "V");
        }
        {
            // GLV_SOC
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.glv_soc = obj;
            lv_obj_set_pos(obj, 326, 78);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "000");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 386, 78);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "%");
        }
        {
            // Acc_volt
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.acc_volt = obj;
            lv_obj_set_pos(obj, 328, 100);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "000.0");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 389, 100);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "V");
        }
        {
            // Acc_SOC
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.acc_soc = obj;
            lv_obj_set_pos(obj, 328, 122);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "000");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 388, 122);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "%");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 74, 144);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Maximum Temperature:");
        }
        {
            // acc_max_temp
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.acc_max_temp = obj;
            lv_obj_set_pos(obj, 328, 144);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "00.0");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 389, 144);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "C");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 77, 166);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Minimum Temperature:");
        }
        {
            // acc_min_temp
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.acc_min_temp = obj;
            lv_obj_set_pos(obj, 329, 166);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "00.0");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 389, 166);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "C");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 72, 187);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Difference Temperature:");
        }
        {
            // acc_diff_temp
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.acc_diff_temp = obj;
            lv_obj_set_pos(obj, 330, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "00.0");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 389, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "C");
        }
    }
    
    tick_screen_factory_bat_sum();
}

void tick_screen_factory_bat_sum() {
}

void create_screen_factory_bat_p1() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.factory_bat_p1 = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 272);
    lv_obj_set_style_bg_image_src(obj, &img_bat_bg, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Text_17
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.text_17 = obj;
            lv_obj_set_pos(obj, 181, 11);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "FACTORY BAT");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 97, 43);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Cell-I");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 334, 43);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Cell-II");
        }
        {
            // C1V1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v1 = obj;
            lv_obj_set_pos(obj, 40, 84);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v2 = obj;
            lv_obj_set_pos(obj, 40, 105);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v3 = obj;
            lv_obj_set_pos(obj, 40, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V4
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v4 = obj;
            lv_obj_set_pos(obj, 40, 146);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V5
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v5 = obj;
            lv_obj_set_pos(obj, 40, 167);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V6
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v6 = obj;
            lv_obj_set_pos(obj, 40, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V7
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v7 = obj;
            lv_obj_set_pos(obj, 40, 208);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V8
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v8 = obj;
            lv_obj_set_pos(obj, 40, 229);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V9
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v9 = obj;
            lv_obj_set_pos(obj, 114, 84);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V10
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v10 = obj;
            lv_obj_set_pos(obj, 114, 105);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V11
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v11 = obj;
            lv_obj_set_pos(obj, 114, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V12
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v12 = obj;
            lv_obj_set_pos(obj, 114, 146);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V13
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v13 = obj;
            lv_obj_set_pos(obj, 114, 167);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V14
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v14 = obj;
            lv_obj_set_pos(obj, 114, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1T1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1t1 = obj;
            lv_obj_set_pos(obj, 114, 208);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1T2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1t2 = obj;
            lv_obj_set_pos(obj, 114, 229);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1T3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1t3 = obj;
            lv_obj_set_pos(obj, 188, 84);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1T4
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1t4 = obj;
            lv_obj_set_pos(obj, 188, 105);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1T5
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1t5 = obj;
            lv_obj_set_pos(obj, 188, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1T6
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1t6 = obj;
            lv_obj_set_pos(obj, 188, 146);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1TU
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1tu = obj;
            lv_obj_set_pos(obj, 188, 167);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1TL
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1tl = obj;
            lv_obj_set_pos(obj, 188, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1TD
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1td = obj;
            lv_obj_set_pos(obj, 188, 208);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1TA
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1ta = obj;
            lv_obj_set_pos(obj, 188, 229);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v1 = obj;
            lv_obj_set_pos(obj, 280, 84);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v2 = obj;
            lv_obj_set_pos(obj, 280, 105);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v3 = obj;
            lv_obj_set_pos(obj, 280, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V4
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v4 = obj;
            lv_obj_set_pos(obj, 280, 146);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V5
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v5 = obj;
            lv_obj_set_pos(obj, 280, 167);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V6
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v6 = obj;
            lv_obj_set_pos(obj, 280, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V7
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v7 = obj;
            lv_obj_set_pos(obj, 280, 208);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V8
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v8 = obj;
            lv_obj_set_pos(obj, 280, 229);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V9
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v9 = obj;
            lv_obj_set_pos(obj, 354, 84);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V10
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v10 = obj;
            lv_obj_set_pos(obj, 354, 105);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V11
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v11 = obj;
            lv_obj_set_pos(obj, 354, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V12
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v12 = obj;
            lv_obj_set_pos(obj, 354, 146);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V13
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v13 = obj;
            lv_obj_set_pos(obj, 354, 167);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V14
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v14 = obj;
            lv_obj_set_pos(obj, 354, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2T1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2t1 = obj;
            lv_obj_set_pos(obj, 354, 208);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2T2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2t2 = obj;
            lv_obj_set_pos(obj, 354, 229);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2T3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2t3 = obj;
            lv_obj_set_pos(obj, 428, 84);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2T4
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2t4 = obj;
            lv_obj_set_pos(obj, 428, 105);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2T5
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2t5 = obj;
            lv_obj_set_pos(obj, 428, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2T6
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2t6 = obj;
            lv_obj_set_pos(obj, 428, 146);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2TU
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2tu = obj;
            lv_obj_set_pos(obj, 428, 167);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2TL
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2tl = obj;
            lv_obj_set_pos(obj, 428, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2TD
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2td = obj;
            lv_obj_set_pos(obj, 428, 208);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2TA
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2ta = obj;
            lv_obj_set_pos(obj, 428, 229);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
    }
    
    tick_screen_factory_bat_p1();
}

void tick_screen_factory_bat_p1() {
}

void create_screen_factory_bat_p2() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.factory_bat_p2 = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 272);
    lv_obj_set_style_bg_image_src(obj, &img_bat_bg, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Text_18
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.text_18 = obj;
            lv_obj_set_pos(obj, 181, 11);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "FACTORY BAT");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 92, 43);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Cell-III");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 331, 43);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Cell-IV");
        }
        {
            // C1V1_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v1_1 = obj;
            lv_obj_set_pos(obj, 40, 84);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V2_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v2_1 = obj;
            lv_obj_set_pos(obj, 40, 105);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V3_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v3_1 = obj;
            lv_obj_set_pos(obj, 40, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V4_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v4_1 = obj;
            lv_obj_set_pos(obj, 40, 146);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V5_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v5_1 = obj;
            lv_obj_set_pos(obj, 40, 167);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V6_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v6_1 = obj;
            lv_obj_set_pos(obj, 40, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V7_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v7_1 = obj;
            lv_obj_set_pos(obj, 40, 208);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V8_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v8_1 = obj;
            lv_obj_set_pos(obj, 40, 229);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V9_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v9_1 = obj;
            lv_obj_set_pos(obj, 114, 84);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V10_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v10_1 = obj;
            lv_obj_set_pos(obj, 114, 105);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V11_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v11_1 = obj;
            lv_obj_set_pos(obj, 114, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V12_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v12_1 = obj;
            lv_obj_set_pos(obj, 114, 146);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V13_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v13_1 = obj;
            lv_obj_set_pos(obj, 114, 167);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V14_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v14_1 = obj;
            lv_obj_set_pos(obj, 114, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1T1_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1t1_1 = obj;
            lv_obj_set_pos(obj, 114, 208);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1T2_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1t2_1 = obj;
            lv_obj_set_pos(obj, 114, 229);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1T3_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1t3_1 = obj;
            lv_obj_set_pos(obj, 188, 84);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1T4_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1t4_1 = obj;
            lv_obj_set_pos(obj, 188, 105);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1T5_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1t5_1 = obj;
            lv_obj_set_pos(obj, 188, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1T6_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1t6_1 = obj;
            lv_obj_set_pos(obj, 188, 146);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1TU_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1tu_1 = obj;
            lv_obj_set_pos(obj, 188, 167);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1TL_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1tl_1 = obj;
            lv_obj_set_pos(obj, 188, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1TD_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1td_1 = obj;
            lv_obj_set_pos(obj, 188, 208);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1TA_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1ta_1 = obj;
            lv_obj_set_pos(obj, 188, 229);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V1_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v1_1 = obj;
            lv_obj_set_pos(obj, 280, 84);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V2_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v2_1 = obj;
            lv_obj_set_pos(obj, 280, 105);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V3_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v3_1 = obj;
            lv_obj_set_pos(obj, 280, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V4_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v4_1 = obj;
            lv_obj_set_pos(obj, 280, 146);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V5_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v5_1 = obj;
            lv_obj_set_pos(obj, 280, 167);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V6_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v6_1 = obj;
            lv_obj_set_pos(obj, 280, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V7_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v7_1 = obj;
            lv_obj_set_pos(obj, 280, 208);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V8_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v8_1 = obj;
            lv_obj_set_pos(obj, 280, 229);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V9_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v9_1 = obj;
            lv_obj_set_pos(obj, 354, 84);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V10_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v10_1 = obj;
            lv_obj_set_pos(obj, 354, 105);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V11_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v11_1 = obj;
            lv_obj_set_pos(obj, 354, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V12_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v12_1 = obj;
            lv_obj_set_pos(obj, 354, 146);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V13_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v13_1 = obj;
            lv_obj_set_pos(obj, 354, 167);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V14_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v14_1 = obj;
            lv_obj_set_pos(obj, 354, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2T1_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2t1_1 = obj;
            lv_obj_set_pos(obj, 354, 208);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2T2_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2t2_1 = obj;
            lv_obj_set_pos(obj, 354, 229);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2T3_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2t3_1 = obj;
            lv_obj_set_pos(obj, 428, 84);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2T4_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2t4_1 = obj;
            lv_obj_set_pos(obj, 428, 105);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2T5_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2t5_1 = obj;
            lv_obj_set_pos(obj, 428, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2T6_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2t6_1 = obj;
            lv_obj_set_pos(obj, 428, 146);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2TU_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2tu_1 = obj;
            lv_obj_set_pos(obj, 428, 167);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2TL_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2tl_1 = obj;
            lv_obj_set_pos(obj, 428, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2TD_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2td_1 = obj;
            lv_obj_set_pos(obj, 428, 208);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2TA_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2ta_1 = obj;
            lv_obj_set_pos(obj, 428, 229);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
    }
    
    tick_screen_factory_bat_p2();
}

void tick_screen_factory_bat_p2() {
}

void create_screen_factory_bat_p3() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.factory_bat_p3 = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 272);
    lv_obj_set_style_bg_image_src(obj, &img_bat_bg, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Text_19
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.text_19 = obj;
            lv_obj_set_pos(obj, 181, 11);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "FACTORY BAT");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 94, 43);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Cell-V");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 331, 43);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Cell-VI");
        }
        {
            // C1V1_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v1_2 = obj;
            lv_obj_set_pos(obj, 40, 84);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V2_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v2_2 = obj;
            lv_obj_set_pos(obj, 40, 105);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V3_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v3_2 = obj;
            lv_obj_set_pos(obj, 40, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V4_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v4_2 = obj;
            lv_obj_set_pos(obj, 40, 146);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V5_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v5_2 = obj;
            lv_obj_set_pos(obj, 40, 167);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V6_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v6_2 = obj;
            lv_obj_set_pos(obj, 40, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V7_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v7_2 = obj;
            lv_obj_set_pos(obj, 40, 208);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V8_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v8_2 = obj;
            lv_obj_set_pos(obj, 40, 229);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V9_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v9_2 = obj;
            lv_obj_set_pos(obj, 114, 84);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V10_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v10_2 = obj;
            lv_obj_set_pos(obj, 114, 105);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V11_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v11_2 = obj;
            lv_obj_set_pos(obj, 114, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V12_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v12_2 = obj;
            lv_obj_set_pos(obj, 114, 146);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V13_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v13_2 = obj;
            lv_obj_set_pos(obj, 114, 167);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V14_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v14_2 = obj;
            lv_obj_set_pos(obj, 114, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1T1_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1t1_2 = obj;
            lv_obj_set_pos(obj, 114, 208);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1T2_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1t2_2 = obj;
            lv_obj_set_pos(obj, 114, 229);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1T3_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1t3_2 = obj;
            lv_obj_set_pos(obj, 188, 84);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1T4_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1t4_2 = obj;
            lv_obj_set_pos(obj, 188, 105);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1T5_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1t5_2 = obj;
            lv_obj_set_pos(obj, 188, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1T6_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1t6_2 = obj;
            lv_obj_set_pos(obj, 188, 146);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1TU_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1tu_2 = obj;
            lv_obj_set_pos(obj, 188, 167);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1TL_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1tl_2 = obj;
            lv_obj_set_pos(obj, 188, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1TD_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1td_2 = obj;
            lv_obj_set_pos(obj, 188, 208);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1TA_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1ta_2 = obj;
            lv_obj_set_pos(obj, 188, 229);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V1_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v1_2 = obj;
            lv_obj_set_pos(obj, 280, 84);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V2_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v2_2 = obj;
            lv_obj_set_pos(obj, 280, 105);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V3_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v3_2 = obj;
            lv_obj_set_pos(obj, 280, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V4_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v4_2 = obj;
            lv_obj_set_pos(obj, 280, 146);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V5_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v5_2 = obj;
            lv_obj_set_pos(obj, 280, 167);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V6_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v6_2 = obj;
            lv_obj_set_pos(obj, 280, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V7_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v7_2 = obj;
            lv_obj_set_pos(obj, 280, 208);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V8_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v8_2 = obj;
            lv_obj_set_pos(obj, 280, 229);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V9_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v9_2 = obj;
            lv_obj_set_pos(obj, 354, 84);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V10_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v10_2 = obj;
            lv_obj_set_pos(obj, 354, 105);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V11_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v11_2 = obj;
            lv_obj_set_pos(obj, 354, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V12_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v12_2 = obj;
            lv_obj_set_pos(obj, 354, 146);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V13_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v13_2 = obj;
            lv_obj_set_pos(obj, 354, 167);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V14_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v14_2 = obj;
            lv_obj_set_pos(obj, 354, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2T1_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2t1_2 = obj;
            lv_obj_set_pos(obj, 354, 208);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2T2_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2t2_2 = obj;
            lv_obj_set_pos(obj, 354, 229);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2T3_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2t3_2 = obj;
            lv_obj_set_pos(obj, 428, 84);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2T4_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2t4_2 = obj;
            lv_obj_set_pos(obj, 428, 105);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2T5_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2t5_2 = obj;
            lv_obj_set_pos(obj, 428, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2T6_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2t6_2 = obj;
            lv_obj_set_pos(obj, 428, 146);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2TU_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2tu_2 = obj;
            lv_obj_set_pos(obj, 428, 167);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2TL_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2tl_2 = obj;
            lv_obj_set_pos(obj, 428, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2TD_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2td_2 = obj;
            lv_obj_set_pos(obj, 428, 208);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2TA_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2ta_2 = obj;
            lv_obj_set_pos(obj, 428, 229);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
    }
    
    tick_screen_factory_bat_p3();
}

void tick_screen_factory_bat_p3() {
}

void create_screen_factory_bat_p4() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.factory_bat_p4 = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 272);
    lv_obj_set_style_bg_image_src(obj, &img_bat_bg, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Text_20
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.text_20 = obj;
            lv_obj_set_pos(obj, 181, 11);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "FACTORY BAT");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 88, 43);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Cell-VII");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 325, 43);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Cell-VIII");
        }
        {
            // C1V1_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v1_3 = obj;
            lv_obj_set_pos(obj, 40, 84);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V2_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v2_3 = obj;
            lv_obj_set_pos(obj, 40, 105);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V3_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v3_3 = obj;
            lv_obj_set_pos(obj, 40, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V4_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v4_3 = obj;
            lv_obj_set_pos(obj, 40, 146);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V5_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v5_3 = obj;
            lv_obj_set_pos(obj, 40, 167);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V6_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v6_3 = obj;
            lv_obj_set_pos(obj, 40, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V7_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v7_3 = obj;
            lv_obj_set_pos(obj, 40, 208);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V8_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v8_3 = obj;
            lv_obj_set_pos(obj, 40, 229);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V9_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v9_3 = obj;
            lv_obj_set_pos(obj, 114, 84);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V10_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v10_3 = obj;
            lv_obj_set_pos(obj, 114, 105);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V11_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v11_3 = obj;
            lv_obj_set_pos(obj, 114, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V12_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v12_3 = obj;
            lv_obj_set_pos(obj, 114, 146);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V13_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v13_3 = obj;
            lv_obj_set_pos(obj, 114, 167);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1V14_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1v14_3 = obj;
            lv_obj_set_pos(obj, 114, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1T1_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1t1_3 = obj;
            lv_obj_set_pos(obj, 114, 208);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1T2_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1t2_3 = obj;
            lv_obj_set_pos(obj, 114, 229);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1T3_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1t3_3 = obj;
            lv_obj_set_pos(obj, 188, 84);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1T4_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1t4_3 = obj;
            lv_obj_set_pos(obj, 188, 105);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1T5_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1t5_3 = obj;
            lv_obj_set_pos(obj, 188, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1T6_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1t6_3 = obj;
            lv_obj_set_pos(obj, 188, 146);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1TU_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1tu_3 = obj;
            lv_obj_set_pos(obj, 188, 167);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1TL_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1tl_3 = obj;
            lv_obj_set_pos(obj, 188, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1TD_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1td_3 = obj;
            lv_obj_set_pos(obj, 188, 208);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C1TA_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c1ta_3 = obj;
            lv_obj_set_pos(obj, 188, 229);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V1_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v1_3 = obj;
            lv_obj_set_pos(obj, 280, 84);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V2_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v2_3 = obj;
            lv_obj_set_pos(obj, 280, 105);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V3_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v3_3 = obj;
            lv_obj_set_pos(obj, 280, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V4_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v4_3 = obj;
            lv_obj_set_pos(obj, 280, 146);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V5_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v5_3 = obj;
            lv_obj_set_pos(obj, 280, 167);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V6_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v6_3 = obj;
            lv_obj_set_pos(obj, 280, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V7_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v7_3 = obj;
            lv_obj_set_pos(obj, 280, 208);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V8_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v8_3 = obj;
            lv_obj_set_pos(obj, 280, 229);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V9_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v9_3 = obj;
            lv_obj_set_pos(obj, 354, 84);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V10_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v10_3 = obj;
            lv_obj_set_pos(obj, 354, 105);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V11_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v11_3 = obj;
            lv_obj_set_pos(obj, 354, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V12_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v12_3 = obj;
            lv_obj_set_pos(obj, 354, 146);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V13_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v13_3 = obj;
            lv_obj_set_pos(obj, 354, 167);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2V14_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2v14_3 = obj;
            lv_obj_set_pos(obj, 354, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2T1_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2t1_3 = obj;
            lv_obj_set_pos(obj, 354, 208);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2T2_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2t2_3 = obj;
            lv_obj_set_pos(obj, 354, 229);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2T3_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2t3_3 = obj;
            lv_obj_set_pos(obj, 428, 84);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2T4_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2t4_3 = obj;
            lv_obj_set_pos(obj, 428, 105);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2T5_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2t5_3 = obj;
            lv_obj_set_pos(obj, 428, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2T6_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2t6_3 = obj;
            lv_obj_set_pos(obj, 428, 146);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2TU_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2tu_3 = obj;
            lv_obj_set_pos(obj, 428, 167);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2TL_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2tl_3 = obj;
            lv_obj_set_pos(obj, 428, 188);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2TD_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2td_3 = obj;
            lv_obj_set_pos(obj, 428, 208);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
        {
            // C2TA_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.c2ta_3 = obj;
            lv_obj_set_pos(obj, 428, 229);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "5.5");
        }
    }
    
    tick_screen_factory_bat_p4();
}

void tick_screen_factory_bat_p4() {
}

void create_screen_factory_mot() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.factory_mot = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 272);
    lv_obj_set_style_bg_image_src(obj, &img_motor_bg, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Text_14
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.text_14 = obj;
            lv_obj_set_pos(obj, 181, 11);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "FACTORY MOT");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 135, 58);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Gate 1  :");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 135, 75);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Gate 2 :");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 135, 92);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Gate 3 :");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 135, 109);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Temp  :");
        }
        {
            // M1G1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.m1g1 = obj;
            lv_obj_set_pos(obj, 191, 58);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "000");
        }
        {
            // M1G2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.m1g2 = obj;
            lv_obj_set_pos(obj, 191, 75);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "000");
        }
        {
            // M1G3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.m1g3 = obj;
            lv_obj_set_pos(obj, 191, 92);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "000");
        }
        {
            // M1T
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.m1t = obj;
            lv_obj_set_pos(obj, 191, 109);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "000");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 220, 58);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "C");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 220, 75);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "C");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 220, 92);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "C");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 220, 109);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "C");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 251, 58);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Gate 1  :");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 251, 75);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Gate 2 :");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 251, 92);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Gate 3 :");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 251, 109);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Temp  :");
        }
        {
            // M2G1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.m2g1 = obj;
            lv_obj_set_pos(obj, 307, 58);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "000");
        }
        {
            // M2G2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.m2g2 = obj;
            lv_obj_set_pos(obj, 307, 75);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "000");
        }
        {
            // M2G3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.m2g3 = obj;
            lv_obj_set_pos(obj, 307, 92);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "000");
        }
        {
            // M2T
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.m2t = obj;
            lv_obj_set_pos(obj, 307, 109);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "000");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 336, 58);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "C");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 336, 75);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "C");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 336, 92);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "C");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 336, 109);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "C");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 135, 160);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Gate 1  :");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 135, 177);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Gate 2 :");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 135, 194);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Gate 3 :");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 135, 211);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Temp  :");
        }
        {
            // M3G1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.m3g1 = obj;
            lv_obj_set_pos(obj, 191, 160);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "000");
        }
        {
            // M3G2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.m3g2 = obj;
            lv_obj_set_pos(obj, 191, 177);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "000");
        }
        {
            // M3G3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.m3g3 = obj;
            lv_obj_set_pos(obj, 191, 194);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "000");
        }
        {
            // M3T
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.m3t = obj;
            lv_obj_set_pos(obj, 191, 211);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "000");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 220, 160);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "C");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 220, 177);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "C");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 220, 194);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "C");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 220, 211);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "C");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 251, 160);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Gate 1  :");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 251, 177);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Gate 2 :");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 251, 194);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Gate 3 :");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 251, 211);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Temp  :");
        }
        {
            // M4G1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.m4g1 = obj;
            lv_obj_set_pos(obj, 307, 160);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "000");
        }
        {
            // M4G2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.m4g2 = obj;
            lv_obj_set_pos(obj, 307, 177);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "000");
        }
        {
            // M4G3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.m4g3 = obj;
            lv_obj_set_pos(obj, 307, 194);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "000");
        }
        {
            // M4T
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.m4t = obj;
            lv_obj_set_pos(obj, 307, 211);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "000");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 336, 160);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "C");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 336, 177);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "C");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 336, 194);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "C");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 336, 211);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "C");
        }
    }
    
    tick_screen_factory_mot();
}

void tick_screen_factory_mot() {
}



typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_welcome,
    tick_screen_speed,
    tick_screen_racing,
    tick_screen_factory_bat_sum,
    tick_screen_factory_bat_p1,
    tick_screen_factory_bat_p2,
    tick_screen_factory_bat_p3,
    tick_screen_factory_bat_p4,
    tick_screen_factory_mot,
};
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

void create_screens() {
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    create_screen_welcome();
    create_screen_speed();
    create_screen_racing();
    create_screen_factory_bat_sum();
    create_screen_factory_bat_p1();
    create_screen_factory_bat_p2();
    create_screen_factory_bat_p3();
    create_screen_factory_bat_p4();
    create_screen_factory_mot();
}
