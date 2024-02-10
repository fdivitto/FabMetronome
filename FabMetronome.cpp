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


#include "FabMetronome.h"



// initialize BLE and setup sound tick task
void BLECharacteristicCallbacksImpl::init()
{
  BLEDevice::init(DEVICENAME);

  BLEServer * pServer = BLEDevice::createServer();
  pServer->setCallbacks(&m_bleServerCallbacks);

  BLEService * pService = pServer->createService(BLEUUID(MIDI_SERVICE_UUID));

  BLECharacteristic * pCharacteristic = pService->createCharacteristic(BLEUUID(MIDI_CHARACTERISTIC_UUID), NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::WRITE_NR);
  pCharacteristic->setCallbacks(this);
  
  pService->start();

  BLEAdvertising * pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(pService->getUUID());
  pAdvertising->start();

  // reset tone. Waiting 1s to avoid some random noise on pin 25 (I've got it on some circumstances)
  delay(1000);
  tone(AUDIO_GPIO, RESET_TONE_FREQ_HZ, RESET_TONE_MS);

  // should select a different core?
  xTaskCreatePinnedToCore(tickTask, "tickTask", 10000, this, 0, NULL, 1);  
}


// receive and process BLE-MIDI packets, adjust tick time
void BLECharacteristicCallbacksImpl::onWrite(BLECharacteristic * pCharacteristic)
{
  NimBLEAttValue value = pCharacteristic->getValue();

  auto packet     = value.data();
  auto packetSize = value.size();

  if (packetSize < 3)
    return;

  auto ptr = packet;

  #if DEBUG >= 2
  for (int i = 0; i < packetSize; ++i) Serial.printf("%02X ", packet[i]); Serial.printf("\r\n");
  #endif
  
  // get header (timestamp high bits). Fixed (with one exception) in case of multiple message in one packet. Must be increased if low bits overlap (the exception).
  if ((*ptr & 0x80) == 0) {
    #if DEBUG >= 1
    Serial.printf("invalid header\r\n");
    #endif
    return; // invalid header
  }
  uint16_t timeStampHigh = *ptr++ & 0x3f;

  int pTimeStampLow = -1;

  // loop for each message in packet
  while (ptr - packet < packetSize) {

    // bypass unprocessed data bytes (only when at least two bytes has been processed)
    if (ptr > packet + 2) {
      for (;(*ptr & 0x80) == 0; ++ptr) {
        #if DEBUG >= 1
        Serial.printf("bypass data\r\n");
        #endif
      }
    }

    // get timestamp low bits
    if ((*ptr & 0x80) == 0) {
      #if DEBUG >= 1
      Serial.printf("invalid timestamp\r\n");
      #endif
      return; // invalid timestamp
    }
    int timeStampLow = *ptr++ & 0x7f;

    // timestamp low overlapped in the same message?
    if (pTimeStampLow > -1 && timeStampLow < pTimeStampLow) 
      ++timeStampHigh;
    pTimeStampLow = timeStampLow;    

    // compose imestamp for this message
    int timeStamp = (timeStampHigh << 7) | timeStampLow;

    // get MIDI status
    if ((*ptr & 0x80) == 0) {
      #if DEBUG >= 1
      Serial.printf("invalid MIDI status\r\n");
      #endif
      return; // invalid MIDI status
    }
    uint8_t midi_status = *ptr++;

    #if DEBUG >= 3
    Serial.printf("timeStamp=%d midi_status=%02X\r\n", timeStamp, midi_status);
    #endif

    // decode MIDI status
    switch (midi_status) {

      // 0xFA: MIDI Clock, Start
      // 0xFB: MIDI Clock, Continue
      case 0xFA:
      case 0xFB:
        m_clockCount = 0;
        m_dtime      = 0;
        m_running    = true;
        tone(AUDIO_GPIO, TICK_TONE_FREQ_HZ, TICK_TONE_MS); // initial click is out of sync, this is good for me, maybe not good for others!
        break;

      // 0xFC: MIDI Clock, Stop
      case 0xFC:
        m_running    = false;
        m_clockCount = 0;
        break;

      // 0xF8: MIDI Clock, Clock  (24 times per quarter)
      case 0xF8:        
        if (m_clockCount == 0) {
          m_pTimeStamp24 = timeStamp;
          ++m_clockCount;
        } else if (m_clockCount == 24) {
          m_dtime = timeStamp + (m_pTimeStamp24 > timeStamp ? 8192 : 0) - m_pTimeStamp24;
          m_clockCount = 0;
        } else
          ++m_clockCount;
        break;

      default:
        break;
    }

  }

  vTaskDelay(0);
}


// generate sound-ticks every "dtime" milliseconds
void BLECharacteristicCallbacksImpl::tickTask(void * pvParameters)
{
  auto ths = (BLECharacteristicCallbacksImpl *) pvParameters;

  while (1) {
    
    // this is to avoid dtime changes inside the IF (it is not enough make it Atomic!)
    int curr_dtime = ths->m_dtime;

    if (curr_dtime > 0 && ths->m_running) {

      #if DEBUG >= 1
      Serial.printf("%d\r\n", curr_dtime);
      #endif
      
      tone(AUDIO_GPIO, TICK_TONE_FREQ_HZ, TICK_TONE_MS); // this is just a queue add, no time is lost

      curr_dtime -= TICK_TASK_JIT_MS;

      // wait (other possibilities: delayMicroseconds(1000 * curr_dtime) or ets_delay_us(1000 * curr_dtime) or delay(curr_dtime)
      vTaskDelay(curr_dtime / portTICK_PERIOD_MS);

    }
  }
}




