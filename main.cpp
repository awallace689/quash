/*
 * Compile with 'make quash'.
 */


#include <string>
#include <iostream>


/**************************************************
 * Constants
 **************************************************/


std::string TITLE = 
" #####\n"                              
"#     # #    #   ##    ####  #    #\n"
"#     # #    #  #  #  #      #    #\n" 
"#     # #    # #    #  ####  ######\n"
"#   # # #    # ######      # #    #\n"
"#    #  #    # #    # #    # #    #\n"
"####  #  ####  #    #  ####  #    #\n";

// Terminal prompt string
std::string PROMPT_PREFIX = "<put pwd here> $ ";


// Enum used in switch for passing input string to proper handler.
enum QuashOperation
{
  Init = 0, // Couldn't figure out how else to initialize 'op' in the main loop
  Error = 1,
  Exit = 2,
  
  // Maybe define SpawnProcess if input starts with '/' or './', for example. 
  // The handler function for this could extract any parameters and pass them to execl.
};

/**************************************************
 * END Constants
 **************************************************/


/**************************************************
 * Generic IO Functions
 **************************************************/


std::string prompt()
{
  std::string user_in;

  std::cout << PROMPT_PREFIX;
  std::cin >> user_in;

  return user_in;
}


void echo(std::string s)
{
  std::cout << s;
}


/**************************************************
 * END Generic IO Functions
 **************************************************/


/**************************************************
 * Quash Process Input Handling
 **************************************************/

bool isExitCommand(std::string in)
{
  if (in == "exit" || in == "quit")
  {
    return true;
  }
  
  return false;
}


// Examine user input string to determine what operation handler 
// the string should be passed to.
QuashOperation getCommand(std::string in)
{
  if (isExitCommand(in))
  {
    return Exit;
  }

  return Error;
}


void handleOp(QuashOperation op, std::string* argv)
{
  switch(op)
  {
    case Exit:
      break;
  }
}

/**************************************************
 * END Quash Process Input Handling
 **************************************************/


/**************************************************
 * MAIN
 **************************************************/


int main()
{
  QuashOperation op = Init;
  echo(TITLE + "\n\n\n\n\n\n");

  while (op != Exit)
  {
    std::string uin = prompt();
    op = getCommand(uin);
    // handle op
  }

  return 0;
}