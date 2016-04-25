/*
   Copyright (c) 2015 Intel Corporation.  All rights reserved.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <CurieBle.h>

enum {
  enNone = 0
  , enStart
  , enStop
};

BLEPeripheral blePeripheral;  // BLE Peripheral Device (the board you're programming)
BLEService kanaService("19B10000-E8F2-537E-4F6C-D104768A1214"); // BLE LED Service

// BLE LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEUnsignedCharCharacteristic vibCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

const int distancePin = 3; // 彼との距離
const int vibPin = 13; // 会いたくて震える
const int threshold = 50;

void setup() {
  Serial.begin(9600);

  pinMode(vibPin, OUTPUT);
  pinMode(10, OUTPUT);
  //11番ピンをデジタル入力
  pinMode(11, INPUT);

  // set advertised local name and service UUID:
  blePeripheral.setLocalName("N_Kasa");
  blePeripheral.setAdvertisedServiceUuid(kanaService.uuid());

  // add service and characteristic:
  blePeripheral.addAttribute(kanaService);
  blePeripheral.addAttribute(vibCharacteristic);

  // set the initial value for the characeristic:
  vibCharacteristic.setValue(0);

  // begin advertising BLE service:
  blePeripheral.begin();

  Serial.println("BLE LED Peripheral");
}

void loop() {

  int val;
  int state = enNone;
  int characteristic;
  /*
    for (;;) {
      val = distance();
      if (val == 1) {
        //vibrationOn();
        Serial.println("近づく");
      }
      else if (val == 0) {
        //vibrationOff();
        Serial.println("離れる");
      }
      else {
        //無視
      }
    }
  */
  // listen for BLE peripherals to connect:
  BLECentral central = blePeripheral.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());

    // while the central is still connected to peripheral:
    while (central.connected()) {
      // Set State
      if (vibCharacteristic.written()) {
        characteristic = vibCharacteristic.value();
        if (characteristic == 1) {   // any value other than 0
          state = enStart;
        }
        else if (characteristic == 2) {
          vibrationOff();
          state = enStop;
        }
      }
      // Start State Action
      if (state == enStart) {
        val = distance();
        if (val == 1) {
          vibrationOff();
        }
        else if (val == 0) {
          vibrationOn();
          delay(10);
          vibrationOff();
          delay(20);
          vibrationOn();
          delay(30);
          vibrationOff();
          delay(500);
        }
        else {
          //無視;
        }
      }
    }
    vibrationOff();
    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
}

void vibrationOn() {
  digitalWrite(vibPin, HIGH);
}

void vibrationOff() {
  digitalWrite(vibPin, LOW);
}

int distance() {
  static int far_count = 0;
  int val;

  val = sense();
  Serial.println(val);

  if (val > threshold) {
    far_count = 0;
    return 1; // near
  }
  else {
    far_count++;
    if (far_count > 100) {
      return 0; // far
    }
    else {
      return -1;
    }
  }
}

int sense() {
  //静電容量変化量の変数を用意
  int val = 0;
  //10番ピンをHIGHで出力
  digitalWrite(10, HIGH);
  //指が触れたとき11番ピンがHIGHになるまでをカウント
  while (digitalRead(11) != HIGH) {
    //カウントする
    val++;
  }
  delay(1);
  //10番ピンをLOWにする
  digitalWrite(10, LOW);

  return val;
}


