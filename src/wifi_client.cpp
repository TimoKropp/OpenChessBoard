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

String fetchLatestVersion(const char* metadata_url) {
  WiFiClientSecure client;
  client.setInsecure();  // Disable SSL certificate validation

  if (!client.connect("raw.githubusercontent.com", 443)) {
    Serial.println("Connection to server failed!");
    return "";
  }

  // Send HTTP GET request for the metadata file
  client.print(String("GET ") + metadata_url + " HTTP/1.1\r\n" +
               "Host: raw.githubusercontent.com\r\n" +
               "User-Agent: ESP32\r\n" +
               "Connection: close\r\n\r\n");

  // Wait for response
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 10000) {  // 10 seconds timeout
      Serial.println("Timeout waiting for response headers");
      client.stop();
      return "";
    }
  }

  // Skip HTTP headers
  while (client.available()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break;
  }

  // Read the JSON response
  String response = client.readString();
  client.stop();
  Serial.println("Raw Response:");
  Serial.println(response);  // This will show the exact response
  return response;
}

bool isNewVersionAvailable(const String& json, String& firmware_url, String& latest_version) {
  // Search for the "latest_version" key and extract the value
  int versionIndex = json.indexOf("\"latest_version\":");
  if (versionIndex == -1) {
    Serial.println("Failed to find latest_version in JSON.");
    return false;
  }
  
  // Move the index to the value after the colon
  versionIndex = json.indexOf("\"", versionIndex + 17);  // 17 is the length of `"latest_version":`
  int endIndex = json.indexOf("\"", versionIndex + 1);
  
  if (versionIndex == -1 || endIndex == -1) {
    Serial.println("Failed to extract version.");
    return false;
  }

  latest_version = json.substring(versionIndex + 1, endIndex);
  
  // Search for the "firmware_file" key and extract the value
  int fileIndex = json.indexOf("\"firmware_file\":");
  if (fileIndex == -1) {
    Serial.println("Failed to find firmware_file in JSON.");
    return false;
  }

  fileIndex = json.indexOf("\"", fileIndex + 15);  // 15 is the length of `"firmware_file":`
  endIndex = json.indexOf("\"", fileIndex + 1);
  
  if (fileIndex == -1 || endIndex == -1) {
    Serial.println("Failed to extract firmware file name.");
    return false;
  }

  firmware_url = json.substring(fileIndex + 1, endIndex);

  // Get stored version from Preferences
  String current_version = preferences.getString("firmware_version", "1.0.0");

  Serial.printf("Current Version: %s, Latest Version: %s\n", current_version.c_str(), latest_version.c_str());

  // Compare versions
  return (latest_version > current_version);  // Basic lexicographical comparison
}

void wifi_firmwareUpdate(void) {
  const char* metadata_url = "/TimoKropp/OPENCHESSBOARD_WiFi/main_nano_esp32/release/version.json";

  String metadata = fetchLatestVersion(metadata_url);
  if (metadata.isEmpty()) {
    Serial.println("Failed to fetch metadata");
    return;
  }

  String firmware_url, latest_version;
  if (!isNewVersionAvailable(metadata, firmware_url, latest_version)) {
    Serial.println("No new firmware available");
    return;
  }

  Serial.printf("New firmware available: %s\n", firmware_url.c_str());

  // Now download the firmware
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();

    if (client.connect("raw.githubusercontent.com", 443)) {
      Serial.println("Connected to server for firmware download");

      // Send HTTP GET request for the firmware file
      client.print(String("GET /TimoKropp/OPENCHESSBOARD_WiFi/main_nano_esp32/release/") + firmware_url + " HTTP/1.1\r\n" +
                   "Host: raw.githubusercontent.com\r\n" +
                   "User-Agent: ESP32\r\n" +
                   "Connection: close\r\n\r\n");

      unsigned long timeout = millis();
      while (client.available() == 0) {
        if (millis() - timeout > 10000) {  // 10 seconds timeout
          Serial.println("Timeout waiting for response body");
          client.stop();
          return;
        }
      }

      // Skip HTTP headers
      while (client.available()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") break;
      }

      Serial.println("Start OTA Update");
      // Start OTA update
      if (Update.begin(UPDATE_SIZE_UNKNOWN)) {
        size_t written = 0;
        while (client.available()) {
          uint8_t buffer[128];
          size_t len = client.read(buffer, sizeof(buffer));
          if (len > 0) {
            written += Update.write(buffer, len);
            Serial.printf("Written %d bytes so far\n", written);
          } else {
            Serial.println("Error reading data");
            break;
          }
        }
        Serial.printf("Total written: %d bytes\n", written);
        if (Update.end() && Update.isFinished()) {
          Serial.println("Update finished. Rebooting...");

          // Save the new version to Preferences
          preferences.putString("firmware_version", latest_version);
          ESP.restart();
        } else {
          Serial.printf("Update failed: %s\n", Update.getError());
        }
      } else {
        Serial.println("Failed to begin update.");
      }
    }
  }
}

