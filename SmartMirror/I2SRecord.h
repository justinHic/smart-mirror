#pragma once
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"

// THIS I2S RECORD FILE UPDATED BASED ON SOURCES COMING FROM A TUTORIAL VIDEO: https://www.bilibili.com/video/BV1xA411Q76y/?vd_source=b0d154d074af70a07354b1bd14eb46ab&spm_id_from=333.788.videopod.sections

class I2SRecord
{
public:
  bool InitInput(i2s_bits_per_sample_t BPS,
                 int bckPin,
                 int wsPin,
                 int dataInPin,
                 int sampleRate);

  size_t Read(char* data, int numData);

  size_t Write(char* data, int numData);

  void End();
};