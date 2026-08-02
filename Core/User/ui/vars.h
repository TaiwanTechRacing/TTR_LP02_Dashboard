#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations

// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_SOC = 0,
    FLOW_GLOBAL_VARIABLE_READY = 1,
    FLOW_GLOBAL_VARIABLE_LABEL_SOC_VALUE = 2,
    FLOW_GLOBAL_VARIABLE_SPEED = 3,
    FLOW_GLOBAL_VARIABLE_LABEL_LV_VALUE = 4,
    FLOW_GLOBAL_VARIABLE_LABEL_HV_VALUE = 5,
    FLOW_GLOBAL_VARIABLE_MODE = 6,
    FLOW_GLOBAL_VARIABLE_LEOPARD02 = 7,
    FLOW_GLOBAL_VARIABLE_LV = 8,
    FLOW_GLOBAL_VARIABLE_SDC_IMD = 9,
    FLOW_GLOBAL_VARIABLE_SDC_AMS = 10,
    FLOW_GLOBAL_VARIABLE_SDC_BSPD = 11,
    FLOW_GLOBAL_VARIABLE_SDC_PDOC = 12,
    FLOW_GLOBAL_VARIABLE_SDC_CSB = 13,
    FLOW_GLOBAL_VARIABLE_SDC_LSB = 14,
    FLOW_GLOBAL_VARIABLE_SDC_RSB = 15,
    FLOW_GLOBAL_VARIABLE_SDC_INRT = 16,
    FLOW_GLOBAL_VARIABLE_SDC_BOTS = 17,
    FLOW_GLOBAL_VARIABLE_SDC_MCU_IL = 18,
    FLOW_GLOBAL_VARIABLE_SDC_M1_IL = 19,
    FLOW_GLOBAL_VARIABLE_SDC_M2_IL = 20,
    FLOW_GLOBAL_VARIABLE_SDC_M3_IL = 21,
    FLOW_GLOBAL_VARIABLE_SDC_M4_IL = 22,
    FLOW_GLOBAL_VARIABLE_SDC_MSD = 23,
    FLOW_GLOBAL_VARIABLE_ECU_MCU1 = 24,
    FLOW_GLOBAL_VARIABLE_ECU_MCU2 = 25,
    FLOW_GLOBAL_VARIABLE_ECU_MCU3 = 26,
    FLOW_GLOBAL_VARIABLE_ECU_MCU4 = 27,
    FLOW_GLOBAL_VARIABLE_ECU_AMS = 28,
    FLOW_GLOBAL_VARIABLE_ECU_IMU = 29,
    FLOW_GLOBAL_VARIABLE_ECU_GPS = 30,
    FLOW_GLOBAL_VARIABLE_TETRIS_SCORE = 31,
    FLOW_GLOBAL_VARIABLE_APPS1_BAR = 32,
    FLOW_GLOBAL_VARIABLE_APPS1_TEXT = 33,
    FLOW_GLOBAL_VARIABLE_APPS2_BAR = 34,
    FLOW_GLOBAL_VARIABLE_APPS2_TEXT = 35,
    FLOW_GLOBAL_VARIABLE_BSE_FRONT_BAR = 36,
    FLOW_GLOBAL_VARIABLE_BSE_FRONT_TEXT = 37,
    FLOW_GLOBAL_VARIABLE_BSE_REAR_BAR = 38,
    FLOW_GLOBAL_VARIABLE_BSE_REAR_TEXT = 39,
    FLOW_GLOBAL_VARIABLE_STEERING_DEG = 40,
    FLOW_GLOBAL_VARIABLE_STEERING_TEXT = 41,
    FLOW_GLOBAL_VARIABLE_GAME_MODE_TEXT = 42,
    FLOW_GLOBAL_VARIABLE_BSE_FRONT_PRESS = 43,
    FLOW_GLOBAL_VARIABLE_BSE_REAR_PRESS = 44
};

// Native global variables

extern int32_t get_var_soc();
extern void set_var_soc(int32_t value);
extern const char *get_var_ready();
extern void set_var_ready(const char *value);
extern const char *get_var_label_soc_value();
extern void set_var_label_soc_value(const char *value);
extern const char *get_var_speed();
extern void set_var_speed(const char *value);
extern const char *get_var_label_lv_value();
extern void set_var_label_lv_value(const char *value);
extern const char *get_var_label_hv_value();
extern void set_var_label_hv_value(const char *value);
extern const char *get_var_mode();
extern void set_var_mode(const char *value);
extern const char *get_var_leopard02();
extern void set_var_leopard02(const char *value);
extern float get_var_lv();
extern void set_var_lv(float value);
extern int32_t get_var_sdc_imd();
extern void set_var_sdc_imd(int32_t value);
extern int32_t get_var_sdc_ams();
extern void set_var_sdc_ams(int32_t value);
extern int32_t get_var_sdc_bspd();
extern void set_var_sdc_bspd(int32_t value);
extern int32_t get_var_sdc_pdoc();
extern void set_var_sdc_pdoc(int32_t value);
extern int32_t get_var_sdc_csb();
extern void set_var_sdc_csb(int32_t value);
extern int32_t get_var_sdc_lsb();
extern void set_var_sdc_lsb(int32_t value);
extern int32_t get_var_sdc_rsb();
extern void set_var_sdc_rsb(int32_t value);
extern int32_t get_var_sdc_inrt();
extern void set_var_sdc_inrt(int32_t value);
extern int32_t get_var_sdc_bots();
extern void set_var_sdc_bots(int32_t value);
extern int32_t get_var_sdc_mcu_il();
extern void set_var_sdc_mcu_il(int32_t value);
extern int32_t get_var_sdc_m1_il();
extern void set_var_sdc_m1_il(int32_t value);
extern int32_t get_var_sdc_m2_il();
extern void set_var_sdc_m2_il(int32_t value);
extern int32_t get_var_sdc_m3_il();
extern void set_var_sdc_m3_il(int32_t value);
extern int32_t get_var_sdc_m4_il();
extern void set_var_sdc_m4_il(int32_t value);
extern int32_t get_var_sdc_msd();
extern void set_var_sdc_msd(int32_t value);
extern int32_t get_var_ecu_mcu1();
extern void set_var_ecu_mcu1(int32_t value);
extern int32_t get_var_ecu_mcu2();
extern void set_var_ecu_mcu2(int32_t value);
extern int32_t get_var_ecu_mcu3();
extern void set_var_ecu_mcu3(int32_t value);
extern int32_t get_var_ecu_mcu4();
extern void set_var_ecu_mcu4(int32_t value);
extern int32_t get_var_ecu_ams();
extern void set_var_ecu_ams(int32_t value);
extern int32_t get_var_ecu_imu();
extern void set_var_ecu_imu(int32_t value);
extern int32_t get_var_ecu_gps();
extern void set_var_ecu_gps(int32_t value);
extern const char *get_var_tetris_score();
extern void set_var_tetris_score(const char *value);
extern int32_t get_var_apps1_bar();
extern void set_var_apps1_bar(int32_t value);
extern const char *get_var_apps1_text();
extern void set_var_apps1_text(const char *value);
extern int32_t get_var_apps2_bar();
extern void set_var_apps2_bar(int32_t value);
extern const char *get_var_apps2_text();
extern void set_var_apps2_text(const char *value);
extern int32_t get_var_bse_front_bar();
extern void set_var_bse_front_bar(int32_t value);
extern const char *get_var_bse_front_text();
extern void set_var_bse_front_text(const char *value);
extern int32_t get_var_bse_rear_bar();
extern void set_var_bse_rear_bar(int32_t value);
extern const char *get_var_bse_rear_text();
extern void set_var_bse_rear_text(const char *value);
extern int32_t get_var_steering_deg();
extern void set_var_steering_deg(int32_t value);
extern const char *get_var_steering_text();
extern void set_var_steering_text(const char *value);
extern const char *get_var_game_mode_text();
extern void set_var_game_mode_text(const char *value);
extern const char *get_var_bse_front_press();
extern void set_var_bse_front_press(const char *value);
extern const char *get_var_bse_rear_press();
extern void set_var_bse_rear_press(const char *value);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/