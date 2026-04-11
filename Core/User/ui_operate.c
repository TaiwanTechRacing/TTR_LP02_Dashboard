/*
 * ui_operate.c
 *
 *  Created on: Feb 5, 2026
 *      Author: Dino
 */


#include "ui_operate.h"

void Icon_Set( lv_obj_t* obj, bool show){
	lv_opa_t OPA = show ? LV_OPA_100 : LV_OPA_0;
	lv_obj_set_style_opa(obj, OPA, LV_PART_MAIN | LV_STATE_DEFAULT);
}

