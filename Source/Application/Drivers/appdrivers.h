/*
* This file is part of the DZ09 project.
*
* Copyright (C) 2019 AJScorp
*
* This program is free software; you can redistribute it and/or modify 
* it under the terms of the GNU General Public License as published by 
* the Free Software Foundation; version 2 of the License.
*
* This program is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License 
* along with this program; if not, write to the Free Software 
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. 
*/
#ifndef _APPDRIVERS_H_
#define _APPDRIVERS_H_

/* LCD driver */
#if defined(_LCD_DRIVER_ASSIGNED_)
#undefine _LCD_DRIVER_ASSIGNED_
#endif
#if defined(_ILI9341_LCD_DRIVER_)
#include "ili9341.h"
#define LCDDRV_Initialize()                 ILI9341_Initialize()
#define LCDDRV_Sleep()                      ILI9341_SleepLCD()
#define LCDDRV_Resume()                     ILI9341_ResumeLCD()
#define LCDDRV_SetOutputWindow(a, b, c, d)  ILI9341_SetOutputWindow(a, b, c, d)
#define _LCD_DRIVER_ASSIGNED_
#endif
#if defined(_NO_LCD_DRIVER_)
#if defined(_LCD_DRIVER_ASSIGNED_)
#error Duplicate LCD driver definition!
#else
#define LCDDRV_Initialize()                 false
#define LCDDRV_Sleep()
#define LCDDRV_Resume()
#define LCDDRV_SetOutputWindow(a, b, c, d)  NULL
#define _LCD_DRIVER_ASSIGNED_
#endif
#endif
#if !defined(_LCD_DRIVER_ASSIGNED_)
#error LCD driver not assigned!
#endif

/* Touch screen driver */
#if defined(_TS_DRIVER_ASSIGNED_)
#undefine _TS_DRIVER_ASSIGNED_
#endif
#if defined(_FT6236_TS_DRIVER_)
#include "ft6236.h"
#define TSDRV_Initialize()                  FT6236_Initialize()
#define TSDRV_Sleep()                       FT6236_Sleep()
#define TSDRV_Resume()                      FT6236_Resume()
#define _TS_DRIVER_ASSIGNED_
#endif
#if defined(_NO_TS_DRIVER_)
#if defined(_TS_DRIVER_ASSIGNED_)
#error Duplicate touch screen driver definition!
#else
#define TSDRV_Initialize()                  false
#define TSDRV_Sleep()
#define TSDRV_Resume()
#define _TS_DRIVER_ASSIGNED_
#endif
#endif
#if !defined(_TS_DRIVER_ASSIGNED_)
#error Touch screen driver not assigned!
#endif

#endif /* _APPDRIVERS_H_ */
