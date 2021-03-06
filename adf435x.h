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

#ifndef _ADF4351_H_
#define _ADF4351_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum{
	E_ADF435X_VERBOSITY_LOW = 0,
	E_ADF435X_VERBOSITY_MEDIUM = 1,
	E_ADF435X_VERBOSITY_HIGH = 2,
} ADF435X_teVerbosity;

typedef enum {
    E_ADF435X_DEVICE_TYPE_ADF4350 = 0,
    E_ADF435X_DEVICE_TYPE_ADF4351 = 1,
} ADF435X_teDeviceType;

typedef enum {
    E_ADF435X_LOW_NOISE_MODE = 0,
    E_ADF435X_LOW_SPUR_MODE = 1,
} ADF435X_teLowNoiseOrLowSpurMode;

typedef enum {
    E_ADF435X_MUX_OUT_TRISTATE = 0,
    E_ADF435X_MUX_OUT_DVDD = 1,
    E_ADF435X_MUX_OUT_DGND = 2,
    E_ADF435X_MUX_OUT_R_COUNTER_OUTPUT = 3,
    E_ADF435X_MUX_OUT_N_DIVIDER_OUTPUT = 4,
    E_ADF435X_MUX_OUT_ANALOG_LOCK_DETECT = 5,
    E_ADF435X_MUX_OUT_DIGITAL_LOCK_DETECT = 6,
} ADF435X_teMuxOut;

typedef enum {
    E_ADF435X_PD_POLARITY_NEGATIVE = 0,
    E_ADF435X_PD_POLARITY_POSITIVE = 1,
} ADF435X_tePDPolarity;

typedef enum {
    E_ADF435X_BAND_SELECT_CLOCK_MODE_LOW = 0,
    E_ADF435X_BAND_SELECT_CLOCK_MODE_HIGH = 1,
} ADF435X_teBandSelectClockMode;

typedef enum {
    E_ADF435X_CLOCK_DIVIDER_MODE_OFF = 0,
    E_ADF435X_CLOCK_DIVIDER_MODE_FAST_LOCK_ENABLE = 1,
    E_ADF435X_CLOCK_DIVIDER_MODE_RESYNC_ENABLE = 2,
} ADF435X_teClockDviderMode;

typedef enum {
    E_ADF435X_FEEDBACK_SELECT_DIVIDER = 0,
    E_ADF435X_FEEDBACK_SELECT_FUNDAMENTAL = 1,
} ADF435X_teFeedbackSelect;

typedef enum {
    E_ADF435X_AUX_OUTPUT_SELECT_DIVIDED_OUTPUT = 0,
    E_ADF435X_AUX_OUTPUT_SELECT_FUNDAMENTAL = 1,
} ADF435X_teAuxOutputSelect;

typedef enum {
    E_ADF435X_LDPIN_MODE_LOW = 0,
    E_ADF435X_LDPIN_MODE_DIGITAL_LOCK_DETECT = 1,
    E_ADF435X_LDPIN_MODE_DIGITAL_LOCK_HIGH = 2,
} ADF435X_teLDPinMode;

typedef enum {
    E_ADF435X_PRESCALER_4_OVER_5 = 0,
    E_ADF435X_PRESCALER_8_OVER_9 = 1,
} ADF435X_tePrescaler;

typedef enum {
    E_ADF435X_OUTPUT_POWER_MINUS_4dBm = 0,
    E_ADF435X_OUTPUT_POWER_MINUS_1dBm = 1,
    E_ADF435X_OUTPUT_POWER_PLUS_2dBm = 2,
    E_ADF435X_OUTPUT_POWER_PLUS_5dBm = 3,
} ADF435X_teOutputPower;

typedef struct {
    ADF435X_teDeviceType eDeviceType;
    ADF435X_teFeedbackSelect eFeedbackSelect;
    ADF435X_teBandSelectClockMode eBandSelectClockMode;
    ADF435X_teLowNoiseOrLowSpurMode eLowNoiseOrLowSpurMode;
    ADF435X_teMuxOut eMuxOut;
    ADF435X_tePDPolarity ePDPolarity;
    ADF435X_teClockDviderMode eClockDivMode;
    ADF435X_teAuxOutputSelect eAuxOutputSelect;
    ADF435X_teLDPinMode eLDPinMode;
    ADF435X_tePrescaler ePrescaler;
    ADF435X_teOutputPower eOutputPower;
    ADF435X_teOutputPower eAuxOutputPower;

    uint64_t u64ReferenceFrequencyHz;
    uint64_t u64ChannelSpacingHz;

    uint32_t u32RCounter;
    uint32_t u32PhaseValue;
    uint32_t u32ClockDividerValue;

    float fChargePumpCurrent;
    float fLDP;
    float fABP;

    bool bEnableGCD;
    bool bRefDoubler;
    bool bRefDiv2;
    bool bDoubleBufR4;
    bool bPowerDown;
    bool bCPTristate;
    bool bCounterReset;
    bool bChargeCancel;
    bool bCSR;
    bool bVCOPowerDown;
    bool bMuteTillLockDetect;
    bool bAuxOutputEnable;
    bool bOutputEnable;

} ADF435x_tsOptions;

typedef struct {
    uint64_t u64Int;
    uint64_t u64Mod;
    uint64_t u64Frac;
    uint64_t u64OutputDivider;
    uint64_t u64BandSelectClockDivider;
} ADF435X_tsSettings;

typedef union {
    struct {
        uint32_t u32Register0;
        uint32_t u32Register1;
        uint32_t u32Register2;
        uint32_t u32Register3;
        uint32_t u32Register4;
        uint32_t u32Register5;
    };
    uint32_t au32[6];
} ADF435X_tuRegisters;

void ADF435x_vInit(ADF435X_teVerbosity eVerbosity);
void ADF435x_vGetOptions(ADF435x_tsOptions *psOptions);
bool ADF435x_bCalculateSettings(uint64_t u64Frequency, ADF435x_tsOptions *psOptions, ADF435X_tsSettings *psSettings);
bool ADF435x_bGenerateRegisters(ADF435x_tsOptions *psOptions, ADF435X_tsSettings *psSettings, ADF435X_tuRegisters *puRegisters);


#endif // _ADF4351_H_