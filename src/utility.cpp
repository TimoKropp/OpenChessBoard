#include "OpenChessBoard.h"
/* ---------------------------------------
 *  function to get substring between string firstdel and enddel
 *  @params[in] void
 *  @return substring
*/
String GetStringBetweenStrings(String input, String firstdel, String enddel) {
  int posfrom = input.indexOf(firstdel) + firstdel.length();
  int posto   = input.indexOf(enddel);
  return input.substring(posfrom, posto);
}

/* ---------------------------------------
 *  function to check if move is part of castling move and wait for rook move to complete castling
 *  @params[in] String move_input
 *  @return void
*/
void checkCastling(String move_input) {
  //check if last move was king move from castling
  if(((move_input == "e1g1") || (move_input == "e1c1") ||  (move_input == "e8g8") || (move_input == "e8c8")) && is_castling_allowed){
    
   DEBUG_SERIAL.println("Castling... Wait for rook move...");
   
   bool is_castling = true;
   
   //wait until move was rook move from castling
   while(is_castling && is_game_running){
    
    displayMove(lastMove);
    move_input = getMoveInput();
    DEBUG_SERIAL.println(move_input);
    
    if((move_input == "h1f1") || (move_input == "a1d1") ||  (move_input == "h8f8") || (move_input == "a8d8"))
    {
      is_castling = false;
      is_castling_allowed = false;
    }
  
   }
  }  
}


/* ---------------------------------------
 *  function to set booting state and initializes state variables
 *  @params[in] void
 *  @return void
*/
void setStateBooting(void){
  is_game_running = false;
  is_booting = false;
  is_connecting = true;
  lastMove = "xx";
  myMove = "xx";
  moves = "no";
  currentGameID = "no";
  myturn = false;
}

/* ---------------------------------------
 *  function to set connecting state and initializes state variables
 *  @params[in] void
 *  @return void
*/
void setStateConnecting(void){
  is_game_running = false;
  is_booting = false;
  is_connecting  = true;
  lastMove = "xx";
  myMove = "xx";
  moves = "no";
  currentGameID = "no";
  myturn = false;
}


void setStatePlaying(void){    
  is_game_running = true;
  is_connecting = false;
}

String urlDecode(const String &encoded) {
  String decoded = "";
  char ch;
  int i = 0;
  while (i < encoded.length()) {
    ch = encoded.charAt(i);
    if (ch == '%') {
      // Get the next two characters after %
      String hexCode = encoded.substring(i + 1, i + 3);
      decoded += (char) strtol(hexCode.c_str(), NULL, 16);
      i += 3;
    } else if (ch == '+') {
      // Decode the + as a space
      decoded += ' ';
      i++;
    } else {
      decoded += ch;
      i++;
    }
  }
  return decoded;
}

void readSettings(void){

  preferences.begin("settings", true);  // read only
  
  // Check if parameters exist and read from the preferences
  if (preferences.isKey("ssid")) {
    wifi_ssid = urlDecode(preferences.getString("ssid", ""));
    wifi_password = urlDecode(preferences.getString("password", ""));
    lichess_api_token = urlDecode(preferences.getString("token", ""));
    board_gameMode = urlDecode(preferences.getString("gameMode", ""));
    board_startupType = preferences.getString("startupType", "");
    DEBUG_SERIAL.println("Settings Loaded from Flash:");
    DEBUG_SERIAL.println("SSID: " + wifi_ssid);
    DEBUG_SERIAL.println("Password: " + wifi_password);
    DEBUG_SERIAL.println("Token: " + lichess_api_token);
    DEBUG_SERIAL.println("Game Mode: " + board_gameMode);
    DEBUG_SERIAL.println("Startup Type: " + board_startupType);
  } else {
    Serial.println("No settings found, using default values.");
  }
  preferences.end();
}