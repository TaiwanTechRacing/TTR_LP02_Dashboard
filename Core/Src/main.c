/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lvgl.h"
#include "ui.h"
#include "screens.h"
#include <string.h>
#include <stdio.h>
#include "ttr_can.h"
#include "bsp_mpu.h"
#include "bsp_sdram.h"
#include "bsp_display.h"
#include "can_rx.h"
#include "debug_overlay.h"
#include "bsp_qspi.h"
#include "vehicle_data.h"
#include "can_decode.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* LCD_WIDTH / LCD_HEIGHT 現在定義在 bsp_display.h */

#define COLOR_RED 0xF800
#define COLOR_ORANGE 0xFD20
#define COLOR_YELLOW 0xFFE0
#define COLOR_GREEN 0x07E0
#define COLOR_BLUE 0x001F
#define COLOR_INDIGO 0x4810
#define COLOR_VIOLET 0x9011

#define HOLD 70

#define MIN_SCR_ID 1
#define MAX_SCR_ID 2

#define GLV_LOW_VOLT 19
#define MAX_MOTOR_SPEED 45535
#define HV_LOW_VOLT 350

/*
 * 下面這幾個週期以前是用「主迴圈跑幾圈」來計算的,但迴圈速度會隨著畫面複雜度
 * 變動,同一個數字在不同頁面代表的時間不一樣。現在一律用 HAL_GetTick() 的
 * 毫秒數,行為才可預測。
 */
#define WELCOME_HOLD_MS        3000U   /* 開機歡迎頁停留時間 */
#define UI_UPDATE_PERIOD_MS      25U   /* 把車輛資料寫進 widget 的頻率(40 Hz) */
#define BUTTON_SCAN_PERIOD_MS     5U   /* 按鍵取樣週期 */
#define BUTTON_DEBOUNCE_SCANS     5U   /* 連續 5 次讀到按下才算數 = 25 ms 去彈跳 */
#define DEBUG_TOGGLE_SCANS      200U   /* 兩鍵同時按住 200 x 5ms = 1 秒,切換 debug 疊層 */

#define NUM_OF_CELLS 112 //電芯數量
#define DATA_PER_PACK 4 //每個封包有4個電芯的電壓讀值
#define TOTAL_SEG 8 //8個單元
#define NUM_OF_TSENSOR 80
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

DMA2D_HandleTypeDef hdma2d;

FDCAN_HandleTypeDef hfdcan1;
FDCAN_HandleTypeDef hfdcan2;

LTDC_HandleTypeDef hltdc;

/* USER CODE BEGIN PV */
/*
 * framebuffer 搬到 SDRAM 了(以前是內部 RAM_D1 的一個 261 KB 陣列,吃掉半塊
 * 內部記憶體)。這個巨集只是給 CubeMX 產生的 MX_LTDC_Init() 取初始顯示位址用,
 * 之後的換頁由 bsp_display.c 負責。
 */
#define FRAMEBUFFER ((uint16_t (*)[LCD_WIDTH])SDRAM_FB0_ADDR)

FDCAN_FilterTypeDef sFilterConfig;
FDCAN_TxHeaderTypeDef TxHeader;
FDCAN_RxHeaderTypeDef RxHeader;

/* FDCAN 中斷收包用的暫存區。TTR_CAN_MAX_DLC 是 48。 */
uint8_t RX[TTR_CAN_MAX_DLC] = {0};

/* 目前顯示的頁面索引,由 ScanButtons() 換頁 */
uint8_t screen_ID_now = 0;

/* 硬體濾波器放行的 CAN ID。順序不重要,結尾的 0 是列表結束標記。
 * 這裡改了要記得 can_decode.c 那邊也要有對應的 case,否則收到也不會被解開。 */
static uint16_t STD_ID_LIST_CAN[35] = {
    TTR_CAN_ID_VCU_VCU_STATE,   TTR_CAN_ID_VCU_VCU_SDC,
    TTR_CAN_ID_VCU_VCU_SENSOR1, TTR_CAN_ID_VCU_VCU_SENSOR2,
    TTR_CAN_ID_VCU_VCU_SYSTEM_STATUS, TTR_CAN_ID_VCU_VCU_ERROR,
    TTR_CAN_ID_VCU_VCU_GPS,     TTR_CAN_ID_AMS_AMS_STATUS_BASIC,
    TTR_CAN_ID_AMS_AMS_MODULE_1, TTR_CAN_ID_AMS_AMS_MODULE_2,
    TTR_CAN_ID_AMS_AMS_MODULE_3, TTR_CAN_ID_AMS_AMS_MODULE_4,
    TTR_CAN_ID_AMS_AMS_MODULE_5, TTR_CAN_ID_AMS_AMS_MODULE_6,
    TTR_CAN_ID_AMS_AMS_MODULE_7, TTR_CAN_ID_AMS_AMS_MODULE_8,
    0
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_LTDC_Init(void);
static void MX_DMA2D_Init(void);
static void MX_FDCAN2_Init(void);
static void MX_FDCAN1_Init(void);
/* USER CODE BEGIN PFP */
static void ScanButtons(void);
/*
 * 換頁順序。index 0 是開機的歡迎頁,不列入按鍵循環;
 * MIN/MAX_SCR_ID 界定按鍵能循環的範圍。
 */
enum ScreensEnum screens[] = {
    SCREEN_ID_WELCOME,   /* 0  開機畫面,只在啟動時顯示 */
    SCREEN_ID_MAIN,      /* 1 */
    SCREEN_ID_DEBUG1,    /* 2 */
};



/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  bool welcome_done = false;
  uint32_t last_ui_update = 0;
  uint32_t last_button_scan = 0;
  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /*
   * 重設 MPU 並開啟 I-cache / D-cache。
   *
   * 上面那行 MPU_Config() 是 CubeMX 產生的,它把 0x60000000~0xDFFFFFFF 設成
   * no-access,SDRAM(0xC0000000)正好落在裡面 —— 這是之前板子上那顆 32 MB
   * SDRAM 完全用不了的原因。下面這個函式會整個覆蓋掉它。
   */
  BSP_MPU_ConfigAndEnableCache();
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */
  /* SDRAM 必須在 MX_LTDC_Init() 之前備妥 —— LTDC 一啟動就會開始從
   * framebuffer 位址讀資料。 */
  BSP_SDRAM_Init();
  if (!BSP_SDRAM_SelfTest())
  {
    Error_Handler();
  }

  /*
   * 板上的 QSPI Flash。目前還沒有東西放在裡面,先接起來是為了驗證驅動
   * 能不能正確認到晶片(用 BSP_QSPI_GetJedecId() / GetFlashSize() 看)。
   *
   * 刻意不檢查回傳值:這顆是給 UI 改版之後放圖片用的備援空間,沒有它
   * 儀表照樣能跑,不值得為它讓整個畫面黑掉。
   */
  (void)BSP_QSPI_Init();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_LTDC_Init();
  MX_DMA2D_Init();
  MX_FDCAN2_Init();
  MX_FDCAN1_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(BL_ENABLE_GPIO_Port, BL_ENABLE_Pin, 1);
  VehicleData_Init();

  BSP_Display_Init();
  ui_init();
  DebugOverlay_Init();

  TxHeader.Identifier = 0x580;
  TxHeader.IdType = FDCAN_STANDARD_ID;
  TxHeader.TxFrameType = FDCAN_DATA_FRAME;
  TxHeader.DataLength = FDCAN_DLC_BYTES_8;
  TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  TxHeader.BitRateSwitch = FDCAN_BRS_ON;
  TxHeader.FDFormat = FDCAN_FD_CAN;
  TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  TxHeader.MessageMarker = 0x0;
  /*
  if(HAL_FDCAN_AddMessageToTxBuffer(&hfdcan2, &TxHeader, TX, FDCAN_TX_BUFFER0) != HAL_OK)
  {
  	Error_Handler();
  }
  */



	HAL_FDCAN_ConfigGlobalFilter(&hfdcan2,
		FDCAN_REJECT, FDCAN_REJECT,
		FDCAN_REJECT, FDCAN_REJECT);



	FDCAN_FilterTypeDef sFilterConfigCAN2 = {0};
	sFilterConfigCAN2.IdType      = FDCAN_STANDARD_ID;
	sFilterConfigCAN2.FilterType  = FDCAN_FILTER_DUAL ;
	sFilterConfigCAN2.FilterConfig= FDCAN_FILTER_TO_RXFIFO0;

	//CAN2
	for (uint32_t i = 0; i < (sizeof(STD_ID_LIST_CAN)/sizeof(STD_ID_LIST_CAN[0])); i++)
	{
		sFilterConfigCAN2.FilterIndex = i;
		sFilterConfigCAN2.FilterID1   = STD_ID_LIST_CAN[i];
		sFilterConfigCAN2.FilterID2   = 0x7FF;
		if (HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfigCAN2) != HAL_OK) {
			Error_Handler();
		}
	}

	  /* Start the FDCAN module */
	if(HAL_FDCAN_Start(&hfdcan2) != HAL_OK)
	{
	Error_Handler();
	}

	HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);


  HAL_GPIO_WritePin(USR_LED_GPIO_Port, USR_LED_Pin, 1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    const uint32_t now = HAL_GetTick();

    /* 先把 CAN 資料解完,再讓 LVGL 畫 —— 這樣這一圈畫出來的就是最新的值。 */
    CAN_Poll();

    /*
     * LVGL 的 timer 與繪圖。
     *
     * 以前這裡是 lv_timer_handler() 之後接一個 HAL_Delay(time_till_next),
     * 整個主迴圈會睡滿一整個刷新週期(最多 33 ms)。後果是按鍵和資料更新都
     * 被拖著一起等,而且 updatescreen() 改完 widget 還得再等下一輪才畫得出來
     * —— CAN 資料到畫面的延遲是兩個刷新週期。現在迴圈不睡了,節流改由下面
     * 各自的時間判斷負責。
     */
    lv_timer_handler();
    BSP_Display_Service();

    if (!welcome_done && (now >= WELCOME_HOLD_MS))
    {
      welcome_done = true;
      screen_ID_now = 1;
      loadScreen(screens[screen_ID_now]);
    }

    /*
     * 讓 EEZ 產生的畫面去讀一次 get_var_xxx()(實作在 ui_bind.c)。
     * 舊版是由 updatescreen() 直接寫 widget,新版改成 EEZ 自己輪詢繫結的變數,
     * 所以這一行不能少 —— 少了畫面會停在預設值,而且不會有任何錯誤訊息。
     */
    if ((now - last_ui_update) >= UI_UPDATE_PERIOD_MS)
    {
      last_ui_update = now;
      ui_tick();
    }

    if ((now - last_button_scan) >= BUTTON_SCAN_PERIOD_MS)
    {
      last_button_scan = now;
      ScanButtons();
    }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
  PeriphClkInitStruct.PLL2.PLL2M = 2;
  PeriphClkInitStruct.PLL2.PLL2N = 12;
  PeriphClkInitStruct.PLL2.PLL2P = 2;
  PeriphClkInitStruct.PLL2.PLL2Q = 5;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief DMA2D Initialization Function
  * @param None
  * @retval None
  */
static void MX_DMA2D_Init(void)
{

  /* USER CODE BEGIN DMA2D_Init 0 */

  /* USER CODE END DMA2D_Init 0 */

  /* USER CODE BEGIN DMA2D_Init 1 */

  /* USER CODE END DMA2D_Init 1 */
  hdma2d.Instance = DMA2D;
  hdma2d.Init.Mode = DMA2D_M2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
  hdma2d.Init.OutputOffset = 0;
  hdma2d.LayerCfg[1].InputOffset = 0;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565;
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0;
  hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;
  hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;
  hdma2d.LayerCfg[1].ChromaSubSampling = DMA2D_NO_CSS;
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DMA2D_Init 2 */

  /* USER CODE END DMA2D_Init 2 */

}

/**
  * @brief FDCAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 16;
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 1;
  hfdcan1.Init.NominalTimeSeg2 = 1;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 1;
  hfdcan1.Init.DataTimeSeg2 = 1;
  hfdcan1.Init.MessageRAMOffset = 0;
  hfdcan1.Init.StdFiltersNbr = 0;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.RxFifo0ElmtsNbr = 0;
  hfdcan1.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.RxFifo1ElmtsNbr = 0;
  hfdcan1.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.RxBuffersNbr = 0;
  hfdcan1.Init.RxBufferSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.TxEventsNbr = 0;
  hfdcan1.Init.TxBuffersNbr = 0;
  hfdcan1.Init.TxFifoQueueElmtsNbr = 0;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  hfdcan1.Init.TxElmtSize = FDCAN_DATA_BYTES_8;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */

}

/**
  * @brief FDCAN2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_FDCAN2_Init(void)
{

  /* USER CODE BEGIN FDCAN2_Init 0 */

  /* USER CODE END FDCAN2_Init 0 */

  /* USER CODE BEGIN FDCAN2_Init 1 */

  /* USER CODE END FDCAN2_Init 1 */
  hfdcan2.Instance = FDCAN2;
  hfdcan2.Init.FrameFormat = FDCAN_FRAME_FD_NO_BRS;
  hfdcan2.Init.Mode = FDCAN_MODE_BUS_MONITORING;
  hfdcan2.Init.AutoRetransmission = DISABLE;
  hfdcan2.Init.TransmitPause = DISABLE;
  hfdcan2.Init.ProtocolException = DISABLE;
  hfdcan2.Init.NominalPrescaler = 3;
  hfdcan2.Init.NominalSyncJumpWidth = 4;
  hfdcan2.Init.NominalTimeSeg1 = 7;
  hfdcan2.Init.NominalTimeSeg2 = 2;
  hfdcan2.Init.DataPrescaler = 5;
  hfdcan2.Init.DataSyncJumpWidth = 2;
  hfdcan2.Init.DataTimeSeg1 = 7;
  hfdcan2.Init.DataTimeSeg2 = 2;
  hfdcan2.Init.MessageRAMOffset = 0x4E4;
  hfdcan2.Init.StdFiltersNbr = 50;
  hfdcan2.Init.ExtFiltersNbr = 0;
  hfdcan2.Init.RxFifo0ElmtsNbr = 32;
  hfdcan2.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_64;
  hfdcan2.Init.RxFifo1ElmtsNbr = 0;
  hfdcan2.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_64;
  hfdcan2.Init.RxBuffersNbr = 0;
  hfdcan2.Init.RxBufferSize = FDCAN_DATA_BYTES_8;
  hfdcan2.Init.TxEventsNbr = 0;
  hfdcan2.Init.TxBuffersNbr = 1;
  hfdcan2.Init.TxFifoQueueElmtsNbr = 0;
  hfdcan2.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  hfdcan2.Init.TxElmtSize = FDCAN_DATA_BYTES_8;
  if (HAL_FDCAN_Init(&hfdcan2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN2_Init 2 */

  /* USER CODE END FDCAN2_Init 2 */

}

/**
  * @brief LTDC Initialization Function
  * @param None
  * @retval None
  */
static void MX_LTDC_Init(void)
{

  /* USER CODE BEGIN LTDC_Init 0 */

  /* USER CODE END LTDC_Init 0 */

  LTDC_LayerCfgTypeDef pLayerCfg = {0};

  /* USER CODE BEGIN LTDC_Init 1 */

  /* USER CODE END LTDC_Init 1 */
  hltdc.Instance = LTDC;
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AH;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc.Init.HorizontalSync = 3;
  hltdc.Init.VerticalSync = 3;
  hltdc.Init.AccumulatedHBP = 46;
  hltdc.Init.AccumulatedVBP = 15;
  hltdc.Init.AccumulatedActiveW = 526;
  hltdc.Init.AccumulatedActiveH = 287;
  hltdc.Init.TotalWidth = 534;
  hltdc.Init.TotalHeigh = 295;
  hltdc.Init.Backcolor.Blue = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Red = 0;
  if (HAL_LTDC_Init(&hltdc) != HAL_OK)
  {
    Error_Handler();
  }
  pLayerCfg.WindowX0 = 0;
  pLayerCfg.WindowX1 = 480;
  pLayerCfg.WindowY0 = 0;
  pLayerCfg.WindowY1 = 272;
  pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
  pLayerCfg.Alpha = 255;
  pLayerCfg.Alpha0 = 0;
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
  pLayerCfg.FBStartAdress = (uint32_t)FRAMEBUFFER;
  pLayerCfg.ImageWidth = 480;
  pLayerCfg.ImageHeight = 272;
  pLayerCfg.Backcolor.Blue = 0;
  pLayerCfg.Backcolor.Green = 0;
  pLayerCfg.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LTDC_Init 2 */

  /* USER CODE END LTDC_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, USR_LED_Pin|BL_ENABLE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : USR_LED_Pin BL_ENABLE_Pin */
  GPIO_InitStruct.Pin = USR_LED_Pin|BL_ENABLE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : BUTTON_1_Pin BUTTON_2_Pin */
  GPIO_InitStruct.Pin = BUTTON_1_Pin|BUTTON_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/*
 * 讀取兩顆換頁按鍵。
 *
 * 原本是兩段幾乎一模一樣的程式碼,各自帶一個 counter 和一個 flag。這裡改成
 * 表格驅動:counter 加到門檻時觸發一次,之後就停在門檻不動,直到按鍵放開才
 * 歸零 —— 這樣本身就有「只觸發一次」的效果,不需要額外的 b1f / b2f 旗標。
 */
static void ScanButtons(void)
{
  static struct {
    GPIO_TypeDef *port;
    uint16_t      pin;
    int8_t        step;
    uint8_t       counter;
  } buttons[] = {
    { BUTTON_1_GPIO_Port, BUTTON_1_Pin, -1, 0 },
    { BUTTON_2_GPIO_Port, BUTTON_2_Pin, +1, 0 },
  };

  /*
   * 先處理「兩顆一起按住」的手勢,而且按住期間直接 return 不翻頁。
   * 如果兩顆是在同一個取樣週期內按下的(25ms 去彈跳生效之前),就完全
   * 不會翻頁;稍微錯開的話會先翻一頁,這是可以接受的取捨。
   */
  static uint16_t both_counter = 0;

  if (HAL_GPIO_ReadPin(BUTTON_1_GPIO_Port, BUTTON_1_Pin) == 0 &&
      HAL_GPIO_ReadPin(BUTTON_2_GPIO_Port, BUTTON_2_Pin) == 0)
  {
    if (both_counter < DEBUG_TOGGLE_SCANS)
    {
      both_counter++;
      if (both_counter == DEBUG_TOGGLE_SCANS)
      {
        DebugOverlay_Toggle();   /* 只在跨過門檻的那一次觸發 */
      }
    }
    return;
  }

  both_counter = 0;

  for (uint8_t i = 0; i < (sizeof(buttons) / sizeof(buttons[0])); i++)
  {
    if (HAL_GPIO_ReadPin(buttons[i].port, buttons[i].pin) != 0)
    {
      buttons[i].counter = 0;   /* 放開了 */
      continue;
    }

    if (buttons[i].counter >= BUTTON_DEBOUNCE_SCANS)
    {
      continue;                 /* 這次按壓已經翻過頁了,等放開 */
    }

    if (++buttons[i].counter < BUTTON_DEBOUNCE_SCANS)
    {
      continue;                 /* 還在去彈跳 */
    }

    int8_t next = (int8_t)screen_ID_now + buttons[i].step;
    if (next < MIN_SCR_ID) next = MAX_SCR_ID;
    if (next > MAX_SCR_ID) next = MIN_SCR_ID;

    screen_ID_now = (uint8_t)next;
    loadScreen(screens[screen_ID_now]);
  }
}


//HAL FDCAN call back
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
  if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
  {
    if(hfdcan->Instance == FDCAN2)
    {
      /*
       * 中斷裡只做一件事:把硬體 FIFO 裡的 frame 搬進軟體佇列,然後馬上結束。
       * 解包留給主迴圈的 CAN_Poll()(見 can_decode.c)。
       *
       * 用 while 一次把 FIFO 清空 —— 收到通知時裡面可能不只一包。
       */
      while(HAL_FDCAN_GetRxFifoFillLevel(&hfdcan2, FDCAN_RX_FIFO0) > 0){
        if(HAL_FDCAN_GetRxMessage(&hfdcan2, FDCAN_RX_FIFO0, &RxHeader, RX) != HAL_OK){
          break;
        }

        ttr_can_frame_t frame;
        frame.id  = RxHeader.Identifier;
        frame.dlc = RxHeader.DataLength;
        memcpy(frame.data, RX, TTR_CAN_MAX_DLC);

        CAN_RX_Enqueue(&frame);
      }
    }
  }
}

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
