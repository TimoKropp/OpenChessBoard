#include "openchessboard.h"

// Create a web server on port 80 (default HTTP port)

// Define Wi-Fi AP (Access Point) credentials
const char* ssidAP = "OPENCHESSBOARD"; // SSID for the ESP32 access point
const char* passwordAP = "123456789";

// Create a web server on port 80 (default HTTP port)
WiFiServer APserver(80);

Preferences preferences;

// Function to extract form data from the POST request
String getFormData(String request, String key) {
  int startIndex = request.indexOf(key + "=");
  if (startIndex == -1) return "";
  
  startIndex += key.length() + 1;  // Skip the key part
  int endIndex = request.indexOf("&", startIndex);
  if (endIndex == -1) endIndex = request.length();
  
  return request.substring(startIndex, endIndex);
}

void AP_setup(WiFiServer &APserver) {
// Start ESP32 in Access Point mode
  WiFi.softAP(ssidAP, passwordAP);
  Serial.println("Access Point Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());  // Ensure this is 192.168.4.1
  
  // Print the WiFi status to debug
  delay(1000); // Allow time for initialization
  Serial.print("WiFi Status: ");
  Serial.println(WiFi.status());  // Check if the ESP32 is properly running as an AP
  
  // Start the server
  APserver.begin();
  Serial.println("Web server started, waiting for clients...");
}

bool handleAPClientRequest(WiFiClient &client) {
  String currentLine = "";
  bool is_submitted = false;
  String ssid = "";
  String password = "";
  String token = "";
  String gameMode = "";
  String startupType = "";
  // Wait for data from the client
  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      Serial.write(c);  // Optional, to debug the received request
      
      // Read the HTTP request line
      if (c == '\n') {
        // Check for the end of the request header
        if (currentLine.length() == 0) {
          // Send HTTP headers
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();

          // Send the HTML form
          String html = "<html>\
                            <body>\
                              <h2>WiFi Configuration</h2>\
                              <form action='/submit' method='POST'>\
                                <label>WiFi SSID:</label><br>\
                                <input type='text' name='ssid' value='" + ssid + "'><br><br>\
                                <label>WiFi Password:</label><br>\
                                <input type='password' name='password' value='" + password + "'><br><br>\
                                <label>Token:</label><br>\
                                <input type='text' name='token' value='" + token + "'><br><br>\
                                <label>Auto Start Game Time Control(WiFi):</label><br>\
                                <select name='gameMode'>\
                                  <option value='5+3'>5+3</option>\
                                  <option value='10+5'>10+5</option>\
                                  <option value='15+10'>15+10</option>\
                                  <option value='30+20'>30+20</option>\
                                </select><br><br>\
                                <label>Default Startup Type:</label><br>\
                                <select name='startupType'>\
                                  <option value='WiFi'>WiFi</option>\
                                  <option value='BLE'>BLE</option>\
                                </select><br><br>\
                                <input type='submit' value='Submit'>\
                              </form>\
                            </body>\
                           </html>";
          client.print(html);
          break;
        } else {
          // Reset current line after reading the header line
          currentLine = "";
        }
      } else if (c != '\r') {
        // Add to current line
        currentLine += c;
      }

      // If form is submitted
      if (currentLine.endsWith("POST /submit")) {
        String requestBody = client.readString();
        // Parse the form data
        ssid = getFormData(requestBody, "ssid");
        password = getFormData(requestBody, "password");
        token = getFormData(requestBody, "token");
        gameMode = getFormData(requestBody, "gameMode");
        startupType = getFormData(requestBody, "startupType");

            // Save to flash
        preferences.begin("settings", false); // read/write
        preferences.putString("ssid", ssid);
        preferences.putString("password", password);
        preferences.putString("token", token);
        preferences.putString("gameMode", gameMode);
        preferences.putString("startupType", startupType);
        preferences.end();

        Serial.println("Settings Saved to Flash:");
        Serial.println("SSID: " + ssid);
        Serial.println("Password: " + password);
        Serial.println("Token: " + token);
        Serial.println("Game Mode: " + urlDecode(gameMode));
        Serial.println("Startup Type: " + startupType);

        // Send a confirmation page
        String confirmation = "<html><body><h2>Settings Saved!</h2>";
        confirmation += "<p>SSID: " + ssid + "</p>";
        confirmation += "<p>Password: " + password + "</p>";
        confirmation += "<p>Token: " + token + "</p>";
        confirmation += "<p>Game Mode: " + urlDecode(gameMode) + "</p>";
        confirmation += "<p>Startup Type: " + startupType + "</p>";
        confirmation += "</body></html>";

        client.print(confirmation);
        is_submitted = true;
        break;
      }
    }
  }

  // Close the connection
  client.stop();
  return is_submitted;
}


void run_APsettings(void){
  AP_setup(APserver);
  bool settings_updated = false;
  while(!settings_updated){
    WiFiClient client = APserver.available();

    if (client) {
      settings_updated = handleAPClientRequest(client);
      DEBUG_SERIAL.println("\nClient handling...");
    }
  }
  delay(2000);
  DEBUG_SERIAL.println("\nRestarting");
  ESP.restart(); 

}