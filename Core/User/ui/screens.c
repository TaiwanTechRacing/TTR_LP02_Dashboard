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

//
// Event handlers
//

lv_obj_t *tick_value_change_obj;

static void event_handler_cb_system_sensor_steering_arc(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            int32_t value = lv_arc_get_value(ta);
            set_var_steering_deg(value);
        }
    }
}

//
// Screens
//

void create_screen_welcome() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.welcome = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 272);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // kmLabel_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.km_label_1 = obj;
            lv_obj_set_pos(obj, 48, 103);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_text(obj);
            lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "TAIWAN TECH RACING");
        }
        {
            // readyLabel_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ready_label_1 = obj;
            lv_obj_set_pos(obj, 0, 26);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_recolor(obj, true);
            add_style_text(obj);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xef3a5d), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_40, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
    }
    
    tick_screen_welcome();
}

void tick_screen_welcome() {
    {
        const char *new_val = get_var_leopard02();
        const char *cur_val = lv_label_get_text(objects.ready_label_1);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ready_label_1;
            lv_label_set_text(objects.ready_label_1, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_main() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 272);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj0 = obj;
            lv_obj_set_pos(obj, 408, 9);
            lv_obj_set_size(obj, 56, 200);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // hvSOCLabel
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.hv_soc_label = obj;
                    lv_obj_set_pos(obj, 2, 174);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "");
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 3, 3);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "SOC");
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // socBar
                            lv_obj_t *obj = lv_bar_create(parent_obj);
                            objects.soc_bar = obj;
                            lv_obj_set_pos(obj, 0, 27);
                            lv_obj_set_size(obj, 50, 138);
                            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                            lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                        }
                    }
                }
            }
        }
        {
            // kmLabel
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.km_label = obj;
            lv_obj_set_pos(obj, 235, 151);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_text(obj);
            lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Km/h");
        }
        {
            // speedLabel
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.speed_label = obj;
            lv_obj_set_pos(obj, -33, -55);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_orbiter_bold_180, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj1 = obj;
            lv_obj_set_pos(obj, 217, 218);
            lv_obj_set_size(obj, 246, 46);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // lvVoltageLabel
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.lv_voltage_label = obj;
                    lv_obj_set_pos(obj, 10, 13);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_label_set_text(obj, "");
                }
                {
                    // hvVoltageLabel
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.hv_voltage_label = obj;
                    lv_obj_set_pos(obj, 127, 12);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_label_set_text(obj, "");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj2 = obj;
            lv_obj_set_pos(obj, 18, 218);
            lv_obj_set_size(obj, 190, 46);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // modeLabel
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.mode_label = obj;
                    lv_obj_set_pos(obj, 0, 2);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_recolor(obj, true);
                    add_style_text(obj);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xd0ff00), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_40, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj3 = obj;
            lv_obj_set_pos(obj, 18, 163);
            lv_obj_set_size(obj, 190, 46);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // readyLabel
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.ready_label = obj;
                    lv_obj_set_pos(obj, -1, 2);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_recolor(obj, true);
                    add_style_text(obj);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x3fff00), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_40, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "");
                }
            }
        }
    }
    
    tick_screen_main();
}

void tick_screen_main() {
    {
        const char *new_val = get_var_label_soc_value();
        const char *cur_val = lv_label_get_text(objects.hv_soc_label);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.hv_soc_label;
            lv_label_set_text(objects.hv_soc_label, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_soc();
        int32_t cur_val = lv_bar_get_value(objects.soc_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.soc_bar;
            lv_bar_set_value(objects.soc_bar, new_val, LV_ANIM_ON);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_speed();
        const char *cur_val = lv_label_get_text(objects.speed_label);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.speed_label;
            lv_label_set_text(objects.speed_label, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_label_lv_value();
        const char *cur_val = lv_label_get_text(objects.lv_voltage_label);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.lv_voltage_label;
            lv_label_set_text(objects.lv_voltage_label, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_label_hv_value();
        const char *cur_val = lv_label_get_text(objects.hv_voltage_label);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.hv_voltage_label;
            lv_label_set_text(objects.hv_voltage_label, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_mode();
        const char *cur_val = lv_label_get_text(objects.mode_label);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.mode_label;
            lv_label_set_text(objects.mode_label, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ready();
        const char *cur_val = lv_label_get_text(objects.ready_label);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ready_label;
            lv_label_set_text(objects.ready_label, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_system_sdc() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.system_sdc = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 272);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 8, 12);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_text(obj);
            lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "SYSTEM-SDC");
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj4 = obj;
            lv_obj_set_pos(obj, 37, 67);
            lv_obj_set_size(obj, 131, 32);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // imdBar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.imd_bar = obj;
                    lv_obj_set_pos(obj, 7, 6);
                    lv_obj_set_size(obj, 31, 21);
                    lv_bar_set_range(obj, 0, 1);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 67, 6);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "IMD");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj5 = obj;
            lv_obj_set_pos(obj, 37, 106);
            lv_obj_set_size(obj, 131, 32);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // pdocBar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.pdoc_bar = obj;
                    lv_obj_set_pos(obj, 7, 6);
                    lv_obj_set_size(obj, 31, 21);
                    lv_bar_set_range(obj, 0, 1);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 54, 5);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "PDOC");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj6 = obj;
            lv_obj_set_pos(obj, 37, 145);
            lv_obj_set_size(obj, 131, 32);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // rsbBar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.rsb_bar = obj;
                    lv_obj_set_pos(obj, 7, 6);
                    lv_obj_set_size(obj, 31, 21);
                    lv_bar_set_range(obj, 0, 1);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 62, 6);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "RSB");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj7 = obj;
            lv_obj_set_pos(obj, 37, 184);
            lv_obj_set_size(obj, 131, 32);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // m1IlBar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.m1_il_bar = obj;
                    lv_obj_set_pos(obj, 7, 6);
                    lv_obj_set_size(obj, 31, 21);
                    lv_bar_set_range(obj, 0, 1);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 59, 5);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "M1-IL");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj8 = obj;
            lv_obj_set_pos(obj, 37, 223);
            lv_obj_set_size(obj, 131, 32);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // m4IlBar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.m4_il_bar = obj;
                    lv_obj_set_pos(obj, 7, 6);
                    lv_obj_set_size(obj, 31, 21);
                    lv_bar_set_range(obj, 0, 1);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 54, 5);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "M4-IL");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj9 = obj;
            lv_obj_set_pos(obj, 175, 67);
            lv_obj_set_size(obj, 131, 32);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // amsBar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.ams_bar = obj;
                    lv_obj_set_pos(obj, 7, 6);
                    lv_obj_set_size(obj, 31, 21);
                    lv_bar_set_range(obj, 0, 1);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 61, 6);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "AMS");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj10 = obj;
            lv_obj_set_pos(obj, 175, 106);
            lv_obj_set_size(obj, 131, 32);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // csbBar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.csb_bar = obj;
                    lv_obj_set_pos(obj, 7, 6);
                    lv_obj_set_size(obj, 31, 21);
                    lv_bar_set_range(obj, 0, 1);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 62, 5);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "CSB");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj11 = obj;
            lv_obj_set_pos(obj, 175, 145);
            lv_obj_set_size(obj, 131, 32);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // inrtBar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.inrt_bar = obj;
                    lv_obj_set_pos(obj, 7, 6);
                    lv_obj_set_size(obj, 31, 21);
                    lv_bar_set_range(obj, 0, 1);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 61, 6);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "INRT");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj12 = obj;
            lv_obj_set_pos(obj, 175, 184);
            lv_obj_set_size(obj, 131, 32);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // m2IlBar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.m2_il_bar = obj;
                    lv_obj_set_pos(obj, 7, 6);
                    lv_obj_set_size(obj, 31, 21);
                    lv_bar_set_range(obj, 0, 1);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 55, 5);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "M2-IL");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj13 = obj;
            lv_obj_set_pos(obj, 175, 223);
            lv_obj_set_size(obj, 131, 32);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // mcuIlBar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.mcu_il_bar = obj;
                    lv_obj_set_pos(obj, 7, 6);
                    lv_obj_set_size(obj, 31, 21);
                    lv_bar_set_range(obj, 0, 1);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 62, 5);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "MCU");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj14 = obj;
            lv_obj_set_pos(obj, 313, 67);
            lv_obj_set_size(obj, 131, 32);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // bspdBar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.bspd_bar = obj;
                    lv_obj_set_pos(obj, 7, 6);
                    lv_obj_set_size(obj, 31, 21);
                    lv_bar_set_range(obj, 0, 1);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 53, 6);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "BSPD");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj15 = obj;
            lv_obj_set_pos(obj, 313, 106);
            lv_obj_set_size(obj, 131, 32);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // lsbBar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.lsb_bar = obj;
                    lv_obj_set_pos(obj, 7, 6);
                    lv_obj_set_size(obj, 31, 21);
                    lv_bar_set_range(obj, 0, 1);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 62, 6);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "LSB");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj16 = obj;
            lv_obj_set_pos(obj, 313, 145);
            lv_obj_set_size(obj, 131, 32);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // botsBar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.bots_bar = obj;
                    lv_obj_set_pos(obj, 7, 6);
                    lv_obj_set_size(obj, 31, 21);
                    lv_bar_set_range(obj, 0, 1);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 54, 5);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "BOTS");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj17 = obj;
            lv_obj_set_pos(obj, 313, 184);
            lv_obj_set_size(obj, 131, 32);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // m3IlBar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.m3_il_bar = obj;
                    lv_obj_set_pos(obj, 7, 6);
                    lv_obj_set_size(obj, 31, 21);
                    lv_bar_set_range(obj, 0, 1);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 54, 4);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "M3-IL");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj18 = obj;
            lv_obj_set_pos(obj, 313, 223);
            lv_obj_set_size(obj, 131, 32);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // msdBar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.msd_bar = obj;
                    lv_obj_set_pos(obj, 7, 6);
                    lv_obj_set_size(obj, 31, 21);
                    lv_bar_set_range(obj, 0, 1);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 54, 5);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "MSD");
                }
            }
        }
    }
    
    tick_screen_system_sdc();
}

void tick_screen_system_sdc() {
    {
        int32_t new_val = get_var_sdc_imd();
        int32_t cur_val = lv_bar_get_value(objects.imd_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.imd_bar;
            lv_bar_set_value(objects.imd_bar, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_sdc_pdoc();
        int32_t cur_val = lv_bar_get_value(objects.pdoc_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.pdoc_bar;
            lv_bar_set_value(objects.pdoc_bar, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_sdc_rsb();
        int32_t cur_val = lv_bar_get_value(objects.rsb_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.rsb_bar;
            lv_bar_set_value(objects.rsb_bar, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_sdc_m1_il();
        int32_t cur_val = lv_bar_get_value(objects.m1_il_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.m1_il_bar;
            lv_bar_set_value(objects.m1_il_bar, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_sdc_m4_il();
        int32_t cur_val = lv_bar_get_value(objects.m4_il_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.m4_il_bar;
            lv_bar_set_value(objects.m4_il_bar, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_sdc_ams();
        int32_t cur_val = lv_bar_get_value(objects.ams_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.ams_bar;
            lv_bar_set_value(objects.ams_bar, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_sdc_csb();
        int32_t cur_val = lv_bar_get_value(objects.csb_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.csb_bar;
            lv_bar_set_value(objects.csb_bar, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_sdc_inrt();
        int32_t cur_val = lv_bar_get_value(objects.inrt_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.inrt_bar;
            lv_bar_set_value(objects.inrt_bar, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_sdc_m2_il();
        int32_t cur_val = lv_bar_get_value(objects.m2_il_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.m2_il_bar;
            lv_bar_set_value(objects.m2_il_bar, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_sdc_mcu_il();
        int32_t cur_val = lv_bar_get_value(objects.mcu_il_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.mcu_il_bar;
            lv_bar_set_value(objects.mcu_il_bar, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_sdc_bspd();
        int32_t cur_val = lv_bar_get_value(objects.bspd_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.bspd_bar;
            lv_bar_set_value(objects.bspd_bar, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_sdc_lsb();
        int32_t cur_val = lv_bar_get_value(objects.lsb_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.lsb_bar;
            lv_bar_set_value(objects.lsb_bar, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_sdc_bots();
        int32_t cur_val = lv_bar_get_value(objects.bots_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.bots_bar;
            lv_bar_set_value(objects.bots_bar, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_sdc_m3_il();
        int32_t cur_val = lv_bar_get_value(objects.m3_il_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.m3_il_bar;
            lv_bar_set_value(objects.m3_il_bar, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_sdc_msd();
        int32_t cur_val = lv_bar_get_value(objects.msd_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.msd_bar;
            lv_bar_set_value(objects.msd_bar, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_system_ecu() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.system_ecu = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 272);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 8, 12);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_text(obj);
            lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "SYSTEM-ECU");
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj19 = obj;
            lv_obj_set_pos(obj, 37, 67);
            lv_obj_set_size(obj, 131, 32);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // mcu1Bar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.mcu1_bar = obj;
                    lv_obj_set_pos(obj, 7, 6);
                    lv_obj_set_size(obj, 31, 21);
                    lv_bar_set_range(obj, 0, 1);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 56, 5);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "MCU1");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj20 = obj;
            lv_obj_set_pos(obj, 37, 106);
            lv_obj_set_size(obj, 131, 32);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // mcu4Bar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.mcu4_bar = obj;
                    lv_obj_set_pos(obj, 7, 6);
                    lv_obj_set_size(obj, 31, 21);
                    lv_bar_set_range(obj, 0, 1);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 54, 5);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "MCU4");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj21 = obj;
            lv_obj_set_pos(obj, 37, 145);
            lv_obj_set_size(obj, 131, 32);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // gpsBar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.gps_bar = obj;
                    lv_obj_set_pos(obj, 7, 6);
                    lv_obj_set_size(obj, 31, 21);
                    lv_bar_set_range(obj, 0, 1);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 62, 6);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "GPS");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj22 = obj;
            lv_obj_set_pos(obj, 175, 67);
            lv_obj_set_size(obj, 131, 32);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // mcu2Bar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.mcu2_bar = obj;
                    lv_obj_set_pos(obj, 7, 6);
                    lv_obj_set_size(obj, 31, 21);
                    lv_bar_set_range(obj, 0, 1);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 52, 6);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "MCU2");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj23 = obj;
            lv_obj_set_pos(obj, 175, 106);
            lv_obj_set_size(obj, 131, 32);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // ecuAmsBar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.ecu_ams_bar = obj;
                    lv_obj_set_pos(obj, 7, 6);
                    lv_obj_set_size(obj, 31, 21);
                    lv_bar_set_range(obj, 0, 1);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 62, 5);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "AMS");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj24 = obj;
            lv_obj_set_pos(obj, 313, 67);
            lv_obj_set_size(obj, 131, 32);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // mcu3Bar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.mcu3_bar = obj;
                    lv_obj_set_pos(obj, 7, 6);
                    lv_obj_set_size(obj, 31, 21);
                    lv_bar_set_range(obj, 0, 1);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 53, 6);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "MCU3");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj25 = obj;
            lv_obj_set_pos(obj, 313, 106);
            lv_obj_set_size(obj, 131, 32);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // imuBar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.imu_bar = obj;
                    lv_obj_set_pos(obj, 7, 6);
                    lv_obj_set_size(obj, 31, 21);
                    lv_bar_set_range(obj, 0, 1);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 62, 6);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "IMU");
                }
            }
        }
    }
    
    tick_screen_system_ecu();
}

void tick_screen_system_ecu() {
    {
        int32_t new_val = get_var_ecu_mcu1();
        int32_t cur_val = lv_bar_get_value(objects.mcu1_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.mcu1_bar;
            lv_bar_set_value(objects.mcu1_bar, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_ecu_mcu4();
        int32_t cur_val = lv_bar_get_value(objects.mcu4_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.mcu4_bar;
            lv_bar_set_value(objects.mcu4_bar, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_ecu_gps();
        int32_t cur_val = lv_bar_get_value(objects.gps_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.gps_bar;
            lv_bar_set_value(objects.gps_bar, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_ecu_mcu2();
        int32_t cur_val = lv_bar_get_value(objects.mcu2_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.mcu2_bar;
            lv_bar_set_value(objects.mcu2_bar, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_ecu_ams();
        int32_t cur_val = lv_bar_get_value(objects.ecu_ams_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.ecu_ams_bar;
            lv_bar_set_value(objects.ecu_ams_bar, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_ecu_mcu3();
        int32_t cur_val = lv_bar_get_value(objects.mcu3_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.mcu3_bar;
            lv_bar_set_value(objects.mcu3_bar, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_ecu_imu();
        int32_t cur_val = lv_bar_get_value(objects.imu_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.imu_bar;
            lv_bar_set_value(objects.imu_bar, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_system_sensor() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.system_sensor = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 272);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 8, 12);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_text(obj);
            lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "SYSTEM-SENSOR");
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj26 = obj;
            lv_obj_set_pos(obj, 8, 54);
            lv_obj_set_size(obj, 68, 191);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // apps1Label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.apps1_label = obj;
                    lv_obj_set_pos(obj, 3, 168);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "");
                }
                {
                    // apps1Bar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.apps1_bar = obj;
                    lv_obj_set_pos(obj, 3, 27);
                    lv_obj_set_size(obj, 61, 138);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 13, 4);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "AP1");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj27 = obj;
            lv_obj_set_pos(obj, 84, 54);
            lv_obj_set_size(obj, 68, 191);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // apps2Label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.apps2_label = obj;
                    lv_obj_set_pos(obj, 3, 168);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "");
                }
                {
                    // apps2Bar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.apps2_bar = obj;
                    lv_obj_set_pos(obj, 3, 27);
                    lv_obj_set_size(obj, 61, 138);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2df321), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 9, 4);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "AP2");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj28 = obj;
            lv_obj_set_pos(obj, 160, 54);
            lv_obj_set_size(obj, 68, 191);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // bseFrontLabel
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.bse_front_label = obj;
                    lv_obj_set_pos(obj, 3, 168);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "");
                }
                {
                    // bseFrontBar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.bse_front_bar = obj;
                    lv_obj_set_pos(obj, 3, 27);
                    lv_obj_set_size(obj, 61, 138);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xf32121), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xf32121), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 14, 4);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "BP1");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj29 = obj;
            lv_obj_set_pos(obj, 236, 54);
            lv_obj_set_size(obj, 68, 191);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // bseRearLabel
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.bse_rear_label = obj;
                    lv_obj_set_pos(obj, 3, 168);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "");
                }
                {
                    // bseRearBar
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    objects.bse_rear_bar = obj;
                    lv_obj_set_pos(obj, 3, 27);
                    lv_obj_set_size(obj, 61, 138);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xf32121), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xf32121), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 9, 4);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "BP2");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj30 = obj;
            lv_obj_set_pos(obj, 312, 54);
            lv_obj_set_size(obj, 160, 122);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // steeringLabel
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.steering_label = obj;
                    lv_obj_set_pos(obj, 58, 71);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "");
                }
                {
                    // kmLabel_4
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.km_label_4 = obj;
                    lv_obj_set_pos(obj, 22, 96);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_recolor(obj, true);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "STEERING");
                }
                {
                    // steeringArc
                    lv_obj_t *obj = lv_arc_create(parent_obj);
                    objects.steering_arc = obj;
                    lv_obj_set_pos(obj, 5, 15);
                    lv_obj_set_size(obj, 150, 150);
                    lv_arc_set_range(obj, -180, 180);
                    lv_arc_set_mode(obj, LV_ARC_MODE_SYMMETRICAL);
                    lv_arc_set_bg_start_angle(obj, 180);
                    lv_arc_set_bg_end_angle(obj, 0);
                    lv_obj_add_event_cb(obj, event_handler_cb_system_sensor_steering_arc, LV_EVENT_ALL, 0);
                    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
                    lv_obj_set_style_arc_rounded(obj, false, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_arc_width(obj, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xfc6a00), LV_PART_KNOB | LV_STATE_DEFAULT);
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj31 = obj;
            lv_obj_set_pos(obj, 312, 184);
            lv_obj_set_size(obj, 160, 61);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // bprLabel
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.bpr_label = obj;
                    lv_obj_set_pos(obj, 64, 35);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "");
                }
                {
                    // bpfLabel
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.bpf_label = obj;
                    lv_obj_set_pos(obj, 64, 8);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "");
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 5, 8);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "BPF: ");
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 5, 35);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_text(obj);
                    lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "BPR: ");
                }
            }
        }
    }
    
    tick_screen_system_sensor();
}

void tick_screen_system_sensor() {
    {
        const char *new_val = get_var_apps1_text();
        const char *cur_val = lv_label_get_text(objects.apps1_label);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.apps1_label;
            lv_label_set_text(objects.apps1_label, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_apps1_bar();
        int32_t cur_val = lv_bar_get_value(objects.apps1_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.apps1_bar;
            lv_bar_set_value(objects.apps1_bar, new_val, LV_ANIM_ON);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_apps2_text();
        const char *cur_val = lv_label_get_text(objects.apps2_label);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.apps2_label;
            lv_label_set_text(objects.apps2_label, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_apps2_bar();
        int32_t cur_val = lv_bar_get_value(objects.apps2_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.apps2_bar;
            lv_bar_set_value(objects.apps2_bar, new_val, LV_ANIM_ON);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_bse_front_text();
        const char *cur_val = lv_label_get_text(objects.bse_front_label);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.bse_front_label;
            lv_label_set_text(objects.bse_front_label, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_bse_front_bar();
        int32_t cur_val = lv_bar_get_value(objects.bse_front_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.bse_front_bar;
            lv_bar_set_value(objects.bse_front_bar, new_val, LV_ANIM_ON);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_bse_rear_text();
        const char *cur_val = lv_label_get_text(objects.bse_rear_label);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.bse_rear_label;
            lv_label_set_text(objects.bse_rear_label, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_bse_rear_bar();
        int32_t cur_val = lv_bar_get_value(objects.bse_rear_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.bse_rear_bar;
            lv_bar_set_value(objects.bse_rear_bar, new_val, LV_ANIM_ON);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_steering_text();
        const char *cur_val = lv_label_get_text(objects.steering_label);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.steering_label;
            lv_label_set_text(objects.steering_label, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_steering_deg();
        int32_t cur_val = lv_arc_get_value(objects.steering_arc);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.steering_arc;
            lv_arc_set_value(objects.steering_arc, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_bse_rear_press();
        const char *cur_val = lv_label_get_text(objects.bpr_label);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.bpr_label;
            lv_label_set_text(objects.bpr_label, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_bse_front_press();
        const char *cur_val = lv_label_get_text(objects.bpf_label);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.bpf_label;
            lv_label_set_text(objects.bpf_label, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_battery() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.battery = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 272);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 8, 12);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_text(obj);
            lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "BATTERY");
        }
    }
    
    tick_screen_battery();
}

void tick_screen_battery() {
}

void create_screen_inverter() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.inverter = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 272);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 8, 12);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_text(obj);
            lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "INVERTER");
        }
    }
    
    tick_screen_inverter();
}

void tick_screen_inverter() {
}

void create_screen_debug1() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.debug1 = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 272);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 8, 12);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_text(obj);
            lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "DEBUG1");
        }
        {
            // GIF
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.gif = obj;
            lv_obj_set_pos(obj, 90, 59);
            lv_obj_set_size(obj, 300, 200);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_debug1();
}

void tick_screen_debug1() {
}

void create_screen_debug2() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.debug2 = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 272);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 8, 12);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_text(obj);
            lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "DEBUG2");
        }
        {
            // GIF_1
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.gif_1 = obj;
            lv_obj_set_pos(obj, 90, 59);
            lv_obj_set_size(obj, 300, 200);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_debug2();
}

void tick_screen_debug2() {
}

void create_screen_debug3() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.debug3 = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 272);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 8, 12);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_text(obj);
            lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "DEBUG3");
        }
        {
            // GIF_2
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.gif_2 = obj;
            lv_obj_set_pos(obj, 90, 59);
            lv_obj_set_size(obj, 300, 200);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_debug3();
}

void tick_screen_debug3() {
}

void create_screen_game1() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.game1 = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 272);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // tetris_game_canva
            lv_obj_t *obj = lv_canvas_create(parent_obj);
            objects.tetris_game_canva = obj;
            lv_obj_set_pos(obj, 188, 11);
            lv_obj_set_size(obj, 150, 240);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // next_block
            lv_obj_t *obj = lv_canvas_create(parent_obj);
            objects.next_block = obj;
            lv_obj_set_pos(obj, 369, 35);
            lv_obj_set_size(obj, 80, 80);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 377, 11);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_text(obj);
            lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "NEXT");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 368, 125);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_text(obj);
            lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "SCORE");
        }
        {
            // hvSOCLabel_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.hv_soc_label_1 = obj;
            lv_obj_set_pos(obj, 396, 148);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_text(obj);
            lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // kmLabel_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.km_label_2 = obj;
            lv_obj_set_pos(obj, 5, 12);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_text(obj);
            lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "TAIWAN \nTECH \nRACING");
        }
        {
            // kmLabel_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.km_label_3 = obj;
            lv_obj_set_pos(obj, 5, 115);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_recolor(obj, true);
            add_style_text(obj);
            lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xef3a5d), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "TETRIS");
        }
        {
            // place_gif
            lv_obj_t *obj = lv_canvas_create(parent_obj);
            objects.place_gif = obj;
            lv_obj_set_pos(obj, 11, 149);
            lv_obj_set_size(obj, 136, 102);
        }
        {
            // modeLabel_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.mode_label_1 = obj;
            lv_obj_set_pos(obj, 168, 94);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_recolor(obj, true);
            add_style_text(obj);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xd0ff00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
    }
    
    tick_screen_game1();
}

void tick_screen_game1() {
    {
        const char *new_val = get_var_tetris_score();
        const char *cur_val = lv_label_get_text(objects.hv_soc_label_1);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.hv_soc_label_1;
            lv_label_set_text(objects.hv_soc_label_1, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_game_mode_text();
        const char *cur_val = lv_label_get_text(objects.mode_label_1);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.mode_label_1;
            lv_label_set_text(objects.mode_label_1, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_welcome,
    tick_screen_main,
    tick_screen_system_sdc,
    tick_screen_system_ecu,
    tick_screen_system_sensor,
    tick_screen_battery,
    tick_screen_inverter,
    tick_screen_debug1,
    tick_screen_debug2,
    tick_screen_debug3,
    tick_screen_game1,
};
void tick_screen(int screen_index) {
    if (screen_index >= 0 && screen_index < 11) {
        tick_screen_funcs[screen_index]();
    }
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen(screenId - 1);
}

//
// Fonts
//

ext_font_desc_t fonts[] = {
    { "orbitron_bold_20", &ui_font_orbitron_bold_20 },
    { "orbitron_bold_30", &ui_font_orbitron_bold_30 },
    { "orbitron_bold_40", &ui_font_orbitron_bold_40 },
    { "orbitron_bold_50", &ui_font_orbitron_bold_50 },
    { "ORBITER_BOLD_180", &ui_font_orbiter_bold_180 },
#if LV_FONT_MONTSERRAT_8
    { "MONTSERRAT_8", &lv_font_montserrat_8 },
#endif
#if LV_FONT_MONTSERRAT_10
    { "MONTSERRAT_10", &lv_font_montserrat_10 },
#endif
#if LV_FONT_MONTSERRAT_12
    { "MONTSERRAT_12", &lv_font_montserrat_12 },
#endif
#if LV_FONT_MONTSERRAT_14
    { "MONTSERRAT_14", &lv_font_montserrat_14 },
#endif
#if LV_FONT_MONTSERRAT_16
    { "MONTSERRAT_16", &lv_font_montserrat_16 },
#endif
#if LV_FONT_MONTSERRAT_18
    { "MONTSERRAT_18", &lv_font_montserrat_18 },
#endif
#if LV_FONT_MONTSERRAT_20
    { "MONTSERRAT_20", &lv_font_montserrat_20 },
#endif
#if LV_FONT_MONTSERRAT_22
    { "MONTSERRAT_22", &lv_font_montserrat_22 },
#endif
#if LV_FONT_MONTSERRAT_24
    { "MONTSERRAT_24", &lv_font_montserrat_24 },
#endif
#if LV_FONT_MONTSERRAT_26
    { "MONTSERRAT_26", &lv_font_montserrat_26 },
#endif
#if LV_FONT_MONTSERRAT_28
    { "MONTSERRAT_28", &lv_font_montserrat_28 },
#endif
#if LV_FONT_MONTSERRAT_30
    { "MONTSERRAT_30", &lv_font_montserrat_30 },
#endif
#if LV_FONT_MONTSERRAT_32
    { "MONTSERRAT_32", &lv_font_montserrat_32 },
#endif
#if LV_FONT_MONTSERRAT_34
    { "MONTSERRAT_34", &lv_font_montserrat_34 },
#endif
#if LV_FONT_MONTSERRAT_36
    { "MONTSERRAT_36", &lv_font_montserrat_36 },
#endif
#if LV_FONT_MONTSERRAT_38
    { "MONTSERRAT_38", &lv_font_montserrat_38 },
#endif
#if LV_FONT_MONTSERRAT_40
    { "MONTSERRAT_40", &lv_font_montserrat_40 },
#endif
#if LV_FONT_MONTSERRAT_42
    { "MONTSERRAT_42", &lv_font_montserrat_42 },
#endif
#if LV_FONT_MONTSERRAT_44
    { "MONTSERRAT_44", &lv_font_montserrat_44 },
#endif
#if LV_FONT_MONTSERRAT_46
    { "MONTSERRAT_46", &lv_font_montserrat_46 },
#endif
#if LV_FONT_MONTSERRAT_48
    { "MONTSERRAT_48", &lv_font_montserrat_48 },
#endif
};

//
// Color themes
//

uint32_t active_theme_index = 0;

//
//
//

void create_screens() {

// Set default LVGL theme
    lv_display_t *dispp = lv_display_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_display_set_theme(dispp, theme);
    
    // Initialize screens
    // Create screens
    create_screen_welcome();
    create_screen_main();
    create_screen_system_sdc();
    create_screen_system_ecu();
    create_screen_system_sensor();
    create_screen_battery();
    create_screen_inverter();
    create_screen_debug1();
    create_screen_debug2();
    create_screen_debug3();
    create_screen_game1();
}