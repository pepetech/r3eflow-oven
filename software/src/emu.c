#include "emu.h"

uint8_t g_ubAVDDLow = 0;
uint8_t g_ubAltAVDDLow = 0;
uint8_t g_ubDVDDLow = 0;
uint8_t g_ubIOVDDLow = 0;

static uint8_t emu_vmon_thresh_calc(uint8_t ubThresh1V86, uint8_t ubThresh2V98, float fTargetVoltage)
{
    // Convert from BCD
    float fThresh1V86 = 10 * ((ubThresh1V86 >> 4) & 0xF) + ((ubThresh1V86 >> 0) & 0xF);
    float fThresh2V98 = 10 * ((ubThresh2V98 >> 4) & 0xF) + ((ubThresh2V98 >> 0) & 0xF);

    // Interpolate between the two calibration thresholds
    float fSlope = (2.98f - 1.86f) / (fThresh2V98 - fThresh1V86);
    float fOffset = 1.86f - (fSlope * fThresh1V86);

    // Find the Code for the target voltage
    float fTargetThresh = roundf((fTargetVoltage - fOffset) / fSlope);

    // Convert to BCD
    uint8_t ubDecThresh = fTargetThresh / 10.f;
    uint8_t ubUniThresh = fTargetThresh - (ubDecThresh * 10);

    uint8_t ubTargetThresh = ((ubDecThresh & 0xF) << 4) | (ubUniThresh & 0xF);

    return ubTargetThresh;
}
static float emu_vmon_thresh_get(uint8_t ubThresh1V86, uint8_t ubThresh2V98, uint8_t ubCurrentThresh)
{
    // Convert from BCD
    float fCurrentThresh = 10 * ((ubCurrentThresh >> 4) & 0xF) + ((ubCurrentThresh >> 0) & 0xF);
    float fThresh1V86 = 10 * ((ubThresh1V86 >> 4) & 0xF) + ((ubThresh1V86 >> 0) & 0xF);
    float fThresh2V98 = 10 * ((ubThresh2V98 >> 4) & 0xF) + ((ubThresh2V98 >> 0) & 0xF);

    // Interpolate between the two calibration thresholds
    float fSlope = (2.98f - 1.86f) / (fThresh2V98 - fThresh1V86);
    float fOffset = 1.86f - (fSlope * fThresh1V86);

    // Find the Code for the target voltage
    float fCurrentVoltage = (fCurrentThresh * fSlope) + fOffset;

    return fCurrentVoltage;
}

void _emu_isr()
{
    uint32_t ulFlags = EMU->IFC;

    if(ulFlags & EMU_IFC_VMONAVDDFALL)
        g_ubAVDDLow = 1;
    else if(ulFlags & EMU_IFC_VMONAVDDRISE)
        g_ubAVDDLow = 0;

    if(ulFlags & EMU_IFC_VMONALTAVDDFALL)
        g_ubAltAVDDLow = 1;
    else if(ulFlags & EMU_IFC_VMONALTAVDDRISE)
        g_ubAltAVDDLow = 0;

    if(ulFlags & EMU_IFC_VMONDVDDFALL)
        g_ubDVDDLow = 1;
    else if(ulFlags & EMU_IFC_VMONDVDDRISE)
        g_ubDVDDLow = 0;

    if(ulFlags & EMU_IFC_VMONIO0FALL)
        g_ubIOVDDLow = 1;
    else if(ulFlags & EMU_IFC_VMONIO0RISE)
        g_ubIOVDDLow = 0;
}

void emu_init()
{
    EMU->PWRCTRL = EMU_PWRCTRL_IMMEDIATEPWRSWITCH | EMU_PWRCTRL_REGPWRSEL_DVDD | EMU_PWRCTRL_ANASW_AVDD;

    EMU->IFC = _EMU_IFC_MASK; // Clear pending IRQs
    IRQ_CLEAR(EMU_IRQn); // Clear pending vector
    IRQ_SET_PRIO(EMU_IRQn, 3, 1); // Set priority 3,1 (min)
    IRQ_ENABLE(EMU_IRQn); // Enable vector
}
float emu_get_temperature()
{
    EMU->IFC = EMU_IFC_TEMP;

    while(!(EMU->IF & EMU_IF_TEMP));

    float fCalibrationTemp = (DEVINFO->CAL & _DEVINFO_CAL_TEMP_MASK) >> _DEVINFO_CAL_TEMP_SHIFT;
    float fEMUCalibrationTemp = (DEVINFO->EMUTEMP & _DEVINFO_EMUTEMP_EMUTEMPROOM_MASK) >> _DEVINFO_EMUTEMP_EMUTEMPROOM_SHIFT;
    float fTempCoefEM01 = 0.278f + fEMUCalibrationTemp / 100.f;
    float fEMUTemp = fCalibrationTemp + fTempCoefEM01 * (fEMUCalibrationTemp - EMU->TEMP);

    return fEMUTemp;
}
void emu_vmon_avdd_config(uint8_t ubEnable, float fLowThresh, float *pfLowThresh, float fHighThresh, float *pfHighThresh)
{
    if(!ubEnable)
    {
        EMU->VMONAVDDCTRL &= ~EMU_VMONAVDDCTRL_EN;
        EMU->IEN &= ~(EMU_IEN_VMONAVDDRISE | EMU_IEN_VMONAVDDFALL);

        return;
    }

    uint8_t ubThresh1V86 = (DEVINFO->VMONCAL0 >> 0) & 0xFF;
    uint8_t ubThresh2V98 = (DEVINFO->VMONCAL0 >> 8) & 0xFF;

    uint8_t ubLowThresh = emu_vmon_thresh_calc(ubThresh1V86, ubThresh2V98, fLowThresh);
    uint8_t ubHighThresh = emu_vmon_thresh_calc(ubThresh1V86, ubThresh2V98, fHighThresh);

    if(pfLowThresh)
        *pfLowThresh = emu_vmon_thresh_get(ubThresh1V86, ubThresh2V98, ubLowThresh);

    if(pfHighThresh)
        *pfHighThresh = emu_vmon_thresh_get(ubThresh1V86, ubThresh2V98, ubHighThresh);

    EMU->IFC = EMU_IFC_VMONAVDDRISE | EMU_IFC_VMONAVDDFALL;
    EMU->IEN |= EMU_IEN_VMONAVDDRISE | EMU_IEN_VMONAVDDFALL;
    EMU->VMONAVDDCTRL = ((uint32_t)ubLowThresh << _EMU_VMONAVDDCTRL_FALLTHRESFINE_SHIFT) | ((uint32_t)ubHighThresh << _EMU_VMONAVDDCTRL_RISETHRESFINE_SHIFT) | EMU_VMONAVDDCTRL_EN;
}
void emu_vmon_altavdd_config(uint8_t ubEnable, float fLowThresh, float *pfLowThresh)
{
    if(!ubEnable)
    {
        EMU->VMONALTAVDDCTRL &= ~EMU_VMONALTAVDDCTRL_EN;
        EMU->IEN &= ~(EMU_IEN_VMONALTAVDDRISE | EMU_IEN_VMONALTAVDDFALL);

        return;
    }

    uint8_t ubThresh1V86 = (DEVINFO->VMONCAL0 >> 16) & 0xFF;
    uint8_t ubThresh2V98 = (DEVINFO->VMONCAL0 >> 24) & 0xFF;

    uint8_t ubLowThresh = emu_vmon_thresh_calc(ubThresh1V86, ubThresh2V98, fLowThresh);

    if(pfLowThresh)
        *pfLowThresh = emu_vmon_thresh_get(ubThresh1V86, ubThresh2V98, ubLowThresh);

    EMU->IFC = EMU_IFC_VMONALTAVDDRISE | EMU_IFC_VMONALTAVDDFALL;
    EMU->IEN |= EMU_IEN_VMONALTAVDDRISE | EMU_IEN_VMONALTAVDDFALL;
    EMU->VMONALTAVDDCTRL = ((uint32_t)ubLowThresh << _EMU_VMONALTAVDDCTRL_THRESFINE_SHIFT) | EMU_VMONALTAVDDCTRL_EN;
}
void emu_vmon_dvdd_config(uint8_t ubEnable, float fLowThresh, float *pfLowThresh)
{
    if(!ubEnable)
    {
        EMU->VMONDVDDCTRL &= ~EMU_VMONDVDDCTRL_EN;
        EMU->IEN &= ~(EMU_IEN_VMONDVDDRISE | EMU_IEN_VMONDVDDFALL);

        return;
    }

    uint8_t ubThresh1V86 = (DEVINFO->VMONCAL1 >> 0) & 0xFF;
    uint8_t ubThresh2V98 = (DEVINFO->VMONCAL1 >> 8) & 0xFF;

    uint8_t ubLowThresh = emu_vmon_thresh_calc(ubThresh1V86, ubThresh2V98, fLowThresh);

    if(pfLowThresh)
        *pfLowThresh = emu_vmon_thresh_get(ubThresh1V86, ubThresh2V98, ubLowThresh);

    EMU->IFC = EMU_IFC_VMONDVDDRISE | EMU_IFC_VMONDVDDFALL;
    EMU->IEN |= EMU_IEN_VMONDVDDRISE | EMU_IEN_VMONDVDDFALL;
    EMU->VMONDVDDCTRL = ((uint32_t)ubLowThresh << _EMU_VMONDVDDCTRL_THRESFINE_SHIFT) | EMU_VMONDVDDCTRL_EN;
}
void emu_vmon_iovdd_config(uint8_t ubEnable, float fLowThresh, float *pfLowThresh)
{
    if(!ubEnable)
    {
        EMU->VMONIO0CTRL &= ~EMU_VMONIO0CTRL_EN;
        EMU->IEN &= ~(EMU_IEN_VMONIO0RISE | EMU_IEN_VMONIO0FALL);

        return;
    }

    uint8_t ubThresh1V86 = (DEVINFO->VMONCAL1 >> 16) & 0xFF;
    uint8_t ubThresh2V98 = (DEVINFO->VMONCAL1 >> 24) & 0xFF;

    uint8_t ubLowThresh = emu_vmon_thresh_calc(ubThresh1V86, ubThresh2V98, fLowThresh);

    if(pfLowThresh)
        *pfLowThresh = emu_vmon_thresh_get(ubThresh1V86, ubThresh2V98, ubLowThresh);

    EMU->IFC = EMU_IFC_VMONIO0RISE | EMU_IFC_VMONIO0FALL;
    EMU->IEN |= EMU_IEN_VMONIO0RISE | EMU_IEN_VMONIO0FALL;
    EMU->VMONIO0CTRL = ((uint32_t)ubLowThresh << _EMU_VMONIO0CTRL_THRESFINE_SHIFT) | EMU_VMONIO0CTRL_EN;
}