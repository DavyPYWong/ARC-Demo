#ifndef MLX90615_H_
#define MLX90615_H_

#include <stdint.h>

#include "embARC.h"
#include "embARC_debug.h"
#include "embARC_error.h"

typedef enum { MLX90615_EEPROM_ACCESS = 0, MLX90615_RAM_ACCESS = 1 } MLX90615_ACCESS_TYPE_T;
typedef enum {
  MLX90615_STATUS_UNINITIALIZED = 0,
  MLX90615_STATUS_INITIALIZED = 1
} MLX90615_STATUS_T;

typedef struct MLX90615_INFO_S {
  int32_t iic_dev_id;
  uint8_t iic_addr;
  DEV_IIC_PTR iic_dev_ptr;
  MLX90615_STATUS_T status;
} MLX90615_INFO_t;

typedef struct MLX90615_S {
  MLX90615_INFO_t info;
  int32_t (*mlx90615_read)(MLX90615_ACCESS_TYPE_T access_type, const uint8_t addr, uint16_t *data);
  int32_t (*mlx90615_write)(MLX90615_ACCESS_TYPE_T access_type, const uint8_t addr, uint16_t data);
} MLX90615_OBJ, *MLX90615_OBJ_PTR;

uint8_t CRC8_CCITT(const uint8_t *data, uint32_t len);
MLX90615_OBJ_PTR mlx90615_getobj(int32_t iic_dev_id, uint32_t mlx90615_iic_addr);
uint16_t emissivityConversion(float emissivity);

#endif