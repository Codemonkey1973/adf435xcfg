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

#ifndef COMMON_H
#define COMMON_H

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <stdint.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/*--------------------------------------------------------------------------*/
/*          Boolean Constants                                               */
/*--------------------------------------------------------------------------*/

#if !defined FALSE && !defined TRUE
#define TRUE            (1)   /* page 207 K+R 2nd Edition */
#define FALSE           (0)
#endif /* !defined FALSE && #if !defined TRUE */

/*--------------------------------------------------------------------------*/
/*          Null pointers                                                   */
/*--------------------------------------------------------------------------*/

/* NULL data pointer */
#if !defined NULL
#define NULL                    ( (void *) 0 )
#endif

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

typedef uint8_t                 bool_t;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

#endif // COMMON_H

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
