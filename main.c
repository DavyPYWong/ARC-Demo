// Sensors
#include "bme680.h"
#include "MLX90615.h"

// Protocol
#include "dev_gpio.h"
#include "dev_iic.h"
#include "dev_uart.h"

// IoTDK+std
#include "embARC.h"
#include "embARC_debug.h"
#include "embARC_error.h"
#include "stdio.h"

// Define Thermal Sensor 
#define MLX90615_IIC_ADDR 0x5B
#define MLX90615_CHECK_EXP_NORTN(EXPR) CHECK_EXP_NOERCD(EXPR, error_exit)
#define GPIO_CHECK_EXP_NORTN(EXPR) CHECK_EXP_NOERCD(EXPR, error_exit)
//DEV_GPIO_PTR gpio4b2 = NULL;
#define SW0_OFFSET 3
#define SW0_MASK (0x1) << SW0_OFFSET

// Define IAQ
#define UART_CHECK_EXP_NORTN(EXPR) CHECK_EXP_NOERCD(EXPR, error_exit)
#define GPIO_CHECK_EXP_NORTN(EXPR) CHECK_EXP_NOERCD(EXPR, error_exit)
//DEV_GPIO_PTR gpio4b2 = NULL;
DEV_UART_PTR uart_obj = NULL;
#define SW0_OFFSET 3
#define SW0_MASK (0x1) << SW0_OFFSET
#define BME680_UART_ID DFSS_UART_1_ID
#define BME680_BAUD 9600

int main(void) 
{
  // Global initialisation
  int32_t ercd = E_OK;

  // *** Initialise Thermal Sensor ***
  // *** Initialise Thermal Sensor ***
  //const double emissivityValue = 0.95;
  MLX90615_OBJ_PTR mlx90615_obj_ptr = mlx90615_getobj(0, MLX90615_IIC_ADDR);
  //EMBARC_PRINTF("Write %04X to EEPROM\r\n", emissivityConversion(emissivityValue));
  //int ret =
  //    mlx90615_obj_ptr->mlx90615_write(MLX90615_EEPROM_ACCESS, 0x03, emissivityConversion(emissivityValue));
  //EMBARC_PRINTF("ret: %d\r\n", ret);
  //uint16_t eCheck = 0;
  //mlx90615_obj_ptr->mlx90615_read(MLX90615_EEPROM_ACCESS, 0x03, &eCheck);
  //EMBARC_PRINTF("Emissivity readback: %04X\r\n", eCheck);
  char cBuf[64] = {0};
  uint16_t result = 0;
  float TO = 0.0, TA = 0.0;

  // *** Initialise IAQ ***
  // *** Initialise IAQ ***

  uart_obj = uart_get_dev(BME680_UART_ID);
  UART_CHECK_EXP_NORTN(uart_obj != NULL);

  /**open uart port, if already opened, then set baudrate*/
  if (uart_obj->uart_open(BME680_BAUD) == E_OPNED) {
    ercd = uart_obj->uart_control(UART_CMD_SET_BAUD, (void *)(BME680_BAUD));
  }

  uint8_t cmd1[] = {0xA5, 0x55, 0x3F, 0x00};       // Output all the data
  for (int i = 0; i < 3; i++) cmd1[3] += cmd1[i];  // Calculate checksum
  uart_obj->uart_write((void *)cmd1, sizeof(cmd1));
  //board_delay_ms(100, 1);

  uint8_t cmd2[] = {0xA5, 0x56, 0x02, 0x00};       // Continuous output
  for (int i = 0; i < 3; i++) cmd2[3] += cmd2[i];  // Calculate checksum
  uart_obj->uart_write((void *)cmd2, sizeof(cmd2));
  //board_delay_ms(100, 1);

  while (1) 
  {
    // *** Loop Thermal Sensor ***
    // *** Loop Thermal Sensor ***
    mlx90615_obj_ptr->mlx90615_read(MLX90615_RAM_ACCESS, 0x07, &result);
    TO = (float)result * 0.02 - 273.15;
    mlx90615_obj_ptr->mlx90615_read(MLX90615_RAM_ACCESS, 0x06, &result);
    TA = (float)result * 0.02 - 273.15;
    //snprintf(cBuf, sizeof(cBuf), "TO (Object): %.2f C, TA (Sensor): %.2f C", TO, TA);
    EMBARC_PRINTF("*** Temperature ***\n");
    snprintf(cBuf, sizeof(cBuf), "Object Temperature: %.2f C", TO);
    EMBARC_PRINTF("%s\r\n", cBuf);
    
    // *** Loop IAQ ***
    // *** Loop IAQ ***
    uint8_t buf[32] = {0};  // Store entire frame without leading 0x5A5A
    uint8_t current_pos = 2;
    uint8_t data_pos[BME680_NUM_OF_DATATYPES] = {0};
    // Frame start
    uart_obj->uart_read((void *)buf, 1);
    if (buf[0] != 0x5A) continue;
    uart_obj->uart_read((void *)buf, 1);
    if (buf[0] != 0x5A) continue;
    // Receive data indicator and total data length
    uart_obj->uart_read((void *)buf, 2);
    // buf[1] is the total data length. Should not be greater than 15
    if (buf[1] > 15) continue;
    // Receive data
    uart_obj->uart_read((void *)(&(buf[2])), buf[1] + 1);  // 1 byte for checksum
    // Extract data
    for (int i = 0; i < BME680_NUM_OF_DATATYPES; i++) 
    {
      if (buf[0] & (1 << i)) 
      {
        data_pos[i] = current_pos;
        current_pos += BME680_DATA_LENGTH[i];
      }
    }

    if (data_pos[BME680_TEMPERATURE_INDEX] != 0) {  // Precense of temperature
      uint16_t temp = 0;
      temp |= buf[data_pos[BME680_TEMPERATURE_INDEX]];
      temp <<= 8;
      temp |= buf[data_pos[BME680_TEMPERATURE_INDEX] + 1];

      EMBARC_PRINTF("Environment temperature: %d.%d\r\n\n", temp / 100, temp % 100);
    }
    EMBARC_PRINTF("*** Air quality ***\n");
    if (data_pos[BME680_IAQ_INDEX] != 0) {  // Precense of IAQ
      uint8_t precision = 0;
      uint16_t iaq = 0;
      precision = buf[data_pos[BME680_IAQ_INDEX]] >> 4;
      iaq |= (buf[data_pos[BME680_IAQ_INDEX]] & 0x0F);
      iaq <<= 8;
      iaq |= buf[data_pos[BME680_IAQ_INDEX] + 1];

      EMBARC_PRINTF("IAQ precision: %d, IAQ: %d\r\n", precision, iaq);
    }
    if (data_pos[BME680_GAS_INDEX] != 0) {  // Precense of GAS
      uint32_t GAS_ohm = 0;
      GAS_ohm |= buf[data_pos[BME680_GAS_INDEX]];
      GAS_ohm <<= 8;
      GAS_ohm |= buf[data_pos[BME680_GAS_INDEX] + 1];
      GAS_ohm <<= 8;
      GAS_ohm |= buf[data_pos[BME680_GAS_INDEX] + 2];
      GAS_ohm <<= 8;
      GAS_ohm |= buf[data_pos[BME680_GAS_INDEX] + 3];

      EMBARC_PRINTF("GAS sensor resistance: %d\r\n", GAS_ohm);
    }
    EMBARC_PRINTF("\r\n");
    board_delay_ms(500, 1);
  }

error_exit:
  return ercd;
}
