/*
 * Compile with 'make quash'.
 */


#include <string>
#include <iostream>


/**************************************************
 * Constants
 **************************************************/


std::string TITLE = 
"\n #####\n"                              
"#     # #    #   ##    ####  #    #\n"
"#     # #    #  #  #  #      #    #\n" 
"#     # #    # #    #  ####  ######\n"
"#   # # #    # ######      # #    #\n"
"#    #  #    # #    # #    # #    #\n"
"####  #  ####  #    #  ####  #    #\n";

// Terminal prompt string. Can convert to function once PWD is defined.
std::string PROMPT_PREFIX = "<put pwd here> $ ";


// Used in handleOp's switch to pass input string to its corresponding set of instructions.
enum QuashOperation
{
  Init = 0, // Couldn't figure out how else to initialize 'op' in the main loop.
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


// Examine user input string to determine what QuashOperation 
// the string corresponds with.
QuashOperation getCommand(std::string in)
{
  if (isExitCommand(in))
  {
    return Exit;
  }

  return Error;
}


// Execute some sequence of code depending on what the operation is.
void handleOp(QuashOperation op, std::string input)
{
  switch(op)
  {
    case Error:
      echo("Invalid input.\n\n");
      break;

    case Exit:
      echo("Exiting...\n\n");
      break;

    //case SpawnProcess could handle spawning a new process with parameters
    // extracted from string 'input'.
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

  // The run loop. Get input, determine what operation was specified, run (handle) the operation.
  while (op != Exit)
  {
    std::string uin = prompt();
    op = getCommand(uin);
    handleOp(op, uin);
  }

  return 0;
}