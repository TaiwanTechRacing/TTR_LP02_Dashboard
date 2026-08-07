/*
 * bsp_sdram.c
 *
 *  W9825G6KH SDRAM brought up through the FMC registers directly.
 *
 *  Why not HAL_SDRAM: the HAL version vendored in this project matches neither
 *  local CubeH7 package, so dropping in stm32h7xx_hal_sdram.c and
 *  stm32h7xx_ll_fmc.c risks a version mismatch. SDRAM setup is four registers;
 *  writing them directly is clearer anyway.
 */

#include "bsp_sdram.h"

/*
 * Timings are computed for SDCLK = HCLK3 / 2 = 240 MHz / 2 = 120 MHz (8.33 ns).
 * The vendor example ships 100 MHz values, which do not transfer.
 * Changing the system clock means recomputing everything below.
 */
#define SDRAM_SDCLK_HZ        120000000UL

/* W9825G6KH datasheet figures (ns) converted to SDCLK cycles, rounded up */
#define SDRAM_TMRD_CYCLES     2U   /* Load Mode Register -> Active */
#define SDRAM_TXSR_CYCLES     9U   /* 72 ns, exit self-refresh */
#define SDRAM_TRAS_CYCLES     6U   /* 42 ns, minimum row active time */
#define SDRAM_TRC_CYCLES      8U   /* 60 ns, row cycle */
#define SDRAM_TWR_CYCLES      3U   /* write recovery; must be >= TRAS-TRCD (=3), so 2 is not allowed */
#define SDRAM_TRP_CYCLES      3U   /* 18 ns, precharge */
#define SDRAM_TRCD_CYCLES     3U   /* 18 ns, row -> column */

/*
 * Refresh counter: COUNT = (64 ms x SDCLK) / 8192 rows - 20 (safety margin)
 *                        = (0.064 x 120e6) / 8192 - 20 = 937 - 20 = 917
 */
#define SDRAM_REFRESH_COUNT   917U

/* Mode register: burst length 1, sequential, CAS latency 3, single location write */
#define SDRAM_MODEREG_VALUE   0x0230U

/* SDCMR MODE field */
#define SDRAM_CMD_NORMAL      0U
#define SDRAM_CMD_CLK_ENABLE  1U
#define SDRAM_CMD_PALL        2U
#define SDRAM_CMD_AUTOREFRESH 3U
#define SDRAM_CMD_LOAD_MODE   4U

#define SDRAM_CMD_TIMEOUT_MS  100U

/* This CMSIS header defines only MODES and RE for SDSR, not BUSY.
 * Per RM0433 FMC_SDSR, BUSY is bit 5. */
#define FMC_SDSR_BUSY         (0x1UL << 5)

static void sdram_gpio_init(void);
static bool sdram_send_cmd(uint32_t mode, uint32_t auto_refresh_num, uint32_t mode_reg);

void BSP_SDRAM_Init(void)
{
    sdram_gpio_init();

    __HAL_RCC_FMC_CLK_ENABLE();

    /* FMC kernel clock = rcc_hclk3 (240 MHz). This is the reset default; it is
     * written explicitly so the timings above rest on a stated value rather
     * than an assumed one. */
    MODIFY_REG(RCC->D1CCIPR, RCC_D1CCIPR_FMCSEL, 0U);

    /* --- SDCR1: control register ---
     * SDCLK, RBURST and RPIPE are only read from SDCR1 regardless of bank. */
    FMC_Bank5_6_R->SDCR[0] =
          (1U << FMC_SDCRx_NC_Pos)        /* 9 column bits  (00=8, 01=9, 10=10, 11=11) */
        | (2U << FMC_SDCRx_NR_Pos)        /* 13 row bits    (00=11, 01=12, 10=13) */
        | (1U << FMC_SDCRx_MWID_Pos)      /* 16-bit data bus */
        | (1U << FMC_SDCRx_NB_Pos)        /* 4 internal banks */
        | (3U << FMC_SDCRx_CAS_Pos)       /* CAS latency 3 */
        | (2U << FMC_SDCRx_SDCLK_Pos)     /* SDCLK = HCLK3 / 2 = 120 MHz */
        | FMC_SDCRx_RBURST;               /* enable burst read */

    /* --- SDTR1: timing register. Each field holds (cycles - 1). --- */
    FMC_Bank5_6_R->SDTR[0] =
          ((SDRAM_TMRD_CYCLES - 1U) << FMC_SDTRx_TMRD_Pos)
        | ((SDRAM_TXSR_CYCLES - 1U) << FMC_SDTRx_TXSR_Pos)
        | ((SDRAM_TRAS_CYCLES - 1U) << FMC_SDTRx_TRAS_Pos)
        | ((SDRAM_TRC_CYCLES  - 1U) << FMC_SDTRx_TRC_Pos)
        | ((SDRAM_TWR_CYCLES  - 1U) << FMC_SDTRx_TWR_Pos)
        | ((SDRAM_TRP_CYCLES  - 1U) << FMC_SDTRx_TRP_Pos)
        | ((SDRAM_TRCD_CYCLES - 1U) << FMC_SDTRx_TRCD_Pos);

    /* H7 specific: after BCR/SDCR are set the FMC controller itself must be
     * enabled. Without this line nothing on the FMC works at all. */
    FMC_Bank1_R->BTCR[0] |= FMC_BCR1_FMCEN;

    /* --- power-up sequence required by the W9825G6KH --- */
    sdram_send_cmd(SDRAM_CMD_CLK_ENABLE, 1U, 0U);
    HAL_Delay(1);                                        /* datasheet requires >= 200 us */
    sdram_send_cmd(SDRAM_CMD_PALL, 1U, 0U);
    sdram_send_cmd(SDRAM_CMD_AUTOREFRESH, 8U, 0U);       /* 8 consecutive auto-refresh cycles */
    sdram_send_cmd(SDRAM_CMD_LOAD_MODE, 1U, SDRAM_MODEREG_VALUE);

    FMC_Bank5_6_R->SDRTR |= (SDRAM_REFRESH_COUNT << FMC_SDRTR_COUNT_Pos);
}

void *BSP_SDRAM_Alloc(uint32_t size_bytes)
{
    static uint32_t next = SDRAM_FREE_ADDR;

    /* Keep every block on a cache line, so two of them can never share one. */
    const uint32_t aligned = (size_bytes + 31u) & ~31u;

    if (aligned > (SDRAM_FREE_ADDR + SDRAM_FREE_SIZE - next)) {
        return NULL;
    }

    void *p = (void *)next;
    next += aligned;
    return p;
}

bool BSP_SDRAM_SelfTest(void)
{
    /* A handful of addresses spread across rows and banks, enough to catch a
     * bad address line or timings that are too tight. Sweeping the whole chip
     * would cost hundreds of milliseconds, which is not worth it at boot. */
    static const uint32_t offsets[] = {
        0x00000000UL, 0x00000004UL, 0x00001000UL, 0x00080000UL,
        0x00400000UL, 0x01000000UL, SDRAM_SIZE_BYTES - 4U
    };

    volatile uint32_t *ram = (volatile uint32_t *)SDRAM_BASE_ADDR;
    const uint32_t seed = 0xA5C3F00DUL;

    for (uint32_t i = 0U; i < (sizeof(offsets) / sizeof(offsets[0])); i++) {
        ram[offsets[i] / 4U] = seed ^ offsets[i];
    }

    /* Write everything first, then read back: that is what exposes shorted
     * address lines, where two addresses alias to the same cell. */
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
        | FMC_SDCMR_CTB1                                       /* target SDRAM bank 1 */
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
    gpio.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;   /* 120 MHz bus - very high is mandatory */
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
