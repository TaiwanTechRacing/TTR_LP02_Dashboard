/*
 * bsp_qspi.c
 *
 *  W25Qxx QSPI Flash,直接操作 QUADSPI 暫存器(理由同 bsp_sdram.c:
 *  專案的 HAL 版本和本機 CubeH7 套件對不上,而且 HAL_QSPI 這個模組
 *  根本沒被 CubeMX 複製進來)。
 */

#include "bsp_qspi.h"
#include "bsp_mpu.h"
#include "stm32h7xx_hal.h"

/* W25Qxx 指令 */
#define CMD_WRITE_ENABLE        0x06U
#define CMD_READ_STATUS_REG2    0x35U
#define CMD_WRITE_STATUS_REG2   0x31U
#define CMD_READ_JEDEC_ID       0x9FU
#define CMD_FAST_READ_QUAD_OUT  0x6BU

/*
 * 讀取指令選 0x6B(Fast Read Quad Output)而不是更快的 0xEB
 * (Fast Read Quad I/O)。
 *
 * 0x6B 是 1-1-4:指令和位址走單線,只有資料走四線,固定 8 個 dummy cycle,
 * 不需要 alternate byte。0xEB 的位址也走四線、還要送 M7-M0 模式位元組,
 * dummy cycle 數會隨頻率和廠牌設定變動,設錯就是讀出整片垃圾而且很難查。
 *
 * 對我們的用途(整批讀圖片)來說,位址階段的差異可以忽略 —— 資料階段
 * 一樣是四線全速。用可靠的那個。
 */
#define QSPI_DUMMY_CYCLES       8U

/*
 * QUADSPI kernel clock 預設是 hclk3 = 240MHz。除以 4 得到 60MHz。
 *
 * W25Q 系列在 Fast Read Quad Output 下規格大多可到 80MHz 以上,這裡取 60MHz
 * 留裕度:這條路是備援,穩定比極速重要。四線 60MHz 約 30 MB/s,一張全螢幕
 * 圖(261KB)大約 9ms,對切頁來說夠用。
 * 之後真的需要更快,把 PRESCALER 調小即可。
 */
#define QSPI_PRESCALER          3U      /* 除頻 = PRESCALER + 1 = 4 */

#define QSPI_TIMEOUT_MS         100U

static uint32_t s_flash_size;
static uint32_t s_jedec_id;

static void qspi_gpio_init(void);
static bool  qspi_wait_not_busy(void);
static bool  qspi_read_jedec_id(uint32_t *id);
static bool  qspi_enable_quad_mode(void);
static void  qspi_enable_memory_mapped(void);

bool BSP_QSPI_Init(void)
{
    s_flash_size = 0;
    s_jedec_id = 0;

    qspi_gpio_init();
    __HAL_RCC_QSPI_CLK_ENABLE();

    /* 先把 QUADSPI 關掉再設定,避免殘留狀態 */
    QUADSPI->CR = 0;

    QUADSPI->CR = (QSPI_PRESCALER << QUADSPI_CR_PRESCALER_Pos)
                | (3U << QUADSPI_CR_FTHRES_Pos);          /* FIFO 門檻 4 bytes */

    /*
     * FSIZE 先填最大值(2^32)。真正的容量要讀完 JEDEC ID 才知道,
     * 但讀 ID 本身就需要 QUADSPI 先能動,所以這裡給一個夠大的暫定值。
     */
    QUADSPI->DCR = (31U << QUADSPI_DCR_FSIZE_Pos)
                 | (7U  << QUADSPI_DCR_CSHT_Pos);         /* CS 最短高電位 8 cycle */

    QUADSPI->CR |= QUADSPI_CR_EN;

    if (!qspi_read_jedec_id(&s_jedec_id)) {
        return false;
    }

    /*
     * JEDEC ID 第三個 byte 是容量的 2 次冪指數:
     * W25Q64 = 0xEF4017 -> 2^23 = 8MB,W25Q128 = 0xEF4018 -> 2^24 = 16MB。
     */
    const uint8_t capacity_exp = (uint8_t)(s_jedec_id & 0xFFU);
    if (capacity_exp < 16U || capacity_exp > 25U) {
        /* 不是合理的容量代碼,大概是沒讀到晶片(全 0 或全 F) */
        return false;
    }
    s_flash_size = 1UL << capacity_exp;

    /* 用實際容量重設 FSIZE */
    QUADSPI->CR &= ~QUADSPI_CR_EN;
    QUADSPI->DCR = ((uint32_t)(capacity_exp - 1U) << QUADSPI_DCR_FSIZE_Pos)
                 | (7U << QUADSPI_DCR_CSHT_Pos);
    QUADSPI->CR |= QUADSPI_CR_EN;

    if (!qspi_enable_quad_mode()) {
        s_flash_size = 0;
        return false;
    }

    qspi_enable_memory_mapped();

    /* 映射區域要等容量確定才能開,否則 MPU 擋著讀不到 */
    BSP_MPU_EnableQspiRegion(s_flash_size);

    return true;
}

uint32_t BSP_QSPI_GetFlashSize(void)
{
    return s_flash_size;
}

uint32_t BSP_QSPI_GetJedecId(void)
{
    return s_jedec_id;
}

static bool qspi_wait_not_busy(void)
{
    const uint32_t start = HAL_GetTick();

    while ((QUADSPI->SR & QUADSPI_SR_BUSY) != 0U) {
        if ((HAL_GetTick() - start) > QSPI_TIMEOUT_MS) {
            return false;
        }
    }

    return true;
}

/* 等待傳輸完成旗標(TCF),然後清掉它 */
static bool qspi_wait_transfer_complete(void)
{
    const uint32_t start = HAL_GetTick();

    while ((QUADSPI->SR & QUADSPI_SR_TCF) == 0U) {
        if ((HAL_GetTick() - start) > QSPI_TIMEOUT_MS) {
            return false;
        }
    }

    QUADSPI->FCR = QUADSPI_FCR_CTCF;
    return true;
}

static bool qspi_read_jedec_id(uint32_t *id)
{
    if (!qspi_wait_not_busy()) {
        return false;
    }

    QUADSPI->DLR = 3U - 1U;                                  /* 讀 3 bytes */
    QUADSPI->CCR = (1U << QUADSPI_CCR_IMODE_Pos)             /* 指令走單線 */
                 | (1U << QUADSPI_CCR_DMODE_Pos)             /* 資料走單線 */
                 | (1U << QUADSPI_CCR_FMODE_Pos)             /* 間接讀取 */
                 | (CMD_READ_JEDEC_ID << QUADSPI_CCR_INSTRUCTION_Pos);

    uint32_t value = 0;
    for (uint8_t i = 0; i < 3U; i++) {
        const uint32_t start = HAL_GetTick();
        while ((QUADSPI->SR & QUADSPI_SR_FTF) == 0U &&
               (QUADSPI->SR & QUADSPI_SR_TCF) == 0U) {
            if ((HAL_GetTick() - start) > QSPI_TIMEOUT_MS) {
                return false;
            }
        }
        value = (value << 8) | (*(volatile uint8_t *)&QUADSPI->DR);
    }

    if (!qspi_wait_transfer_complete()) {
        return false;
    }

    *id = value;
    return true;
}

/*
 * 0x6B 需要 Flash 的 Quad Enable 位元先打開,否則 IO2/IO3 不會被驅動,
 * 讀出來會是垃圾。W25Q 的 QE 在 Status Register 2 的 bit 1。
 */
static bool qspi_enable_quad_mode(void)
{
    /* --- 先讀 SR2 --- */
    if (!qspi_wait_not_busy()) {
        return false;
    }

    QUADSPI->DLR = 0U;                                        /* 1 byte */
    QUADSPI->CCR = (1U << QUADSPI_CCR_IMODE_Pos)
                 | (1U << QUADSPI_CCR_DMODE_Pos)
                 | (1U << QUADSPI_CCR_FMODE_Pos)
                 | (CMD_READ_STATUS_REG2 << QUADSPI_CCR_INSTRUCTION_Pos);

    const uint32_t start = HAL_GetTick();
    while ((QUADSPI->SR & QUADSPI_SR_FTF) == 0U &&
           (QUADSPI->SR & QUADSPI_SR_TCF) == 0U) {
        if ((HAL_GetTick() - start) > QSPI_TIMEOUT_MS) {
            return false;
        }
    }

    const uint8_t sr2 = *(volatile uint8_t *)&QUADSPI->DR;

    if (!qspi_wait_transfer_complete()) {
        return false;
    }

    if ((sr2 & 0x02U) != 0U) {
        return true;    /* QE 已經是開的,不必再寫一次 */
    }

    /* --- Write Enable --- */
    if (!qspi_wait_not_busy()) {
        return false;
    }

    QUADSPI->CCR = (1U << QUADSPI_CCR_IMODE_Pos)
                 | (CMD_WRITE_ENABLE << QUADSPI_CCR_INSTRUCTION_Pos);

    if (!qspi_wait_transfer_complete()) {
        return false;
    }

    /* --- 寫回 SR2,把 QE 設起來 --- */
    if (!qspi_wait_not_busy()) {
        return false;
    }

    QUADSPI->DLR = 0U;
    QUADSPI->CCR = (1U << QUADSPI_CCR_IMODE_Pos)
                 | (1U << QUADSPI_CCR_DMODE_Pos)
                 | (0U << QUADSPI_CCR_FMODE_Pos)              /* 間接寫入 */
                 | (CMD_WRITE_STATUS_REG2 << QUADSPI_CCR_INSTRUCTION_Pos);

    *(volatile uint8_t *)&QUADSPI->DR = (uint8_t)(sr2 | 0x02U);

    if (!qspi_wait_transfer_complete()) {
        return false;
    }

    /* 寫狀態暫存器是非揮發性動作,晶片內部要一點時間 */
    HAL_Delay(10);

    return qspi_wait_not_busy();
}

static void qspi_enable_memory_mapped(void)
{
    QUADSPI->CCR = (1U << QUADSPI_CCR_IMODE_Pos)              /* 指令單線 */
                 | (1U << QUADSPI_CCR_ADMODE_Pos)             /* 位址單線 */
                 | (2U << QUADSPI_CCR_ADSIZE_Pos)             /* 24-bit 位址 */
                 | (3U << QUADSPI_CCR_DMODE_Pos)              /* 資料四線 */
                 | (QSPI_DUMMY_CYCLES << QUADSPI_CCR_DCYC_Pos)
                 | (3U << QUADSPI_CCR_FMODE_Pos)              /* 記憶體映射 */
                 | (CMD_FAST_READ_QUAD_OUT << QUADSPI_CCR_INSTRUCTION_Pos);
}

static void qspi_gpio_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();

    gpio.Mode  = GPIO_MODE_AF_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    /* PB2 CLK 走 AF9 */
    gpio.Pin       = GPIO_PIN_2;
    gpio.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOB, &gpio);

    /* PB6 NCS 走 AF10 */
    gpio.Pin       = GPIO_PIN_6;
    gpio.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIOB, &gpio);

    /* PF6 IO3 / PF7 IO2 走 AF9 */
    gpio.Pin       = GPIO_PIN_6 | GPIO_PIN_7;
    gpio.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOF, &gpio);

    /* PF8 IO0 / PF9 IO1 走 AF10 */
    gpio.Pin       = GPIO_PIN_8 | GPIO_PIN_9;
    gpio.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIOF, &gpio);
}
