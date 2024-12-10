
#include "IntentChunkedUploader.h"
#include <WiFi.h>
#include "WiFiClientSecure.h"
#include "I2SRecord.h"

#define BUTTON_PIN 7
// Define INMP441 I²S pins
#define I2S_SCK_PIN 5   // BCLK
#define I2S_WS_PIN 6    // LRCLK
#define I2S_SD_PIN 4    // DOUT
#define I2S_AMPLIFY 20  // Amplification factor
#define BUFFER_SIZE 1024

// Define button pin
#define BUTTON_PIN 7    // GPIO pin for button
#define DEBOUNCE_MS 50  // Debounce delay in milliseconds

// Recording configuration
#define SAMPLE_RATE 16000    // Sampling rate in Hz
#define RECORD_TIME 3        // Recording duration in seconds
#define LED_PIN 8

String access_token = "Bearer DUYP5MQ3OFFSZWFZXBE2VIIB3XHMJRB6";


const int waveDataSize = SAMPLE_RATE * RECORD_TIME * 2;
char* ssid = "test";
char* pass = "abqnm2002";
// AudioCapture audioCapture;
// IntentChunkedUploader uploader;
WiFiClientSecure client;
//AudioCapture* audioCapture;
IntentChunkedUploader* uploader;
volatile bool buttonPressed = false;
int status = WL_IDLE_STATUS;
I2SRecord i2sRecorder;
int32_t communicationData[BUFFER_SIZE];
char partWavData[BUFFER_SIZE];

void printWifiStatus() {
  /* -------------------------------------------------------------------------- */
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}



void toggleLight(String cmd){
  Serial.println(cmd);
  if (cmd == "on") {
    digitalWrite(LED_PIN, HIGH);
  }
  if (cmd == "off") {
    digitalWrite(LED_PIN, LOW);
  }
}

void fetchCalendar(){
  Serial.println("calendar fetched");
}


void processIntent(Intent intent) {
  String intent_name = String(intent.intent_name.c_str());
  Serial.println("Process intent");
  Serial.println(intent_name);
  if(intent_name == "turn_on_off" && intent.trait_confidence > 0.95){
    toggleLight(String(intent.trait_value.c_str()));
  }
  else if (intent_name == "calendar_fetch" && intent.intent_confidence > 0.95){
    fetchCalendar();
  }
  else{
    Serial.println("else");
  }
}

void executeCommandISR() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();

  if (interrupt_time - last_interrupt_time > 200){
  buttonPressed = true; 
  }
  last_interrupt_time = interrupt_time;
  
}

void executeCommand() {
  if (!buttonPressed) return;
  
  buttonPressed = false;
  Serial.println("Button pressed");
  uploader = new IntentChunkedUploader(access_token);

  if (!uploader->connected()) {
    Serial.println("uploader not connected");
    return;
  }

  Serial.println("Listening");
  digitalWrite(LED_BUILTIN, HIGH);
  const int waveDataSize = SAMPLE_RATE * RECORD_TIME * 2;

  for (int j = 0; j < waveDataSize / BUFFER_SIZE; j++) {
    auto sz = i2sRecorder.Read((char*)communicationData, BUFFER_SIZE * 4);
    char* p = (char*)(communicationData);

    for (int i = 0; i < sz / 4; i++) {
      communicationData[i] *= I2S_AMPLIFY; // Amplify sound
      if (i % 2 == 0) { // Process right channel
        partWavData[i] = p[4 * i + 2];
        partWavData[i + 1] = p[4 * i + 3];
      }
    }
    uploader->startChunk(BUFFER_SIZE * sizeof(char));
    uploader->sendChunkData((const uint8_t*)partWavData, BUFFER_SIZE * sizeof(char));
    uploader->finishChunk();
  }
  digitalWrite(LED_BUILTIN, LOW);
  


  // int32_t* buffer = audioCapture->captureAudio(BUFFER_SIZE, RECORD_TIME, I2S_AMPLIFY);
  // Serial.println("Audio recorded");



  // Serial.println("start chunk");
  // uploader->startChunk(BUFFER_SIZE * sizeof(int32_t));
  // Serial.println("send chunk");
  // uploader->sendChunkData((const uint8_t *)buffer, BUFFER_SIZE * sizeof(int32_t));
  // Serial.println("finish chunk");
  // uploader->finishChunk();
  Serial.println("get result");
  // unsigned long start_time = millis();

  // while(millis() - start_time < 3000) {}
  Intent intent = uploader->getResults();
  processIntent(intent);

  delete (uploader);
  // Serial.println("Results");
  // Serial.println(intent.text.c_str());
  // Serial.println(intent.intent_name.c_str());
  // Serial.println(String(intent.intent_confidence));
  // Serial.println(intent.device_name.c_str());
  // Serial.println(String(intent.device_confidence));
  // Serial.println(intent.trait_value.c_str());
  // Serial.println(String(intent.trait_confidence));
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
  }

  WiFi.mode(WIFI_STA);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network.
    status = WiFi.begin(ssid, pass);
    //status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(1000);
  }
  Serial.println("Connected to WiFi!");
  printWifiStatus();

  if (!i2sRecorder.InitInput(I2S_BITS_PER_SAMPLE_32BIT, I2S_SCK_PIN, I2S_WS_PIN, I2S_SD_PIN, SAMPLE_RATE)) {
    Serial.println("Initialization failed");
    return;
  }


  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), executeCommandISR, FALLING);
  //audioCapture = new AudioCapture(I2S_SCK_PIN, I2S_WS_PIN, I2S_SD_PIN, SAMPLE_RATE);
}

void loop() {
  executeCommand();
}


