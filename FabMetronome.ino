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



BLECharacteristicCallbacksImpl bleCharacteristicCallbacksImpl;


void setup() 
{
  #if DEBUG >= 0
  Serial.begin(250000);
  #endif

  bleCharacteristicCallbacksImpl.init();
}


void loop()
{ 
  // we don't need a looping task anymore
  vTaskDelete(nullptr);
}

