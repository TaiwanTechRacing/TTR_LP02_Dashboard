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
#include "ui_operate.h"
#include <string.h>
#include <stdio.h>
#include "ttr_can.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LCD_WIDTH 480
#define LCD_HEIGHT 272
#define BYTES_PER_PIXEL 2

#define COLOR_RED 0xF800
#define COLOR_ORANGE 0xFD20
#define COLOR_YELLOW 0xFFE0
#define COLOR_GREEN 0x07E0
#define COLOR_BLUE 0x001F
#define COLOR_INDIGO 0x4810
#define COLOR_VIOLET 0x9011

#define HOLD 70

#define MIN_SCR_ID 1
#define MAX_SCR_ID 8

#define GLV_LOW_VOLT 19
#define MAX_MOTOR_SPEED 45535
#define HV_LOW_VOLT 350

#define HOLD_BUTTON 3
#define WELCOMEDELAY 150

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
uint16_t FRAMEBUFFER[272][480];
static uint8_t buf1[LCD_WIDTH * LCD_HEIGHT / 10 * BYTES_PER_PIXEL];
FDCAN_FilterTypeDef sFilterConfig;
FDCAN_TxHeaderTypeDef TxHeader;
FDCAN_RxHeaderTypeDef RxHeader;

extern objects_t objects;
bool glv_low_volt = 0;
bool hv_low_volt = 0;
bool b1f = 0;
bool b2f = 0;

uint8_t TX[8]={0,1,2,3,4,5,6,7};
uint8_t RX[48]={0};
uint8_t screen_ID_now=0;
uint8_t button1counter=0;
uint8_t button2counter=0;
uint8_t RX_COPY[8] = {0};



//For can update.
volatile static uint8_t drive_mode=0;
volatile static uint8_t vcu_err_type=0;
volatile static uint8_t vcu_warn_type =0;
volatile static uint8_t lat=0;
volatile static uint8_t lng=0;
volatile static uint16_t carSpeedTransmit = 0;
volatile static uint16_t sdcStatus=0;

volatile static float steeringTransmit=0;
volatile static float apps1Transmit=0;
volatile static float bseRearPUTransmit=0;
volatile static float glvVoltTransmit = 0;
volatile static float busVoltage = 0;
volatile static float busSoc = 0;
volatile static float AccMaxTemp = 0;
volatile static float AccMinTemp = 0;
volatile static float AccDiffTemp = 0;
volatile static float CellsVolt[NUM_OF_CELLS] = {0};
volatile static float CellsTemp[NUM_OF_TSENSOR] = {0};
volatile static bool RTD_SIGNAL=0;
volatile static bool COOL_SIGNAL=0;
volatile static bool CELL_OVER_TEMP = 0;
volatile static bool TEBPPC = 0;
volatile static bool AMS_RDY = 0;
volatile static bool sdc_read_test = 0;
uint32_t level;

/*
static uint16_t STD_ID_LIST_CAN[35] = {
		AMS_STATUS_CMD0_ID, VCU_STATUS_CMD_SYSTEM1_ID, VCU_STATUS_CMD_SYSTEM2_ID, VCU_STATUS_CMD_SENSOR1_ID,
		VCU_STATUS_CMD_SENSOR2_ID, VCU_STATUS_CMD_SENSOR3_ID, AMS_VOLTAGE_STATUS_ID_START, 0
};
*/
static uint16_t STD_ID_LIST_CAN[35] = {TTR_CAN_ID_VCU_VCU_STATE, TTR_CAN_ID_VCU_VCU_SDC, TTR_CAN_ID_VCU_VCU_SENSOR1, TTR_CAN_ID_VCU_VCU_SENSOR2 , TTR_CAN_ID_VCU_VCU_SENSOR3, TTR_CAN_ID_AMS_AMS_STATUS0, TTR_CAN_ID_AMS_AMS_MODULE_1, TTR_CAN_ID_AMS_AMS_MODULE_2, TTR_CAN_ID_AMS_AMS_MODULE_3, TTR_CAN_ID_AMS_AMS_MODULE_4, TTR_CAN_ID_AMS_AMS_MODULE_5, TTR_CAN_ID_AMS_AMS_MODULE_6, TTR_CAN_ID_AMS_AMS_MODULE_7, TTR_CAN_ID_AMS_AMS_MODULE_8, TTR_CAN_ID_VCU_VCU_ERROR, TTR_CAN_ID_VCU_VCU_GPS, 0};



/*
volatile static uint16_t rawApps1Angle=0;
volatile static uint16_t rawSteeringAngle=0;
volatile static uint16_t rawBseRearPUTAngle=0;
volatile static uint16_t GLV_V=0;

*/

//Test Test
//Test TEST222222222222
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
void lvgl_init_pack(void);
void my_flush_cb(lv_display_t *, const lv_area_t *, uint8_t*);
enum ScreensEnum screens[]={SCREEN_ID_WELCOME,SCREEN_ID_SPEED,SCREEN_ID_RACING,SCREEN_ID_FACTORY_BAT_SUM,SCREEN_ID_FACTORY_BAT_P1,SCREEN_ID_FACTORY_BAT_P2,SCREEN_ID_FACTORY_BAT_P3,SCREEN_ID_FACTORY_BAT_P4,SCREEN_ID_FACTORY_MOT};
void updatescreen(void);
void writeCellsValue(uint8_t cells);
void PushValueAsFloat(lv_obj_t* target, float value);
void PushValueAsInt(lv_obj_t* target, uint8_t value);
bool CMP_BIN(uint16_t target , uint8_t bit);
void ReadSDCStatus(uint16_t sdc);
void Little_Eendian_Merge(uint8_t StartIndex, volatile float* target);



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
  //卡歡迎頁面的counter與flag

  uint8_t welcome_counter = 0;
  bool welcome_stop = 0;
  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_LTDC_Init();
  MX_DMA2D_Init();
  MX_FDCAN2_Init();
  MX_FDCAN1_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(BL_ENABLE_GPIO_Port, BL_ENABLE_Pin, 1);
  lvgl_init_pack();
  ui_init();

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

  //init racing page

  //把racing page的bar條初始化
  Icon_Set(objects.power_indicator, 0);
  Icon_Set(objects.fans_indicator, 0);
  Icon_Set(objects.pressure_indicator, 0);
  Icon_Set(objects.temp_indicator, 0);

  lv_arc_set_value(objects.speed_process, (int32_t)0);
  lv_arc_set_value(objects.steering_wheel_dir, (int32_t)50);
  lv_arc_set_value(objects.acc_process, (int32_t)0);
  lv_arc_set_value(objects.brake_process, (int32_t)0);
  lv_obj_set_style_opa(objects.tebppc_warn, LV_OPA_0, LV_PART_MAIN | LV_STATE_DEFAULT);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  level = HAL_FDCAN_GetRxFifoFillLevel(
	              &hfdcan2,
	              FDCAN_RX_FIFO0);

	  //歡迎介面的counter
	  if(welcome_counter < WELCOMEDELAY){
		  welcome_counter++;
	  }
	  else{
		  if(welcome_stop == 0){
			  loadScreen(screens[1]);
			  screen_ID_now = 1;
		  }
		  welcome_stop = 1;
	  }

	  //LVGL的timer
	  uint32_t time_till_next = lv_timer_handler();
	  if(time_till_next == LV_NO_TIMER_READY) time_till_next = LV_DEF_REFR_PERIOD; /*handle LV_NO_TIMER_READY. Another option is to `sleep` for longer*/
	  HAL_Delay(time_till_next);


	  //刷新螢幕物件
	  updatescreen();



	  //讀取螢幕按鍵狀態

	  if (HAL_GPIO_ReadPin(BUTTON_1_GPIO_Port, BUTTON_1_Pin) == 0 ){
		  button1counter++;

		  if(button1counter >= HOLD_BUTTON && b1f == 0 ){
			  b1f=1;
			  if(screen_ID_now > MIN_SCR_ID){
				  screen_ID_now--;
			  }
			  else{
				  screen_ID_now=MAX_SCR_ID;
			  }
			  loadScreen(screens[screen_ID_now]);
			  button1counter=0;
		  }


	  }
	  else{
		  button1counter = 0;
		  b1f=0;
	  }

	  if (HAL_GPIO_ReadPin(BUTTON_2_GPIO_Port, BUTTON_2_Pin) == 0 ){
		button2counter++;
		if(button2counter >= HOLD_BUTTON && b2f == 0){
			b2f=1;
		  	if(screen_ID_now < MAX_SCR_ID){
		  		screen_ID_now++;
		  	}
		  	else{
		  		screen_ID_now=MIN_SCR_ID;
		  	}
		  	loadScreen(screens[screen_ID_now]);
		  	button2counter = 0;
		}


	  }
	  else{
		  button2counter = 0;
		  b2f=0;
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

void lvgl_init_pack(void){
	lv_init();
	lv_tick_set_cb(HAL_GetTick);
	lv_display_t * display1 = lv_display_create(LCD_WIDTH, LCD_HEIGHT);
	lv_display_set_buffers(display1, buf1, NULL, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
	lv_display_set_flush_cb(display1, my_flush_cb);
}


void my_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    // 計算區域參數
    int32_t x1 = area->x1;
    int32_t y1 = area->y1;
    int32_t x2 = area->x2;
    int32_t y2 = area->y2;

    int32_t width = x2 - x1 + 1;
    int32_t height = y2 - y1 + 1;

    // 取得 Framebuffer 基底地址
    uint16_t *fb = (uint16_t *)FRAMEBUFFER;  // 使用您定義的 FRAMEBUFFER

    // 計算目標位置
    uint16_t *dest = fb + (y1 * LCD_WIDTH + x1);
    uint16_t *src = (uint16_t *)px_map;

    // 使用 DMA2D 進行記憶體到記憶體傳輸
    DMA2D->CR &= ~DMA2D_CR_START;              // 停止 DMA2D
    DMA2D->CR = DMA2D_M2M;                     // Memory to Memory 模式

    // 設定來源地址 (LVGL buffer)
    DMA2D->FGMAR = (uint32_t)src;

    // 設定目標地址 (Framebuffer)
    DMA2D->OMAR = (uint32_t)dest;

    // 設定前景層偏移 (連續資料，無偏移)
    DMA2D->FGOR = 0;

    // 設定輸出偏移 (每行結束後需跳過的像素數)
    DMA2D->OOR = LCD_WIDTH - width;

    // 設定像素格式為 RGB565
    DMA2D->FGPFCCR = DMA2D_INPUT_RGB565;
    DMA2D->OPFCCR = DMA2D_OUTPUT_RGB565;

    // 設定傳輸的行數和每行像素數
    DMA2D->NLR = (uint32_t)(width << DMA2D_NLR_PL_Pos) |
                 (uint32_t)(height << DMA2D_NLR_NL_Pos);

    // 啟動 DMA2D 傳輸
    DMA2D->CR |= DMA2D_CR_START;

    // 等待 DMA2D 完成 (輪詢方式)
    uint32_t timeout = 0;
    while (DMA2D->CR & DMA2D_CR_START) {
        timeout++;
        if (timeout > 0x1FFFFF) {
            // 超時處理
            break;
        }
    }

    // 清除傳輸完成標誌
    DMA2D->IFCR = DMA2D_IFCR_CTCIF;

    // 通知 LVGL 刷新完成 (LVGL v9.x API)
    lv_display_flush_ready(display);
}


void CAN_ProcessMsg(){

    // 1. 將硬體的 CAN Frame 格式，轉換成 ttr_can_frame_t 格式
    ttr_can_frame_t frame;
    frame.id = RxHeader.Identifier;
    frame.dlc = RxHeader.DataLength; // 或者是你的硬體驅動定義的長度變數（如 RxHeader.DataLength）


    // 假設標準 CAN 最大 8 byte，如果是 CAN FD 可以依硬體實際長度複製
    for(int i = 0; i < TTR_CAN_MAX_DLC; i++) {
        frame.data[i] = RX[i];
    }

    // 2. 依據 ID 進行 Switch 判斷與解包
    switch(frame.id){
		case TTR_CAN_ID_VCU_VCU_STATE: {
			ttr_vcu_vcu_state_t vcu_st;
			ttr_vcu_vcu_state_unpack(&vcu_st, &frame);
			RTD_SIGNAL = vcu_st.RDY_TO_DRIVE_ACTIVE;
			COOL_SIGNAL = vcu_st.COOLING_SYSTEM_ACTIVE;
			drive_mode = vcu_st.SYS_DRIVE_MODE;
			TEBPPC = vcu_st.TEBPPC_ACTIVE;
			AMS_RDY = vcu_st.AMS_RDY;
			break;
		}
		case TTR_CAN_ID_VCU_VCU_SDC: { // 替換原本的 VCU_STATUS_CMD_SYSTEM1_ID
			ttr_vcu_vcu_sdc_t sdc;
			ttr_vcu_vcu_sdc_unpack(&sdc, &frame);
			sdc_read_test = sdc.CSB_STATUS;
				// 直接取得實體數值，不再需要手動用 & 遮罩和移位
			sdcStatus = (sdcStatus & ~(1U << 4)) | ((sdc.CSB_STATUS & 1U) << 4);
			sdcStatus = (sdcStatus & ~(1U << 5)) | ((sdc.LSB_STATUS & 1U) << 5);
			sdcStatus = (sdcStatus & ~(1U << 6)) | ((sdc.RSB_STATUS & 1U) << 6);
			sdcStatus = (sdcStatus & ~(1U << 7)) | ((sdc.INRT_STATUS & 1U) << 7);
			sdcStatus = (sdcStatus & ~(1U << 8)) | ((sdc.BOTS_STATUS & 1U) << 8);
			sdcStatus = (sdcStatus & ~(1U << 9)) | ((sdc.MCU_IL_STATUS & 1U) << 9);
			sdcStatus = (sdcStatus & ~(1U << 10)) | ((sdc.M1_IL_STATUS & 1U) << 10);
			sdcStatus = (sdcStatus & ~(1U << 11)) | ((sdc.M2_IL_STATUS & 1U) << 11);
			sdcStatus = (sdcStatus & ~(1U << 12)) | ((sdc.M3_IL_STATUS & 1U) << 12);
			sdcStatus = (sdcStatus & ~(1U << 13)) | ((sdc.M4_IL_STATUS & 1U) << 13);
			sdcStatus = (sdcStatus & ~(1U << 14)) | ((sdc.TSMS_STATUS & 1U) << 14);
			sdcStatus = (sdcStatus & ~(1U << 15)) | ((sdc.MSD_STATUS & 1U) << 15);
			break;
		}

		case TTR_CAN_ID_VCU_VCU_SENSOR1: { // 替換原本的 VCU_STATUS_CMD_SENSOR1_ID
			ttr_vcu_vcu_sensor1_t sensor1;
			ttr_vcu_vcu_sensor1_unpack(&sensor1, &frame);
			bseRearPUTransmit = sensor1.BSE_REAR_PU;

			break;
		}

		case TTR_CAN_ID_VCU_VCU_SENSOR2: { // 替換原本的 VCU_STATUS_CMD_SENSOR2_ID
			ttr_vcu_vcu_sensor2_t sensor2;
			ttr_vcu_vcu_sensor2_unpack(&sensor2, &frame);
			steeringTransmit = (100- (sensor2.STEERING_ANGLE+180.0f) / 360.0f * 100);
			apps1Transmit    = sensor2.APPS1_PU;
			carSpeedTransmit = sensor2.CAR_SPEED;
			break;
		}

		case TTR_CAN_ID_VCU_VCU_SENSOR3: { // 替換原本的 VCU_STATUS_CMD_SENSOR3_ID
			ttr_vcu_vcu_sensor3_t sensor3;
			ttr_vcu_vcu_sensor3_unpack(&sensor3, &frame);
			 // 解包函式會自動幫你除以 2185.0f，你直接拿來用就好！
			glvVoltTransmit = sensor3.GLV_VOLTAGE; // 欄位名稱請對照 ttr_can.h 內的定義
			break;
		}

		case TTR_CAN_ID_VCU_VCU_ERROR:{
			ttr_vcu_vcu_error_t vcu_err;
			ttr_vcu_vcu_error_unpack(&vcu_err, &frame);
			vcu_err_type = (vcu_err.MCU1_ERR) | (vcu_err.MCU2_ERR<<1) | (vcu_err.MCU3_ERR<<2) | (vcu_err.MCU4_ERR<<3) | (vcu_err.AMS_ERR<<4);
			break;
		}

		case TTR_CAN_ID_AMS_AMS_STATUS0: { // 替換原本的 AMS_STATUS_CMD0_ID
			ttr_ams_ams_status0_t ams_status0;
			ttr_ams_ams_status0_unpack(&ams_status0, &frame);
			busVoltage = ams_status0.PACK_VOLTAGE;
			busSoc = ams_status0.PACK_SOC;
			AccMaxTemp = ams_status0.TEMPERATURE_MAX;
			AccMinTemp = ams_status0.TEMPERATURE_MIN;
			AccDiffTemp = ams_status0.TEMPERATURE_DELTA;
			CELL_OVER_TEMP = ams_status0.CELL_OVER_TEMP_ERR;
			break;
		}

		case TTR_CAN_ID_VCU_VCU_GPS:{
			ttr_vcu_vcu_gps_t vcu_gps;
			ttr_vcu_vcu_gps_unpack(&vcu_gps, &frame);
			lng = vcu_gps.LONGTITUDE;
			lat = vcu_gps.LATITUDE;
			break;
		}

    case TTR_CAN_ID_AMS_AMS_MODULE_1:{
    	ttr_ams_ams_module_1_t m1;
    	ttr_ams_ams_module_1_unpack(&m1, &frame);

    	CellsVolt[0*14+0]=m1.C0;
    	CellsVolt[0*14+1]=m1.C1;
    	CellsVolt[0*14+2]=m1.C2;
    	CellsVolt[0*14+3]=m1.C3;
    	CellsVolt[0*14+4]=m1.C4;
    	CellsVolt[0*14+5]=m1.C5;
    	CellsVolt[0*14+6]=m1.C6;
    	CellsVolt[0*14+7]=m1.C7;
    	CellsVolt[0*14+8]=m1.C8;
    	CellsVolt[0*14+9]=m1.C9;
    	CellsVolt[0*14+10]=m1.C10;
    	CellsVolt[0*14+11]=m1.C11;
    	CellsVolt[0*14+12]=m1.C12;
    	CellsVolt[0*14+13]=m1.C13;

    	CellsTemp[0*10+0] = m1.T0;
    	CellsTemp[0*10+1] = m1.T1;
    	CellsTemp[0*10+2] = m1.T2;
    	CellsTemp[0*10+3] = m1.T3;
    	CellsTemp[0*10+4] = m1.T4;
    	CellsTemp[0*10+5] = m1.T5;
    	CellsTemp[0*10+6] = m1.T6;
    	break;
    }
    case TTR_CAN_ID_AMS_AMS_MODULE_2:{
    	ttr_ams_ams_module_2_t m2;
    	ttr_ams_ams_module_2_unpack(&m2, &frame);
    	CellsVolt[1*14+0]=m2.C0;
    	CellsVolt[1*14+1]=m2.C1;
    	CellsVolt[1*14+2]=m2.C2;
    	CellsVolt[1*14+3]=m2.C3;
    	CellsVolt[1*14+4]=m2.C4;
    	CellsVolt[1*14+5]=m2.C5;
    	CellsVolt[1*14+6]=m2.C6;
    	CellsVolt[1*14+7]=m2.C7;
    	CellsVolt[1*14+8]=m2.C8;
    	CellsVolt[1*14+9]=m2.C9;
    	CellsVolt[1*14+10]=m2.C10;
    	CellsVolt[1*14+11]=m2.C11;
    	CellsVolt[1*14+12]=m2.C12;
    	CellsVolt[1*14+13]=m2.C13;

    	CellsTemp[1*10+0] = m2.T0;
    	CellsTemp[1*10+1] = m2.T1;
    	CellsTemp[1*10+2] = m2.T2;
    	CellsTemp[1*10+3] = m2.T3;
    	CellsTemp[1*10+4] = m2.T4;
    	CellsTemp[1*10+5] = m2.T5;
    	CellsTemp[1*10+6] = m2.T6;
    	break;
    }
    case TTR_CAN_ID_AMS_AMS_MODULE_3:{
    	ttr_ams_ams_module_3_t m3;
    	ttr_ams_ams_module_3_unpack(&m3, &frame);
    	CellsVolt[2*14+0]=m3.C0;
    	CellsVolt[2*14+1]=m3.C1;
    	CellsVolt[2*14+2]=m3.C2;
    	CellsVolt[2*14+3]=m3.C3;
    	CellsVolt[2*14+4]=m3.C4;
    	CellsVolt[2*14+5]=m3.C5;
    	CellsVolt[2*14+6]=m3.C6;
    	CellsVolt[2*14+7]=m3.C7;
    	CellsVolt[2*14+8]=m3.C8;
    	CellsVolt[2*14+9]=m3.C9;
    	CellsVolt[2*14+10]=m3.C10;
    	CellsVolt[2*14+11]=m3.C11;
    	CellsVolt[2*14+12]=m3.C12;
    	CellsVolt[2*14+13]=m3.C13;

    	CellsTemp[2*10+0] = m3.T0;
    	CellsTemp[2*10+1] = m3.T1;
    	CellsTemp[2*10+2] = m3.T2;
    	CellsTemp[2*10+3] = m3.T3;
    	CellsTemp[2*10+4] = m3.T4;
    	CellsTemp[2*10+5] = m3.T5;
    	CellsTemp[2*10+6] = m3.T6;
    	break;
    }
    case TTR_CAN_ID_AMS_AMS_MODULE_4:{
    	ttr_ams_ams_module_4_t m4;
    	ttr_ams_ams_module_4_unpack(&m4, &frame);
    	CellsVolt[3*14+0]=m4.C0;
    	CellsVolt[3*14+1]=m4.C1;
    	CellsVolt[3*14+2]=m4.C2;
    	CellsVolt[3*14+3]=m4.C3;
    	CellsVolt[3*14+4]=m4.C4;
    	CellsVolt[3*14+5]=m4.C5;
    	CellsVolt[3*14+6]=m4.C6;
    	CellsVolt[3*14+7]=m4.C7;
    	CellsVolt[3*14+8]=m4.C8;
    	CellsVolt[3*14+9]=m4.C9;
    	CellsVolt[3*14+10]=m4.C10;
    	CellsVolt[3*14+11]=m4.C11;
    	CellsVolt[3*14+12]=m4.C12;
    	CellsVolt[3*14+13]=m4.C13;

    	CellsTemp[3*10+0] = m4.T0;
    	CellsTemp[3*10+1] = m4.T1;
    	CellsTemp[3*10+2] = m4.T2;
    	CellsTemp[3*10+3] = m4.T3;
    	CellsTemp[3*10+4] = m4.T4;
    	CellsTemp[3*10+5] = m4.T5;
    	CellsTemp[3*10+6] = m4.T6;
    	break;

    }
    case TTR_CAN_ID_AMS_AMS_MODULE_5:{
    	ttr_ams_ams_module_5_t m5;
    	ttr_ams_ams_module_5_unpack(&m5, &frame);
    	CellsVolt[4*14+0]=m5.C0;
    	CellsVolt[4*14+1]=m5.C1;
    	CellsVolt[4*14+2]=m5.C2;
    	CellsVolt[4*14+3]=m5.C3;
    	CellsVolt[4*14+4]=m5.C4;
    	CellsVolt[4*14+5]=m5.C5;
    	CellsVolt[4*14+6]=m5.C6;
    	CellsVolt[4*14+7]=m5.C7;
    	CellsVolt[4*14+8]=m5.C8;
    	CellsVolt[4*14+9]=m5.C9;
    	CellsVolt[4*14+10]=m5.C10;
    	CellsVolt[4*14+11]=m5.C11;
    	CellsVolt[4*14+12]=m5.C12;
    	CellsVolt[4*14+13]=m5.C13;

    	CellsTemp[4*10+0] = m5.T0;
    	CellsTemp[4*10+1] = m5.T1;
    	CellsTemp[4*10+2] = m5.T2;
    	CellsTemp[4*10+3] = m5.T3;
    	CellsTemp[4*10+4] = m5.T4;
    	CellsTemp[4*10+5] = m5.T5;
    	CellsTemp[4*10+6] = m5.T6;
    	break;
    }
    case TTR_CAN_ID_AMS_AMS_MODULE_6:{
    	ttr_ams_ams_module_6_t m6;
    	ttr_ams_ams_module_6_unpack(&m6, &frame);
    	CellsVolt[5*14+0]=m6.C0;
    	CellsVolt[5*14+1]=m6.C1;
    	CellsVolt[5*14+2]=m6.C2;
    	CellsVolt[5*14+3]=m6.C3;
    	CellsVolt[5*14+4]=m6.C4;
    	CellsVolt[5*14+5]=m6.C5;
    	CellsVolt[5*14+6]=m6.C6;
    	CellsVolt[5*14+7]=m6.C7;
    	CellsVolt[5*14+8]=m6.C8;
    	CellsVolt[5*14+9]=m6.C9;
    	CellsVolt[5*14+10]=m6.C10;
    	CellsVolt[5*14+11]=m6.C11;
    	CellsVolt[5*14+12]=m6.C12;
    	CellsVolt[5*14+13]=m6.C13;

    	CellsTemp[5*10+0] = m6.T0;
    	CellsTemp[5*10+1] = m6.T1;
    	CellsTemp[5*10+2] = m6.T2;
    	CellsTemp[5*10+3] = m6.T3;
    	CellsTemp[5*10+4] = m6.T4;
    	CellsTemp[5*10+5] = m6.T5;
    	CellsTemp[5*10+6] = m6.T6;
    	break;
    }
    case TTR_CAN_ID_AMS_AMS_MODULE_7:{
    	ttr_ams_ams_module_7_t m7;
    	ttr_ams_ams_module_7_unpack(&m7, &frame);
    	CellsVolt[6*14+0]=m7.C0;
    	CellsVolt[6*14+1]=m7.C1;
    	CellsVolt[6*14+2]=m7.C2;
    	CellsVolt[6*14+3]=m7.C3;
    	CellsVolt[6*14+4]=m7.C4;
    	CellsVolt[6*14+5]=m7.C5;
    	CellsVolt[6*14+6]=m7.C6;
    	CellsVolt[6*14+7]=m7.C7;
    	CellsVolt[6*14+8]=m7.C8;
    	CellsVolt[6*14+9]=m7.C9;
    	CellsVolt[6*14+10]=m7.C10;
    	CellsVolt[6*14+11]=m7.C11;
    	CellsVolt[6*14+12]=m7.C12;
    	CellsVolt[6*14+13]=m7.C13;

    	CellsTemp[6*10+0] = m7.T0;
    	CellsTemp[6*10+1] = m7.T1;
    	CellsTemp[6*10+2] = m7.T2;
    	CellsTemp[6*10+3] = m7.T3;
    	CellsTemp[6*10+4] = m7.T4;
    	CellsTemp[6*10+5] = m7.T5;
    	CellsTemp[6*10+6] = m7.T6;
    	break;
    }
    case TTR_CAN_ID_AMS_AMS_MODULE_8:{
    	ttr_ams_ams_module_8_t m8;
    	ttr_ams_ams_module_8_unpack(&m8, &frame);
    	CellsVolt[7*14+0]=m8.C0;
    	CellsVolt[7*14+1]=m8.C1;
    	CellsVolt[7*14+2]=m8.C2;
    	CellsVolt[7*14+3]=m8.C3;
    	CellsVolt[7*14+4]=m8.C4;
    	CellsVolt[7*14+5]=m8.C5;
    	CellsVolt[7*14+6]=m8.C6;
    	CellsVolt[7*14+7]=m8.C7;
    	CellsVolt[7*14+8]=m8.C8;
    	CellsVolt[7*14+9]=m8.C9;
    	CellsVolt[7*14+10]=m8.C10;
    	CellsVolt[7*14+11]=m8.C11;
    	CellsVolt[7*14+12]=m8.C12;
    	CellsVolt[7*14+13]=m8.C13;

    	CellsTemp[7*10+0] = m8.T0;
    	CellsTemp[7*10+1] = m8.T1;
    	CellsTemp[7*10+2] = m8.T2;
    	CellsTemp[7*10+3] = m8.T3;
    	CellsTemp[7*10+4] = m8.T4;
    	CellsTemp[7*10+5] = m8.T5;
    	CellsTemp[7*10+6] = m8.T6;
    	break;
    }

    default:
        // 對於下方大範圍的電池電壓（CellsVolt），如果 DBC 也有定義，也可以用同樣方法解包；
        // 如果 DBC 裡面沒有包含這段 Cell 輪詢的 ID，可以先保留原本的處理邏輯。


        break;
    }
}

void Little_Eendian_Merge(uint8_t StartIndex,volatile float* target){
	*target = ((uint16_t)RX[StartIndex] | ((uint16_t)RX[StartIndex+1]<<8));
}

//HAL FDCAN call back
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
  if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
  {
    if(hfdcan->Instance == FDCAN2)
    {
      /* Retrieve Rx messages from RX FIFO0 */
  	  if(HAL_FDCAN_GetRxFifoFillLevel(&hfdcan2, FDCAN_RX_FIFO0) > 0 ){
  		if(HAL_FDCAN_GetRxMessage(&hfdcan2, FDCAN_RX_FIFO0, &RxHeader, RX) == HAL_OK ){
  	    	CAN_ProcessMsg();
  	    }
  	    else{
  	      Error_Handler();

  	    }
  	  }
    }
  }
}

bool CMP_BIN(uint16_t target , uint8_t bit){
	return (target & (1U << bit)) != 0;
}

void ReadSDCStatus(uint16_t sdc){
	lv_obj_t* SDCUIitem[]={objects.ios_0, objects.ios__2, objects.ios__1, objects.ios__4,objects.ios__3,objects.ios__7, objects.ios__8, objects.ios__9, objects.ios__10, objects.ios__11, objects.ios__5, objects.ios__6};

	for(int i = 0 ; i < (sizeof(SDCUIitem)/sizeof(SDCUIitem[0])) ; i++){

		uint8_t brightness = CMP_BIN( sdc, i+4 ) ? 255 : 0;
		lv_led_set_brightness(SDCUIitem[i], brightness);
	}
}

void updatescreen(void){
	static char buffer[10]={0};
	static char buf[128];
	static uint8_t warn_counter=0;

	switch(screen_ID_now){
	case 1:
		//speed
		snprintf(buffer,sizeof(buffer), "%03u", carSpeedTransmit); //2. monitor this variable.
		lv_label_set_text(objects.speed_pure, buffer);
		//drive mode
		switch(drive_mode){
			case 0:
				lv_label_set_text(objects.drive_mode_pure, "OFF");
				break;
			case 1:
				lv_label_set_text(objects.drive_mode_pure, "E-DIFF");
				break;
			case 2:
				lv_label_set_text(objects.drive_mode_pure, "RATIO");
				break;
			case 3:
				lv_label_set_text(objects.drive_mode_pure, "DYC");
				break;

		}
		//RTD
		if(RTD_SIGNAL != 0){
			lv_label_set_text(objects.rtd_pure, "D");
			lv_obj_set_style_text_color(objects.rtd, lv_color_make(0x00, 0xe8, 0xFF), 0);
		}
		else{
			lv_label_set_text(objects.rtd_pure, "P");
			lv_obj_set_style_text_color(objects.rtd, lv_color_make(0xFF, 0x00, 0x00), 0);
		}

		//ERR-code
		//vcu_err_type = 31; //test-remove-before-racing

		if (vcu_err_type == 0) {
		    lv_label_set_text(objects.error_pure, "");
		}
		else{
			// 用 static 確保記憶體在函式結束後不會被釋放
			buf[0] = '\0'; // 清空緩衝區
			strcat(buf, "ERR - ");
			// 依序檢查每個 Bit 並拼接字串
			if (vcu_err_type & (1 << 4)) strcat(buf, "AMS ");
			if (vcu_err_type & (1 << 3)) strcat(buf, "MCU4 ");
			if (vcu_err_type & (1 << 2)) strcat(buf, "MCU3 ");
			if (vcu_err_type & (1 << 1)) strcat(buf, "MCU2 ");
			if (vcu_err_type & (1 << 0)) strcat(buf, "MCU1 ");
			lv_label_set_text(objects.error_pure, buf);
		}

		//WARN-CODE
		vcu_warn_type = (TEBPPC) | (CELL_OVER_TEMP<<1);

		if(vcu_warn_type == 1){
			if(warn_counter%5==0){
				lv_obj_set_style_opa(objects.tebppc_warn, LV_OPA_100, LV_PART_MAIN | LV_STATE_DEFAULT);
			}
			else{
				lv_obj_set_style_opa(objects.tebppc_warn, LV_OPA_0, LV_PART_MAIN | LV_STATE_DEFAULT);
			}


			warn_counter++;


		}
		else{
			lv_obj_set_style_opa(objects.tebppc_warn, LV_OPA_0, LV_PART_MAIN | LV_STATE_DEFAULT);
			warn_counter=0;
		}

		//vcu_warn_type = 3; //test-remove-before-racing
		if (vcu_warn_type == 0) {
			lv_label_set_text(objects.error_pure_1, "");
		}
		else{
			// 用 static 確保記憶體在函式結束後不會被釋放
			buf[0] = '\0'; // 清空緩衝區
			strcat(buf, "WARN - ");
			// 依序檢查每個 Bit 並拼接字串
			if (vcu_warn_type & (1 << 1)) strcat(buf, "CELL_OVT");
			if (vcu_warn_type & (1 << 0)) strcat(buf, "TEBPPC ");
			lv_label_set_text(objects.error_pure_1, buf);
		}

		break;

	case 2:

		//RACING PAGE ID:2
		ReadSDCStatus(sdcStatus);
		if(drive_mode == 0){
			lv_label_set_text(objects.drive_mode, "OFF");
		}
		else if(drive_mode == 1){
			lv_label_set_text(objects.drive_mode, "E-DIFF");
		}
		else{
			lv_label_set_text(objects.drive_mode, "DYC");
		}


		if(RTD_SIGNAL != 0){
			lv_label_set_text(objects.rtd, "D");
			lv_obj_set_style_text_color(objects.rtd, lv_color_make(0x00, 0xe8, 0xFF), 0);
		}
		else{
			lv_label_set_text(objects.rtd, "P");
			lv_obj_set_style_text_color(objects.rtd, lv_color_make(0xFF, 0x00, 0x00), 0);
		}

		if(COOL_SIGNAL == 0){
			lv_obj_set_style_opa(objects.temp_indicator, LV_OPA_100, LV_PART_MAIN | LV_STATE_DEFAULT);
		}
		else{
			lv_obj_set_style_opa(objects.temp_indicator, LV_OPA_0, LV_PART_MAIN | LV_STATE_DEFAULT);
		}


		if(COOL_SIGNAL == 0){
			lv_obj_set_style_opa(objects.fans_indicator, LV_OPA_100, LV_PART_MAIN | LV_STATE_DEFAULT);

		}
		else{
			lv_obj_set_style_opa(objects.fans_indicator, LV_OPA_0, LV_PART_MAIN | LV_STATE_DEFAULT);
		}


		glv_low_volt = (glvVoltTransmit < GLV_LOW_VOLT ) ? 1:0;
		hv_low_volt = (busVoltage < HV_LOW_VOLT ) ? 1:0;

		if(hv_low_volt == 1 || glv_low_volt == 1){
			lv_obj_set_style_opa(objects.power_indicator, LV_OPA_100, LV_PART_MAIN | LV_STATE_DEFAULT);
		}
		else{
			lv_obj_set_style_opa(objects.power_indicator, LV_OPA_0, LV_PART_MAIN | LV_STATE_DEFAULT);
		}

		snprintf(buffer,sizeof(buffer), "%03u", carSpeedTransmit); //2. monitor this variable.
		lv_label_set_text(objects.speed_r, buffer);

		PushValueAsFloat(objects.glv_volt_r, glvVoltTransmit);

		PushValueAsFloat(objects.hv_volt_r, busVoltage);
		PushValueAsFloat(objects.hv_soc_r, busSoc);

		lv_arc_set_value(objects.speed_process, (int32_t)carSpeedTransmit);
		lv_arc_set_value(objects.steering_wheel_dir, (int32_t)steeringTransmit);
		lv_arc_set_value(objects.acc_process, (int32_t)(apps1Transmit+0.5f));
		lv_arc_set_value(objects.brake_process, (int32_t)(bseRearPUTransmit+0.5f));

	  break;

	case 3: //BAT_SUM
		//GLV volt
		PushValueAsFloat(objects.glv_v, glvVoltTransmit);

		//GLV SOC...
		snprintf(buffer,sizeof(buffer), "%03u", 0);
		lv_label_set_text(objects.glv_soc, buffer);

		//ACC volt
		PushValueAsFloat(objects.acc_volt, busVoltage);

		//ACC SOC
		PushValueAsFloat(objects.acc_soc, busSoc);

		//Max temp
		PushValueAsFloat(objects.acc_max_temp, AccMaxTemp);

		//Min temp
		PushValueAsFloat(objects.acc_min_temp, AccMinTemp);

		//Diff temp
		PushValueAsFloat(objects.acc_diff_temp, AccDiffTemp);


		break;

	case 4:
		//Cell 1,2 volts
		writeCellsValue(1);
		writeCellsValue(2);
		//Cell 1,2 temps

		break;

	case 5:
		//cell 3,4 volts
		writeCellsValue(3);
		writeCellsValue(4);
		//cell 3,4 temps
		break;

	case 6:
		//cell 5,6 volts
		writeCellsValue(5);
		writeCellsValue(6);
		//cell 5,6 temps
		break;

	case 7:
		//cell 7,8 volts
		writeCellsValue(7);
		writeCellsValue(8);
		//cell 7,8 temps
		break;

	default:
	  break;
	}

}

void PushValueAsFloat(lv_obj_t* target, float value){
	char buffer[16]={0};

	snprintf(buffer,sizeof(buffer), "%.1f", value);
	lv_label_set_text(target, buffer);
}

void PushValueAsInt(lv_obj_t* target, uint8_t value){
	static char buffer[4]={0};

	snprintf(buffer,sizeof(buffer), "%03u", value);
	lv_label_set_text(target, buffer);
}

void writeCellsValue(uint8_t cells){

	switch(cells){
	case 1:
		//Cell 1
		lv_obj_t* cell1_volts[]={objects.c1v1,objects.c1v2,objects.c1v3,objects.c1v4,objects.c1v5,objects.c1v6,objects.c1v7,objects.c1v8,objects.c1v9,objects.c1v10,objects.c1v11,objects.c1v12,objects.c1v13,objects.c1v14};
		lv_obj_t* cell1_temps[]={objects.c1t1,objects.c1t2,objects.c1t3,objects.c1t4,objects.c1t5,objects.c1t6,objects.c1tu,objects.c1tl,objects.c1td,objects.c1ta};
		for(uint8_t i=0 ; i<NUM_OF_CELLS/TOTAL_SEG ; i++ ){
			PushValueAsFloat(cell1_volts[i], CellsVolt[i]);
			PushValueAsFloat(cell1_temps[i], CellsTemp[i]);
		}

		break;

	case 2:
		//Cell 2
		lv_obj_t* cell2_volts[]={objects.c2v1,objects.c2v2,objects.c2v3,objects.c2v4,objects.c2v5,objects.c2v6,objects.c2v7,objects.c2v8,objects.c2v9,objects.c2v10,objects.c2v11,objects.c2v12,objects.c2v13,objects.c2v14};
		lv_obj_t* cell2_temps[]={objects.c2t1,objects.c2t2,objects.c2t3,objects.c2t4,objects.c2t5,objects.c2t6,objects.c2tu,objects.c2tl,objects.c2td,objects.c2ta};
		for(uint8_t i=0 ; i<NUM_OF_CELLS/TOTAL_SEG ; i++ ){
			PushValueAsFloat(cell2_volts[i], CellsVolt[i + (NUM_OF_CELLS/TOTAL_SEG) ]);
			PushValueAsFloat(cell2_temps[i], CellsTemp[i + (NUM_OF_TSENSOR/TOTAL_SEG)]);
		}
		break;

	case 3:
		//Cell 3

		lv_obj_t* cell3_volts[]={objects.c1v1_1,objects.c1v2_1,objects.c1v3_1,objects.c1v4_1,objects.c1v5_1,objects.c1v6_1,objects.c1v7_1,objects.c1v8_1,objects.c1v9_1,objects.c1v10_1,objects.c1v11_1,objects.c1v12_1,objects.c1v13_1,objects.c1v14_1};
		lv_obj_t* cell3_temps[]={objects.c1t1_1,objects.c1t2_1,objects.c1t3_1,objects.c1t4_1,objects.c1t5_1,objects.c1t6_1,objects.c1tu_1,objects.c1tl_1,objects.c1td_1,objects.c1ta_1};
		for(uint8_t i=0 ; i<NUM_OF_CELLS/TOTAL_SEG ; i++ ){
			PushValueAsFloat(cell3_volts[i], CellsVolt[i + 2*(NUM_OF_CELLS/TOTAL_SEG) ]);
			PushValueAsFloat(cell3_temps[i], CellsTemp[i + 2*(NUM_OF_TSENSOR/TOTAL_SEG)]);
		}
		break;

	case 4:
		//Cell 4
		lv_obj_t* cell4_volts[]={objects.c2v1_1,objects.c2v2_1,objects.c2v3_1,objects.c2v4_1,objects.c2v5_1,objects.c2v6_1,objects.c2v7_1,objects.c2v8_1,objects.c2v9_1,objects.c2v10_1,objects.c2v11_1,objects.c2v12_1,objects.c2v13_1,objects.c2v14_1};
		lv_obj_t* cell4_temps[]={objects.c2t1_1,objects.c2t2_1,objects.c2t3_1,objects.c2t4_1,objects.c2t5_1,objects.c2t6_1,objects.c2tu_1,objects.c2tl_1,objects.c2td_1,objects.c2ta_1};
		for(uint8_t i=0 ; i<NUM_OF_CELLS/TOTAL_SEG ; i++ ){
			PushValueAsFloat(cell4_volts[i], CellsVolt[i + 3*(NUM_OF_CELLS/TOTAL_SEG) ]);
			PushValueAsFloat(cell4_temps[i], CellsTemp[i + 3*(NUM_OF_TSENSOR/TOTAL_SEG)]);
		}
		break;

	case 5:
		//Cell 5
		lv_obj_t* cell5_volts[]={objects.c1v1_2,objects.c1v2_2,objects.c1v3_2,objects.c1v4_2,objects.c1v5_2,objects.c1v6_2,objects.c1v7_2,objects.c1v8_2,objects.c1v9_2,objects.c1v10_2,objects.c1v11_2,objects.c1v12_2,objects.c1v13_2,objects.c1v14_2};
		lv_obj_t* cell5_temps[]={objects.c1t1_2,objects.c1t2_2,objects.c1t3_2,objects.c1t4_2,objects.c1t5_2,objects.c1t6_2,objects.c1tu_2,objects.c1tl_2,objects.c1td_2,objects.c1ta_2};
		for(uint8_t i=0 ; i<NUM_OF_CELLS/TOTAL_SEG ; i++ ){
			PushValueAsFloat(cell5_volts[i], CellsVolt[i+ 4*(NUM_OF_CELLS/TOTAL_SEG) ]);
			PushValueAsFloat(cell5_temps[i], CellsTemp[i + 4*(NUM_OF_TSENSOR/TOTAL_SEG)]);
		}
		break;

	case 6:
		//Cell	6
		lv_obj_t* cell6_volts[]={objects.c2v1_2,objects.c2v2_2,objects.c2v3_2,objects.c2v4_2,objects.c2v5_2,objects.c2v6_2,objects.c2v7_2,objects.c2v8_2,objects.c2v9_2,objects.c2v10_2,objects.c2v11_2,objects.c2v12_2,objects.c2v13_2,objects.c2v14_2};
		lv_obj_t* cell6_temps[]={objects.c2t1_2,objects.c2t2_2,objects.c2t3_2,objects.c2t4_2,objects.c2t5_2,objects.c2t6_2,objects.c2tu_2,objects.c2tl_2,objects.c2td_2,objects.c2ta_2};
		for(uint8_t i=0 ; i<NUM_OF_CELLS/TOTAL_SEG ; i++ ){
			PushValueAsFloat(cell6_volts[i], CellsVolt[i+ 5*(NUM_OF_CELLS/TOTAL_SEG) ]);
			PushValueAsFloat(cell6_temps[i], CellsTemp[i + 5*(NUM_OF_TSENSOR/TOTAL_SEG)]);
		}
		break;

	case 7:
		//Cell 7
		lv_obj_t* cell7_volts[]={objects.c1v1_3,objects.c1v2_3,objects.c1v3_3,objects.c1v4_3,objects.c1v5_3,objects.c1v6_3,objects.c1v7_3,objects.c1v8_3,objects.c1v9_3,objects.c1v10_3,objects.c1v11_3,objects.c1v12_3,objects.c1v13_3,objects.c1v14_3};
		lv_obj_t* cell7_temps[]={objects.c1t1_3,objects.c1t2_3,objects.c1t3_3,objects.c1t4_3,objects.c1t5_3,objects.c1t6_3,objects.c1tu_3,objects.c1tl_3,objects.c1td_3,objects.c1ta_3};
		for(uint8_t i=0 ; i<NUM_OF_CELLS/TOTAL_SEG ; i++ ){
			PushValueAsFloat(cell7_volts[i], CellsVolt[i+ 6*(NUM_OF_CELLS/TOTAL_SEG) ]);
			PushValueAsFloat(cell7_temps[i], CellsTemp[i + 6*(NUM_OF_TSENSOR/TOTAL_SEG)]);
		}
		break;

	case 8:
		//Cell 8
		lv_obj_t* cell8_volts[]={objects.c2v1_3,objects.c2v2_3,objects.c2v3_3,objects.c2v4_3,objects.c2v5_3,objects.c2v6_3,objects.c2v7_3,objects.c2v8_3,objects.c2v9_3,objects.c2v10_3,objects.c2v11_3,objects.c2v12_3,objects.c2v13_3,objects.c2v14_3};
		lv_obj_t* cell8_temps[]={objects.c2t1_3,objects.c2t2_3,objects.c2t3_3,objects.c2t4_3,objects.c2t5_3,objects.c2t6_3,objects.c2tu_3,objects.c2tl_3,objects.c2td_3,objects.c2ta_3};
		for(uint8_t i=0 ; i<NUM_OF_CELLS/TOTAL_SEG ; i++ ){
			PushValueAsFloat(cell8_volts[i], CellsVolt[i+ 7*(NUM_OF_CELLS/TOTAL_SEG) ]);
			PushValueAsFloat(cell8_temps[i], CellsTemp[i + 7*(NUM_OF_TSENSOR/TOTAL_SEG)]);
		}
		break;

	default:
		break;
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
