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
    FLOW_GLOBAL_VARIABLE_MODE = 6
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

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/