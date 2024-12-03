#include "openchessboard.h"

/* ---------------------------------------
 *  setup function for wifi module. 
 *  Checks for wifi firmware  update 
 *  and connects wifi module to network.
 *  @params[in] void
 *  @return void
*/
void wifi_setup(void){

  WiFi.begin(wifi_ssid, wifi_password);

  DEBUG_SERIAL.print("Connecting to Wifi");
  while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      DEBUG_SERIAL.print(".");
  }
  DEBUG_SERIAL.println("");
  
  printWiFiStatus();
  }

/* ---------------------------------------
 *  function to print wifi status.
 *  @params[in] void
 *  @return void
 *  
*/
void printWiFiStatus(void) {
  // print the SSID of the network you're attached to:
  DEBUG_SERIAL.print("SSID: ");
  DEBUG_SERIAL.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  DEBUG_SERIAL.print("IP Address: ");
  DEBUG_SERIAL.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  DEBUG_SERIAL.print("signal strength (RSSI):");
  DEBUG_SERIAL.print(rssi);
  DEBUG_SERIAL.println(" dBm");
}

void wifi_firmwareUpdate(void){
  
  const char* firmware_url = "https://github.com/yourusername/OPENCHESSBOARD_WiFi/release/firmware.bin";
  // Check if the Wi-Fi is connected
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;

    // Set the root certificate for HTTPS
    client.setCACert(rootCA);  // Set the certificate for server verification

    // Connect to the server
    if (!client.connect("github.com", 443)) {
      Serial.println("Connection failed");
      return;
    }
    Serial.println("Connected to GitHub");

    // Send the HTTP GET request to download the firmware
    client.print(String("GET ") + firmware_url + " HTTP/1.1\r\n" +
                 "Host: github.com\r\n" +
                 "Connection: close\r\n\r\n");

    // Wait for the response
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {  // Timeout if no response
        Serial.println("Timeout waiting for response");
        client.stop();
        return;
      }
    }

    // Skip the HTTP headers (until we reach the body)
    while (client.available()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        break;
      }
    }

    // Now the firmware file is available in the response body
    if (Update.begin(UPDATE_SIZE_UNKNOWN)) {
      size_t written = Update.writeStream(client);
      if (written == client.available()) {
        Serial.println("Firmware update successful!");
        if (Update.end()) {
          if (Update.isFinished()) {
            Serial.println("Update finished. Rebooting...");
            ESP.restart();  // Reboot to apply the update
          } else {
            Serial.println("Update failed: Update not finished.");
          }
        } else {
          Serial.println("Update failed: " + String(Update.getError()));
        }
      } else {
        Serial.println("Error writing firmware to flash.");
      }
    } else {
      Serial.println("Failed to begin update. Not enough space?");
    }

    client.stop();  // Close the connection
  }
}