#ifndef EEZ_LVGL_UI_IMAGES_H
#define EEZ_LVGL_UI_IMAGES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_img_dsc_t img_logo;
extern const lv_img_dsc_t img_steering_wheel;
extern const lv_img_dsc_t img_racing_page_background;
extern const lv_img_dsc_t img_cooling;
extern const lv_img_dsc_t img_temp;
extern const lv_img_dsc_t img_pressure;
extern const lv_img_dsc_t img_factory_page_background;
extern const lv_img_dsc_t img_power;
extern const lv_img_dsc_t img_ttr_logo_2;
extern const lv_img_dsc_t img_pwr_v2;
extern const lv_img_dsc_t img_face;
extern const lv_img_dsc_t img_rb;
extern const lv_img_dsc_t img_tire;
extern const lv_img_dsc_t img_motor;
extern const lv_img_dsc_t img_m2;
extern const lv_img_dsc_t img_motor_amk;
extern const lv_img_dsc_t img_new_bg;
extern const lv_img_dsc_t img_motor_bg;
extern const lv_img_dsc_t img_swbg;
extern const lv_img_dsc_t img_bat;
extern const lv_img_dsc_t img_rp;
extern const lv_img_dsc_t img_bat_bg;
extern const lv_img_dsc_t img_partner_logo;
extern const lv_img_dsc_t img_0622;
extern const lv_img_dsc_t img_0622_rbg2;
extern const lv_img_dsc_t img_speed_page;

#ifndef EXT_IMG_DESC_T
#define EXT_IMG_DESC_T
typedef struct _ext_img_desc_t {
    const char *name;
    const lv_img_dsc_t *img_dsc;
} ext_img_desc_t;
#endif

extern const ext_img_desc_t images[26];


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_IMAGES_H*/