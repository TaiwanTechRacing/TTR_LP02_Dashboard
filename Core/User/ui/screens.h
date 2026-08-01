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
    SCREEN_ID_DEBUG1 = 3,
    _SCREEN_ID_LAST = 3
};

typedef struct _objects_t {
    lv_obj_t *welcome;
    lv_obj_t *main;
    lv_obj_t *debug1;
    lv_obj_t *tt;
    lv_obj_t *ready_label;
    lv_obj_t *hv_soc_label;
    lv_obj_t *lv_voltage_label;
    lv_obj_t *speed_label;
    lv_obj_t *km_label;
    lv_obj_t *hv_soc_bar;
    lv_obj_t *hv_voltage_label;
} objects_t;

extern objects_t objects;

void create_screen_welcome();
void tick_screen_welcome();

void create_screen_main();
void tick_screen_main();

void create_screen_debug1();
void tick_screen_debug1();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/