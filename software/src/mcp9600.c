
#include "mcp9600.h"

static uint8_t mcp9600_read_register(uint8_t ubAddr, uint8_t ubReg)
{
    uint8_t ubVal;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        i2c0_write_byte(MCP9600_I2C_ADDR | (ubAddr & 0x07), ubReg, I2C_STOP);
    //}

    //delay_ms(1);

    //ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    //{
        ubVal = i2c0_read_byte(MCP9600_I2C_ADDR | (ubAddr & 0x07), I2C_STOP);
    }

    //delay_ms(1);

    return ubVal;
}
static uint16_t mcp9600_read_register16(uint8_t ubAddr, uint8_t ubReg)
{
    uint8_t ubBuf[2];

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        i2c0_write_byte(MCP9600_I2C_ADDR | (ubAddr & 0x07), ubReg, I2C_STOP);
    //}

    //delay_ms(1);

    //ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    //{
        i2c0_read(MCP9600_I2C_ADDR | (ubAddr & 0x07), ubBuf, 2, I2C_STOP);
    }

    //delay_ms(1);

    return ((uint16_t)ubBuf[0] << 8) | ubBuf[1];
}
static uint32_t mcp9600_read_register24(uint8_t ubAddr, uint8_t ubReg)
{
    uint8_t ubBuf[3];

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        i2c0_write_byte(MCP9600_I2C_ADDR | (ubAddr & 0x07), ubReg, I2C_STOP);
    //}

    //delay_ms(1);

    //ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    //{
        i2c0_read(MCP9600_I2C_ADDR | (ubAddr & 0x07), ubBuf, 3, I2C_STOP);
    }

    //delay_ms(1);

    return (((uint32_t)ubBuf[0] << 16) | ((uint32_t)ubBuf[1] << 8) | ubBuf[2]);
}
static void mcp9600_write_register(uint8_t ubAddr, uint8_t ubReg, uint8_t ubVal)
{
    uint8_t ubBuf[2];

    ubBuf[0] = ubReg;
    ubBuf[1] = ubVal;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        i2c0_write(MCP9600_I2C_ADDR | (ubAddr & 0x07), ubBuf, 2, I2C_STOP);
    }

    //delay_ms(1);
}
static void mcp9600_write_register16(uint8_t ubAddr, uint8_t ubReg, uint16_t usVal)
{
    uint8_t ubBuf[3];

    ubBuf[0] = ubReg;
    ubBuf[1] = usVal >> 8;
    ubBuf[2] = usVal &  0xFF;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        i2c0_write(MCP9600_I2C_ADDR | (ubAddr & 0x07), ubBuf, 3, I2C_STOP);
    }

    //delay_ms(1);
}

uint8_t mcp9600_init(uint8_t ubAddr)
{
    //delay_ms(1);

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        if(!i2c0_write(MCP9600_I2C_ADDR | (ubAddr & 0x07), 0, 0, I2C_STOP)) // Check ACK from the expected address
            return 0;
    }

    //delay_ms(1);

    return 1;
}

float mcp9600_get_hj_temp(uint8_t ubAddr)
{
    uint16_t usTemp = mcp9600_read_register16(ubAddr, MCP9600_REG_HJT);

    if(usTemp & 0x8000)
    {
        usTemp = ~usTemp + 1;

        return -0.0625f * usTemp;
    }
    else
    {
        return 0.0625f * usTemp;
    }
}
float mcp9600_get_cj_temp(uint8_t ubAddr)
{
    uint16_t usTemp = mcp9600_read_register16(ubAddr, MCP9600_REG_CJT);

    if(usTemp & 0x8000)
    {
        usTemp = ~usTemp + 1;

        return -0.0625f * usTemp;
    }
    else
    {
        return 0.0625f * usTemp;
    }
}
float mcp9600_get_temp_delta(uint8_t ubAddr)
{
    uint16_t usTemp = mcp9600_read_register16(ubAddr, MCP9600_REG_JTD);

    if(usTemp & 0x8000)
    {
        usTemp = ~usTemp + 1;

        return -0.0625f * usTemp;
    }
    else
    {
        return 0.0625f * usTemp;
    }
}

uint32_t mcp9600_get_adc(uint8_t ubAddr)
{
    return mcp9600_read_register24(ubAddr, MCP9600_REG_ADC);
}

uint8_t mcp9600_get_id(uint8_t ubAddr)
{
    return mcp9600_read_register16(ubAddr, MCP9600_REG_ID) >> 8;
}
uint8_t mcp9600_get_revision(uint8_t ubAddr)
{
    return mcp9600_read_register16(ubAddr, MCP9600_REG_ID) & 0xFF;
}

void mcp9600_set_status(uint8_t ubAddr, uint8_t ubStatus)
{
    mcp9600_write_register(ubAddr, MCP9600_REG_STAT, ubStatus);
}
uint8_t mcp9600_get_status(uint8_t ubAddr)
{
    return mcp9600_read_register(ubAddr, MCP9600_REG_STAT);
}

void mcp9600_set_sensor_config(uint8_t ubAddr, uint8_t ubConfig)
{
    mcp9600_write_register(ubAddr, MCP9600_REG_TCFG, ubConfig);
}
uint8_t mcp9600_get_sensor_config(uint8_t ubAddr)
{
    return mcp9600_read_register(ubAddr, MCP9600_REG_TCFG);
}

void mcp9600_set_config(uint8_t ubAddr, uint8_t ubConfig)
{
    mcp9600_write_register(ubAddr, MCP9600_REG_CFG, ubConfig);
}
uint8_t mcp9600_get_config(uint8_t ubAddr)
{
    return mcp9600_read_register(ubAddr, MCP9600_REG_CFG);
}

void mcp9600_set_alert_config(uint8_t ubAddr, uint8_t ubAlert, uint8_t ubConfig)
{
    mcp9600_write_register(ubAddr, MCP9600_REG_ALRT_CFG | ubAlert, ubConfig);
}
uint8_t mcp9600_get_alert_config(uint8_t ubAddr, uint8_t ubAlert)
{
    return mcp9600_read_register(ubAddr, MCP9600_REG_ALRT_CFG | ubAlert);
}
void mcp9600_set_alert_hysteresis(uint8_t ubAddr, uint8_t ubAlert, uint8_t ubHysteresis)
{
    mcp9600_write_register(ubAddr, MCP9600_REG_ALRT_HYS | ubAlert, ubHysteresis);
}
void mcp9600_set_alert_limit(uint8_t ubAddr, uint8_t ubAlert, float fLimit)
{
    mcp9600_write_register16(ubAddr, MCP9600_REG_ALRT_LIM | ubAlert, (uint16_t)(fLimit / 0.0625f));
}