#ifndef BME680_H_
#define BME680_H_

#include <stdint.h>

#include "embARC.h"
#include "embARC_debug.h"
#include "embARC_error.h"

#define BME680_NUM_OF_DATATYPES 6

// data presence indicator bit definition in GYMCU680 custom protocol
#define BME680_TEMPERATURE_INDEX 0
#define BME680_HUMIDITY_INDEX 1
#define BME680_PRESSURE_INDEX 2
#define BME680_IAQ_INDEX 3
#define BME680_GAS_INDEX 4
#define BME680_ALTITUDE_INDEX 5

// data length in byte defined in GYMCU680 custom protocol
const uint8_t BME680_DATA_LENGTH[] = {2, 2, 3, 2, 4, 2};

#endif