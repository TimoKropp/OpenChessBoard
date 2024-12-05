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

String fetchMetaData(const char* metadata_url) {
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
    if (millis() - timeout > 5000) {  // 5 seconds timeout
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
  delay(1000);

  return response;
}

bool isNewVersionAvailable(String latest_version) {
  // Get stored version from Preferences
  preferences.begin("versions", false); //read only 
  String current_version = preferences.getString("firmware_version", "0.0.0");
  preferences.end();
  Serial.printf("Current Version: %s, Latest Version: %s\n", current_version.c_str(), latest_version.c_str());

  // Compare versions
  return (latest_version > current_version); 
}




bool fetchLatestVersion(String& latest_version) {   
    const char* url = "/TimoKropp/OPENCHESSBOARD_WiFi/main_nano_esp32/release/version.json";
    const int maxRetries = 3;  // Maximum number of retries for both fetching and writing
    const unsigned long retryDelay = 3000;  // Delay between retries in milliseconds
    int fetchRetries = 0;

    String metadata;

    while (fetchRetries < maxRetries) {
        metadata = fetchMetaData(url);
        if (!metadata.isEmpty()) {
            break;  // Exit loop if metadata is successfully fetched
        } else {
            Serial.println("Failed to fetch metadata, retrying...");
            fetchRetries++;
            delay(retryDelay);
        }
    }

    if (metadata.isEmpty()) {
        Serial.println("Failed to fetch metadata after retries");
        return false;
    }

    // Search for the "latest_version" key and extract the value
    int versionIndex = metadata.indexOf("\"latest_version\":");
    if (versionIndex == -1) {
      Serial.println("Failed to find latest_version in JSON.");
      return false;
    }
    
    // Move the index to the value after the colon
    versionIndex = metadata.indexOf("\"", versionIndex + 17);  // 17 is the length of `"latest_version":`
    int endIndex = metadata.indexOf("\"", versionIndex + 1);
    
    if (versionIndex == -1 || endIndex == -1) {
      Serial.println("Failed to extract version.");
      return false;
    }

    latest_version = metadata.substring(versionIndex + 1, endIndex);

    return latest_version;
}

bool downloadFirmware(String latest_version) {
    const int maxRetries = 3;  // Maximum number of retries for both fetching and writing
    size_t chunkSize = 5120;
    const int retryDelay = 3000;



    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected");
        return false;
    }

    WiFiClientSecure client;
    client.setInsecure();

    int downloadRetries = 0;
    size_t totalWritten = 0;

    size_t currentByte = 0;
    int contentLength = -1;

    while (downloadRetries < maxRetries) {
        if (client.connect("raw.githubusercontent.com", 443)) {
            Serial.println("Connected to server for downloading firmware version: " +  latest_version);

            client.print(String("GET /TimoKropp/OPENCHESSBOARD_WiFi/main_nano_esp32/release/") + "firmware_v" + latest_version +".bin"+ " HTTP/1.1\r\n" +
                         "Host: raw.githubusercontent.com\r\n" +
                         "User-Agent: ESP32\r\n" +
                         "Connection: keep-alive\r\n\r\n");

            unsigned long timeout = millis();
            while (client.available() == 0) {
                if (millis() - timeout > 5000) {
                    Serial.println("Timeout waiting for response body");
                    client.stop();
                    break;
                }
            }

            String headers;
            while (client.available()) {
                String line = client.readStringUntil('\n');
                //Serial.println(line); //debug header
                headers += line + "\n";
                if (line.startsWith("Content-Length: ")) {
                    contentLength = line.substring(15).toInt();
                }
                if (line == "\r") {
                    break;
                }
            }

            if (contentLength <= 0) {
                Serial.println("Invalid or missing Content-Length header. Aborting.");
                return false;
            } else {
                Serial.printf("Total Content Length: %d bytes\n", contentLength);
            }

            if (Update.begin(contentLength)) {
                uint8_t buffer[chunkSize];
                while (client.connected() && totalWritten < contentLength) {
                    String rangeHeader = "Range: bytes=" + String(currentByte) + "-" + String(currentByte + chunkSize - 1);
                    client.print(String("GET /TimoKropp/OPENCHESSBOARD_WiFi/main_nano_esp32/release/") + "firmware_v" + latest_version + " HTTP/1.1\r\n" +
                                 "Host: raw.githubusercontent.com\r\n" +
                                 "User-Agent: ESP32\r\n" +
                                 "Connection: keep-alive\r\n" +
                                 rangeHeader + "\r\n\r\n");

                    unsigned long timeout = millis();
                    while (client.available() == 0) {
                        if (millis() - timeout > 5000) {
                            Serial.println("Timeout waiting for response body");
                            client.stop();
                            break;
                        }
                    }

                    size_t len = client.read(buffer, sizeof(buffer));
                    if (len > 0) {
                        size_t written = Update.write(buffer, len);
                        if (written != len) {
                            Serial.println("Error: Mismatch in written and read data");
                            break;
                        }
                        totalWritten += written;
                        currentByte += written;
                        Serial.printf("Written %d/%d bytes\n", totalWritten, contentLength);
                    } else {
                        Serial.println("No data available from client. Retrying...");
                        break;
                    }
                }

                if (Update.end() && Update.isFinished()) {

                    preferences.begin("versions", false);
                    preferences.putString("firmware_version", latest_version);
                    preferences.end();
                    Serial.println("Update finished. Rebooting...");
                    delay(3000);
                    ESP.restart();
                    return true;
                } else {
                    Serial.printf("Update failed: %s\n", Update.getError());
                }
            } else {
                Serial.println("Failed to begin update.");
            }
        } else {
            Serial.println("Failed to connect to server, retrying...");
        }

        downloadRetries++;
        delay(retryDelay);
    }

    return false;
}

void wifi_firmwareUpdate() {
    String latest_version; 
    if (fetchLatestVersion(latest_version)){
      
      if (!isNewVersionAvailable(latest_version)) {
          Serial.println("Firmware is not the latest.");
          return;
      }
      else{
        Serial.println("Download firmware:" + latest_version);
        if (!downloadFirmware(latest_version)) {
            Serial.println("Firmware update failed");
            return;
        }
      }

    }
    else{
      Serial.println("No new Version available");
      return;
    }


}
