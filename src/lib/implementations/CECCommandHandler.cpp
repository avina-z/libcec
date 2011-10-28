/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011 Pulse-Eight Limited.  All rights reserved.
 * libCEC(R) is an original work, containing original code.
 *
 * libCEC(R) is a trademark of Pulse-Eight Limited.
 *
 * This program is dual-licensed; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * Alternatively, you can license this library under a commercial license,
 * please contact Pulse-Eight Licensing for more information.
 *
 * For more information contact:
 * Pulse-Eight Licensing       <license@pulse-eight.com>
 *     http://www.pulse-eight.com/
 *     http://www.pulse-eight.net/
 */

#include "CECCommandHandler.h"
#include "../devices/CECBusDevice.h"
#include "../CECProcessor.h"

using namespace CEC;

CCECCommandHandler::CCECCommandHandler(CCECBusDevice *busDevice)
{
  m_busDevice = busDevice;
}

bool CCECCommandHandler::HandleCommand(const cec_command &command)
{
  bool bHandled(true);

  if (command.destination == m_busDevice->GetMyLogicalAddress())
  {
    switch(command.opcode)
    {
    case CEC_OPCODE_GIVE_PHYSICAL_ADDRESS:
      HandleGivePhysicalAddress(command);
      break;
    case CEC_OPCODE_GIVE_OSD_NAME:
      HandleGiveOSDName(command);
      break;
    case CEC_OPCODE_GIVE_DEVICE_VENDOR_ID:
      HandleGiveDeviceVendorId(command);
      break;
    case CEC_OPCODE_DEVICE_VENDOR_ID:
      HandleDeviceVendorId(command);
      break;
    case CEC_OPCODE_VENDOR_COMMAND_WITH_ID:
      HandleDeviceVendorCommandWithId(command);
      break;
    case CEC_OPCODE_GIVE_DECK_STATUS:
      HandleGiveDeckStatus(command);
      break;
    case CEC_OPCODE_MENU_REQUEST:
      HandleMenuRequest(command);
      break;
    case CEC_OPCODE_GIVE_DEVICE_POWER_STATUS:
      HandleGiveDevicePowerStatus(command);
      break;
    case CEC_OPCODE_GET_CEC_VERSION:
      HandleGetCecVersion(command);
      break;
    case CEC_OPCODE_USER_CONTROL_PRESSED:
      HandleUserControlPressed(command);
      break;
    case CEC_OPCODE_USER_CONTROL_RELEASE:
      HandleUserControlRelease(command);
      break;
    default:
      UnhandledCommand(command);
      bHandled = false;
      break;
    }
  }
  else if (command.destination == CECDEVICE_BROADCAST)
  {
    CStdString strLog;
    switch (command.opcode)
    {
    case CEC_OPCODE_REQUEST_ACTIVE_SOURCE:
      HandleRequestActiveSource(command);
      break;
    case CEC_OPCODE_SET_STREAM_PATH:
      HandleSetStreamPath(command);
      break;
    case CEC_OPCODE_ROUTING_CHANGE:
      HandleRoutingChange(command);
      break;
    case CEC_OPCODE_DEVICE_VENDOR_ID:
      HandleDeviceVendorId(command);
      break;
    case CEC_OPCODE_VENDOR_COMMAND_WITH_ID:
      HandleDeviceVendorCommandWithId(command);
     break;
    default:
      UnhandledCommand(command);
      bHandled = false;
      break;
    }
  }
  else
  {
    CStdString strLog;
    strLog.Format("ignoring frame: destination: %u != %u", command.destination, (uint8_t)m_busDevice->GetMyLogicalAddress());
    m_busDevice->AddLog(CEC_LOG_DEBUG, strLog.c_str());
    bHandled = false;
  }

  return bHandled;
}

bool CCECCommandHandler::HandleDeviceVendorCommandWithId(const cec_command &command)
{
  m_busDevice->GetProcessor()->ParseVendorId(command.initiator, command.parameters);
  return true;
}

bool CCECCommandHandler::HandleDeviceVendorId(const cec_command &command)
{
  m_busDevice->GetProcessor()->ParseVendorId(command.initiator, command.parameters);
  return true;
}

bool CCECCommandHandler::HandleGetCecVersion(const cec_command &command)
{
  m_busDevice->GetProcessor()->ReportCECVersion(command.initiator);
  return true;
}

bool CCECCommandHandler::HandleGiveDeckStatus(const cec_command &command)
{
  // need to support opcodes play and deck control before doing anything with this
  m_busDevice->GetProcessor()->TransmitAbort(command.initiator, CEC_OPCODE_GIVE_DECK_STATUS);
  return true;
}

bool CCECCommandHandler::HandleGiveDevicePowerStatus(const cec_command &command)
{
  m_busDevice->GetProcessor()->ReportPowerState(command.initiator);
  return true;
}

bool CCECCommandHandler::HandleGiveDeviceVendorId(const cec_command &command)
{
  m_busDevice->GetProcessor()->ReportVendorID(command.initiator);
  return true;
}

bool CCECCommandHandler::HandleGiveOSDName(const cec_command &command)
{
  m_busDevice->GetProcessor()->ReportOSDName(command.initiator);
  return true;
}

bool CCECCommandHandler::HandleGivePhysicalAddress(const cec_command &command)
{
  m_busDevice->GetProcessor()->ReportPhysicalAddress();
  return true;
}

bool CCECCommandHandler::HandleMenuRequest(const cec_command &command)
{
  if (command.parameters[0] == CEC_MENU_REQUEST_TYPE_QUERY)
    m_busDevice->GetProcessor()->ReportMenuState(command.initiator);
  return true;
}

bool CCECCommandHandler::HandleRequestActiveSource(const cec_command &command)
{
  CStdString strLog;
  strLog.Format(">> %i requests active source", (uint8_t) command.initiator);
  m_busDevice->AddLog(CEC_LOG_DEBUG, strLog.c_str());
  m_busDevice->GetProcessor()->BroadcastActiveSource();
  return true;
}

bool CCECCommandHandler::HandleRoutingChange(const cec_command &command)
{
  if (command.parameters.size == 4)
  {
    uint16_t iOldAddress = ((uint16_t)command.parameters[0] << 8) | ((uint16_t)command.parameters[1]);
    uint16_t iNewAddress = ((uint16_t)command.parameters[2] << 8) | ((uint16_t)command.parameters[3]);

    m_busDevice->SetPhysicalAddress(iNewAddress, iOldAddress);
  }
  return true;
}

bool CCECCommandHandler::HandleSetStreamPath(const cec_command &command)
{
  if (command.parameters.size >= 2)
  {
    int streamaddr = ((uint16_t)command.parameters[0] << 8) | ((uint16_t)command.parameters[1]);
    CStdString strLog;
    strLog.Format(">> %i requests stream path from physical address %04x", command.initiator, streamaddr);
    m_busDevice->AddLog(CEC_LOG_DEBUG, strLog.c_str());
    if (streamaddr == m_busDevice->GetMyPhysicalAddress())
      m_busDevice->GetProcessor()->BroadcastActiveSource();
  }
  return true;
}

bool CCECCommandHandler::HandleUserControlPressed(const cec_command &command)
{
  if (command.parameters.size > 0)
  {
    m_busDevice->GetProcessor()->AddKey();

    if (command.parameters[0] <= CEC_USER_CONTROL_CODE_MAX)
    {
      CStdString strLog;
      strLog.Format("key pressed: %1x", command.parameters[0]);
      m_busDevice->AddLog(CEC_LOG_DEBUG, strLog.c_str());

      m_busDevice->GetProcessor()->SetCurrentButton((cec_user_control_code) command.parameters[0]);
    }
  }
  return true;
}

bool CCECCommandHandler::HandleUserControlRelease(const cec_command &command)
{
  m_busDevice->GetProcessor()->AddKey();
  return true;
}

void CCECCommandHandler::UnhandledCommand(const cec_command &command)
{
  m_busDevice->GetProcessor()->AddCommand(command);;
}