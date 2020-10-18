/*
 * Compile with 'make quash'.
 */

#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

// Used in runOp's switch to pass input string to its corresponding set of instructions.
enum QuashOperation
{
  Init = 0, // Couldn't figure out how else to initialize 'op' in the main loop.
  Error = 1,
  Exit = 2,
  ExecNoParam = 3, // ./idonthaveparemeters or /test/idonthaveparameters
};

/**************************************************
 * END Constants
 **************************************************/

/**************************************************
 * Generic IO Functions
 **************************************************/

// trim pwd to current directory and append to prompt prefix
std::string getPromptPrefix()
{
  std::string pwd = getenv("PWD");
  auto lastSlashIndex = pwd.find_last_of("/");
  if (lastSlashIndex != 0)
  {
    pwd = pwd.substr(lastSlashIndex + 1, sizeof(pwd));
  }

  return ("User@Quash " + pwd + " $ ");
}

std::string prompt()
{
  std::string user_in;

  std::cout << getPromptPrefix();
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

bool isExecNoParam(std::string in)
{
  int len = 0;
  while (in[len] != '\0')
  {
    len += 1;
  }

  // valid program path has at least 2 characters and no spaces (unless it has parameters).
  // ExecNoParam does not apply to commands with parameters.
  if (len < 2 || in.find(' ') != std::string::npos)
  {
    return false;
  }
  // command is relative path './ ...'.
  else if (in[0] == '.' && in[1] == '/')
  {
    return true;
  }
  // command uses absolute path '/ ...'.
  else if (in[0] == '/')
  {
    return true;
  }

  return false;
}

/**************************************************
 * END Quash Process Input Parsing
 **************************************************/

/**************************************************
 * Quash Process Operation Handling
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

// Handle 'ExecNoParam' op in runOp switch
void handleExecNoParam(std::string input)
{
  auto pid = fork();
  
  // if error
  if (pid < 0) {
    echo("Error creating process.\n");
    return;
  }
  // if child
  if (pid == 0) {
    auto path = input;
    auto executableName = input.substr(input.find_last_of('/') + 1, sizeof(input));
    auto error = execlp(path.c_str(), executableName.c_str(), NULL);

    if (error < 0)
    {
      echo("quash: error loading file: '" + path + "'\n");
    }

    exit(0);
  }
  // if parent
  else {
    waitpid(pid, NULL, 0);
  }
}

/**************************************************
 * END Quash Process Operation Handling
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
  else if (isExecNoParam(in))
  {
    return ExecNoParam;
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

  case ExecNoParam:
    handleExecNoParam(input);
    break;
  }
}

int main(int argc, char **argv, char **envp)
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