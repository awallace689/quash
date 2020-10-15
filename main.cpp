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

// Used in runOp's switch to pass input string to its corresponding set of instructions.
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
  std::getline(std::cin, user_in);

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
 * Quash Process Input Parsing
 **************************************************/

bool isExitCommand(std::string in)
{
  if (in == "exit" || in == "quit")
  {
    return true;
  }

  return false;
}

/**************************************************
 * END Quash Process Input Parsing
 **************************************************/

/**************************************************
 * Quash Process Input Handling
 **************************************************/

// Handle 'Error' op in runOp switch
void handleError(std::string input)
{
  std::string command;

  auto spacePos = input.find(' ');
  if (spacePos != std::string::npos)
  {
    command = input.substr(0, spacePos);
  }
  else
  {
    command = input;
  }

  echo("quash: command not found: " + command + " \n");
}

/**************************************************
 * END Quash Process Input Handling
 **************************************************/

/**************************************************
 * 'main' and Main Loop Steps
 **************************************************/

// Examine user input string to determine what QuashOperation
// the string corresponds with.
QuashOperation getOp(std::string in)
{
  if (isExitCommand(in))
  {
    return Exit;
  }

  return Error;
}

// Execute some sequence of code depending on what the operation is.
void runOp(QuashOperation op, std::string input)
{
  switch (op)
  {
  case Error:
    handleError(input);
    break;

  case Exit:
    echo("Exiting...\n");
    break;

    //case SpawnProcess could handle spawning a new process with parameters
    // extracted from string 'input'.
  }
}

int main()
{
  QuashOperation op = Init;
  echo(TITLE + "\n\n\n\n\n\n");

  // The run loop. Get input, determine what operation was specified, run (handle) the operation.
  while (op != Exit)
  {
    std::string uin = prompt();
    op = getOp(uin);
    runOp(op, uin);
  }

  return 0;
}