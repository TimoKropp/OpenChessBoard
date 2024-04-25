#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "cert.h"
#include "settings.h"

// Debug Settings
//#define MANUAL_MOVE_INPUT
#define DEBUG true  //set to true for debug output, false for no debug output
#define DEBUG_SERIAL if(DEBUG)Serial

// WiFi variables
extern int status;
extern char server[];  // name address for lichess (using DNS)

extern WiFiClientSecure StreamClient;
extern WiFiClientSecure PostClient;

//lichess variables
extern String username;
extern String currentGameID;
extern bool myturn;
extern String lastMove;
extern String myMove;
extern bool is_castling_allowed;

// LED and state variables
extern bool boot_flipstate;
extern bool is_booting;
extern bool isr_first_run;
extern bool connect_flipstate;
extern bool is_connecting;
extern bool is_game_running;
