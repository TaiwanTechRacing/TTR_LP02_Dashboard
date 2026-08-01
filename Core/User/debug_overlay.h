/*
 * debug_overlay.h
 *
 *  Debug 疊層:在畫面左上角顯示更新率(FPS)與 LVGL 的 CPU 佔比。
 *
 *  操作方式:兩顆按鍵同時按住約 1 秒切換顯示/隱藏。
 *  開機預設是關閉的 —— 比賽時不該有東西蓋在儀表畫面上。
 *
 *  疊層是掛在 LVGL 的 sysmon layer,不屬於任何一個 page,所以切頁時會一直
 *  留在畫面上,不需要每頁各自處理。
 *
 *  要在正式版完全編譯掉的話,把 Drivers/lv_conf.h 的 LV_USE_SYSMON 改回 0,
 *  下面這些函式會自動變成空實作,呼叫端不用改。
 */

#ifndef DEBUG_OVERLAY_H
#define DEBUG_OVERLAY_H

#include <stdbool.h>

/**
 * 初始化並確保疊層一開始是隱藏的。
 * 必須在 BSP_Display_Init() 之後呼叫。
 */
void DebugOverlay_Init(void);

/** 切換顯示/隱藏。 */
void DebugOverlay_Toggle(void);

/** 目前是否顯示中。 */
bool DebugOverlay_IsVisible(void);

#endif /* DEBUG_OVERLAY_H */
