/*
pseudocode logic:

main loop
{
   getInput

   switch (gameState)
   {
   case menuRunning
      if userClickedStart
          loadGameData
          gameState = gameRunning
      if userClickedExit
          clean up and exit program

   case gameRunning
      handle game input and logic
      if userFinishedLevel
          loadLevel2
      if userPressedEscape
          loadPauseMenu
          gameState = paused

   case paused
      if userPressedEscape
          loadGameData
          gameState = running
   }

   draw gui
   draw 3d

}
*/

enum  GameState
    {
		StateTitleScreen,
        StateGamePlaying,
        StateGameOver,
    };
	
 switch (currentState)
      {
        case 1:
          Console.WriteLine("Menu Running");
          break;
        case 2:
          Console.WriteLine("Game Running");
		case 3:
         // load and draw cockpit scene
		case 4:
		//Go back to previous gameState
          break;
      }

GameState ReturnGameState()
{
    GameState s = StateGamePlaying; 
    return s;
}
