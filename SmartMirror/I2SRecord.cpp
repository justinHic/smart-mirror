#include "I2SRecord.h"

bool I2SRecord::InitInput(i2s_bits_per_sample_t BPS,
                      int bckPin,
                      int wsPin,
                      int dataInPin,
                      int sampleRate)
{
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = sampleRate,
    .bits_per_sample = BPS,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = 0,
    .dma_buf_count = 16,
    .dma_buf_len = 60,
    .use_apll = false
  };

  i2s_pin_config_t pin_config;
  memset(&pin_config,0,sizeof(i2s_pin_config_t));
  pin_config.bck_io_num = bckPin;
  pin_config.ws_io_num = wsPin;
  pin_config.data_out_num = I2S_PIN_NO_CHANGE;
  pin_config.data_in_num = dataInPin;

  if(ESP_OK!=i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL))
  {
    Serial.println("install i2s driver failed");
    return false;
  }
  if(ESP_OK!=i2s_set_pin(I2S_NUM_0, &pin_config))
  {
    Serial.println("i2s set pin failed");
    return false;
  }
  return true;
}

size_t I2SRecord::Read(char* data, int numData)
{
  size_t recvSize;
  i2s_read(I2S_NUM_0, (void*)data, numData, &recvSize, portMAX_DELAY);
  return recvSize;
}

size_t I2SRecord::Write(char* data, int numData)
{
  size_t sendSize;
  i2s_write(I2S_NUM_0, (void*)data, numData, &sendSize, portMAX_DELAY);
  return sendSize;
}

void I2SRecord::End()
{
  i2s_driver_uninstall(I2S_NUM_0);
}