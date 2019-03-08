#ifndef __MCP9600_H__
#define __MCP9600_H__

#define MCP9600_I2C_ADDR 0x60       // 0x60 to 0x67 depending on ADDR pin

#define MCP9600_DEV_ID 0x40

#define MCP9600_REG_HJT 0x00        // Hot-Junction Temperature – TH
#define MCP9600_REG_JTD 0x01        // Junctions Temperature Delta – T∆
#define MCP9600_REG_CJT 0x02        // Cold-Junction Temperature – TC
#define MCP9600_REG_ADC 0x03        // Raw Data ADC
#define MCP9600_REG_STAT 0x04       // STATUS
#define MCP9600_REG_TCFG 0x05       // Thermocouple Sensor Configuration
#define MCP9600_REG_CFG 0x06        // Device Configuration
#define MCP9600_REG_ALRT1CFG 0x08   // Alert 1 Configuration
#define MCP9600_REG_ALRT2CFG 0x09   // Alert 2 Configuration
#define MCP9600_REG_ALRT3CFG 0x0A   // Alert 3 Configuration
#define MCP9600_REG_ALRT4CFG 0x0B   // Alert 4 Configuration
#define MCP9600_REG_ALRT1HYS 0x0C   // Alert 1 Hysteresis
#define MCP9600_REG_ALRT2HYS 0x0D   // Alert 2 Hysteresis
#define MCP9600_REG_ALRT3HYS 0x0E   // Alert 3 Hysteresis
#define MCP9600_REG_ALRT4HYS 0x0F   // Alert 4 Hysteresis
#define MCP9600_REG_ALRT1LIM 0x10   // Alert 1 Limit
#define MCP9600_REG_ALRT2LIM 0x11   // Alert 2 Limit
#define MCP9600_REG_ALRT3LIM 0x12   // Alert 3 Limit
#define MCP9600_REG_ALRT4LIM 0x13   // Alert 4 Limit
#define MCP9600_REG_ID 0x20         // Device ID/Revision

#endif