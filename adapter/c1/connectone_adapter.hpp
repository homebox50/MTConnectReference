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

#ifndef CONNECTONE_ADAPTER_HPP
#define CONNECTONE_ADAPTER_HPP

#include "adapter.hpp"
#include "service.hpp"
#include "device_datum.hpp"
#include "serial.hpp"
#include "XBee.h"
#include "connectone_device.hpp"
#include <string>
#include <vector>

class ConnectOneAdapter : public Adapter, public MTConnectService
{
public:
  ConnectOneAdapter(int aPort);
  ~ConnectOneAdapter();
  
  virtual void initialize(int aArgc, const char *aArgv[]);
  virtual void start();
  virtual void stop();
  
  virtual void gatherDeviceData();
  
protected:
  // Heartbeats
  void unavailable();
  void unavailable(ConnectOneDevice *aDevice);
  virtual void periodicWork();

  void handleRxResponse(XBeeResponse &aResponse);
  void handleAtCommand(XBeeResponse &aResponse);
  void handleXBeeDiscovery(XBeeResponse &aResponse);
  void handleXBeeTxStatus(XBeeResponse &aResponse);

  void handleMTConnectSamples(ConnectOneDevice *dev, std::string &aStr);
  void handleMTConnectCommand(ConnectOneDevice *dev, std::string &aStr);

  ConnectOneDevice *getOrCreateDevice(XBeeAddress64 &aAddr);

  void initializeXBee();

  /* Events */
  std::vector<ConnectOneDevice*> mDevices; 
  
  int mConnected;
  std::string mSerialPort;
  std::string mParity;
  int mBaud, mDataBits, mStopBits;
  int mTimeout;
  int mSilenceTimeout;
  bool mHonorTimestamp;
  int mHeartBeat;

  int mLastND;

  Serial *mSerial;
  XBee   mXBee;  
};

#endif

