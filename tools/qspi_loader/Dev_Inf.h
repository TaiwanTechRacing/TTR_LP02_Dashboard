/*
 * Dev_Inf.h
 *
 *  Device descriptor layout that STM32CubeProgrammer reads out of an external
 *  loader. The field order and the section name are fixed by the tool - it
 *  looks up the symbol "StorageInfo" in the .Dev_Info section and parses these
 *  structures directly, so nothing here can be reordered or renamed.
 */

#ifndef DEV_INF_H
#define DEV_INF_H

#include <stdint.h>

#define MCU_FLASH        1
#define NAND_FLASH       2
#define NOR_FLASH        3
#define SRAM             4
#define PSRAM            5
#define PC_CARD          6
#define SPI_FLASH        7
#define I2C_FLASH        8
#define SDRAM            9
#define I2C_EEPROM      10

/* CubeProgrammer stops reading sector descriptors at the first all-zero entry,
 * so the array needs a terminator even when only one description is used. */
#define SECTOR_NUM      10

struct DeviceSectors {
    uint32_t SectorNum;     /* how many sectors of this size */
    uint32_t SectorSize;    /* bytes */
};

struct StorageInfo {
    char     DeviceName[100];
    uint16_t DeviceType;
    uint32_t DeviceStartAddress;
    uint32_t DeviceSize;
    uint32_t PageSize;
    uint8_t  EraseValue;
    struct DeviceSectors sectors[SECTOR_NUM];
};

#endif /* DEV_INF_H */
