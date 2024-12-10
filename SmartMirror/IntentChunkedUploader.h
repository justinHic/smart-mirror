#include <string>

#include "WiFiClientSecure.h"

typedef struct
{
    std::string text;
    std::string intent_name;
    float intent_confidence;
    std::string device_name;
    float device_confidence;
    std::string trait_value;
    float trait_confidence;
} Intent;


class IntentChunkedUploader{
  private:
    WiFiClientSecure *client;
  public:
    IntentChunkedUploader(String accessKey);
    bool connected();
    void startChunk(int size_in_bytes);
    void sendChunkData(const uint8_t *data, int size_in_bytes);
    void finishChunk();
    Intent getResults();
    ~IntentChunkedUploader();
};

