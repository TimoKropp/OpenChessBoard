
/* ---------------------------------------
    interupt handler function. Changes LED states for booting and connection sequence.
    When game is running, this function periodically checks for the game status from
    the stream of the StreamClient.
    @params[in] void
    @return void
*/

void timerCallback(void)
{

  if (is_booting)
  {
    displayBootWait();
    boot_flipstate = !boot_flipstate;
  }

  if (is_connecting)
  { 
    displayConnectWait();
    connect_flipstate = !connect_flipstate;
  }

  if (is_game_running && !is_booting && !is_connecting)
  {
    
    char* char_response = catchResponseFromClient(StreamClient);

    String moves = parseValueFromResponse(char_response, "moves");
    String game_status = parseValueFromResponse(char_response, "status");

    // Detect Game restart
    if (game_status != "started" && game_status != "no")
    {
      DEBUG_SERIAL.print("Game Status: ");
      DEBUG_SERIAL.println(game_status);
      setStateConnecting();
      return;
    }

    // Check Move
    if (moves.length() > 3)
    {
      DEBUG_SERIAL.print("move received: ");
      int startstr = moves.length() - 4; 
      lastMove = moves.substring(startstr);
      DEBUG_SERIAL.println(lastMove);
    }
    
    if (lastMove != myMove)
    {
      myturn = true;
    }
  }
}                   



/* ---------------------------------------
    Function to set up interupt service routine.
    Sets blinking frequency by isr interval.
    @params[in] void
    @return void
*/
void isr_setup(void) {
  timer.attach(0.3, timerCallback);
}

void disableISR() {
  timer.detach(); // This will stop the timer from calling the callback
}

void enableISR() {
  timer.attach(0.3, timerCallback); // This will re-enable the timer callback
}
