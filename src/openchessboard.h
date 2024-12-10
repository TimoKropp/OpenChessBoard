#pragma once
#include <Arduino.h>
#include <FreeRTOS.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "settings.h"
#include "utility.h"
#include "timer.h"
#include "wifi_client.h"
#include "lichess_client.h"
#include "board_driver.h"
#include <Ticker.h>
#include "settings_accesspoint.h"
#include <Preferences.h>
#include "ble_app.h"
#include "wifi_app.h"
#include <Update.h>


// Debug Settings
//#define MANUAL_MOVE_INPUT
#define DEBUG true  //set to true for debug output, false for no debug output
#define DEBUG_SERIAL if(DEBUG)Serial

// WiFi variables
extern int status;
extern char server[];  // name address for lichess (using DNS)

extern WiFiClientSecure StreamClient;
extern WiFiClientSecure PostClient;

extern Ticker timer;

//lichess variables
extern String username;
extern String currentGameID;
extern bool myturn;
extern String lastMove;
extern String myMove;
extern String moves; 
extern bool is_castling_allowed;

// LED and state variables
extern bool boot_flipstate;
extern bool is_booting;
extern bool connect_flipstate;
extern bool is_connecting;
extern bool is_game_running;
extern bool is_seeking;


extern Preferences preferences;

extern String wifi_ssid;
extern String wifi_password;
extern String lichess_api_token;
extern String board_gameMode;
extern String board_startupType;