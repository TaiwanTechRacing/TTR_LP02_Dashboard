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
#include "ui_bind.h"
#include "gif_pages.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* LCD_WIDTH / LCD_HEIGHT now live in bsp_display.h */

#define COLOR_RED 0xF800
#define COLOR_ORANGE 0xFD20
#define COLOR_YELLOW 0xFFE0
#define COLOR_GREEN 0x07E0
#define COLOR_BLUE 0x001F
#define COLOR_INDIGO 0x4810
#define COLOR_VIOLET 0x9011

#define HOLD 70

#define MIN_SCR_ID 1
#define MAX_SCR_ID 4

#define GLV_LOW_VOLT 19
#define MAX_MOTOR_SPEED 45535
#define HV_LOW_VOLT 350

/*
 * These periods used to be counted in main loop iterations, but loop speed
 * varies with scene complexity, so the same number meant different amounts of
 * time on different pages. They are all wall-clock milliseconds from
 * HAL_GetTick() now, which behaves predictably.
 */
#define WELCOME_HOLD_MS        3000U   /* how long the splash screen stays up */
#define UI_UPDATE_PERIOD_MS      25U   /* rate at which the UI re-reads vehicle data (40 Hz) */
#define BUTTON_SCAN_PERIOD_MS     5U   /* button sampling period */
#define BUTTON_DEBOUNCE_SCANS     5U   /* 5 consecutive samples to accept a press = 25 ms debounce */
#define DEBUG_TOGGLE_SCANS      200U   /* both buttons held 200 x 5 ms = 1 s toggles the debug overlay */

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
 * The framebuffer now lives in SDRAM. It used to be a 261 KB array in internal
 * RAM_D1, which consumed half of it. This macro only supplies the initial
 * display address to the CubeMX-generated MX_LTDC_Init(); page flipping is
 * handled by bsp_display.c.
 */
#define FRAMEBUFFER ((uint16_t (*)[LCD_WIDTH])SDRAM_FB0_ADDR)

FDCAN_FilterTypeDef sFilterConfig;
FDCAN_TxHeaderTypeDef TxHeader;
FDCAN_RxHeaderTypeDef RxHeader;

/* Scratch buffer the FDCAN ISR receives into. TTR_CAN_MAX_DLC is 48. */
uint8_t RX[TTR_CAN_MAX_DLC] = {0};

/* Index of the page on screen; ScanButtons() moves it */
uint8_t screen_ID_now = 0;

/*
 * QSPI write-path check, triggered from a debugger.
 *
 * Set g_qspi_run_write_test to 1 in a watch window and the main loop runs
 * BSP_QSPI_SelfTestWrite() once, leaving the outcome in
 * g_qspi_write_test_result: 1 for pass, 0 for fail, -1 for not yet run.
 *
 * Triggered rather than run at boot for two reasons: it erases a sector, so
 * doing it every startup wears the part for no reason, and a chip erase or a
 * failed erase blocking for seconds during boot would look like a hang.
 */
volatile uint8_t g_qspi_run_write_test = 0;
volatile int8_t  g_qspi_write_test_result = -1;

/* CAN IDs the hardware filter admits. Order does not matter; the trailing 0
 * terminates the list. Adding one here also needs a matching case in
 * can_decode.c, otherwise the frame arrives and is silently dropped. */
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
 * Page order. Index 0 is the splash screen shown at boot and is excluded from
 * the button cycle; MIN/MAX_SCR_ID bound what the buttons can reach.
 */
enum ScreensEnum screens[] = {
    SCREEN_ID_WELCOME,   /* 0  splash, boot only */
    SCREEN_ID_MAIN,      /* 1 */
    SCREEN_ID_DEBUG1,    /* 2 */
    SCREEN_ID_DEBUG2,    /* 3 */
    SCREEN_ID_DEBUG3,    /* 4 */
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
   * Reconfigure the MPU and turn on I-cache and D-cache.
   *
   * The MPU_Config() call above is CubeMX-generated and marks
   * 0x60000000..0xDFFFFFFF as no-access, a range that contains the SDRAM at
   * 0xC0000000 - the reason the board's 32 MB of SDRAM was unusable. The call
   * below replaces that configuration entirely.
   */
  BSP_MPU_ConfigAndEnableCache();
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */
  /* SDRAM must be ready before MX_LTDC_Init(): LTDC starts fetching from the
   * framebuffer address the moment it is enabled. */
  BSP_SDRAM_Init();
  if (!BSP_SDRAM_SelfTest())
  {
    Error_Handler();
  }

  /*
   * QSPI flash on the core board. Nothing is stored there yet; it is wired up
   * so the driver can be verified against real hardware - check
   * BSP_QSPI_GetJedecId() and BSP_QSPI_GetFlashSize().
   *
   * The return value is deliberately ignored: this chip is spare capacity for
   * artwork after the UI redesign, the dashboard runs fine without it, and it
   * is not worth blanking the display over.
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

  /* Animations come from QSPI. Does nothing if the part is blank, so a
   * board that has never been programmed still boots normally. */
  GifPages_Init();

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

    if (g_qspi_run_write_test != 0u)
    {
      g_qspi_run_write_test = 0u;
      g_qspi_write_test_result = BSP_QSPI_SelfTestWrite() ? 1 : 0;
    }

    /* Decode CAN before LVGL renders, so this pass draws the newest values. */
    CAN_Poll();

    /*
     * LVGL timers and rendering.
     *
     * This used to be lv_timer_handler() followed by
     * HAL_Delay(time_till_next), which put the whole main loop to sleep for a
     * full refresh period (up to 33 ms). Buttons and data updates were dragged
     * along with it, and widgets touched by updatescreen() had to wait another
     * round to appear - two refresh periods from CAN frame to pixel. The loop
     * no longer sleeps; each task below throttles itself on wall-clock time.
     */
    lv_timer_handler();
    BSP_Display_Service();

    if (!welcome_done && (now >= WELCOME_HOLD_MS))
    {
      welcome_done = true;
      screen_ID_now = 1;
      loadScreen(screens[screen_ID_now]);
      GifPages_SetVisiblePage(screen_ID_now);
      UIBind_ArmStartupSweep();
    }

    /*
     * Let the EEZ-generated screens poll get_var_xxx() once (implemented in
     * ui_bind.c). The old code wrote widgets directly from updatescreen();
     * the new code has EEZ read its bound variables instead, so this line is
     * essential - without it the display sits at its default values and
     * nothing reports an error.
     */
    if ((now - last_ui_update) >= UI_UPDATE_PERIOD_MS)
    {
      last_ui_update = now;
      ui_tick();
      UIBind_ApplyDynamicStyles();
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
 * Read the two page-change buttons.
 *
 * This was two nearly identical blocks, each with its own counter and flag.
 * It is now table-driven: the counter fires once on reaching the threshold and
 * then saturates there until the button is released, which gives the
 * fire-once behaviour for free - no separate b1f / b2f flags needed.
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
   * Handle the both-buttons gesture first and return while it is held, so no
   * page change happens. Pressed within the same sampling window (before the
   * 25 ms debounce elapses) nothing flips at all; slightly staggered presses
   * cost one page change first, which is an acceptable trade.
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
        DebugOverlay_Toggle();   /* fire only on the sample that crosses the threshold */
      }
    }
    return;
  }

  both_counter = 0;

  for (uint8_t i = 0; i < (sizeof(buttons) / sizeof(buttons[0])); i++)
  {
    if (HAL_GPIO_ReadPin(buttons[i].port, buttons[i].pin) != 0)
    {
      buttons[i].counter = 0;   /* released */
      continue;
    }

    if (buttons[i].counter >= BUTTON_DEBOUNCE_SCANS)
    {
      continue;                 /* already flipped for this press; wait for release */
    }

    if (++buttons[i].counter < BUTTON_DEBOUNCE_SCANS)
    {
      continue;                 /* still debouncing */
    }

    int8_t next = (int8_t)screen_ID_now + buttons[i].step;
    if (next < MIN_SCR_ID) next = MAX_SCR_ID;
    if (next > MAX_SCR_ID) next = MIN_SCR_ID;

    screen_ID_now = (uint8_t)next;
    loadScreen(screens[screen_ID_now]);
    GifPages_SetVisiblePage(screen_ID_now);
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
