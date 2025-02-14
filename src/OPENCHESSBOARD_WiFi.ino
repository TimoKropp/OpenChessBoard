#include "openchessboard.h"


//board configuration
String board_gameMode;
String board_startupType;

//#define PLUG_AT_TOP

void setup() {


  initHW();
  setStateBooting();

#if DEBUG == true
  //Initialize DEBUG_SERIAL and wait for port to open:
  DEBUG_SERIAL.begin(115200);
  delay(1000);
  while (!Serial);
#endif
  
readSettings();
readBoardSelection();

if (board_startupType == "WiFi"){
  isr_setup();
   DEBUG_SERIAL.println("\nRun WiFi App...");   
   run_WiFi_app();
}
else if (board_startupType == "BLE"){
  DEBUG_SERIAL.println("\nRun BLE App...");
  run_BLE_app();
}
else if (board_startupType == "PUZZLE"){
  DEBUG_SERIAL.println("\nRun queen puzzle App...");
  run_queen_puzzle();
}
else{
    DEBUG_SERIAL.println("\nRun Access Point to Fetch Settings...");
    run_APsettings(); // runs until settings are submitted
}

}


void loop() {

}