
#include "mcp9600.h"

uint8_t mcp9600_init()
{
    return 1;
}

double mcp9600_get_hj_temp()
{
    uint8_t arrubBuf[2];
    // i2c read here

    uint16_t sTemperatureReg = ((uint16_t)arrubBuf[0] << 8) + (uint16_t)arrubBuf[1];

    if(sTemperatureReg & 0x8000)
    {
        sTemperatureReg = (~(sTemperatureReg) + 1);
        return  -(0.0625f * (double)sTemperatureReg);
    }
    else
    {
        return  (0.0625f * (double)sTemperatureReg);
    }
}

double mcp9600_get_cj_temp()
{
    uint8_t arrubBuf[2];
    // i2c read here

    uint16_t sTemperatureReg = ((uint16_t)arrubBuf[0] << 8) + (uint16_t)arrubBuf[1];

    if(sTemperatureReg & 0x8000)
    {
        sTemperatureReg = (~(sTemperatureReg) + 1);
        return  -(0.0625f * (double)sTemperatureReg);
    }
    else
    {
        return  (0.0625f * (double)sTemperatureReg);
    }
}

double mcp9600_get_temp_delta()
{
    uint8_t arrubBuf[2];
    // i2c read here

    uint16_t sTemperatureReg = ((uint16_t)arrubBuf[0] << 8) + (uint16_t)arrubBuf[1];

    if(sTemperatureReg & 0x8000)
    {
        sTemperatureReg = (~(sTemperatureReg) + 1);
        return  -(0.0625f * (double)sTemperatureReg);
    }
    else
    {
        return  (0.0625f * (double)sTemperatureReg);
    }
}

uint32_t mcp9600_get_adc()
{
    uint8_t arrubBuf[3];
    // i2c read here

    uint32_t ulAdcReg = ((uint32_t)arrubBuf[0] << 16) + ((uint32_t)arrubBuf[0] << 8) + (uint32_t)arrubBuf[0];
}

uint8_t mcp9600_get_id()
{
    uint8_t ubBuf;
    //i2c read here

    return ubBuf;
}

uint8_t mcp9600_get_revision()
{
    uint8_t arrubBuf[2];
    // i2c read here

    return arrubBuf[1];
}

void mcp9600_set_status(uint8_t ubStatus)
{
    // i2c write here
}

uint8_t mcp9600_get_status()
{
    uint8_t ubBuf;
    //i2c read here

    return ubBuf;
}

void mcp9600_set_sensor_config(uint8_t ubConfig)
{
    // i2c write here
}

uint8_t mcp9600_get_sensor_config()
{
    uint8_t ubBuf;
    //i2c read here

    return ubBuf;
}

void mcp9600_set_config(uint8_t ubConfig)
{
    // i2c write here
}

uint8_t mcp9600_get_config()
{
    uint8_t ubBuf;
    //i2c read here

    return ubBuf;
}

void mcp9600_set_alert_config(uint8_t ubAlert, uint8_t ubConfig)
{
    // i2c write here
    // REG to write is:
    // (MCP9600_REG_ALRT_CFG | ubAlert)
}

uint8_t mcp9600_get_alert_config()
{
    uint8_t ubBuf;
    //i2c read here
    // REG to read is:
    // (MCP9600_REG_ALRT_CFG | ubAlert)

    return ubBuf;
}

void mcp9600_set_alert_hysteresis(uint8_t ubAlert, uint8_t ubHysteresis)
{
    // i2c write here
    // REG to write is:
    // (MCP9600_REG_ALRT_HYS | ubAlert)
}

void mcp9600_set_alert_limit(uint8_t ubAlert, double dLimit)
{
    uint16_t usTempLim = (uint16_t)(dLimit / 0.0625f);
    // i2c write here
    // REG to write is:
    // (MCP9600_REG_ALRT_LIM | ubAlert)
}