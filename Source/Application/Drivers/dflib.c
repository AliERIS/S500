// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
* This file is part of the DZ09 project.
*
* Copyright (C) 2022 AJScorp
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
#include "systemconfig.h"

const uint8_t GD25LQ64SPI[] = {0x01, 0x06, 0x01, 0x50, 0x03, 0x01, 0x02, 0x02, 0x01, 0x38, 0x00};
const uint8_t GD25LQ64QPI[] = {0x02, 0xC0, 0x12, 0x00};

const TDFCONFIG DFConfigList[] =
{
    {
        0x001760C8, "GD25LQ64", true, false, 256, 32768, BR_64K | BR_32K | BR_4K,
        GD25LQ64SPI, GD25LQ64QPI,
        0x00010000, 0x0C0B3771, 0x52F80210, 0x00000001, 0x00000000, 0x13000013, 0x00040004
    },
    {   0x00000000}
};
