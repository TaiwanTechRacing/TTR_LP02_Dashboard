/*
 * bsp_sdram.c
 *
 *  W9825G6KH SDRAM 初始化,直接操作 FMC 暫存器。
 *
 *  為什麼不用 HAL_SDRAM:專案裡的 STM32H7 HAL 版本和本機 CubeH7 套件都對不上,
 *  硬塞 stm32h7xx_hal_sdram.c + stm32h7xx_ll_fmc.c 進來會有版本不一致的風險。
 *  SDRAM 的設定其實只有四個暫存器,直接寫反而清楚。
 */

#include "bsp_sdram.h"

/*
 * 時序參數是照 SDCLK = HCLK3 / 2 = 240 MHz / 2 = 120 MHz(8.33 ns)算的。
 * 官方範例是 100 MHz 的參數,不能直接抄。
 * 如果之後改了系統時脈,下面這一整段都要重算。
 */
#define SDRAM_SDCLK_HZ        120000000UL

/* W9825G6KH 的規格(ns)換算成 SDCLK 週期數,一律無條件進位 */
#define SDRAM_TMRD_CYCLES     2U   /* Load Mode Register -> Active */
#define SDRAM_TXSR_CYCLES     9U   /* 72 ns  離開 self-refresh */
#define SDRAM_TRAS_CYCLES     6U   /* 42 ns  最短 row active 時間 */
#define SDRAM_TRC_CYCLES      8U   /* 60 ns  row cycle */
#define SDRAM_TWR_CYCLES      3U   /* 寫入回復。必須 >= TRAS-TRCD(=3),所以不能用 2 */
#define SDRAM_TRP_CYCLES      3U   /* 18 ns  precharge */
#define SDRAM_TRCD_CYCLES     3U   /* 18 ns  row -> column */

/*
 * 更新率計數:COUNT = (64 ms x SDCLK) / 8192 列 - 20(安全邊界)
 *                   = (0.064 x 120e6) / 8192 - 20 = 937 - 20 = 917
 */
#define SDRAM_REFRESH_COUNT   917U

/* Mode Register:burst length 1、sequential、CAS latency 3、single location write */
#define SDRAM_MODEREG_VALUE   0x0230U

/* SDCMR 的 MODE 欄位 */
#define SDRAM_CMD_NORMAL      0U
#define SDRAM_CMD_CLK_ENABLE  1U
#define SDRAM_CMD_PALL        2U
#define SDRAM_CMD_AUTOREFRESH 3U
#define SDRAM_CMD_LOAD_MODE   4U

#define SDRAM_CMD_TIMEOUT_MS  100U

/* 這版 CMSIS header 只定義了 SDSR 的 MODES 和 RE,沒有 BUSY。
 * 依 RM0433 的 FMC_SDSR,BUSY 是 bit 5。 */
#define FMC_SDSR_BUSY         (0x1UL << 5)

static void sdram_gpio_init(void);
static bool sdram_send_cmd(uint32_t mode, uint32_t auto_refresh_num, uint32_t mode_reg);

void BSP_SDRAM_Init(void)
{
    sdram_gpio_init();

    __HAL_RCC_FMC_CLK_ENABLE();

    /* FMC kernel clock 選 rcc_hclk3(240 MHz)。這是重置後的預設值,寫出來是為了
     * 讓上面的時序計算有明確依據,不要依賴預設。 */
    MODIFY_REG(RCC->D1CCIPR, RCC_D1CCIPR_FMCSEL, 0U);

    /* --- SDCR1:控制暫存器 ---
     * SDCLK / RBURST / RPIPE 這三個欄位不管用哪個 bank,都只看 SDCR1。 */
    FMC_Bank5_6_R->SDCR[0] =
          (1U << FMC_SDCRx_NC_Pos)        /* 9 個 column bit  (00=8, 01=9, 10=10, 11=11) */
        | (2U << FMC_SDCRx_NR_Pos)        /* 13 個 row bit    (00=11, 01=12, 10=13) */
        | (1U << FMC_SDCRx_MWID_Pos)      /* 資料匯流排 16 bit */
        | (1U << FMC_SDCRx_NB_Pos)        /* 內部 4 個 bank */
        | (3U << FMC_SDCRx_CAS_Pos)       /* CAS latency 3 */
        | (2U << FMC_SDCRx_SDCLK_Pos)     /* SDCLK = HCLK3 / 2 = 120 MHz */
        | FMC_SDCRx_RBURST;               /* 開啟 burst read */

    /* --- SDTR1:時序暫存器。每個欄位填的是「週期數 - 1」--- */
    FMC_Bank5_6_R->SDTR[0] =
          ((SDRAM_TMRD_CYCLES - 1U) << FMC_SDTRx_TMRD_Pos)
        | ((SDRAM_TXSR_CYCLES - 1U) << FMC_SDTRx_TXSR_Pos)
        | ((SDRAM_TRAS_CYCLES - 1U) << FMC_SDTRx_TRAS_Pos)
        | ((SDRAM_TRC_CYCLES  - 1U) << FMC_SDTRx_TRC_Pos)
        | ((SDRAM_TWR_CYCLES  - 1U) << FMC_SDTRx_TWR_Pos)
        | ((SDRAM_TRP_CYCLES  - 1U) << FMC_SDTRx_TRP_Pos)
        | ((SDRAM_TRCD_CYCLES - 1U) << FMC_SDTRx_TRCD_Pos);

    /* H7 特有:設定完 BCR/SDCR 之後還要把 FMC 控制器本身打開,
     * 少了這一行整個 FMC 都不會動。 */
    FMC_Bank1_R->BTCR[0] |= FMC_BCR1_FMCEN;

    /* --- W9825G6KH 要求的上電初始化順序 --- */
    sdram_send_cmd(SDRAM_CMD_CLK_ENABLE, 1U, 0U);
    HAL_Delay(1);                                        /* 規格要求 >= 200 us */
    sdram_send_cmd(SDRAM_CMD_PALL, 1U, 0U);
    sdram_send_cmd(SDRAM_CMD_AUTOREFRESH, 8U, 0U);       /* 連續 8 次 auto-refresh */
    sdram_send_cmd(SDRAM_CMD_LOAD_MODE, 1U, SDRAM_MODEREG_VALUE);

    FMC_Bank5_6_R->SDRTR |= (SDRAM_REFRESH_COUNT << FMC_SDRTR_COUNT_Pos);
}

bool BSP_SDRAM_SelfTest(void)
{
    /* 挑幾個分散在不同 row / bank 的位址,能抓到位址線沒焊好或時序太緊的狀況。
     * 全部掃一遍要好幾百毫秒,開機時不值得。 */
    static const uint32_t offsets[] = {
        0x00000000UL, 0x00000004UL, 0x00001000UL, 0x00080000UL,
        0x00400000UL, 0x01000000UL, SDRAM_SIZE_BYTES - 4U
    };

    volatile uint32_t *ram = (volatile uint32_t *)SDRAM_BASE_ADDR;
    const uint32_t seed = 0xA5C3F00DUL;

    for (uint32_t i = 0U; i < (sizeof(offsets) / sizeof(offsets[0])); i++) {
        ram[offsets[i] / 4U] = seed ^ offsets[i];
    }

    /* 先全部寫完再讀,這樣位址線短路(不同位址對到同一格)才會被抓出來。 */
    for (uint32_t i = 0U; i < (sizeof(offsets) / sizeof(offsets[0])); i++) {
        if (ram[offsets[i] / 4U] != (seed ^ offsets[i])) {
            return false;
        }
    }

    return true;
}

static bool sdram_send_cmd(uint32_t mode, uint32_t auto_refresh_num, uint32_t mode_reg)
{
    uint32_t start = HAL_GetTick();

    while ((FMC_Bank5_6_R->SDSR & FMC_SDSR_BUSY) != 0U) {
        if ((HAL_GetTick() - start) > SDRAM_CMD_TIMEOUT_MS) {
            return false;
        }
    }

    FMC_Bank5_6_R->SDCMR =
          (mode << FMC_SDCMR_MODE_Pos)
        | FMC_SDCMR_CTB1                                       /* 目標是 SDRAM bank 1 */
        | ((auto_refresh_num - 1U) << FMC_SDCMR_NRFS_Pos)
        | (mode_reg << FMC_SDCMR_MRD_Pos);

    start = HAL_GetTick();
    while ((FMC_Bank5_6_R->SDSR & FMC_SDSR_BUSY) != 0U) {
        if ((HAL_GetTick() - start) > SDRAM_CMD_TIMEOUT_MS) {
            return false;
        }
    }

    return true;
}

static void sdram_gpio_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();

    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_PULLUP;
    gpio.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;   /* 120 MHz 的匯流排,一定要 very high */
    gpio.Alternate = GPIO_AF12_FMC;

    /* PC0 SDNWE, PC2 SDNE0, PC3 SDCKE0 */
    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3;
    HAL_GPIO_Init(GPIOC, &gpio);

    /* PD0/1 D2-D3, PD8/9/10 D13-D15, PD14/15 D0-D1 */
    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10
             | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOD, &gpio);

    /* PE0/1 NBL0-1, PE7..PE15 D4-D12 */
    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9
             | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13
             | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOE, &gpio);

    /* PF0..PF5 A0-A5, PF11 SDNRAS, PF12..PF15 A6-A9 */
    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4
             | GPIO_PIN_5 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13
             | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOF, &gpio);

    /* PG0..PG2 A10-A12, PG4/5 BA0-BA1, PG8 SDCLK, PG15 SDNCAS */
    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5
             | GPIO_PIN_8 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOG, &gpio);
}
