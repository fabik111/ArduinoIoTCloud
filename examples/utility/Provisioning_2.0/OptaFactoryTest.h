/*
  Copyright (c) 2025 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once
#include <Arduino.h>
#include "opta_info.h"
class OptaFactoryTestClass {
public:
  void begin();
  void optaIDTest();
  bool poll();
private:
  void ledManage();
  void inputManage();
  void printInfo();
  void printModel();
  bool _all_on = false;
  bool _test_running = false;
  OptaBoardInfo *_info;
  uint32_t _ms10 = 0;
  uint32_t _ms100 = 0;
  uint32_t _nextBoardInfoPrint = 0;
};

extern OptaFactoryTestClass OptaFactoryTest;
