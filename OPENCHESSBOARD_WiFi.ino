#include "OpenChessBoard.h"

// WiFi variables
int status = WL_IDLE_STATUS;
char server[] = "lichess.org";  // name address for lichess (using DNS)

WiFiClientSecure StreamClient;
WiFiClientSecure PostClient;

//lichess variables
String username = "no";
String currentGameID = "no";
bool myturn = true;
String lastMove = "no";
String myMove = "no";
bool is_castling_allowed = true;

// LED and state variables
bool boot_flipstate = true;
bool is_booting = true;
bool isr_first_run = false;
bool connect_flipstate = false;
bool is_connecting = false;
bool is_game_running = false;


void setup() {

  //Initialize HW
  //initHW();
  isr_setup();
 
  
#if DEBUG == true
  //Initialize DEBUG_SERIAL and wait for port to open:
  DEBUG_SERIAL.begin(9600);
  delay(1000);
  while (!Serial);
#endif  

  wifi_setup();

  PostClient.setCACert(rootCA);
  StreamClient.setCACert(rootCA);
  DEBUG_SERIAL.println("\nStarting connection to server...");
}


void loop() {

  is_booting = false;
  is_connecting = true;
  isr_first_run = false;
  lastMove = "xx";
  myMove = "ff";

  DEBUG_SERIAL.println("\nConnected to Server...");
  
  if (StreamClient.connect(server, 443))
  {
    DEBUG_SERIAL.println("Find ongoing game");
    
    getGameID(StreamClient); // checks whos turn it is

    if (currentGameID != "no")
    {
      DEBUG_SERIAL.println("Start move stream from game");
      getStream(StreamClient);    
      
      delay(500);// make sure first move is catched by isr
      
      is_game_running = true;
      is_connecting = false;
        
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
