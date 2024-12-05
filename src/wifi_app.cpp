#include "openchessboard.h"



// LED and state variables
bool boot_flipstate = true;
bool is_booting = true;
bool connect_flipstate = false;
bool is_connecting = false;
bool is_game_running = false;
bool is_seeking = false;
// WiFi, and timer variables
String wifi_ssid;
String wifi_password;
int status = WL_IDLE_STATUS;
WiFiClientSecure StreamClient;
WiFiClientSecure PostClient;
Ticker timer;

//lichess variables
String lichess_api_token;
char server[] = "lichess.org"; 
String username = "no";
String currentGameID = "no";
bool myturn = true;
String lastMove = "no";
String myMove = "no";
String moves = "no";
bool is_castling_allowed = true;


void run_WiFi_app(void){
 
  wifi_setup();

  wifi_firmwareUpdate();

  PostClient.setCACert(rootCA);
  StreamClient.setCACert(rootCA);
  DEBUG_SERIAL.println("\nStarting connection to server...");

  while (1){
    setStateConnecting();

    DEBUG_SERIAL.println("\nConnected to Server...");
    
    if (StreamClient.connect(server, 443))
    {
        if (board_gameMode != "None"  && !is_seeking){

          DEBUG_SERIAL.println("\nStart Game with prefered settings: "+ board_gameMode);   
          postNewGame(PostClient,  board_gameMode);

        }
        DEBUG_SERIAL.println("Find ongoing game");
        
        getGameID(StreamClient); // checks whos turn it is

        if (currentGameID != "no")
        {
        DEBUG_SERIAL.println("Start move stream from game");
        getStream(StreamClient);    
        
        delay(500);// make sure first move is catched by isr
        
        setStatePlaying();
            
        while (is_game_running)
        {   
            if (myturn && is_game_running)
            { 
            String accept_move = "no";  
            
            //print last move if move was detected
            if (lastMove.length() > 3){
                DEBUG_SERIAL.print("opponents move: ");
                DEBUG_SERIAL.println(lastMove);

                // wait for oppents move to be played
                DEBUG_SERIAL.println("wait for move accept...");
        
                while(accept_move != lastMove && is_game_running){
                displayMove(lastMove);
                accept_move = getMoveInput();

                // if king move is a castling move, wait for rook move
                checkCastling(accept_move);
                }
    
                DEBUG_SERIAL.println("move accepted!");
            }

            // blocking, waits for move input
            postMove(PostClient);
            
            }
        }
        }
        DEBUG_SERIAL.print("reset game...");  
        disableClient(StreamClient);
    }
  }
}