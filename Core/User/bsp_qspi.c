/*
 * bsp_qspi.c
 *
 *  W25Qxx QSPI flash driven through the QUADSPI registers directly. Same
 *  reasoning as bsp_sdram.c: the vendored HAL version matches no local CubeH7
 *  package, and CubeMX never copied the HAL_QSPI module into the project at
 *  all.
 */

#include "bsp_qspi.h"
#include "bsp_mpu.h"
#include "stm32h7xx_hal.h"

/* W25Qxx commands */
#define CMD_WRITE_ENABLE        0x06U
#define CMD_READ_STATUS_REG2    0x35U
#define CMD_WRITE_STATUS_REG2   0x31U
#define CMD_READ_JEDEC_ID       0x9FU
#define CMD_FAST_READ_QUAD_OUT  0x6BU
#define CMD_READ_STATUS_REG1    0x05U
#define CMD_PAGE_PROGRAM        0x02U
#define CMD_SECTOR_ERASE_4K     0x20U
#define CMD_CHIP_ERASE          0xC7U

/* Status register 1, bit 0. Set while an erase or program is in progress. */
#define SR1_BUSY                0x01U

/*
 * Erase and program are slow in a way reads are not, and the datasheet limits
 * are far above the typical figures: a 4 KB sector erase is usually tens of
 * milliseconds but may take 400 ms, and a chip erase may take 100 s. Timing out
 * early would leave the chip mid-operation, so these are generous.
 */
#define QSPI_ERASE_SECTOR_TIMEOUT_MS   2000U
#define QSPI_ERASE_CHIP_TIMEOUT_MS   200000U
#define QSPI_PROGRAM_TIMEOUT_MS         100U

/*
 * Reads use 0x6B (Fast Read Quad Output) rather than the faster 0xEB
 * (Fast Read Quad I/O).
 *
 * 0x6B is 1-1-4: command and address on a single line, only data on four,
 * with a fixed 8 dummy cycles and no alternate byte. 0xEB sends the address
 * on four lines plus an M7-M0 mode byte, and its dummy cycle count varies with
 * frequency and vendor settings - get it wrong and you read pure garbage,
 * which is painful to diagnose.
 *
 * For bulk image reads the address phase is negligible; the data phase runs at
 * full quad speed either way. Take the reliable one.
 */
#define QSPI_DUMMY_CYCLES       8U

/*
 * The QUADSPI kernel clock defaults to hclk3 = 240 MHz; divide by 4 for 60 MHz.
 *
 * Most W25Q parts are rated above 80 MHz for Fast Read Quad Output, so 60 MHz
 * leaves margin. This path is a fallback and stability matters more than peak
 * throughput. Four lines at 60 MHz is roughly 30 MB/s, about 9 ms for a
 * full-screen 261 KB image - fine for a page change.
 * Lower the prescaler later if more speed is genuinely needed.
 */
#define QSPI_PRESCALER          3U      /* divider = PRESCALER + 1 = 4 */

#define QSPI_TIMEOUT_MS         100U

/*
 * Deliberately non-static so a debugger can watch them by name without a file
 * qualifier. Nothing has ever confirmed this chip responds on real hardware -
 * the pinout came from the vendor example and the timings are calculated - so
 * being able to type "g_qspi_jedec_id" into a watch window is the whole point.
 *
 *   0xEF4017 = W25Q64  (8 MB)
 *   0xEF4018 = W25Q128 (16 MB)
 *   0 or 0xFFFFFF     = nothing answered
 */
uint32_t g_qspi_jedec_id;
uint32_t g_qspi_flash_size;

static void qspi_gpio_init(void);
static bool  qspi_wait_not_busy(void);
static bool  qspi_read_jedec_id(uint32_t *id);
static bool  qspi_enable_quad_mode(void);
static void  qspi_enable_memory_mapped(void);

bool BSP_QSPI_Init(void)
{
    g_qspi_flash_size = 0;
    g_qspi_jedec_id = 0;

    qspi_gpio_init();
    __HAL_RCC_QSPI_CLK_ENABLE();

    /* Disable QUADSPI before configuring so no stale state survives */
    QUADSPI->CR = 0;

    QUADSPI->CR = (QSPI_PRESCALER << QUADSPI_CR_PRESCALER_Pos)
                | (3U << QUADSPI_CR_FTHRES_Pos);          /* FIFO threshold 4 bytes */

    /*
     * Start with FSIZE at maximum (2^32). The real capacity is only known after
     * reading the JEDEC ID, and reading it requires a working QUADSPI, so a
     * generous provisional value goes in first.
     */
    QUADSPI->DCR = (31U << QUADSPI_DCR_FSIZE_Pos)
                 | (7U  << QUADSPI_DCR_CSHT_Pos);         /* minimum CS high time, 8 cycles */

    QUADSPI->CR |= QUADSPI_CR_EN;

    if (!qspi_read_jedec_id(&g_qspi_jedec_id)) {
        return false;
    }

    /*
     * The third JEDEC ID byte is the capacity as a power of two:
     * W25Q64 = 0xEF4017 -> 2^23 = 8 MB, W25Q128 = 0xEF4018 -> 2^24 = 16 MB.
     */
    const uint8_t capacity_exp = (uint8_t)(g_qspi_jedec_id & 0xFFU);
    if (capacity_exp < 16U || capacity_exp > 25U) {
        /* Not a plausible capacity code - most likely no chip responded
         * (all zeros or all ones) */
        return false;
    }
    g_qspi_flash_size = 1UL << capacity_exp;

    /* Re-set FSIZE from the real capacity */
    QUADSPI->CR &= ~QUADSPI_CR_EN;
    QUADSPI->DCR = ((uint32_t)(capacity_exp - 1U) << QUADSPI_DCR_FSIZE_Pos)
                 | (7U << QUADSPI_DCR_CSHT_Pos);
    QUADSPI->CR |= QUADSPI_CR_EN;

    if (!qspi_enable_quad_mode()) {
        g_qspi_flash_size = 0;
        return false;
    }

    qspi_enable_memory_mapped();

    /* The mapped window can only be opened once the capacity is known,
     * otherwise the MPU blocks every read */
    BSP_MPU_EnableQspiRegion(g_qspi_flash_size);

    return true;
}

uint32_t BSP_QSPI_GetFlashSize(void)
{
    return g_qspi_flash_size;
}

const uint8_t *BSP_QSPI_GetMappedBase(void)
{
    return (const uint8_t *)QSPI_BASE_ADDR;
}

uint32_t BSP_QSPI_GetJedecId(void)
{
    return g_qspi_jedec_id;
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

/* Wait for the transfer complete flag (TCF), then clear it */
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

    QUADSPI->DLR = 3U - 1U;                                  /* read 3 bytes */
    QUADSPI->CCR = (1U << QUADSPI_CCR_IMODE_Pos)             /* command on one line */
                 | (1U << QUADSPI_CCR_DMODE_Pos)             /* data on one line */
                 | (1U << QUADSPI_CCR_FMODE_Pos)             /* indirect read */
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
 * 0x6B needs the flash's Quad Enable bit set first, otherwise IO2 and IO3 are
 * never driven and reads come back as garbage. On W25Q parts QE is bit 1 of
 * status register 2.
 */
static bool qspi_enable_quad_mode(void)
{
    /* --- read SR2 first --- */
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
        return true;    /* QE already set, no need to write it again */
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

    /* --- write SR2 back with QE set --- */
    if (!qspi_wait_not_busy()) {
        return false;
    }

    QUADSPI->DLR = 0U;
    QUADSPI->CCR = (1U << QUADSPI_CCR_IMODE_Pos)
                 | (1U << QUADSPI_CCR_DMODE_Pos)
                 | (0U << QUADSPI_CCR_FMODE_Pos)              /* indirect write */
                 | (CMD_WRITE_STATUS_REG2 << QUADSPI_CCR_INSTRUCTION_Pos);

    *(volatile uint8_t *)&QUADSPI->DR = (uint8_t)(sr2 | 0x02U);

    if (!qspi_wait_transfer_complete()) {
        return false;
    }

    /* Writing the status register is a non-volatile operation; the chip needs
     * a moment internally */
    HAL_Delay(10);

    return qspi_wait_not_busy();
}

static void qspi_enable_memory_mapped(void)
{
    QUADSPI->CCR = (1U << QUADSPI_CCR_IMODE_Pos)              /* command on one line */
                 | (1U << QUADSPI_CCR_ADMODE_Pos)             /* address on one line */
                 | (2U << QUADSPI_CCR_ADSIZE_Pos)             /* 24-bit address */
                 | (3U << QUADSPI_CCR_DMODE_Pos)              /* data on four lines */
                 | (QSPI_DUMMY_CYCLES << QUADSPI_CCR_DCYC_Pos)
                 | (3U << QUADSPI_CCR_FMODE_Pos)              /* memory-mapped */
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

    /* PB2 CLK on AF9 */
    gpio.Pin       = GPIO_PIN_2;
    gpio.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOB, &gpio);

    /* PB6 NCS on AF10 */
    gpio.Pin       = GPIO_PIN_6;
    gpio.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIOB, &gpio);

    /* PF6 IO3 / PF7 IO2 on AF9 */
    gpio.Pin       = GPIO_PIN_6 | GPIO_PIN_7;
    gpio.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOF, &gpio);

    /* PF8 IO0 / PF9 IO1 on AF10 */
    gpio.Pin       = GPIO_PIN_8 | GPIO_PIN_9;
    gpio.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIOF, &gpio);
}


/* ------------------------------------------------------------------------- */
/* Erase and program                                                          */
/*                                                                            */
/* Init leaves the peripheral in memory-mapped mode, which cannot issue        */
/* commands. Every operation below therefore aborts back to indirect mode,     */
/* does its work, and restores the mapping - see mmap_leave()/mmap_enter().    */
/* ------------------------------------------------------------------------- */

static void mmap_leave(void)
{
    /* ABORT is the documented way out of memory-mapped mode; it self-clears. */
    QUADSPI->CR |= QUADSPI_CR_ABORT;
    while ((QUADSPI->CR & QUADSPI_CR_ABORT) != 0U) {
        /* wait */
    }
    QUADSPI->FCR = QUADSPI_FCR_CTOF | QUADSPI_FCR_CTCF | QUADSPI_FCR_CSMF | QUADSPI_FCR_CTEF;
}

static void mmap_enter(void)
{
    qspi_enable_memory_mapped();

    /*
     * The mapped window is cacheable, so anything just written would otherwise
     * be read back from stale cache lines. Invalidate the whole window rather
     * than tracking ranges - it happens once per operation and costs far less
     * than the erase that preceded it.
     */
    if (g_qspi_flash_size != 0U) {
        SCB_InvalidateDCache_by_Addr((uint32_t *)QSPI_BASE_ADDR, (int32_t)g_qspi_flash_size);
    }
}

/** Issue a single command with no address and no data. */
static bool qspi_simple_cmd(uint32_t instruction)
{
    if (!qspi_wait_not_busy()) {
        return false;
    }

    QUADSPI->CCR = (1U << QUADSPI_CCR_IMODE_Pos)
                 | (instruction << QUADSPI_CCR_INSTRUCTION_Pos);

    return qspi_wait_transfer_complete();
}

/** Poll status register 1 until BUSY clears. */
static bool qspi_wait_write_complete(uint32_t timeout_ms)
{
    const uint32_t start = HAL_GetTick();

    for (;;) {
        if (!qspi_wait_not_busy()) {
            return false;
        }

        QUADSPI->DLR = 0U;                                   /* 1 byte */
        QUADSPI->CCR = (1U << QUADSPI_CCR_IMODE_Pos)
                     | (1U << QUADSPI_CCR_DMODE_Pos)
                     | (1U << QUADSPI_CCR_FMODE_Pos)         /* indirect read */
                     | (CMD_READ_STATUS_REG1 << QUADSPI_CCR_INSTRUCTION_Pos);

        const uint32_t poll_start = HAL_GetTick();
        while ((QUADSPI->SR & QUADSPI_SR_FTF) == 0U &&
               (QUADSPI->SR & QUADSPI_SR_TCF) == 0U) {
            if ((HAL_GetTick() - poll_start) > QSPI_TIMEOUT_MS) {
                return false;
            }
        }

        const uint8_t sr1 = *(volatile uint8_t *)&QUADSPI->DR;

        if (!qspi_wait_transfer_complete()) {
            return false;
        }

        if ((sr1 & SR1_BUSY) == 0U) {
            return true;
        }

        if ((HAL_GetTick() - start) > timeout_ms) {
            return false;
        }
    }
}

bool BSP_QSPI_EraseSector(uint32_t addr)
{
    if (g_qspi_flash_size == 0U || addr >= g_qspi_flash_size) {
        return false;
    }

    mmap_leave();

    bool ok = qspi_simple_cmd(CMD_WRITE_ENABLE);

    if (ok) {
        ok = qspi_wait_not_busy();
    }

    if (ok) {
        QUADSPI->CCR = (1U << QUADSPI_CCR_IMODE_Pos)
                     | (1U << QUADSPI_CCR_ADMODE_Pos)
                     | (2U << QUADSPI_CCR_ADSIZE_Pos)        /* 24-bit address */
                     | (CMD_SECTOR_ERASE_4K << QUADSPI_CCR_INSTRUCTION_Pos);
        QUADSPI->AR = addr & ~(QSPI_SECTOR_SIZE - 1U);
        ok = qspi_wait_transfer_complete();
    }

    if (ok) {
        ok = qspi_wait_write_complete(QSPI_ERASE_SECTOR_TIMEOUT_MS);
    }

    mmap_enter();
    return ok;
}

bool BSP_QSPI_EraseChip(void)
{
    if (g_qspi_flash_size == 0U) {
        return false;
    }

    mmap_leave();

    bool ok = qspi_simple_cmd(CMD_WRITE_ENABLE);

    if (ok) {
        ok = qspi_simple_cmd(CMD_CHIP_ERASE);
    }

    if (ok) {
        ok = qspi_wait_write_complete(QSPI_ERASE_CHIP_TIMEOUT_MS);
    }

    mmap_enter();
    return ok;
}

/** Program at most one page, without crossing its boundary. */
static bool qspi_program_page(uint32_t addr, const uint8_t *data, uint32_t len)
{
    if (!qspi_simple_cmd(CMD_WRITE_ENABLE)) {
        return false;
    }

    if (!qspi_wait_not_busy()) {
        return false;
    }

    QUADSPI->DLR = len - 1U;
    QUADSPI->CCR = (1U << QUADSPI_CCR_IMODE_Pos)
                 | (1U << QUADSPI_CCR_ADMODE_Pos)
                 | (2U << QUADSPI_CCR_ADSIZE_Pos)
                 | (1U << QUADSPI_CCR_DMODE_Pos)
                 | (0U << QUADSPI_CCR_FMODE_Pos)             /* indirect write */
                 | (CMD_PAGE_PROGRAM << QUADSPI_CCR_INSTRUCTION_Pos);
    QUADSPI->AR = addr;

    for (uint32_t i = 0U; i < len; i++) {
        const uint32_t start = HAL_GetTick();
        while ((QUADSPI->SR & QUADSPI_SR_FTF) == 0U) {
            if ((HAL_GetTick() - start) > QSPI_TIMEOUT_MS) {
                return false;
            }
        }
        *(volatile uint8_t *)&QUADSPI->DR = data[i];
    }

    if (!qspi_wait_transfer_complete()) {
        return false;
    }

    return qspi_wait_write_complete(QSPI_PROGRAM_TIMEOUT_MS);
}

bool BSP_QSPI_Program(uint32_t addr, const uint8_t *data, uint32_t len)
{
    if (g_qspi_flash_size == 0U || data == NULL) {
        return false;
    }

    if ((addr + len) > g_qspi_flash_size) {
        return false;
    }

    mmap_leave();

    bool ok = true;
    uint32_t done = 0U;

    while (ok && done < len) {
        /* A page program wraps within its page instead of continuing into the
         * next one, so the first chunk is only as long as the remainder of the
         * current page. */
        const uint32_t page_left = QSPI_PAGE_SIZE - ((addr + done) % QSPI_PAGE_SIZE);
        uint32_t chunk = len - done;
        if (chunk > page_left) {
            chunk = page_left;
        }

        ok = qspi_program_page(addr + done, &data[done], chunk);
        done += chunk;
    }

    mmap_enter();
    return ok;
}

bool BSP_QSPI_Read(uint32_t addr, uint8_t *data, uint32_t len)
{
    if (g_qspi_flash_size == 0U || data == NULL) {
        return false;
    }

    if ((addr + len) > g_qspi_flash_size) {
        return false;
    }

    const uint8_t *mapped = (const uint8_t *)(QSPI_BASE_ADDR + addr);
    for (uint32_t i = 0U; i < len; i++) {
        data[i] = mapped[i];
    }

    return true;
}

bool BSP_QSPI_SelfTestWrite(void)
{
    if (g_qspi_flash_size < QSPI_SECTOR_SIZE) {
        return false;
    }

    /* Last sector: least likely to hold anything wanted. */
    const uint32_t addr = g_qspi_flash_size - QSPI_SECTOR_SIZE;

    static uint8_t pattern[QSPI_PAGE_SIZE];
    static uint8_t readback[QSPI_PAGE_SIZE];

    for (uint32_t i = 0U; i < QSPI_PAGE_SIZE; i++) {
        pattern[i] = (uint8_t)(i ^ 0xA5U);
    }

    if (!BSP_QSPI_EraseSector(addr)) {
        return false;
    }

    /* Erased flash reads 0xFF; if it does not, the erase silently failed. */
    if (!BSP_QSPI_Read(addr, readback, QSPI_PAGE_SIZE)) {
        return false;
    }
    for (uint32_t i = 0U; i < QSPI_PAGE_SIZE; i++) {
        if (readback[i] != 0xFFU) {
            return false;
        }
    }

    if (!BSP_QSPI_Program(addr, pattern, QSPI_PAGE_SIZE)) {
        return false;
    }

    if (!BSP_QSPI_Read(addr, readback, QSPI_PAGE_SIZE)) {
        return false;
    }
    for (uint32_t i = 0U; i < QSPI_PAGE_SIZE; i++) {
        if (readback[i] != pattern[i]) {
            return false;
        }
    }

    return true;
}
