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

#define MIN_SCR_ID 2
#define MAX_SCR_ID 9

#define GLV_LOW_VOLT 19
#define MAX_MOTOR_SPEED 45535
#define HV_LOW_VOLT 350

#define HOLD_BUTTON 3
#define WELCOMEDELAY 100

#define NUM_OF_CELLS 112 //電芯數量
#define DATA_PER_PACK 4 //每個封包有4個電芯的電壓讀值
#define TOTAL_SEG 8 //8個單元

#define AMS_STATUS_CMD0_ID 0x536 //AMS
#define VCU_STATUS_CMD_SYSTEM1_ID 0x420 //SDC status
#define VCU_STATUS_CMD_SYSTEM2_ID 0x421 //Speed
#define VCU_STATUS_CMD_SENSOR1_ID 0x430 //BSE
#define VCU_STATUS_CMD_SENSOR2_ID 0x431 //APPS
#define VCU_STATUS_CMD_SENSOR3_ID 0x432//GLV Batt V,I
#define AMS_VOLTAGE_STATUS_ID_START 0x500 //AMS volt init ID
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
uint8_t RX[8]={0};
uint8_t screen_ID_now=0;
uint8_t button1counter=0;
uint8_t button2counter=0;
uint8_t RX_COPY[8] = {0};



//For can update.
volatile static uint16_t carSpeedTransmit = 0;
volatile static uint16_t sdcStatus=0;
volatile static float steeringTransmit=0;
volatile static float apps1Transmit=0;
volatile static float bseRearPUTransmit=0;
volatile static float glvVoltTransmit = 0;
volatile static float busVoltage = 0;
volatile static float CellsVolt[NUM_OF_CELLS] = {0};
volatile static bool RTD_SIGNAL=0;
volatile static bool COOL_SIGNAL=0;



static uint16_t STD_ID_LIST_CAN[35] = {
		AMS_STATUS_CMD0_ID, VCU_STATUS_CMD_SYSTEM1_ID, VCU_STATUS_CMD_SYSTEM2_ID, VCU_STATUS_CMD_SENSOR1_ID,
		VCU_STATUS_CMD_SENSOR2_ID, VCU_STATUS_CMD_SENSOR3_ID, AMS_VOLTAGE_STATUS_ID_START, 0
};





/*
volatile static uint16_t rawApps1Angle=0;
volatile static uint16_t rawSteeringAngle=0;
volatile static uint16_t rawBseRearPUTAngle=0;
volatile static uint16_t GLV_V=0;

*/

//Test Test
//Test TEST
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
enum ScreensEnum screens[10]={SCREEN_ID_WELCOME,SCREEN_ID_CHECK,SCREEN_ID_RACING,SCREEN_ID_FACTORY_SWITCH,SCREEN_ID_FACTORY_BAT_SUM,SCREEN_ID_FACTORY_BAT_P1,SCREEN_ID_FACTORY_BAT_P2,SCREEN_ID_FACTORY_BAT_P3,SCREEN_ID_FACTORY_BAT_P4,SCREEN_ID_FACTORY_MOT};
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
  uint16_t AMS_Volt_Start_ID_POS = 0;
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

	//找到AMS傳送volt封包的ID被定義在陣列中的位置
	for(uint16_t i=0 ; i < ( sizeof(STD_ID_LIST_CAN)/ sizeof(STD_ID_LIST_CAN[0])) ; i++){
		if(STD_ID_LIST_CAN[i] == AMS_VOLTAGE_STATUS_ID_START){
			AMS_Volt_Start_ID_POS = i;
		}
	}

	//將(112/4) - 1個ID自AMS_VOLTAGE_STATUS_ID_START遞增並分配到自起始位置後的空位
	for(uint16_t i=0 ; i < (NUM_OF_CELLS / DATA_PER_PACK) ; i++ ){
		STD_ID_LIST_CAN[AMS_Volt_Start_ID_POS + 1 + i ] = AMS_VOLTAGE_STATUS_ID_START+i+1;
	}

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


  //while(HAL_FDCAN_EnableTxBufferRequest(&hfdcan2, FDCAN_TX_BUFFER0) == 1);

  /*
  if(HAL_FDCAN_GetRxMessage(&hfdcan2, FDCAN_RX_FIFO0, &RxHeader, RX) != HAL_OK ){
	Error_Handler();
  }
  */

  //HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);

  HAL_GPIO_WritePin(USR_LED_GPIO_Port, USR_LED_Pin, 1);

  //init racing page

  //把racing page的bar條初始化
  Icon_Set(objects.power_indicator, 0);
  Icon_Set(objects.fans_indicator, 0);
  Icon_Set(objects.pressure_indicator, 0);
  Icon_Set(objects.temp_indicator, 0);

  /*
  lv_obj_set_style_opa(objects.power_indicator, LV_OPA_0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_opa(objects.fans_indicator, LV_OPA_0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_opa(objects.pressure_indicator, LV_OPA_0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_opa(objects.temp_indicator, LV_OPA_0, LV_PART_MAIN | LV_STATE_DEFAULT);
  */
  lv_arc_set_value(objects.speed_process, (int32_t)0);
  lv_arc_set_value(objects.steering_wheel_dir, (int32_t)50);
  lv_arc_set_value(objects.acc_process, (int32_t)0);
  lv_arc_set_value(objects.brake_process, (int32_t)0);



  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  //歡迎介面的counter
	  if(welcome_counter < WELCOMEDELAY){
		  welcome_counter++;
	  }
	  else{
		  if(welcome_stop == 0){
			  loadScreen(screens[2]);
			  screen_ID_now = 2;
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
  hfdcan2.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan2.Init.Mode = FDCAN_MODE_BUS_MONITORING;
  hfdcan2.Init.AutoRetransmission = DISABLE;
  hfdcan2.Init.TransmitPause = DISABLE;
  hfdcan2.Init.ProtocolException = DISABLE;
  hfdcan2.Init.NominalPrescaler = 3;
  hfdcan2.Init.NominalSyncJumpWidth = 1;
  hfdcan2.Init.NominalTimeSeg1 = 7;
  hfdcan2.Init.NominalTimeSeg2 = 2;
  hfdcan2.Init.DataPrescaler = 1;
  hfdcan2.Init.DataSyncJumpWidth = 9;
  hfdcan2.Init.DataTimeSeg1 = 3;
  hfdcan2.Init.DataTimeSeg2 = 3;
  hfdcan2.Init.MessageRAMOffset = 0;
  hfdcan2.Init.StdFiltersNbr = 50;
  hfdcan2.Init.ExtFiltersNbr = 0;
  hfdcan2.Init.RxFifo0ElmtsNbr = 64;
  hfdcan2.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan2.Init.RxFifo1ElmtsNbr = 0;
  hfdcan2.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan2.Init.RxBuffersNbr = 64;
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
	uint8_t PositionOfPack=0;

	switch(RxHeader.Identifier){
	case VCU_STATUS_CMD_SYSTEM1_ID:
		RTD_SIGNAL = RX[0] & 0b00000001;
		COOL_SIGNAL = RX[0] & 0b10000000;
		sdcStatus = (uint16_t)RX[3] | ((uint16_t)RX[4] <<8);
		break;
	case VCU_STATUS_CMD_SYSTEM2_ID:
		uint16_t raw_carSpeedTransmiut = (uint16_t)RX[0] | ( (uint16_t)RX[1] << 8);
		carSpeedTransmit = (raw_carSpeedTransmiut * 300.0f) / 65535.0f;
		break;
	case VCU_STATUS_CMD_SENSOR1_ID:
		uint16_t raw_bseRearPUTransmit = (uint16_t)RX[0] | ((uint16_t)RX[1] << 8 );
		bseRearPUTransmit = ((float)raw_bseRearPUTransmit / 65535.0f) * 100.0f;
		break;
	case VCU_STATUS_CMD_SENSOR2_ID:
		uint16_t raw_steeringTransmit = (uint16_t)RX[4] | ( (uint16_t)RX[5]<<8) ;
		steeringTransmit = (100.0f - ((float)raw_steeringTransmit / 65535.0f) * 100.0f);
		uint16_t raw_apps1Transmit = (uint16_t)RX[0] | ((uint16_t)RX[1] << 8 );
		apps1Transmit = ((float)raw_apps1Transmit / 65535.0f) * 100.0f;
		break;
	case VCU_STATUS_CMD_SENSOR3_ID:
		uint16_t raw_glvVoltTransmit = ((uint16_t)RX[1] << 8 ) | (uint16_t)RX[0] ;
		glvVoltTransmit = (float)raw_glvVoltTransmit / 2185.0f;
		break;
	case AMS_STATUS_CMD0_ID:
		uint16_t raw_busVoltage = ((uint16_t)RX[4] << 8 | (uint16_t)RX[3]);
		busVoltage = raw_busVoltage * 0.010638f;
		break;

	default:
		if(RxHeader.Identifier >= AMS_VOLTAGE_STATUS_ID_START && RxHeader.Identifier <= (AMS_VOLTAGE_STATUS_ID_START+ (NUM_OF_CELLS/DATA_PER_PACK) -1) ){
			PositionOfPack = RxHeader.Identifier - AMS_VOLTAGE_STATUS_ID_START; //start from 0 to 27, total 28.
			for(uint8_t i=0 ; i<DATA_PER_PACK ; i++){
				Little_Eendian_Merge(2*i, &CellsVolt[PositionOfPack*DATA_PER_PACK+i]);
				CellsVolt[PositionOfPack*DATA_PER_PACK+i] /= 10000;
			}
		}
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
	//upc++;


	//RACING PAGE

	switch(screen_ID_now){
	case 2:

		//RACING PAGE ID:2
		ReadSDCStatus(sdcStatus);


		if(RTD_SIGNAL != 0){
			lv_label_set_text(objects.rtd, "D");
			lv_obj_set_style_text_color(objects.rtd, lv_color_make(0x00, 0xe8, 0xFF), 0);
		}
		else{
			lv_label_set_text(objects.rtd, "P");
			lv_obj_set_style_text_color(objects.rtd, lv_color_make(0xFF, 0x00, 0x00), 0);
		}

		if(COOL_SIGNAL == 0){
			lv_obj_set_style_opa(objects.fans_indicator, LV_OPA_100, LV_PART_MAIN | LV_STATE_DEFAULT);

		}
		else{
			lv_obj_set_style_opa(objects.fans_indicator, LV_OPA_0, LV_PART_MAIN | LV_STATE_DEFAULT);
		}

		snprintf(buffer,sizeof(buffer), "%03u", carSpeedTransmit); //2. monitor this variable.
		lv_label_set_text(objects.speed, buffer);

		PushValueAsFloat(objects.glv_volt_r, glvVoltTransmit);
		/*
		snprintf(buffer,sizeof(buffer), "%0.1f", glvVoltTransmit);
		lv_label_set_text(objects.glv_volt_r, buffer);
		*/

		PushValueAsFloat(objects.hv_volt_r, busVoltage);
		/*
		snprintf(buffer,sizeof(buffer), "%0.1f", busVoltage);
		lv_label_set_text(objects.hv_volt_r, buffer);
		*/

		lv_arc_set_value(objects.speed_process, (int32_t)carSpeedTransmit);
		lv_arc_set_value(objects.steering_wheel_dir, (int32_t)(steeringTransmit+0.5f));
		lv_arc_set_value(objects.acc_process, (int32_t)(apps1Transmit+0.5f));
		lv_arc_set_value(objects.brake_process, (int32_t)(bseRearPUTransmit+0.5f));


		if(glvVoltTransmit < GLV_LOW_VOLT ){
		  lv_obj_set_style_opa(objects.power_indicator, LV_OPA_100, LV_PART_MAIN | LV_STATE_DEFAULT);
		  glv_low_volt = 1;
		}
		else{
		  if(hv_low_volt == 0){
			lv_obj_set_style_opa(objects.power_indicator, LV_OPA_0, LV_PART_MAIN | LV_STATE_DEFAULT);
		  }

		  glv_low_volt = 0;
		}

		if(busVoltage < HV_LOW_VOLT ){
		  lv_obj_set_style_opa(objects.power_indicator, LV_OPA_100, LV_PART_MAIN | LV_STATE_DEFAULT);
		  hv_low_volt = 1;
		}
		else{
		  if(glv_low_volt == 0){
			  lv_obj_set_style_opa(objects.power_indicator, LV_OPA_0, LV_PART_MAIN | LV_STATE_DEFAULT);
		  }
		  hv_low_volt = 0;
		}

	  break;

	case 3:
		ReadSDCStatus(sdcStatus);
	    break;

	case 4: //BAT_SUM
		//GLV volt
		PushValueAsFloat(objects.glv_v, glvVoltTransmit);
		/*
		snprintf(buffer,sizeof(buffer), "%.1f", glvVoltTransmit);
		lv_label_set_text(objects.glv_v, buffer);
		*/

		//GLV SOC...
		snprintf(buffer,sizeof(buffer), "%03u", 0);
		lv_label_set_text(objects.glv_soc, buffer);

		//ACC volt
		PushValueAsFloat(objects.acc_volt, busVoltage);
		/*
		snprintf(buffer,sizeof(buffer), "%.1f", busVoltage);
		lv_label_set_text(objects.acc_volt, buffer);
		*/

		//ACC SOC
		snprintf(buffer,sizeof(buffer), "%03u", 0);
		lv_label_set_text(objects.acc_soc, buffer);

		break;

	case 5:
		//Cell 1,2 volts
		writeCellsValue(1);
		writeCellsValue(2);
		//Cell 1,2 temps

		break;

	case 6:
		//cell 3,4 volts
		writeCellsValue(3);
		writeCellsValue(4);
		//cell 3,4 temps
		break;

	case 7:
		//cell 5,6 volts
		writeCellsValue(5);
		writeCellsValue(6);
		//cell 5,6 temps
		break;

	case 8:
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
	static char buffer[6]={0};

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
		}

		break;

	case 2:
		//Cell 2
		lv_obj_t* cell2_volts[]={objects.c2v1,objects.c2v2,objects.c2v3,objects.c2v4,objects.c2v5,objects.c2v6,objects.c2v7,objects.c2v8,objects.c2v9,objects.c2v10,objects.c2v11,objects.c2v12,objects.c2v13,objects.c2v14};
		lv_obj_t* cell2_temps[]={objects.c2t1,objects.c2t2,objects.c2t3,objects.c2t4,objects.c2t5,objects.c2t6,objects.c2tu,objects.c2tl,objects.c2td,objects.c2ta};
		for(uint8_t i=0 ; i<NUM_OF_CELLS/TOTAL_SEG ; i++ ){
			PushValueAsFloat(cell2_volts[i], CellsVolt[i+ (NUM_OF_CELLS/TOTAL_SEG) ]);
		}
		break;

	case 3:
		//Cell 3

		lv_obj_t* cell3_volts[]={objects.c1v1_1,objects.c1v2_1,objects.c1v3_1,objects.c1v4_1,objects.c1v5_1,objects.c1v6_1,objects.c1v7_1,objects.c1v8_1,objects.c1v9_1,objects.c1v10_1,objects.c1v11_1,objects.c1v12_1,objects.c1v13_1,objects.c1v14_1};
		lv_obj_t* cell3_temps[]={objects.c1t1_1,objects.c1t2_1,objects.c1t3_1,objects.c1t4_1,objects.c1t5_1,objects.c1t6_1,objects.c1tu_1,objects.c1tl_1,objects.c1td_1,objects.c1ta_1};
		for(uint8_t i=0 ; i<NUM_OF_CELLS/TOTAL_SEG ; i++ ){
			PushValueAsFloat(cell3_volts[i], CellsVolt[i+ 2*(NUM_OF_CELLS/TOTAL_SEG) ]);
		}
		break;

	case 4:
		//Cell 4
		lv_obj_t* cell4_volts[]={objects.c2v1_1,objects.c2v2_1,objects.c2v3_1,objects.c2v4_1,objects.c2v5_1,objects.c2v6_1,objects.c2v7_1,objects.c2v8_1,objects.c2v9_1,objects.c2v10_1,objects.c2v11_1,objects.c2v12_1,objects.c2v13_1,objects.c2v14_1};
		lv_obj_t* cell4_temps[]={objects.c2t1_1,objects.c2t2_1,objects.c2t3_1,objects.c2t4_1,objects.c2t5_1,objects.c2t6_1,objects.c2tu_1,objects.c2tl_1,objects.c2td_1,objects.c2ta_1};
		for(uint8_t i=0 ; i<NUM_OF_CELLS/TOTAL_SEG ; i++ ){
			PushValueAsFloat(cell4_volts[i], CellsVolt[i+ 3*(NUM_OF_CELLS/TOTAL_SEG) ]);
		}
		break;

	case 5:
		//Cell 5
		lv_obj_t* cell5_volts[]={objects.c1v1_2,objects.c1v2_2,objects.c1v3_2,objects.c1v4_2,objects.c1v5_2,objects.c1v6_2,objects.c1v7_2,objects.c1v8_2,objects.c1v9_2,objects.c1v10_2,objects.c1v11_2,objects.c1v12_2,objects.c1v13_2,objects.c1v14_2};
		lv_obj_t* cell5_temps[]={objects.c1t1_2,objects.c1t2_2,objects.c1t3_2,objects.c1t4_2,objects.c1t5_2,objects.c1t6_2,objects.c1tu_2,objects.c1tl_2,objects.c1td_2,objects.c1ta_2};
		for(uint8_t i=0 ; i<NUM_OF_CELLS/TOTAL_SEG ; i++ ){
			PushValueAsFloat(cell5_volts[i], CellsVolt[i+ 4*(NUM_OF_CELLS/TOTAL_SEG) ]);
		}
		break;

	case 6:
		//Cell	6
		lv_obj_t* cell6_volts[]={objects.c2v1_2,objects.c2v2_2,objects.c2v3_2,objects.c2v4_2,objects.c2v5_2,objects.c2v6_2,objects.c2v7_2,objects.c2v8_2,objects.c2v9_2,objects.c2v10_2,objects.c2v11_2,objects.c2v12_2,objects.c2v13_2,objects.c2v14_2};
		lv_obj_t* cell6_temps[]={objects.c2t1_2,objects.c2t2_2,objects.c2t3_2,objects.c2t4_2,objects.c2t5_2,objects.c2t6_2,objects.c2tu_2,objects.c2tl_2,objects.c2td_2,objects.c2ta_2};
		for(uint8_t i=0 ; i<NUM_OF_CELLS/TOTAL_SEG ; i++ ){
			PushValueAsFloat(cell6_volts[i], CellsVolt[i+ 5*(NUM_OF_CELLS/TOTAL_SEG) ]);
		}
		break;

	case 7:
		//Cell 7
		lv_obj_t* cell7_volts[]={objects.c1v1_3,objects.c1v2_3,objects.c1v3_3,objects.c1v4_3,objects.c1v5_3,objects.c1v6_3,objects.c1v7_3,objects.c1v8_3,objects.c1v9_3,objects.c1v10_3,objects.c1v11_3,objects.c1v12_3,objects.c1v13_3,objects.c1v14_3};
		lv_obj_t* cell7_temps[]={objects.c1t1_3,objects.c1t2_3,objects.c1t3_3,objects.c1t4_3,objects.c1t5_3,objects.c1t6_3,objects.c1tu_3,objects.c1tl_3,objects.c1td_3,objects.c1ta_3};
		for(uint8_t i=0 ; i<NUM_OF_CELLS/TOTAL_SEG ; i++ ){
			PushValueAsFloat(cell7_volts[i], CellsVolt[i+ 6*(NUM_OF_CELLS/TOTAL_SEG) ]);
		}
		break;

	case 8:
		//Cell 8
		lv_obj_t* cell8_volts[]={objects.c2v1_3,objects.c2v2_3,objects.c2v3_3,objects.c2v4_3,objects.c2v5_3,objects.c2v6_3,objects.c2v7_3,objects.c2v8_3,objects.c2v9_3,objects.c2v10_3,objects.c2v11_3,objects.c2v12_3,objects.c2v13_3,objects.c2v14_3};
		lv_obj_t* cell8_temps[]={objects.c2t1_3,objects.c2t2_3,objects.c2t3_3,objects.c2t4_3,objects.c2t5_3,objects.c2t6_3,objects.c2tu_3,objects.c2tl_3,objects.c2td_3,objects.c2ta_3};
		for(uint8_t i=0 ; i<NUM_OF_CELLS/TOTAL_SEG ; i++ ){
			PushValueAsFloat(cell8_volts[i], CellsVolt[i+ 7*(NUM_OF_CELLS/TOTAL_SEG) ]);
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
