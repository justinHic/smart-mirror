#include "IntentChunkedUploader.h"
#include "WiFiClientSecure.h"
#include <ArduinoJson.h>

IntentChunkedUploader::IntentChunkedUploader(String accessKey) {
  client = new WiFiClientSecure();
  client->setInsecure();
  char server[] = "api.wit.ai";
  int res = client->connect(server, 443);
  if (res) {
    Serial.println("connected to server");
    // Make a HTTP request:
    client->println("POST /speech?v=20241209 HTTP/1.1");
    client->println("Authorization: " + accessKey);
    client->println("Host: api.wit.ai");
    client->println("content-type: audio/raw; encoding=signed-integer; bits=16; rate=16000; endian=little");
    client->println("transfer-encoding: chunked");
    //client->println("Connection: close");
    client->println();
  } else {
    Serial.println("Connection failed");
    Serial.println(res);
  }
}

bool IntentChunkedUploader::connected() {
  return client->connected();
}

void IntentChunkedUploader::startChunk(int size_in_bytes) {
  client->print(String(size_in_bytes, HEX));
  client->print("\r\n");
}

void IntentChunkedUploader::sendChunkData(const uint8_t *data, int size_in_bytes) {
  client->write(data, size_in_bytes);
}

void IntentChunkedUploader::finishChunk() {
  client->print("\r\n");
}

Intent IntentChunkedUploader::getResults() {
  // finish the chunked request by sending a zero length chunk
  client->print("0\r\n");
  client->print("\r\n");

  // get the headers and the content length
  int status = -1;
  int content_length = 0;
  String httpResponse = "";
  String responseBody = "";

  while (client->connected()) {
    char buffer[255];
    int read = client->readBytesUntil('\n', buffer, 255);
    if (read > 0) {
      buffer[read] = '\0';
      httpResponse += String(buffer);
      // blank line indicates the end of the headers
      if (buffer[0] == '\r') {
        break;
      }
      if (strncmp("HTTP", buffer, 4) == 0) {
        sscanf(buffer, "HTTP/1.1 %d", &status);
      } else if (strncmp("Content-Length:", buffer, 15) == 0) {
        sscanf(buffer, "Content-Length: %d", &content_length);
      }
    }
  }

  // if (content_length > 0) {
  //     char responseBuffer[content_length + 1];  // Buffer to hold the response body
  //     int bytesRead = client->readBytes(responseBuffer, content_length);
  //     responseBuffer[bytesRead] = '\0';  // Null-terminate the response

  //     responseBody = String(responseBuffer);
  //     Serial.println(responseBody);  // Store response body in String
  // }
  Serial.println("Http status is " + String(status) + " with content length of " + String(content_length));
  Serial.println(httpResponse);

  // String response = client->readString();
  // Serial.println(response);
  if (status == 200) {
    // JSON filter for extracting relevant fields
    StaticJsonDocument<500> filter;
    filter["entities"]["device:device"][0]["value"] = true;
    filter["entities"]["device:device"][0]["confidence"] = true;
    filter["text"] = true;
    filter["intents"][0]["name"] = true;
    filter["intents"][0]["confidence"] = true;
    filter["traits"]["wit$on_off"][0]["value"] = true;
    filter["traits"]["wit$on_off"][0]["confidence"] = true;
    filter["type"] = true;

    StaticJsonDocument<500> doc;

    Intent intent = Intent{};

    String inputText = client->readString();
    Serial.println(inputText);

    String jsonObject = "";
    int openingBrackets = 0;
    for (int i = 0; i < inputText.length(); i++) {
      char c = inputText[i];
      if (c == '{') {
        openingBrackets += 1;
        jsonObject += c;
      } else if (c == '}') {
        openingBrackets -= 1;
        jsonObject += c;
        if (openingBrackets == 0) {
          StaticJsonDocument<500> doc;
          DeserializationError error = deserializeJson(doc, jsonObject, DeserializationOption::Filter(filter));
          if (error) {
            Serial.print("JSON parsing error: ");
            Serial.println(error.c_str());
          }

          const char *type = doc["type"];
          //Serial.println(type);
          if (type && (strcmp(type, "PARTIAL_TRANSLATION") == 0 || strcmp(type, "FULL_TRANSLATION") == 0)){
            jsonObject = "";
            continue;
          }

          // Extract relevant fields
          const char *text = doc["text"];
          const char *intent_name = doc["intents"][0]["name"];
          float intent_confidence = doc["intents"][0]["confidence"];
          const char *device_name = doc["entities"]["device:device"][0]["value"];
          float device_confidence = doc["entities"]["device:device"][0]["confidence"];
          const char *trait_value = doc["traits"]["wit$on_off"][0]["value"];
          float trait_confidence = doc["traits"]["wit$on_off"][0]["confidence"];

          // Print the extracted information
          // Serial.print("Text: ");
          // Serial.println(text ? text : "N/A");
          // Serial.print("Intent Name: ");
          // Serial.println(intent_name ? intent_name : "N/A");
          // Serial.print("Intent Confidence: ");
          // Serial.println(intent_confidence);
          // Serial.print("Device Name: ");
          // Serial.println(device_name ? device_name : "N/A");
          // Serial.print("Device Confidence: ");
          // Serial.println(device_confidence);
          // Serial.print("Trait Value: ");
          // Serial.println(trait_value ? trait_value : "N/A");
          // Serial.print("Trait Confidence: ");
          // Serial.println(trait_confidence);
          intent = Intent{
            .text = (text ? text : ""),
            .intent_name = (intent_name ? intent_name : ""),
            .intent_confidence = intent_confidence,
            .device_name = (device_name ? device_name : ""),
            .device_confidence = device_confidence,
            .trait_value = (trait_value ? trait_value : ""),
            .trait_confidence = trait_confidence
          };  // Exit loop once the d
          jsonObject = "";
        }
      } else if (openingBrackets > 0) {
        jsonObject += c;
      }
    }
    return intent;
  }


  // int startIndex = 0;
  // while (startIndex != -1) {
  //     // Locate the start of a JSON object
  //     int jsonStart = inputText.indexOf('{', startIndex);
  //     Serial.println("start");
  //     Serial.println(jsonStart);
  //     if (jsonStart == -1) break;

  //     // Locate the end of the JSON object
  //     int jsonEnd = inputText.indexOf('}', jsonStart);
  //     Serial.println("end");
  //     Serial.println(jsonEnd);
  //     while (jsonEnd != -1) {
  //         // Check for matching brackets
  //         int nestedStart = inputText.substring(jsonStart + 1, jsonEnd).indexOf('{');
  //         if (nestedStart == -1) break; // No nested JSON, valid end found
  //         jsonEnd = inputText.indexOf('}', jsonEnd + 1);
  //     }
  //     Serial.println("After loop");
  //     Serial.println(jsonEnd);

  //     if (jsonEnd == -1) break; // No valid closing bracket found

  //     // Extract the JSON object
  //     String jsonObject = inputText.substring(jsonStart, jsonEnd + 1);
  //     Serial.println("Extracted JSON:");
  //     Serial.println(jsonObject);

  //     // Process the JSON object here
  //     StaticJsonDocument<500> doc;
  //     DeserializationError error = deserializeJson(doc, jsonObject);
  //     if (!error) {
  //         // Example: Print the "type" field
  //         const char *type = doc["type"];
  //         Serial.print("Type: ");
  //         Serial.println(type);
  //     } else {
  //         Serial.print("JSON parsing error: ");
  //         Serial.println(error.c_str());
  //     }

  //     // Move to the next segment
  //     startIndex = jsonEnd + 1;
  // }


  // while (client->connected()) {
  //   client->readStringUntil('{');
  //   String chunk = client->readStringUntil('}');
  //   Serial.println(chunk);
  //   DeserializationError error = deserializeJson(doc, chunk.c_str(), DeserializationOption::Filter(filter));
  //   if (error) {
  //     Serial.print("JSON parsing error: ");
  //     Serial.println(error.c_str());
  //     continue;
  //   }
  //   const char *type = doc["type"];
  //   if (type && strcmp(type, "FINAL_UNDERSTANDING") == 0) {
  //     finalUnderstandingFound = true;

  //     // Extract relevant fields
  //     const char *text = doc["text"];
  //     const char *intent_name = doc["intents"][0]["name"];
  //     float intent_confidence = doc["intents"][0]["confidence"];
  //     const char *device_name = doc["entities"]["device:device"][0]["value"];
  //     float device_confidence = doc["entities"]["device:device"][0]["confidence"];
  //     const char *trait_value = doc["traits"]["wit$on_off"][0]["value"];
  //     float trait_confidence = doc["traits"]["wit$on_off"][0]["confidence"];

  //     // Print the extracted information
  //     Serial.println("FINAL_UNDERSTANDING Found:");
  //     Serial.print("Text: ");
  //     Serial.println(text ? text : "N/A");
  //     Serial.print("Intent Name: ");
  //     Serial.println(intent_name ? intent_name : "N/A");
  //     Serial.print("Intent Confidence: ");
  //     Serial.println(intent_confidence);
  //     Serial.print("Device Name: ");
  //     Serial.println(device_name ? device_name : "N/A");
  //     Serial.print("Device Confidence: ");
  //     Serial.println(device_confidence);
  //     Serial.print("Trait Value: ");
  //     Serial.println(trait_value ? trait_value : "N/A");
  //     Serial.print("Trait Confidence: ");
  //     Serial.println(trait_confidence);
  //     return Intent{
  //       .text = (text ? text : ""),
  //       .intent_name = (intent_name ? intent_name : ""),
  //       .intent_confidence = intent_confidence,
  //       .device_name = (device_name ? device_name : ""),
  //       .device_confidence = device_confidence,
  //       .trait_value = (trait_value ? trait_value : ""),
  //       .trait_confidence = trait_confidence
  //     };  // Exit loop once the de
  //   }
  // }
  Serial.println("no json");
  return Intent{};
}



IntentChunkedUploader::~IntentChunkedUploader() {
  delete client;
}