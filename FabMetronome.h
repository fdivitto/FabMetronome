/*
  Created by Fabrizio Di Vittorio (fdivitto2013@gmail.com) - <https://github.com/fdivitto>
  Copyright (c) 2024 Fabrizio Di Vittorio.
  All rights reserved.


* Please contact fdivitto2013@gmail.com if you need a commercial license.


* This program/library and related software is available under GPL v3.

  FabMetronome is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  FabMetronome is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with FabMetronome.  If not, see <http://www.gnu.org/licenses/>.
*/


#pragma once


// Arduino Library includes
#include <Arduino.h>
#include <NimBLEDevice.h>

// C++ includes
#include <atomic>


// 0 = no debug
// 1 = show errors and minimal info
// 2 = show all incoming packets
// 3 = decoded packets
#define DEBUG 0


// Bad things will happen to you if you change this!
#define DEVICENAME                    "FabMetronome"

// audio configuration
constexpr int AUDIO_GPIO              = 25;
constexpr int TICK_TONE_MS            = 10;
constexpr int TICK_TONE_FREQ_HZ       = 1400;
constexpr int RESET_TONE_FREQ_HZ      = 800;
constexpr int RESET_TONE_MS           = 200;
constexpr int CONNECT_TONE_FREQ_HZ    = 1200;
constexpr int CONNECT_TONE_MS         = 200;
constexpr int DISCONNECT_TONE_FREQ_HZ = 500;
constexpr int DISCONNECT_TONE_MS      = 200;


// we have to take account about task resume time, etc... and subtract it to the waiting time
constexpr int TICK_TASK_JIT_MS = 2;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// to handle connect / disconnect ecents

class BLEServerCallbacksImpl : public BLEServerCallbacks {

  void onConnect(NimBLEServer * pServer) {
    // connected tone
    tone(AUDIO_GPIO, CONNECT_TONE_FREQ_HZ, CONNECT_TONE_MS);
  }

  void onDisconnect(NimBLEServer * pServer) {
    // disconnected tone
    tone(AUDIO_GPIO, DISCONNECT_TONE_FREQ_HZ, DISCONNECT_TONE_MS);
  }

};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// initialization, BLE packet processing, tick task execution

class BLECharacteristicCallbacksImpl : public BLECharacteristicCallbacks {

public:

  void init();

protected:

  // virtual call from BLECharacteristicCallbacks
  void onWrite(BLECharacteristic * pCharacteristic);  

private:

  static void tickTask(void * pvParameters);


  char const *           MIDI_SERVICE_UUID        = "03b80e5a-ede8-4b33-a751-6ce34ec4c700";
  char const *           MIDI_CHARACTERISTIC_UUID = "7772e5db-3868-4112-a1a9-f2669d106bf3";

  BLEServerCallbacksImpl m_bleServerCallbacks;
  
  std::atomic<int>       m_dtime{};
  std::atomic<bool>      m_running{};
  int                    m_pTimeStamp24  = 0;
  int                    m_clockCount    = 0;

}; // instance here!


