// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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
#include "systemconfig.h"
#include "usb.h"
#include "usb9.h"
#include "usbdevice.h"

static boolean USB9_GetDescriptor(pUSBSETUP Setup)
{
    boolean Error = false;

    if (Setup->bmRequestType != USB_CMD_STDDEVIN)
        Error = true;
    else
    {
        switch(Setup->wValue & USB_CMD_DESCMASK)
        {
        case USB_CMD_DEVICE:
            DebugPrint("->DEVICE\r\n");
            USB_PrepareDataTransmit(USB_EP0, (uint8_t *)USBDevDescriptor, min(DEV_LENGTH, Setup->wLength));
            break;
        case USB_CMD_CONFIG:
            DebugPrint("->CONFIG\r\n");
            USB_PrepareDataTransmit(USB_EP0, USBCfgDescriptor, min(USB_DevConfigSize(), Setup->wLength));
            break;
        case USB_CMD_STRING:
        {
            uint8_t Index = Setup->wValue & 0x00FF;
            uint8_t *String = USB_GetString(Index);

            DebugPrint("->STRING\r\n");
            if (String == NULL) Error = true;
            else USB_PrepareDataTransmit(USB_EP0, (uint8_t *)String, min(String[0], Setup->wLength));
        }
        break;
        default:
            Error = true;
            break;
        }
    }
    return Error;
}

static void USB9_HandleStdRequest(pUSBSETUP Setup)
{
    boolean Error = false;

    switch(Setup->bRequest)
    {
    case USB_SET_ADDRESS:
        DebugPrint("SET_ADDRESS %02X\r\n", Setup->bmRequestType);
        USB_SetDeviceAddress(Setup->wValue);
        break;
    case USB_GET_DESCRIPTOR:
        DebugPrint("GET_DESCRIPTOR\r\n");
        Error = USB9_GetDescriptor(Setup);
        break;
    case USB_GET_CONFIGURATION:
        DebugPrint("GET_CONFIGURATION\r\n");
//        USB_PrepareTransmitData(USB_EP0, &DevConfig, 1);
        break;
    case USB_GET_STATUS:
    {
        uint16_t DevStatus = 0;                                                                     // We does not process "Self Powered" and "Remote Wakeup" fields.

        DebugPrint("GET_STATUS\r\n");
//        USB_StartTransmitData(USB_EP0, (uint8_t *)&DevStatus, 2);
//        Error = false;
    }
    break;
    case USB_CLEAR_FEATURE:
        DebugPrint("CLEAR_FEATURE\r\n");
        break;
    case USB_SET_FEATURE:
        DebugPrint("SET_FEATURE\r\n");
        break;
    case USB_SET_CONFIGURATION:
        DebugPrint("SET_CONFIGURATION\r\n");
        break;
    case USB_GET_INTERFACE:
        DebugPrint("GET_INTERFACE\r\n");
        break;
    case USB_SET_INTERFACE:
        DebugPrint("SET_INTERFACE\r\n");
        break;
    case USB_SET_DESCRIPTOR:
    case USB_SYNCH_FRAME:
    default:
        break;
    }

    if (EPState[USB_EP0].Stage == EPSTAGE_IDLE)                                                     // no data to transmit
        USB_UpdateEPState(USB_EP0, true, Error, true);
    else USB_UpdateEPState(USB_EP0, true, Error, false);
}

void USB9_HandleSetupRequest(pUSBSETUP Setup)
{
    switch(Setup->bmRequestType & USB_CMD_TYPEMASK)
    {
    case USB_CMD_STDREQ:
        DebugPrint("STDREQ: ");
        USB9_HandleStdRequest(Setup);
        break;
    case USB_CMD_CLASSREQ:
        if ((Setup->bmRequestType == USB_CMD_CLASSIFIN) ||
                (Setup->bmRequestType == USB_CMD_CLASSIFOUT))
        {
            //USB_ITF_ReqHandler(Setup);
        }
        else USB_UpdateEPState(USB_EP0, true, true, false);
        break;
    case USB_CMD_VENDREQ:
        //USB_ITF_VendorReqHandler(Setup);
        break;
    default:
        USB_UpdateEPState(USB_EP0, true, true, false);
        break;
    }
}
