#ifndef __MCP9600_H__
#define __MCP9600_H__

#include <em_device.h>
#include "i2c.h"
#include "atomic.h"
#include "systick.h"

#define MCP9600_I2C_ADDR 0x60
//#define MCP9600_I2C_ADDR 0x61
//#define MCP9600_I2C_ADDR 0x62
//#define MCP9600_I2C_ADDR 0x63
//#define MCP9600_I2C_ADDR 0x64
//#define MCP9600_I2C_ADDR 0x65
//#define MCP9600_I2C_ADDR 0x66
//#define MCP9600_I2C_ADDR 0x67

#define MCP9600_DEV_ID 0x40

#define MCP9600_REG_HJT 0x00        // Hot-Junction Temperature – TH
#define MCP9600_REG_JTD 0x01        // Junctions Temperature Delta – T∆
#define MCP9600_REG_CJT 0x02        // Cold-Junction Temperature – TC

#define MCP9600_REG_ADC 0x03        // Raw Data ADC

#define MCP9600_REG_STAT 0x04       // STATUS
#define MCP9600_REG_TCFG 0x05       // Thermocouple Sensor Configuration
#define MCP9600_REG_CFG 0x06        // Device Configuration

#define MCP9600_REG_ALRT_CFG 0x08   // Alert Configuration Mask
#define MCP9600_REG_ALRT1CFG 0x08   // Alert 1 Configuration
#define MCP9600_REG_ALRT2CFG 0x09   // Alert 2 Configuration
#define MCP9600_REG_ALRT3CFG 0x0A   // Alert 3 Configuration
#define MCP9600_REG_ALRT4CFG 0x0B   // Alert 4 Configuration

#define MCP9600_REG_ALRT_HYS 0x0C   // Alert Hysteresis mask
#define MCP9600_REG_ALRT1HYS 0x0C   // Alert 1 Hysteresis
#define MCP9600_REG_ALRT2HYS 0x0D   // Alert 2 Hysteresis
#define MCP9600_REG_ALRT3HYS 0x0E   // Alert 3 Hysteresis
#define MCP9600_REG_ALRT4HYS 0x0F   // Alert 4 Hysteresis

#define MCP9600_REG_ALRT_LIM 0x10   // Alert Limit Mask
#define MCP9600_REG_ALRT1LIM 0x10   // Alert 1 Limit
#define MCP9600_REG_ALRT2LIM 0x11   // Alert 2 Limit
#define MCP9600_REG_ALRT3LIM 0x12   // Alert 3 Limit
#define MCP9600_REG_ALRT4LIM 0x13   // Alert 4 Limit

#define MCP9600_REG_ID 0x20         // Device ID/Revision

// MCP9600 status register
#define MCP9600_BURST_COMP  0x80    // Burst Mode Conversions Status Flag bit
#define MCP9600_TH_UPDT     0x40    // Temperature Update Flag bit
#define MCP9600_TIN_RNG     0x10    // Temperature Range Detection bit (read-only)
#define MCP9600_ALRT4       0x08    // Alert 4: Status bit (read-only)
#define MCP9600_ALRT3       0x04    // Alert 3: Status bit (read-only)
#define MCP9600_ALRT2       0x02    // Alert 2: Status bit (read-only)
#define MCP9600_ALRT1       0x01    // Alert 1: Status bit (read-only)

// MCP9600 sensor config register
#define MCP9600_TYPE_K      0x00    // Thermocouple Type Select bits
#define MCP9600_TYPE_J      0x10    // Thermocouple Type Select bits
#define MCP9600_TYPE_T      0x20    // Thermocouple Type Select bits
#define MCP9600_TYPE_N      0x30    // Thermocouple Type Select bits
#define MCP9600_TYPE_S      0x40    // Thermocouple Type Select bits
#define MCP9600_TYPE_E      0x50    // Thermocouple Type Select bits
#define MCP9600_TYPE_B      0x60    // Thermocouple Type Select bits
#define MCP9600_TYPE_R      0x70    // Thermocouple Type Select bits

#define MCP9600_FILT_COEF_0 0x00    // Filter off
#define MCP9600_FILT_COEF_1 0x01    // Minimum filter
#define MCP9600_FILT_COEF_2 0x02    //
#define MCP9600_FILT_COEF_3 0x03    //
#define MCP9600_FILT_COEF_4 0x04    // Mid filter
#define MCP9600_FILT_COEF_5 0x05    //
#define MCP9600_FILT_COEF_6 0x06    //
#define MCP9600_FILT_COEF_7 0x07    // Maximum filter

// MCP9600 config register
#define MCP9600_CJ_RES      0x80    // Cold-Junction Resolution bit

#define MCP9600_ADC_RES     5       // ADC Resolution bits offset
#define MCP9600_ADC_RES_18b 0x00    // ADC Resolution bits - 18-bit Resolution
#define MCP9600_ADC_RES_16b 0x20    // ADC Resolution bits - 16-bit Resolution
#define MCP9600_ADC_RES_14b 0x40    // ADC Resolution bits - 14-bit Resolution
#define MCP9600_ADC_RES_12b 0x60    // ADC Resolution bits - 12-bit Resolution

#define MCP9600_BURST_TS    2       // Burst Mode Temperature Samples offset
#define MCP9600_BURST_TS_1  0x00    // Number of Temperature Samples 1
#define MCP9600_BURST_TS_2  0x02    // Number of Temperature Samples 2
#define MCP9600_BURST_TS_4  0x04    // Number of Temperature Samples 4
#define MCP9600_BURST_TS_8  0x06    // Number of Temperature Samples 8
#define MCP9600_BURST_TS_16  0x08   // Number of Temperature Samples 16
#define MCP9600_BURST_TS_32  0x0A   // Number of Temperature Samples 32
#define MCP9600_BURST_TS_64  0x0C   // Number of Temperature Samples 64
#define MCP9600_BURST_TS_128  0x0E  // Number of Temperature Samples 128

#define MCP9600_MODE        0       // Shutdown Mode bits offset
#define MCP9600_MODE_NORMAL 0x00    // Normal operation
#define MCP9600_MODE_SHUT   0x01    // Shutdown
#define MCP9600_MODE_BURST  0x02    // Burst mode

// MCP9600 alert configuration
#define MCP9600_INT_CLR     0x80    // Interrupt Clear bit
#define MCP9600_TH_TC       0x10    // Monitor TH or TC: Temperature Maintain/Detect bit
#define MCP9600_RISE_FALL   0x08    // Rise/Fall: Alert Temperature Direction bit
#define MCP9600_HIGH_LOW    0x04    // Active-High/Low: Alert State bit
#define MCP9600_COMP_INT    0x02    // Comp./Int.: Alert Mode bit
#define MCP9600_ALRT_EN     0x01    // Alert Enable: Alert Output Enable bit

// MCP9600_FUNCS

uint8_t mcp9600_init(uint8_t ubAddr);

float mcp9600_get_hj_temp(uint8_t ubAddr);
float mcp9600_get_cj_temp(uint8_t ubAddr);
float mcp9600_get_temp_delta(uint8_t ubAddr);

uint32_t mcp9600_get_adc(uint8_t ubAddr);

uint8_t mcp9600_get_id(uint8_t ubAddr);
uint8_t mcp9600_get_revision(uint8_t ubAddr);

void mcp9600_set_status(uint8_t ubAddr, uint8_t ubStatus);
uint8_t mcp9600_get_status(uint8_t ubAddr);

void mcp9600_set_sensor_config(uint8_t ubAddr, uint8_t ubConfig);
uint8_t mcp9600_get_sensor_config(uint8_t ubAddr);

void mcp9600_set_config(uint8_t ubAddr, uint8_t ubConfig);
uint8_t mcp9600_get_config(uint8_t ubAddr);

void mcp9600_set_alert_config(uint8_t ubAddr, uint8_t ubAlert, uint8_t ubConfig);
uint8_t mcp9600_get_alert_config(uint8_t ubAddr, uint8_t ubAlert);
void mcp9600_set_alert_hysteresis(uint8_t ubAddr, uint8_t ubAlert, uint8_t ubHysteresis);
void mcp9600_set_alert_limit(uint8_t ubAddr, uint8_t ubAlert, float fLimit);

#endif  // __MCP9600_H__
