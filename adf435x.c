/****************************************************************************
 *
 * Copyright 2021 Lee Mitchell <lee@indigopepper.com>
 * This file is part of ADF435xCFG (ADF435x Configurator)
 *
 * ADF435xCFG (ADF435x Configurator) is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * ADF435xCFG (ADF435x Configurator) is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ADF435xCFG (ADF435x Configurator).  If not,
 * see <http://www.gnu.org/licenses/>.
 *
 ****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include "adf435x.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

static float ADF435x_fGCD(float a, float b);
static bool ADF435x_bCheckUint(char *acName, uint32_t u32Val, uint32_t u32Max);
static bool ADF435x_bCheckLookupVal(char *acName, float fVal, float *pfArray, int iArrayLen);
static int ADF435x_iLookupVal(float fVal, float *pfArray, int iArrayLen);
static ADF435X_teVerbosity ADF435x_eVerbosity;

bool ADF435x_Init(ADF435X_teVerbosity eVerbosity)
{
    ADF435x_eVerbosity = eVerbosity;
    return true;
}

bool ADF435x_CalculateSettings(uint64_t u64Frequency, ADF435X_tsSettings *psSettings)
{

    ADF435X_teDeviceType eDeviceType = E_ADF435X_DEVICE_TYPE_ADF4351;
    ADF435X_teFeedbackSelect eFeedbackSelect = E_ADF435X_FEEDBACK_SELECT_FUNDAMENTAL;

    ADF435X_teBandSelectClockMode eBandSelectClockMode = E_ADF435X_BAND_SELECT_CLOCK_MODE_LOW;
    uint64_t u64ReferenceFrequencyHz = 25000000;
    uint64_t u64ChannelSpacingHz = 100000;
    uint64_t u64RCounter = 1;
    psSettings->u64BandSelectClockDivider = 0;
    psSettings->u64OutputDivider = 0;

    bool bRefDoubler = false;
    bool bRefDiv2 = false;
    bool bEnableGCD = true;

    uint64_t u64N;
    uint64_t u64Div;
    uint64_t u64PFDScale;
    uint64_t u64BandSelectClockFrequency;

    uint64_t u64PFDFreqHz = ((u64ReferenceFrequencyHz * (bRefDoubler ? 2.0 : 1.0)) / ((bRefDiv2 ? 2.0 : 1.0) * u64RCounter));

    if(ADF435x_eVerbosity >= E_ADF435X_VERBOSITY_HIGH) printf("Frequency = %d.%dMHz   PFD Frequency=%d.%dMHz\n", u64Frequency / 1000000, u64Frequency % 1000000, u64PFDFreqHz / 1000000, u64PFDFreqHz % 1000000);

    // Calculate the output divider
    for(int n = 0; n < 7; n++)
    {
        psSettings->u64OutputDivider = 1 << n;
        if((2200000000 / psSettings->u64OutputDivider) <= u64Frequency)
        {
            break;
        }
    }

    if(ADF435x_eVerbosity >= E_ADF435X_VERBOSITY_HIGH) printf("Output Divider = %d\n", psSettings->u64OutputDivider);

    if(eFeedbackSelect == E_ADF435X_FEEDBACK_SELECT_FUNDAMENTAL)
    {
        u64N = u64Frequency * psSettings->u64OutputDivider * 1000 / u64PFDFreqHz;
    }
    else
    {
        u64N = u64Frequency * 1000 / u64PFDFreqHz;
    }

    psSettings->u64Int = u64N / 1000;
    psSettings->u64Mod = u64ReferenceFrequencyHz / u64ChannelSpacingHz;
    psSettings->u64Frac = ((u64N % 1000) * psSettings->u64Mod) / 1000;

    if(bEnableGCD)
    {
        u64Div = ADF435x_fGCD(psSettings->u64Mod, psSettings->u64Frac);
        psSettings->u64Mod = psSettings->u64Mod / u64Div;
        psSettings->u64Frac = psSettings->u64Frac / u64Div;
    }

    if(ADF435x_eVerbosity >= E_ADF435X_VERBOSITY_HIGH) printf("DIV=%d\n", u64Div);

    if(psSettings->u64Mod == 1)
    {
        psSettings->u64Mod = 2;
    }

    if(ADF435x_eVerbosity >= E_ADF435X_VERBOSITY_HIGH) printf("N=%d.%03d INT=%d MOD=%d FRAC=%d\n", u64N / 1000, u64N % 1000, psSettings->u64Int, psSettings->u64Mod, psSettings->u64Frac);

    if(u64PFDFreqHz > 32000000)
    {
        if(psSettings->u64Frac != 0)
        {
            printf("Maximum PFD frequency in Frac-N mode (FRAC != 0) is 32MHz.\n");
            return false;
        }

        if(psSettings->u64Frac == 0 && eDeviceType == E_ADF435X_DEVICE_TYPE_ADF4351)
        {
            if(u64PFDFreqHz > 90000000)
            {
                printf("Maximum PFD frequency in Int-N mode (FRAC = 0) is 90MHz.\n");
                return false;
            }
            if(eBandSelectClockMode == E_ADF435X_BAND_SELECT_CLOCK_MODE_LOW)
            {
                printf("Band Select Clock Mode must be set to High when PFD is >32MHz in Int-N mode (FRAC = 0).\n");
                return false;
            }
        }
    }

    if(psSettings->u64BandSelectClockDivider == 0)
    {
        if(eBandSelectClockMode == E_ADF435X_BAND_SELECT_CLOCK_MODE_LOW)
        {
            u64PFDScale = 8;
            psSettings->u64BandSelectClockDivider = MIN((8 * u64PFDFreqHz) / 1000000, 255);
        }
        else
        {
            u64PFDScale = 2;
        }
    }

    u64BandSelectClockFrequency = u64PFDFreqHz / psSettings->u64BandSelectClockDivider;

    if(ADF435x_eVerbosity >= E_ADF435X_VERBOSITY_HIGH) printf("BandSelectClockDivider=%d BandSelectClockFrequency=%d\n", psSettings->u64BandSelectClockDivider, u64BandSelectClockFrequency);

    if(u64BandSelectClockFrequency > 500000)
    {
        printf("Band Select Clock Frequency is too High. It must be 500kHz or less. Currently=%d\n", u64BandSelectClockFrequency);
        return false;
    }
    else if(u64BandSelectClockFrequency > 125000000)
    {
        if(eDeviceType == E_ADF435X_DEVICE_TYPE_ADF4351)
        {
            if(eBandSelectClockMode == E_ADF435X_BAND_SELECT_CLOCK_MODE_LOW)
            {
                printf("Band Select Clock Frequency is too high. Reduce to 125kHz or less, or set Band Select Clock Mode to High.\n");
                return false;
            }
        }
        else
        {
            printf("Band Select Clock Frequency is too high. Reduce to 125kHz or less.\n");
            return false;
        }

    }

    if(ADF435x_eVerbosity >= E_ADF435X_VERBOSITY_HIGH) printf("Settings: INT=%d FRAC=%d MOD=%d ClkDiv=%d OutputDiv=%d\n", psSettings->u64Int, psSettings->u64Frac, psSettings->u64Mod, psSettings->u64BandSelectClockDivider, psSettings->u64OutputDivider);

    return true;
}

bool ADF435x_GenerateRegisters(ADF435X_tsSettings *psSettings, ADF435X_tuRegisters *puRegisters)
{

    float afChargePumpCurrent[] = {0.31, 0.63, 0.94, 1.25, 1.56, 1.88, 2.19, 2.50, 2.81, 3.13, 3.44, 3.75, 4.06, 4.38, 4.49, 5.00};
    float afABP[] = {10, 6};
    float afOutputPower[] = {-4, -1, 2, 5};

    ADF435X_teDeviceType eDeviceType = E_ADF435X_DEVICE_TYPE_ADF4351;
    uint32_t u32PhaseValue = 0;
    uint32_t u32BandSelectClockDivider = 200;
    ADF435X_teBandSelectClockMode eBandSelectClockMode = E_ADF435X_BAND_SELECT_CLOCK_MODE_LOW;
    uint32_t u32Prescaler = (8/9);
    ADF435X_teLowNoiseOrLowSpurMode eLowNoiseOrLowSpurMode = E_ADF435X_LOW_NOISE_MODE;
    ADF435X_teMuxOut eMuxOut = E_ADF435X_MUX_OUT_TRISTATE;
    bool bRefDoubler = false;
    bool bRefDiv2 = false;
    uint32_t u32RCounter = 1;
    bool bDoubleBufR4 = false;

    float fChargePumpCurrent = 2.5;
    float fLDP = 10.0;
    ADF435X_tePDPolarity ePDPolarity = E_ADF435X_PD_POLARITY_POSITIVE;
    bool bPowerDown = false;
    bool bCPTristate = false;
    bool bCounterReset = false;
    float fABP = 10;
    bool bChargeCancel = false;
    bool bCSR = false;
    ADF435X_teClockDviderMode eClockDivMode = E_ADF435X_CLOCK_DIVIDER_MODE_OFF;
    uint32_t u32ClockDividerValue = 150;
    ADF435X_teFeedbackSelect eFeedbackSelect = E_ADF435X_FEEDBACK_SELECT_FUNDAMENTAL;
    // uint32_t u32OutputDivider = 64;
    bool bVCOPowerDown = false;
    bool bMuteTillLockDetect = false;
    ADF435X_teAuxOutputSelect eAuxOutputSelect = E_ADF435X_AUX_OUTPUT_SELECT_DIVIDED_OUTPUT;
    bool bAuxOutputEnable = false;
    float fAuxOutputPower = -4.0;
    bool bOutputEnable = true;
    float fOutputPower = 5.0;
    ADF435X_teLDPinMode eLDPinMode = E_ADF435X_LDPIN_MODE_DIGITAL_LOCK_DETECT;

    uint32_t u32OutputDividerSelect;

    bool bOk = true;

    bOk &= ADF435x_bCheckUint("INT", psSettings->u64Int, 65535);
    bOk &= ADF435x_bCheckUint("FRAC", psSettings->u64Frac, 4095);
    bOk &= ADF435x_bCheckUint("MOD", psSettings->u64Mod, 4095);

    bOk &= ADF435x_bCheckLookupVal("ChargePumpCurrent", fChargePumpCurrent, afChargePumpCurrent, sizeof(afChargePumpCurrent) / sizeof(float));
    bOk &= ADF435x_bCheckLookupVal("ABP", fABP, afABP, sizeof(afABP) / sizeof(float));
    bOk &= ADF435x_bCheckLookupVal("AuxOutputPower", fAuxOutputPower, afOutputPower, sizeof(afOutputPower) / sizeof(float));
    bOk &= ADF435x_bCheckLookupVal("OutputPower", fOutputPower, afOutputPower, sizeof(afOutputPower) / sizeof(float));


    u32OutputDividerSelect = log2(psSettings->u64OutputDivider);
    if(u32OutputDividerSelect < 0 || u32OutputDividerSelect > 64 || floor(u32OutputDividerSelect) != u32OutputDividerSelect)
    {
        printf("Output Divider must be a positive integer power of 2, not greater than 64.\n");
        return false;
    }

    if(ADF435x_eVerbosity >= E_ADF435X_VERBOSITY_HIGH) printf("OutputDivider=%d OutputDividerSelect=%x\n", psSettings->u64OutputDivider, u32OutputDividerSelect);

    // R0
    puRegisters->u32Register0 = ((uint32_t)psSettings->u64Int) << 15 |
                                ((uint32_t)psSettings->u64Frac) << 3 |
                                0x0;

    // R1
    puRegisters->u32Register1 = (u32PhaseValue != 0 ? 1 : 0) << 28 |
                                (u32Prescaler == (8/9) ? 1 : 0) << 27 |
                                (u32PhaseValue == 0 ? 1 : 0) << 15 |
                                (uint32_t)psSettings->u64Mod << 3 |
                                0x1;

    // R2
    puRegisters->u32Register2 = (uint32_t)eLowNoiseOrLowSpurMode << 29 |
                                (uint32_t)eMuxOut << 26 |
                                (bRefDoubler ? 1 : 0) << 25 |
                                (bRefDiv2 ? 1 : 0) << 24 |
                                u32RCounter << 14 |
                                (bDoubleBufR4 ? 1 : 0) << 13 |
                                ADF435x_iLookupVal(fChargePumpCurrent, afChargePumpCurrent, sizeof(afChargePumpCurrent) / sizeof(float)) << 9 |
                                (psSettings->u64Frac == 0 ? 0 : 1) << 8 |
                                (fLDP == 10.0 ? 0 : 1) << 7 |
                                (uint32_t)ePDPolarity << 6 |
                                (bPowerDown ? 1 : 0) << 5 |
                                (bCPTristate ? 1 : 0) << 4 |
                                (bCounterReset ? 1 : 0) << 3 |
                                0x2;

    // R3
    puRegisters->u32Register3 = (bCSR ? 1 : 0) << 18 |
                                (uint32_t)eClockDivMode << 15 |
                                u32ClockDividerValue << 3 |
                                0x3;
    
    if(eDeviceType == E_ADF435X_DEVICE_TYPE_ADF4351)
    {
        puRegisters->u32Register3 |= (uint32_t)eBandSelectClockMode << 23 |
                                        ADF435x_iLookupVal(fABP, afABP, sizeof(afABP) / sizeof(float)) << 22 |
                                        (bChargeCancel ? 1 : 0) << 21;
    }

    // R4
    puRegisters->u32Register4 = (uint32_t)eFeedbackSelect << 23 |
                                u32OutputDividerSelect << 20 |
                                u32BandSelectClockDivider << 12 |
                                (bVCOPowerDown ? 1 : 0) << 11 |
                                (bMuteTillLockDetect ? 1 : 0) << 10 |
                                (uint32_t)eAuxOutputSelect << 9 |
                                (bAuxOutputEnable ? 1 : 0) << 8 |
                                ADF435x_iLookupVal(fAuxOutputPower, afOutputPower, sizeof(afOutputPower) / sizeof(float)) << 6 |
                                (bOutputEnable ? 1 : 0) << 5 |
                                ADF435x_iLookupVal(fOutputPower, afOutputPower, sizeof(afOutputPower) / sizeof(float)) << 3 |
                                0x4;

    // R5
    puRegisters->u32Register5 = (uint32_t)eLDPinMode << 22 |
                                3 << 19 |
                                0x5;


    if(ADF435x_eVerbosity >= E_ADF435X_VERBOSITY_HIGH) printf("R0=%8x R1=%8x R2=%8x R3=%8x R4=%8x R5=%8x\n", puRegisters->u32Register0, puRegisters->u32Register1, puRegisters->u32Register2, puRegisters->u32Register3, puRegisters->u32Register4, puRegisters->u32Register5);
    // printf("R0=%8x R1=%8x R2=%8x R3=%8x R4=%8x R5=%8x\n", puRegisters->u32Register0, puRegisters->u32Register1, puRegisters->u32Register2, puRegisters->u32Register3, puRegisters->u32Register4, puRegisters->u32Register5);

    return true;
}

static float ADF435x_fGCD(float a, float b)
{
    while(1)
    {
        if(a == 0.0)
        {
            return b;
        }
        else if(b == 0.0)
        {
            return a;
        }
        else if(a > b)
        {
            a = fmod(a, b);
        }
        else
        {
            b = fmod(b, a);
        }
    }
}

static bool ADF435x_bCheckUint(char *acName, uint32_t u32Val, uint32_t u32Max)
{
    if(u32Val > u32Max)
    {
        printf("%s must be an integer greater than or equal to 0, and less than %d, its currently %d\n", acName, u32Max, u32Val);
        return false;
    }
    return true;
}

static bool ADF435x_bCheckLookupVal(char *acName, float fVal, float *pfArray, int iArrayLen)
{
    for(int n = 0; n < iArrayLen; n++)
    {
        if(pfArray[n] == fVal)
        {
            return true;
        }
    }

    printf("Value %s:%f is not in the array\n", acName, fVal);

    return false;
}

static int ADF435x_iLookupVal(float fVal, float *pfArray, int iArrayLen)
{
    for(int n = 0; n < iArrayLen; n++)
    {
        if(pfArray[n] == fVal)
        {
            return n;
        }
    }

    printf("Value %f is not in the array\n", fVal);

    return 0;
}