/*
* Copyright (c) 2008, AMT – The Association For Manufacturing Technology (“AMT”)
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the AMT nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* DISCLAIMER OF WARRANTY. ALL MTCONNECT MATERIALS AND SPECIFICATIONS PROVIDED
* BY AMT, MTCONNECT OR ANY PARTICIPANT TO YOU OR ANY PARTY ARE PROVIDED "AS IS"
* AND WITHOUT ANY WARRANTY OF ANY KIND. AMT, MTCONNECT, AND EACH OF THEIR
* RESPECTIVE MEMBERS, OFFICERS, DIRECTORS, AFFILIATES, SPONSORS, AND AGENTS
* (COLLECTIVELY, THE "AMT PARTIES") AND PARTICIPANTS MAKE NO REPRESENTATION OR
* WARRANTY OF ANY KIND WHATSOEVER RELATING TO THESE MATERIALS, INCLUDING, WITHOUT
* LIMITATION, ANY EXPRESS OR IMPLIED WARRANTY OF NONINFRINGEMENT,
* MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. 

* LIMITATION OF LIABILITY. IN NO EVENT SHALL AMT, MTCONNECT, ANY OTHER AMT
* PARTY, OR ANY PARTICIPANT BE LIABLE FOR THE COST OF PROCURING SUBSTITUTE GOODS
* OR SERVICES, LOST PROFITS, LOSS OF USE, LOSS OF DATA OR ANY INCIDENTAL,
* CONSEQUENTIAL, INDIRECT, SPECIAL OR PUNITIVE DAMAGES OR OTHER DIRECT DAMAGES,
* WHETHER UNDER CONTRACT, TORT, WARRANTY OR OTHERWISE, ARISING IN ANY WAY OUT OF
* THIS AGREEMENT, USE OR INABILITY TO USE MTCONNECT MATERIALS, WHETHER OR NOT
* SUCH PARTY HAD ADVANCE NOTICE OF THE POSSIBILITY OF SUCH DAMAGES.
*/

#include "internal.hpp"
#include "modbus_adapter.hpp"
#include "yaml.h"
#include <iostream>
#include <fstream>

using namespace std;

ModbusAdapter::ModbusAdapter(int aPort)
  : Adapter(aPort, 1000)
{
  mConnected = 0;

  std::ifstream fin("modbus.yaml");
  YAML::Parser parser(fin);
  YAML::Node doc;
  parser.GetNextDocument(doc);
  const YAML::Node &comms = doc["communications"];
  comms["port"] >> mSerialPort;
  comms["baud"] >> mBaud;
  comms["dataBits"] >> mDataBits;
  comms["stopBits"] >> mStopBits;
  comms["parity"] >> mParity;
  comms["scanDelay"] >> mScanDelay;
  
  // Parse devices
  const YAML::Node &devices = doc["devices"];  
  for(unsigned i = 0; i < devices.size(); i++) {
    ModbusDevice device;
    const YAML::Node &node = devices[i];
    node["address"] >> device.mAddress;
    
    if (node.FindValue("name") != NULL) {
      node["name"] >> device.mName;    
    }
    
    mDevices.push_back(device);
  }
  
  std::vector<ModbusDevice>::iterator device;
  for (device = mDevices.begin(); device != mDevices.end(); device++) {
    const YAML::Node &data = doc["data"];  
    for(unsigned i = 0; i < data.size(); i++) {
      const YAML::Node &node = data[i];
      std::string type;
      ModbusData *data = NULL;
      int address;
    
      node["type"] >> type;
      node["address"] >> address;
    
      if (type == "coil") {
        data = new ModbusCoil(address);
      } else if (type == "register") {
        data = new ModbusRegister(address);
      } else if (type == "ieee_float") {
        data = new ModbusFloat(address);
      } else if (type == "double") {
        int scaling;
        const YAML::Node *scaleNode = node.FindValue("scalingAddress");
        if (scaleNode != NULL) {
          *scaleNode >> scaling;
          data = new ModbusDouble(address, scaling, 0);
        } else if ((scaleNode = node.FindValue("scaler")) != NULL) {
          *scaleNode >> scaling;
          data = new ModbusDouble(address, 0, scaling);
        } else {
          data = new ModbusDouble(address, 0, 1);
        }
      }
      else 
      {
        cerr << "Invalid modbus type: " << type << endl;
        cerr << "Must be: coil, register, or double" << endl;
        exit(1);
      }
    
      std::vector<std::string> nameList;
      const YAML::Node &names = node["names"];
      for(unsigned j = 0; j < names.size(); j++) {
        nameList.push_back(names[j]);
      }
    
      const YAML::Node *sizeNode = node.FindValue("size");
      int size = 1;
      std::vector<int> sizes;
      if (sizeNode != NULL) {
        if (sizeNode->GetType() == YAML::CT_SEQUENCE) {
          *sizeNode >> sizes;
        } else {
          *sizeNode >> size;
        }
      }
      if (sizes.size() == 0) {
        for(unsigned j = 0; j < names.size(); j++)
          sizes.push_back(size);      
      }
    
      data->createDataItems(nameList, sizes);
    
      std::vector<DeviceDatum*>::iterator iter;
      for (iter = data->mDataItems.begin(); iter != data->mDataItems.end(); iter++) {
        if (mDevices.size() > 1 && !device->mName.empty())
          (*iter)->prefixName(device->mName.c_str());
        addDatum(**iter);
      }

      device->mData.push_back(data);
    }
  }
  unavailable();
}

ModbusAdapter::~ModbusAdapter()
{
  modbus_close(&mb_param);
}

void ModbusAdapter::initialize(int aArgc, const char *aArgv[])
{
  MTConnectService::initialize(aArgc, aArgv);
  if (aArgc > 1) {
    mPort = atoi(aArgv[0]);
  }
}

void ModbusAdapter::start()
{
  startServer();
}

void ModbusAdapter::stop()
{
  stopServer();
}

void ModbusAdapter::gatherDeviceData()
{
  if (!mConnected) {
    modbus_init_rtu(&mb_param, mSerialPort.c_str(), mBaud, mParity.c_str(), mDataBits, mStopBits);
    if (gLogger->getLogLevel() == Logger::eDEBUG)
      modbus_set_debug(&mb_param, TRUE);
    if (modbus_connect(&mb_param) == -1) {
      unavailable();
      printf("ERROR Connection failed\n");
      sleep(5);
      return;
    } else {
      mConnected = true;
    }
  }

  std::vector<ModbusDevice>::iterator device;
  for (device = mDevices.begin(); device != mDevices.end(); device++) {
    std::vector<ModbusData*>::iterator iter;
    for (iter = device->mData.begin(); iter != device->mData.end(); iter++) {
      bool success = false;
      if ((*iter)->type() == ModbusData::eCOIL) {
        success = read_input_status(&mb_param, device->mAddress, (*iter)->address(), (*iter)->count(),
                                  (static_cast<ModbusCoil*>(*iter))->data()) > 0;
      } else { 
        success = read_holding_registers(&mb_param, device->mAddress, (*iter)->address(), (*iter)->count(),
                                         (static_cast<ModbusRegister*>(*iter))->data()) > 0;
        if (success && (*iter)->type() == ModbusData::eDOUBLE) {
          ModbusDouble *d = static_cast<ModbusDouble*>(*iter);
          if (d->scalerAddress() != 0) {
            uint16_t scaler[1];
            success = read_holding_registers(&mb_param, device->mAddress, d->scalerAddress(), 1, scaler) > 0;
            if (success) d->scaler = scaler[0];
          }
        }
      }

      if (success) {
        (*iter)->writeValues();
      } else {
        (*iter)->unavailable();
      }      
    }
    
    flush();
  }
}

