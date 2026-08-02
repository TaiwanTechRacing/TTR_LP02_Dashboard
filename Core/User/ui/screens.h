#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_WELCOME = 1,
    SCREEN_ID_MAIN = 2,
    SCREEN_ID_SYSTEM_SDC = 3,
    SCREEN_ID_BATTERY = 4,
    SCREEN_ID_INVERTER = 5,
    SCREEN_ID_DEBUG1 = 6,
    SCREEN_ID_DEBUG2 = 7,
    SCREEN_ID_DEBUG3 = 8,
    _SCREEN_ID_LAST = 8
};

typedef struct _objects_t {
    lv_obj_t *welcome;
    lv_obj_t *main;
    lv_obj_t *system_sdc;
    lv_obj_t *battery;
    lv_obj_t *inverter;
    lv_obj_t *debug1;
    lv_obj_t *debug2;
    lv_obj_t *debug3;
    lv_obj_t *km_label_1;
    lv_obj_t *ready_label_1;
    lv_obj_t *obj0;
    lv_obj_t *hv_soc_label;
    lv_obj_t *soc_bar;
    lv_obj_t *km_label;
    lv_obj_t *speed_label;
    lv_obj_t *obj1;
    lv_obj_t *lv_voltage_label;
    lv_obj_t *hv_voltage_label;
    lv_obj_t *obj2;
    lv_obj_t *mode_label;
    lv_obj_t *obj3;
    lv_obj_t *ready_label;
    lv_obj_t *obj4;
    lv_obj_t *imd_bar;
    lv_obj_t *obj5;
    lv_obj_t *pdoc_bar;
    lv_obj_t *obj6;
    lv_obj_t *soc_bar_3;
    lv_obj_t *obj7;
    lv_obj_t *soc_bar_4;
    lv_obj_t *obj8;
    lv_obj_t *soc_bar_5;
    lv_obj_t *obj9;
    lv_obj_t *ams_bar;
    lv_obj_t *obj10;
    lv_obj_t *soc_bar_7;
    lv_obj_t *obj11;
    lv_obj_t *soc_bar_8;
    lv_obj_t *obj12;
    lv_obj_t *soc_bar_9;
    lv_obj_t *obj13;
    lv_obj_t *soc_bar_10;
    lv_obj_t *obj14;
    lv_obj_t *bspd_bar;
    lv_obj_t *obj15;
    lv_obj_t *soc_bar_12;
    lv_obj_t *obj16;
    lv_obj_t *soc_bar_13;
    lv_obj_t *obj17;
    lv_obj_t *soc_bar_14;
    lv_obj_t *obj18;
    lv_obj_t *soc_bar_15;
    lv_obj_t *gif;
    lv_obj_t *gif_1;
    lv_obj_t *gif_2;
} objects_t;

extern objects_t objects;

void create_screen_welcome();
void tick_screen_welcome();

void create_screen_main();
void tick_screen_main();

void create_screen_system_sdc();
void tick_screen_system_sdc();

void create_screen_battery();
void tick_screen_battery();

void create_screen_inverter();
void tick_screen_inverter();

void create_screen_debug1();
void tick_screen_debug1();

void create_screen_debug2();
void tick_screen_debug2();

void create_screen_debug3();
void tick_screen_debug3();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/