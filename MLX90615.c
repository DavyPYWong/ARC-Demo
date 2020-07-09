#include "MLX90615.h"

#include <stdint.h>

#include "dev_iic.h"
#include "embARC.h"
#include "embARC_debug.h"
#include "embARC_error.h"

#define MLX90615_CHECK_EXP_NORTN(EXPR) CHECK_EXP_NOERCD(EXPR, error_exit)

// extern DEV_IIC_PTR mlx90615_iic;
static MLX90615_OBJ mlx90615_context = {.info = {.iic_dev_id = 0,
                                                 .iic_addr = 0,
                                                 .iic_dev_ptr = NULL,
                                                 .status = MLX90615_STATUS_UNINITIALIZED},
                                        .mlx90615_read = NULL};

static int32_t mlx90615_read_internal(MLX90615_ACCESS_TYPE_T access_type, const uint8_t addr,
                                      uint16_t *data);
static int32_t mlx90615_write_internal(MLX90615_ACCESS_TYPE_T access_type, const uint8_t addr,
                                       uint16_t data);

static const uint8_t CRC8_table[] = {
    0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
    0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65, 0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
    0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5, 0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
    0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
    0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2, 0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
    0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2, 0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
    0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
    0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42, 0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
    0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
    0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
    0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C, 0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
    0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
    0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
    0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, 0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
    0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
    0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3};

uint8_t CRC8_CCITT(const uint8_t *input, uint32_t len) {
  uint8_t data = 0, remainder = 0;
  for (uint32_t i = 0; i < len; i++) {
    data = input[i] ^ remainder;
    remainder = CRC8_table[data];
  }
  return remainder;
}

MLX90615_OBJ_PTR mlx90615_getobj(int32_t iic_dev_id, uint32_t mlx90615_iic_addr) {
  int32_t ercd = E_OK;
  DEV_IIC_PTR iic_ptr = iic_get_dev(iic_dev_id);
  MLX90615_CHECK_EXP_NORTN(iic_ptr != NULL);
  ercd = iic_ptr->iic_open(DEV_MASTER_MODE, IIC_SPEED_STANDARD);
  if (ercd != E_OK) goto error_exit;
  ercd = iic_ptr->iic_control(IIC_CMD_MST_SET_TAR_ADDR, (void *)(mlx90615_iic_addr));
  if (ercd != E_OK) goto error_exit;

  mlx90615_context.info.iic_dev_ptr = iic_ptr;
  mlx90615_context.info.iic_dev_id = iic_dev_id;
  mlx90615_context.info.iic_addr = mlx90615_iic_addr;
  mlx90615_context.info.status = MLX90615_STATUS_INITIALIZED;

  mlx90615_context.mlx90615_read = mlx90615_read_internal;
  mlx90615_context.mlx90615_write = mlx90615_write_internal;

  return &mlx90615_context;
error_exit:
  return NULL;
}

int32_t mlx90615_read_internal(MLX90615_ACCESS_TYPE_T access_type, const uint8_t addr,
                               uint16_t *data) {
  DEV_IIC_PTR mlx90615_iic = mlx90615_context.info.iic_dev_ptr;
  uint8_t cmd = 0;
  uint8_t buf[3] = {0};

  if (mlx90615_context.info.status == MLX90615_STATUS_UNINITIALIZED) return -1;

  if (access_type == MLX90615_EEPROM_ACCESS) {
    cmd = 0x10;
  } else if (access_type == MLX90615_RAM_ACCESS) {
    cmd = 0x20;
  } else {
    return -1;
  }
  cmd |= (addr & 0x0F);

  mlx90615_iic->iic_control(IIC_CMD_MST_SET_NEXT_COND, (void *)(IIC_MODE_RESTART));
  mlx90615_iic->iic_write(&cmd, 1);
  mlx90615_iic->iic_control(IIC_CMD_MST_SET_NEXT_COND, (void *)(IIC_MODE_STOP));
  mlx90615_iic->iic_read(buf, 3);

  *data = 0;
  *data = buf[1];
  *data <<= 8;
  *data |= buf[0];

  return E_OK;
}

static int32_t mlx90615_write_internal(MLX90615_ACCESS_TYPE_T access_type, const uint8_t addr,
                                       uint16_t data) {
  if (access_type != MLX90615_EEPROM_ACCESS) return -1;  // Support EEPROM write only
  if (addr > 0x03) return -1;                            // Protect factory calibration data
  DEV_IIC_PTR mlx90615_iic = mlx90615_context.info.iic_dev_ptr;
  uint8_t cmd = 0;
  uint8_t pec_calc[4] = {0};
  uint8_t buf[4] = {0};

  if (mlx90615_context.info.status == MLX90615_STATUS_UNINITIALIZED) return -1;

  if (access_type == MLX90615_EEPROM_ACCESS) {
    cmd = 0x10;
  } else if (access_type == MLX90615_RAM_ACCESS) {
    cmd = 0x20;
  } else {
    return -1;
  }
  cmd |= (addr & 0x0F);

  // Erase EEPROM first
  pec_calc[0] = (mlx90615_context.info.iic_addr << 1);
  pec_calc[1] = cmd;
  pec_calc[2] = 0;
  pec_calc[3] = 0;
  buf[0] = cmd;
  buf[1] = pec_calc[2];
  buf[2] = pec_calc[3];
  buf[3] = CRC8_CCITT(pec_calc, 4);

  mlx90615_iic->iic_control(IIC_CMD_MST_SET_NEXT_COND, (void *)(IIC_MODE_STOP));
  mlx90615_iic->iic_write(buf, 4);

  board_delay_ms(10, 1);  // EEPROM write needs time

  pec_calc[0] = (mlx90615_context.info.iic_addr << 1);
  pec_calc[1] = cmd;
  pec_calc[2] = data & 0xFF;
  pec_calc[3] = (data >> 8) & 0xFF;
  buf[0] = cmd;
  buf[1] = pec_calc[2];
  buf[2] = pec_calc[3];
  buf[3] = CRC8_CCITT(pec_calc, 4);

  mlx90615_iic->iic_control(IIC_CMD_MST_SET_NEXT_COND, (void *)(IIC_MODE_STOP));
  int ret = mlx90615_iic->iic_write(buf, 4);
  if (ret < 0) {
    return ret;
  } else {
    board_delay_ms(10, 1);  // EEPROM write needs time
    return E_OK;
  }
}

uint16_t emissivityConversion(float emissivity) {
  if (emissivity < 0) return 0;
  if (emissivity > 1.0) emissivity = 1.0;
  return emissivity * 16384;
}
