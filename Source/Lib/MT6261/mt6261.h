/*
* This file is part of the DZ09 project.
*
* Copyright (C) 2020, 2019 AJScorp
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
#ifndef _MT6261_H_
#define _MT6261_H_

#define CONFIG_base                 0xA0010000                                                      //Configuration Registers(Clock, Power Down, Version and Reset)
#define GPIO_Base                   0xA0020000                                                      //General Purpose Inputs/Outputs
#define RGU_Base                    0xA0030000                                                      //Reset generation unit
#define CIRQ_base                   0xA0060000                                                      //Interrupt Controller
#define USART1_base                 0xA0080000                                                      //USART 1
#define USART2_base                 0xA0090000                                                      //USART 2
#define USART3_base                 0xA00A0000                                                      //USART 3 add for MT6261
//#define BTIF_base                   0xA00B0000                                                      //BT interface
#define GPT_base                    0xA00C0000                                                      //General Purpose Timers
#define PWM1_base                   0xA00E0000                                                      //Pulse-Width Modulation Outputs
#define SFI_base                    0xA0140000                                                      //Serial flash interface
#define MIXED_base                  0xA0170000                                                      //Analog Chip Interface Controller (PLL, CLKSQ, FH, CLKSW and SIMLS)
#define PLL_base                    MIXED_base
#define TOPSM_base                  0xA0180000                                                      //TOPSM0
#define PWM4_base                   0xA0280000                                                      //Pulse-Width Modulation Outputs
#define LCDIF_Base                  0xA0450000                                                      //LCD controller
//#define ARM_CONFG_base              0xA0500000
#define BOOT_ENG_base               0xA0510000                                                      //boot engine
#define CACHE_Base                  0xA0530000                                                      //TCM Cache configuration
#define MPU_Base                    0xA0540000                                                      //Memory protection unit
#define PMU_base                    0xA0700000                                                      //PMU mixedsys
#define RTC_Base                    0xA0710000                                                      //Real Time Clock
#define ABBSYS_SD_base              0xA0720000                                                      //Analog baseband (ABB) controller
#define ANA_CFGSYS_base             0xA0730000                                                      //Analog die (MT6100) Configuration Registers (Clock, Reset, etc.)
#define PWM_2CH_base                0xA0740000                                                      //Pulse-Width Modulation (2 channel)
#define ADIE_CIRQ_base              0xA0760000                                                      //Interrupt Controller (16-bit)

#define AUXADC_base                 0xA0790000                                                      //Auxiliary ADC Unit
#define USB_base                    0xA0900000                                                      // USB Controller

//#define KP_base                     0xA00D0000                                                      //Keypad controller
//#define MDCONFIG_base               0x83000000
//#define MD2GCONFG_base              0x82C00000
//#define MODEM_MBIST_CONFIG_base     0x83008000
//#define ARM_CONFG_base              0xA0500000
//#define BT_CONFG_base               0xA3300000
//#define AHB2DSPIO_base              0x82800000
//#define PATCH_base                  0x82CC0000
#define AFE_base                    0x82CD0000                                                      //Analog front-end
//#define BT_BTIF_base                0xA3330000

//Drivers
#include "drivers\rtc.h"
#include "drivers\tcmcache.h"
#include "drivers\gpt.h"
#include "drivers\nvic.h"
#include "drivers\gpio.h"
#include "drivers\pctl.h"
#include "drivers\rgu.h"
#include "drivers\pmu.h"
#include "drivers\usart.h"
#include "drivers\sfi.h"
#include "drivers\emi.h"
#include "drivers\pll.h"
#include "drivers\ustimer.h"
#include "drivers\lcdif.h"
#include "drivers\auxadc.h"
#include "drivers\afe.h"
#include "drivers\pwm.h"
#include "drivers\usb.h"

#endif /* _MT6261_H_ */
