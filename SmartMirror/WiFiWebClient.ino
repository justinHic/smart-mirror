// /*
//   TLS WiFi Web client

//   Board CA Root certificate bundle is embedded inside WiFi firmware:
//   https://github.com/arduino/uno-r4-wifi-usb-bridge/blob/main/certificates/cacrt_all.pem

//   Find the full UNO R4 WiFi Network documentation here:
//   https://docs.arduino.cc/tutorials/uno-r4-wifi/wifi-examples#wi-fi-web-client-ssl
// */

// #include <ArduinoJson.h>

// #include "arduino_secrets.h"

// ///////please enter your sensitive data in the Secret tab/arduino_secrets.h
// char ssid[] = SECRET_SSID;        // your network SSID (name)
// char pass[] = SECRET_PASS;        // your network password (use for WPA, or use as key for WEP)

// int status = WL_IDLE_STATUS;
// // if you don't want to use DNS (and reduce your sketch size)
// // use the numeric IP instead of the name for the server:
// //IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
// char server[] = "api.wit.ai";    // name address for Google (using DNS)
// String auth_token = "Bearer DUYP5MQ3OFFSZWFZXBE2VIIB3XHMJRB6";
// String endpoint = "/message?v=20241126&q=turn%20off%20the%20lights";

// // Initialize the Ethernet client library
// // with the IP address and port of the server
// // that you want to connect to (port 80 is default for HTTP):
// WiFiSSLClient client;

// /* -------------------------------------------------------------------------- */
// void setup() {
// /* -------------------------------------------------------------------------- */
//   //Initialize serial and wait for port to open:
//   Serial.begin(115200);
//   while (!Serial) {
//     ; // wait for serial port to connect. Needed for native USB port only
//   }

//   // check for the WiFi module:
//   if (WiFi.status() == WL_NO_MODULE) {
//     Serial.println("Communication with WiFi module failed!");
//     // don't continue
//     while (true);
//   }

//   String fv = WiFi.firmwareVersion();
//   if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
//     Serial.println("Please upgrade the firmware");
//   }

//   // attempt to connect to WiFi network:
//   while (status != WL_CONNECTED) {
//     Serial.print("Attempting to connect to SSID: ");
//     Serial.println(ssid);
//     // Connect to WPA/WPA2 network.
//     status = WiFi.begin(ssid, pass);

//     // wait 10 seconds for connection:
//     delay(10000);
//   }

//   printWifiStatus();

//   Serial.println("\nStarting connection to server...");
//   // if you get a connection, report back via serial:

//   int res = client.connect(server, 443);
//   if (res) {
//     Serial.println("connected to server");
//     // Make a HTTP request:
//     client.println("GET " + endpoint + " HTTP/1.1");
//     client.println("Authorization: " + auth_token);
//     client.println("Host: api.wit.ai");
//     client.println("Connection: close");
//     client.println();
//   }
//   else{
//     Serial.println("failed");
//     Serial.println(res);
//   }
// }

// /* just wrap the received data up to 80 columns in the serial print*/
// /* -------------------------------------------------------------------------- */
// void read_response() {
// /* -------------------------------------------------------------------------- */
//   uint32_t received_data_num = 0;
//   int status = -1;
//   int content_length = 0;
//   while (client.available()) {
//     // /* actual data reception */
//     // char c = client.read();
//     // /* print data to serial port */
//     // Serial.print(c);
//     // /* wrap data to 80 columns*/
//     // received_data_num++;
//     // if(received_data_num % 80 == 0) {
//     //   Serial.println();
//     // }
//     char buffer[255];
//     //size_t read = client.readBytes(buffer, 255);
//     size_t read = client.readBytesUntil('\n', buffer, 255);
//     if (read > 0)
//     {
//         buffer[read] = '\0';
//         // blank line indicates the end of the headers
//         if (buffer[0] == '\r')
//         {
//             break;
//         }
//         if (strncmp("HTTP", buffer, 4) == 0)
//         {
//             sscanf(buffer, "HTTP/1.1 %d", &status);
//         }
//         else if (strncmp("Content-Length:", buffer, 15) == 0)
//         {
//             sscanf(buffer, "Content-Length: %d", &content_length);
//         }
//     }
//   }
//   Serial.println("Http status is " + String(status) + " with content length of " + String(content_length));
//   if (status == 200)
//     {
//         StaticJsonDocument<500> filter;
//         filter["entities"]["device:device"][0]["value"] = true;
//         filter["entities"]["device:device"][0]["confidence"] = true;
//         filter["text"] = true;
//         filter["intents"][0]["name"] = true;
//         filter["intents"][0]["confidence"] = true;
//         filter["traits"]["wit$on_off"][0]["value"] = true;
//         filter["traits"]["wit$on_off"][0]["confidence"] = true;
//         StaticJsonDocument<500> doc;
//         deserializeJson(doc, client, DeserializationOption::Filter(filter));

//         const char *text = doc["text"];
//         const char *intent_name = doc["intents"][0]["name"];
//         float intent_confidence = doc["intents"][0]["confidence"];
//         const char *device_name = doc["entities"]["device:device"][0]["value"];
//         float device_confidence = doc["entities"]["device:device"][0]["confidence"];
//         const char *trait_value = doc["traits"]["wit$on_off"][0]["value"];
//         float trait_confidence = doc["traits"]["wit$on_off"][0]["confidence"];

//         // return Intent{
//         //     .text = (text ? text : ""),
//         //     .intent_name = (intent_name ? intent_name : ""),
//         //     .intent_confidence = intent_confidence,
//         //     .device_name = (device_name ? device_name : ""),
//         //     .device_confidence = device_confidence,
//         //     .trait_value = (trait_value ? trait_value : ""),
//         //     .trait_confidence = trait_confidence};
//         Serial.println(String(intent_name));
//         Serial.print(String(trait_value));

//         // turn light off or on
        
//     }
//     //return Intent{};
// }

// /* -------------------------------------------------------------------------- */
// void loop() {
// /* -------------------------------------------------------------------------- */
//   read_response();

//   // if the server's disconnected, stop the client:
//   if (!client.connected()) {
//     Serial.println();
//     Serial.println("disconnecting from server.");
//     client.stop();

//     // do nothing forevermore:
//     while (true);
//   }
// }

// /* -------------------------------------------------------------------------- */
// void printWifiStatus() {
// /* -------------------------------------------------------------------------- */
//   // print the SSID of the network you're attached to:
//   Serial.print("SSID: ");
//   Serial.println(WiFi.SSID());

//   // print your board's IP address:
//   IPAddress ip = WiFi.localIP();
//   Serial.print("IP Address: ");
//   Serial.println(ip);

//   // print the received signal strength:
//   long rssi = WiFi.RSSI();
//   Serial.print("signal strength (RSSI):");
//   Serial.print(rssi);
//   Serial.println(" dBm");
// }
