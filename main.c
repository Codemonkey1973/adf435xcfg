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

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include "common.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include "ch341.h"
#include "adf435x.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

typedef enum
{
	E_STATUS_OK,
	E_STATUS_AGAIN,
	E_STATUS_ERROR_TIMEOUT,
	E_STATUS_ERROR_WRITING,
	E_STATUS_OUT_OF_RANGE,
	E_STATUS_NULL_PARAMETER,
	E_STATUS_FAIL
} teStatus;

typedef enum{
	E_VERBOSITY_LOW,
	E_VERBOSITY_MEDIUM,
	E_VERBOSITY_HIGH
} teVerbosity;

typedef struct
{
	volatile bool_t		bExitRequest;
	uint64_t			u64Frequency;
	bool				bSweepMode;
	uint64_t			u64FreqLow;
	uint64_t			u64FreqHigh;
	uint64_t			u64FreqStep;
	teVerbosity			eVerbosity;
	int					iDelay;
} tsInstance;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

static void vParseCommandLineOptions(tsInstance *psInstance, int argc, char *argv[]);

#ifdef _WIN32
static BOOL WINAPI bCtrlHandler(DWORD dwCtrlType);
#endif

void vSetFrequency(uint64_t u64FrequencyHz);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

static tsInstance sInstance;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: main
 *
 * DESCRIPTION:
 * Application entry point
 *
 * RETURNS:
 * int
 *
 ****************************************************************************/
int main(int argc, char *argv[])
{


	int n;

	bool_t bOk = TRUE;

	char acOut[] = {0xaa, 0x55, 0x01, 0x02, 0x03, 0x04};
	char acWord[4] = {0};

	/* Initialise application state and set some defaults */
	sInstance.bExitRequest = FALSE;
	sInstance.eVerbosity = E_VERBOSITY_MEDIUM;

	sInstance.u64Frequency = 50000000;
	sInstance.bSweepMode = false;
	sInstance.u64FreqLow = 50000000;
	sInstance.u64FreqHigh = 100000000;
	sInstance.u64FreqStep = 100000;
	sInstance.iDelay = 1;

	printf("+----------------------------------------------------------------------+\n" \
	"|              ADF435xCFG (ADF435x Configurator)                       |\n" \
	"| Copyright (C) 2021 Lee Mitchell <lee@indigopepper.com>               |\n" \
	"|                                                                      |\n" \
	"| This program comes with ABSOLUTELY NO WARRANTY.                      |\n" \
	"| This is free software, and you are welcome to redistribute it        |\n" \
	"| under certain conditions; See the GNU General Public License         |\n" \
	"| version 3 or later for more details. You should have received a copy |\n" \
	"| of the GNU General Public License along with this software),         |\n" \
	"| If not, see <http://www.gnu.org/licenses/>.                          |\n" \
	"+----------------------------------------------------------------------+\n\n");

    SetConsoleCtrlHandler(bCtrlHandler, TRUE);

	vParseCommandLineOptions(&sInstance, argc, argv);

	if(!CH341DeviceInit())
	{
		printf("Error at line %d\n", __LINE__);
	}

	if(!CH341ChipSelect(0, FALSE))
	{
		printf("Error at line %d\n", __LINE__);
	}

	ADF435x_Init(E_ADF435X_VERBOSITY_LOW);

	if(sInstance.bSweepMode)
	{
		/* Main program loop, execute until we get a signal requesting to exit */
		while(!sInstance.bExitRequest)
		{

			for(uint64_t f = sInstance.u64FreqLow; (f <= sInstance.u64FreqHigh) && !sInstance.bExitRequest; f += sInstance.u64FreqStep)
			{
				printf("\r %d.%06dMHz    ", f / 1000000, f % 1000000);
				vSetFrequency(f);
				Sleep(sInstance.iDelay);
			}

		}

	}
	else
	{
		vSetFrequency(sInstance.u64Frequency);
	}

	CH341DeviceRelease();

	printf("\nDone!\n");


	return EXIT_SUCCESS;
}


/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: vParseCommandLineOptions
 *
 * DESCRIPTION:
 * Parse command line options
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
static void vParseCommandLineOptions(tsInstance *psInstance, int argc, char *argv[])
{

	int n;
	int c;
	int index, value, from, to, port;

	static const struct option lopts[] = {
		{ "freq",			required_argument,	0, 	'f'	},

		{ "sweep",			no_argument,		0, 	's'	},

		{ "low",			required_argument,	0, 	'l'	},
		{ "high",			required_argument,	0, 	'h'	},
		{ "resolution",		required_argument,	0, 	'r'	},
		{ "delay",			required_argument,	0, 	'd'	},

        { "verbosity",     	required_argument, 	0,  'v' },

        { "help",          	required_argument, 	0,  '?' },

		{ NULL, 0, 0, 0 },
	};


	while(1)
	{

		c = getopt_long(argc, argv, "f:sl:h:r:d:v:?:h:", lopts, NULL);

		if (c == -1)
			break;

		switch(c)
		{

		case 'f':
			psInstance->u64Frequency = _atoi64(optarg);
			printf("Frequency = %d.%dMHz\n", psInstance->u64Frequency / 1000000, psInstance->u64Frequency % 1000000);
			break;

		case 's':
			psInstance->bSweepMode = true;
			printf("Sweep mode enabled\n");
			break;

		case 'l':
			psInstance->u64FreqLow = _atoi64(optarg);
			printf("Low Frequency = %d.%dMHz\n", psInstance->u64FreqLow / 1000000, psInstance->u64Frequency % 1000000);
			break;

		case 'h':
			psInstance->u64FreqHigh = _atoi64(optarg);
			printf("High Frequency = %d.%dMHz\n", psInstance->u64FreqHigh / 1000000, psInstance->u64Frequency % 1000000);
			break;

		case 'r':
			psInstance->u64FreqStep = _atoi64(optarg);
			printf("Step Frequency = %d.%dMHz\n", psInstance->u64FreqStep / 1000000, psInstance->u64Frequency % 1000000);
			break;

		case 'd':
			psInstance->iDelay = atoi(optarg);
			printf("Step Delay = %dms\n", psInstance->iDelay);
			break;

		case 'v':
			switch(atoi(optarg))
			{

			case 0:
				psInstance->eVerbosity = E_VERBOSITY_LOW;
				break;
			
			case 1:
				psInstance->eVerbosity = E_VERBOSITY_MEDIUM;
				break;
			
			case 2:
				psInstance->eVerbosity = E_VERBOSITY_HIGH;
				break;

			default:
				psInstance->eVerbosity = E_VERBOSITY_HIGH;
				break;

			}
			break;

        case '?':
		default:
			printf("\nUsage: %s <options>\n\n", argv[0]);
			puts("  -f --frequency <freq>           Set the output frequency to <freq> Hz\n\n"
				"  -s --sweep                       Enable sweep mode\n\n"
				"  -l --low <freq>                  Set the sweep lower frequency to <freq> Hz\n\n"
				"  -h --high <freq>                 Set the sweep upper frequency to <freq> Hz\n\n"

				"  -r --resolution <freq>           Set the sweep step frequency to <freq> Hz\n\n"
				"  -d --delay <delay>               Set the sweep mode step delay to <delay> milliseconds\n\n"
				// "  -v --verbosity <level>           Set verbosity level 0, 1 & 2 are valid\n\n"
				"  -? --help                        Display help\n\n"
				);
			exit(EXIT_FAILURE);
			break;
		}

	}

}


/****************************************************************************
 *
 * NAME: bCtrlHandler
 *
 * DESCRIPTION:
 * Handles Ctrl+C events
 *
 * RETURNS:
 * BOOL
 *
 ****************************************************************************/
#ifdef _WIN32
static BOOL WINAPI bCtrlHandler(DWORD dwCtrlType)
{
    switch (dwCtrlType)
    {

    case CTRL_C_EVENT:
    case CTRL_SHUTDOWN_EVENT:
    case CTRL_BREAK_EVENT:
        printf("\nExit requested\n");
        sInstance.bExitRequest = TRUE;
        return TRUE;

    default:
        return FALSE;
    }
}
#endif


void vSetFrequency(uint64_t u64FrequencyHz)
{

	char acWord[4] = {0};

	ADF435X_tsSettings sSettings;
	ADF435X_tuRegisters uRegisters;

	ADF435x_CalculateSettings(u64FrequencyHz, &sSettings);
	ADF435x_GenerateRegisters(&sSettings, &uRegisters);

	for(int n = 6; n > 0; n--)
	{
	
		// printf("Sending %d - %08x\n", n-1, uRegisters.au32[n-1]);

		acWord[0] = (uRegisters.au32[n-1] >> 24) & 0xff;
		acWord[1] = (uRegisters.au32[n-1] >> 16) & 0xff;
		acWord[2] = (uRegisters.au32[n-1] >> 8) & 0xff;
		acWord[3] = (uRegisters.au32[n-1] >> 0) & 0xff;

		if(!CH341ChipSelect(0, TRUE))
		{
			printf("Error at line %d\n", __LINE__);
		}

		if(CH341WriteSPI(acWord, 4) <= 0)
		{
			printf("Error at line %d\n", __LINE__);
		}

		if(!CH341ChipSelect(0, FALSE))
		{
			printf("Error at line %d\n", __LINE__);
		}

	}

}


/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

