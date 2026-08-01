#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *welcome;
    lv_obj_t *speed;
    lv_obj_t *racing;
    lv_obj_t *factory_bat_sum;
    lv_obj_t *factory_bat_p1;
    lv_obj_t *factory_bat_p2;
    lv_obj_t *factory_bat_p3;
    lv_obj_t *factory_bat_p4;
    lv_obj_t *factory_mot;
    lv_obj_t *tt;
    lv_obj_t *tt_1;
    lv_obj_t *speed_pure;
    lv_obj_t *rtd_pure;
    lv_obj_t *drive_mode_pure;
    lv_obj_t *speed_unit;
    lv_obj_t *error_pure;
    lv_obj_t *error_pure_1;
    lv_obj_t *lap_time;
    lv_obj_t *tebppc_warn;
    lv_obj_t *speed_process;
    lv_obj_t *steering_wheel_dir;
    lv_obj_t *acc_process;
    lv_obj_t *brake_process;
    lv_obj_t *pressure_indicator;
    lv_obj_t *fans_indicator;
    lv_obj_t *temp_indicator;
    lv_obj_t *speed_r;
    lv_obj_t *text;
    lv_obj_t *rtd;
    lv_obj_t *text_10;
    lv_obj_t *power_indicator;
    lv_obj_t *hv_volt_r;
    lv_obj_t *glv_volt_r;
    lv_obj_t *hv_soc_r;
    lv_obj_t *glv_soc_r;
    lv_obj_t *ios_0;
    lv_obj_t *ios__1;
    lv_obj_t *ios__2;
    lv_obj_t *ios__3;
    lv_obj_t *ios__4;
    lv_obj_t *ios__5;
    lv_obj_t *ios__6;
    lv_obj_t *ios__7;
    lv_obj_t *ios__8;
    lv_obj_t *ios__9;
    lv_obj_t *ios__10;
    lv_obj_t *ios__11;
    lv_obj_t *drive_mode;
    lv_obj_t *text_16;
    lv_obj_t *glv_v;
    lv_obj_t *glv_soc;
    lv_obj_t *acc_volt;
    lv_obj_t *acc_soc;
    lv_obj_t *acc_max_temp;
    lv_obj_t *acc_min_temp;
    lv_obj_t *acc_diff_temp;
    lv_obj_t *text_17;
    lv_obj_t *c1v1;
    lv_obj_t *c1v2;
    lv_obj_t *c1v3;
    lv_obj_t *c1v4;
    lv_obj_t *c1v5;
    lv_obj_t *c1v6;
    lv_obj_t *c1v7;
    lv_obj_t *c1v8;
    lv_obj_t *c1v9;
    lv_obj_t *c1v10;
    lv_obj_t *c1v11;
    lv_obj_t *c1v12;
    lv_obj_t *c1v13;
    lv_obj_t *c1v14;
    lv_obj_t *c1t1;
    lv_obj_t *c1t2;
    lv_obj_t *c1t3;
    lv_obj_t *c1t4;
    lv_obj_t *c1t5;
    lv_obj_t *c1t6;
    lv_obj_t *c1tu;
    lv_obj_t *c1tl;
    lv_obj_t *c1td;
    lv_obj_t *c1ta;
    lv_obj_t *c2v1;
    lv_obj_t *c2v2;
    lv_obj_t *c2v3;
    lv_obj_t *c2v4;
    lv_obj_t *c2v5;
    lv_obj_t *c2v6;
    lv_obj_t *c2v7;
    lv_obj_t *c2v8;
    lv_obj_t *c2v9;
    lv_obj_t *c2v10;
    lv_obj_t *c2v11;
    lv_obj_t *c2v12;
    lv_obj_t *c2v13;
    lv_obj_t *c2v14;
    lv_obj_t *c2t1;
    lv_obj_t *c2t2;
    lv_obj_t *c2t3;
    lv_obj_t *c2t4;
    lv_obj_t *c2t5;
    lv_obj_t *c2t6;
    lv_obj_t *c2tu;
    lv_obj_t *c2tl;
    lv_obj_t *c2td;
    lv_obj_t *c2ta;
    lv_obj_t *text_18;
    lv_obj_t *c1v1_1;
    lv_obj_t *c1v2_1;
    lv_obj_t *c1v3_1;
    lv_obj_t *c1v4_1;
    lv_obj_t *c1v5_1;
    lv_obj_t *c1v6_1;
    lv_obj_t *c1v7_1;
    lv_obj_t *c1v8_1;
    lv_obj_t *c1v9_1;
    lv_obj_t *c1v10_1;
    lv_obj_t *c1v11_1;
    lv_obj_t *c1v12_1;
    lv_obj_t *c1v13_1;
    lv_obj_t *c1v14_1;
    lv_obj_t *c1t1_1;
    lv_obj_t *c1t2_1;
    lv_obj_t *c1t3_1;
    lv_obj_t *c1t4_1;
    lv_obj_t *c1t5_1;
    lv_obj_t *c1t6_1;
    lv_obj_t *c1tu_1;
    lv_obj_t *c1tl_1;
    lv_obj_t *c1td_1;
    lv_obj_t *c1ta_1;
    lv_obj_t *c2v1_1;
    lv_obj_t *c2v2_1;
    lv_obj_t *c2v3_1;
    lv_obj_t *c2v4_1;
    lv_obj_t *c2v5_1;
    lv_obj_t *c2v6_1;
    lv_obj_t *c2v7_1;
    lv_obj_t *c2v8_1;
    lv_obj_t *c2v9_1;
    lv_obj_t *c2v10_1;
    lv_obj_t *c2v11_1;
    lv_obj_t *c2v12_1;
    lv_obj_t *c2v13_1;
    lv_obj_t *c2v14_1;
    lv_obj_t *c2t1_1;
    lv_obj_t *c2t2_1;
    lv_obj_t *c2t3_1;
    lv_obj_t *c2t4_1;
    lv_obj_t *c2t5_1;
    lv_obj_t *c2t6_1;
    lv_obj_t *c2tu_1;
    lv_obj_t *c2tl_1;
    lv_obj_t *c2td_1;
    lv_obj_t *c2ta_1;
    lv_obj_t *text_19;
    lv_obj_t *c1v1_2;
    lv_obj_t *c1v2_2;
    lv_obj_t *c1v3_2;
    lv_obj_t *c1v4_2;
    lv_obj_t *c1v5_2;
    lv_obj_t *c1v6_2;
    lv_obj_t *c1v7_2;
    lv_obj_t *c1v8_2;
    lv_obj_t *c1v9_2;
    lv_obj_t *c1v10_2;
    lv_obj_t *c1v11_2;
    lv_obj_t *c1v12_2;
    lv_obj_t *c1v13_2;
    lv_obj_t *c1v14_2;
    lv_obj_t *c1t1_2;
    lv_obj_t *c1t2_2;
    lv_obj_t *c1t3_2;
    lv_obj_t *c1t4_2;
    lv_obj_t *c1t5_2;
    lv_obj_t *c1t6_2;
    lv_obj_t *c1tu_2;
    lv_obj_t *c1tl_2;
    lv_obj_t *c1td_2;
    lv_obj_t *c1ta_2;
    lv_obj_t *c2v1_2;
    lv_obj_t *c2v2_2;
    lv_obj_t *c2v3_2;
    lv_obj_t *c2v4_2;
    lv_obj_t *c2v5_2;
    lv_obj_t *c2v6_2;
    lv_obj_t *c2v7_2;
    lv_obj_t *c2v8_2;
    lv_obj_t *c2v9_2;
    lv_obj_t *c2v10_2;
    lv_obj_t *c2v11_2;
    lv_obj_t *c2v12_2;
    lv_obj_t *c2v13_2;
    lv_obj_t *c2v14_2;
    lv_obj_t *c2t1_2;
    lv_obj_t *c2t2_2;
    lv_obj_t *c2t3_2;
    lv_obj_t *c2t4_2;
    lv_obj_t *c2t5_2;
    lv_obj_t *c2t6_2;
    lv_obj_t *c2tu_2;
    lv_obj_t *c2tl_2;
    lv_obj_t *c2td_2;
    lv_obj_t *c2ta_2;
    lv_obj_t *text_20;
    lv_obj_t *c1v1_3;
    lv_obj_t *c1v2_3;
    lv_obj_t *c1v3_3;
    lv_obj_t *c1v4_3;
    lv_obj_t *c1v5_3;
    lv_obj_t *c1v6_3;
    lv_obj_t *c1v7_3;
    lv_obj_t *c1v8_3;
    lv_obj_t *c1v9_3;
    lv_obj_t *c1v10_3;
    lv_obj_t *c1v11_3;
    lv_obj_t *c1v12_3;
    lv_obj_t *c1v13_3;
    lv_obj_t *c1v14_3;
    lv_obj_t *c1t1_3;
    lv_obj_t *c1t2_3;
    lv_obj_t *c1t3_3;
    lv_obj_t *c1t4_3;
    lv_obj_t *c1t5_3;
    lv_obj_t *c1t6_3;
    lv_obj_t *c1tu_3;
    lv_obj_t *c1tl_3;
    lv_obj_t *c1td_3;
    lv_obj_t *c1ta_3;
    lv_obj_t *c2v1_3;
    lv_obj_t *c2v2_3;
    lv_obj_t *c2v3_3;
    lv_obj_t *c2v4_3;
    lv_obj_t *c2v5_3;
    lv_obj_t *c2v6_3;
    lv_obj_t *c2v7_3;
    lv_obj_t *c2v8_3;
    lv_obj_t *c2v9_3;
    lv_obj_t *c2v10_3;
    lv_obj_t *c2v11_3;
    lv_obj_t *c2v12_3;
    lv_obj_t *c2v13_3;
    lv_obj_t *c2v14_3;
    lv_obj_t *c2t1_3;
    lv_obj_t *c2t2_3;
    lv_obj_t *c2t3_3;
    lv_obj_t *c2t4_3;
    lv_obj_t *c2t5_3;
    lv_obj_t *c2t6_3;
    lv_obj_t *c2tu_3;
    lv_obj_t *c2tl_3;
    lv_obj_t *c2td_3;
    lv_obj_t *c2ta_3;
    lv_obj_t *text_14;
    lv_obj_t *m1g1;
    lv_obj_t *m1g2;
    lv_obj_t *m1g3;
    lv_obj_t *m1t;
    lv_obj_t *m2g1;
    lv_obj_t *m2g2;
    lv_obj_t *m2g3;
    lv_obj_t *m2t;
    lv_obj_t *m3g1;
    lv_obj_t *m3g2;
    lv_obj_t *m3g3;
    lv_obj_t *m3t;
    lv_obj_t *m4g1;
    lv_obj_t *m4g2;
    lv_obj_t *m4g3;
    lv_obj_t *m4t;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_WELCOME = 1,
    SCREEN_ID_SPEED = 2,
    SCREEN_ID_RACING = 3,
    SCREEN_ID_FACTORY_BAT_SUM = 4,
    SCREEN_ID_FACTORY_BAT_P1 = 5,
    SCREEN_ID_FACTORY_BAT_P2 = 6,
    SCREEN_ID_FACTORY_BAT_P3 = 7,
    SCREEN_ID_FACTORY_BAT_P4 = 8,
    SCREEN_ID_FACTORY_MOT = 9,
};

void create_screen_welcome();
void tick_screen_welcome();

void create_screen_speed();
void tick_screen_speed();

void create_screen_racing();
void tick_screen_racing();

void create_screen_factory_bat_sum();
void tick_screen_factory_bat_sum();

void create_screen_factory_bat_p1();
void tick_screen_factory_bat_p1();

void create_screen_factory_bat_p2();
void tick_screen_factory_bat_p2();

void create_screen_factory_bat_p3();
void tick_screen_factory_bat_p3();

void create_screen_factory_bat_p4();
void tick_screen_factory_bat_p4();

void create_screen_factory_mot();
void tick_screen_factory_mot();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/