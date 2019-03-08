
#include "mcp9600.h"

uint8_t mcp9600_init()
{
    return 1;
}

float mcp9600_get_hj_temp()
{
        i2c1_write_byte(MCP9600_I2C_ADDR, MCP9600_REG_HJT, I2C_RESTART);

        uint8_t ubBuf[2];

        i2c1_read(MCP9600_I2C_ADDR, ubBuf, 2, I2C_STOP);

        uint16_t usTemp = ((uint16_t)ubBuf[0] << 8) | ubBuf[1];
        float fTemp = 0.f;

        if(usTemp & 0x8000)
        {
            usTemp = ~usTemp + 1;
            fTemp = -0.0625f * usTemp;
        }
        else
        {
            fTemp = 0.0625f * usTemp;
        }
}

float mcp9600_get_cj_temp()
{
    i2c1_write_byte(MCP9600_I2C_ADDR, MCP9600_REG_HJT, I2C_RESTART);

    uint8_t ubBuf[2];

    i2c1_read(MCP9600_I2C_ADDR, ubBuf, 2, I2C_STOP);

    uint16_t usTemp = ((uint16_t)ubBuf[0] << 8) | ubBuf[1];
    float fTemp = 0.f;

    if(usTemp & 0x8000)
    {
        usTemp = ~usTemp + 1;
        fTemp = -0.0625f * usTemp;
    }
    else
    {
        fTemp = 0.0625f * usTemp;
    }
}

float mcp9600_get_temp_delta()
{
    i2c1_write_byte(MCP9600_I2C_ADDR, MCP9600_REG_JTD, I2C_RESTART);

    uint8_t ubBuf[2];

    i2c1_read(MCP9600_I2C_ADDR, ubBuf, 2, I2C_STOP);

    uint16_t usTemp = ((uint16_t)ubBuf[0] << 8) | ubBuf[1];
    float fTemp = 0.f;

    if(usTemp & 0x8000)
    {
        usTemp = ~usTemp + 1;
        fTemp = -usTemp * 0.0625f;
    }
    else
    {
        fTemp = usTemp * 0.0625f;
    }
}

uint32_t mcp9600_get_adc()
{
    i2c1_write_byte(MCP9600_I2C_ADDR, MCP9600_REG_ADC, I2C_RESTART);

    uint8_t ubBuf[3];

    i2c1_read(MCP9600_I2C_ADDR, ubBuf, 3, I2C_STOP);

    return (((uint32_t)ubBuf[0] << 16) | ((uint32_t)ubBuf[0] << 8) | ubBuf[1]);

}

uint8_t mcp9600_get_id()
{
    i2c1_write_byte(MCP9600_I2C_ADDR, MCP9600_REG_ID, I2C_RESTART);

    return i2c1_read_byte(MCP9600_I2C_ADDR, I2C_STOP);
}

uint8_t mcp9600_get_revision()
{
    i2c1_write_byte(MCP9600_I2C_ADDR, MCP9600_REG_ID, I2C_RESTART);

    uint8_t ubBuf[2];

    i2c1_read(MCP9600_I2C_ADDR, ubBuf, 2, I2C_STOP);

    return ubBuf[1];
}

void mcp9600_set_status(uint8_t ubStatus)
{
    uint8_t ubBuf[2] = {MCP9600_REG_STAT, ubStatus};
    i2c1_write(MCP9600_I2C_ADDR, ubBuf, 2, I2C_STOP);
}

uint8_t mcp9600_get_status()
{
    i2c1_write_byte(MCP9600_I2C_ADDR, MCP9600_REG_STAT, I2C_RESTART);

    return i2c1_read_byte(MCP9600_I2C_ADDR, I2C_STOP);
}

void mcp9600_set_sensor_config(uint8_t ubConfig)
{
    uint8_t ubBuf[2] = {MCP9600_REG_TCFG, ubConfig};
    i2c1_write(MCP9600_I2C_ADDR, ubBuf, 2, I2C_STOP);
}

uint8_t mcp9600_get_sensor_config()
{
    i2c1_write_byte(MCP9600_I2C_ADDR, MCP9600_REG_TCFG, I2C_RESTART);

    return i2c1_read_byte(MCP9600_I2C_ADDR, I2C_STOP);
}

void mcp9600_set_config(uint8_t ubConfig)
{
    uint8_t ubBuf[2] = {MCP9600_REG_CFG, ubConfig};
    i2c1_write(MCP9600_I2C_ADDR, ubBuf, 2, I2C_STOP);
}

uint8_t mcp9600_get_config()
{
    i2c1_write_byte(MCP9600_I2C_ADDR, MCP9600_REG_CFG, I2C_RESTART);

    return i2c1_read_byte(MCP9600_I2C_ADDR, I2C_STOP);
}

void mcp9600_set_alert_config(uint8_t ubAlert, uint8_t ubConfig)
{
    uint8_t ubBuf[2] = {(MCP9600_REG_ALRT_CFG | ubAlert), ubConfig};
    i2c1_write(MCP9600_I2C_ADDR, ubBuf, 2, I2C_STOP);
}

uint8_t mcp9600_get_alert_config(uint8_t ubAlert)
{
    i2c1_write_byte(MCP9600_I2C_ADDR, (MCP9600_REG_ALRT_CFG | ubAlert), I2C_RESTART);

    return i2c1_read_byte(MCP9600_I2C_ADDR, I2C_STOP);
}

void mcp9600_set_alert_hysteresis(uint8_t ubAlert, uint8_t ubHysteresis)
{
    uint8_t ubBuf[2] = {(MCP9600_REG_ALRT_HYS | ubAlert), ubHysteresis};
    i2c1_write(MCP9600_I2C_ADDR, ubBuf, 2, I2C_STOP);
}

void mcp9600_set_alert_limit(uint8_t ubAlert, float dLimit)
{
    uint16_t usTempLim = (uint16_t)(dLimit / 0.0625f);
    uint8_t ubBuf[3] = {(MCP9600_REG_ALRT_LIM | ubAlert), (usTempLim >> 8), (usTempLim & 0x00FF)};
    i2c1_write(MCP9600_I2C_ADDR, ubBuf, 2, I2C_STOP);
}