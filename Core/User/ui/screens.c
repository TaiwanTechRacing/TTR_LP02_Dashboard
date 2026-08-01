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
            // tt
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.tt = obj;
            lv_obj_set_pos(obj, 131, 97);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_ttr_logo_2);
            lv_image_set_scale(obj, 500);
            lv_image_set_inner_align(obj, LV_IMAGE_ALIGN_DEFAULT);
        }
    }
    
    tick_screen_welcome();
}

void tick_screen_welcome() {
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
            // readyLabel
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ready_label = obj;
            lv_obj_set_pos(obj, -119, 102);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_text(obj);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x3fff00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // hvSOCLabel
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.hv_soc_label = obj;
            lv_obj_set_pos(obj, 397, 206);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_text(obj);
            lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // lvVoltageLabel
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.lv_voltage_label = obj;
            lv_obj_set_pos(obj, 239, 239);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_text(obj);
            lv_label_set_text(obj, "");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 397, 10);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_text(obj);
            lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "SOC");
        }
        {
            // speedLabel
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.speed_label = obj;
            lv_obj_set_pos(obj, -51, -32);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_orbiter_bold_180, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // kmLabel
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.km_label = obj;
            lv_obj_set_pos(obj, 251, 171);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_text(obj);
            lv_obj_set_style_text_font(obj, &ui_font_orbitron_bold_40, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Km/h");
        }
        {
            // hvSocBar
            lv_obj_t *obj = lv_bar_create(parent_obj);
            objects.hv_soc_bar = obj;
            lv_obj_set_pos(obj, 408, 43);
            lv_obj_set_size(obj, 53, 157);
            lv_bar_set_range(obj, 18, 30);
            lv_bar_set_mode(obj, LV_BAR_MODE_RANGE);
            lv_bar_set_value(obj, 0, LV_ANIM_ON);
            lv_bar_set_start_value(obj, 0, LV_ANIM_ON);
            add_style_bar(obj);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x02ff02), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x02ff02), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 52, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x02ff3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x02ff3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // hvVoltageLabel
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.hv_voltage_label = obj;
            lv_obj_set_pos(obj, 360, 239);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_text(obj);
            lv_label_set_text(obj, "");
        }
    }
    
    tick_screen_main();
}

void tick_screen_main() {
    {
        const char *new_val = get_var_ready();
        const char *cur_val = lv_label_get_text(objects.ready_label);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ready_label;
            lv_label_set_text(objects.ready_label, new_val);
            tick_value_change_obj = NULL;
        }
    }
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
        const char *new_val = get_var_label_lv_value();
        const char *cur_val = lv_label_get_text(objects.lv_voltage_label);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.lv_voltage_label;
            lv_label_set_text(objects.lv_voltage_label, new_val);
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
        int32_t new_val = get_var_lv();
        int32_t cur_val = lv_bar_get_value(objects.hv_soc_bar);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.hv_soc_bar;
            lv_bar_set_value(objects.hv_soc_bar, new_val, LV_ANIM_ON);
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
    }
    
    tick_screen_debug1();
}

void tick_screen_debug1() {
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_welcome,
    tick_screen_main,
    tick_screen_debug1,
};
void tick_screen(int screen_index) {
    if (screen_index >= 0 && screen_index < 3) {
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
    { "orbitron_bold_25", &ui_font_orbitron_bold_25 },
    { "orbitron_bold_30", &ui_font_orbitron_bold_30 },
    { "orbitron_bold_40", &ui_font_orbitron_bold_40 },
    { "orbitron_bold_50", &ui_font_orbitron_bold_50 },
    { "ORBITER_BOLD_180", &ui_font_orbiter_bold_180 },
    { "ORBITER_BOLD_160", &ui_font_orbiter_bold_160 },
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
    create_screen_debug1();
}