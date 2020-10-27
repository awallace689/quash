/*
 * Compile with 'make quash'.
 */

#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <regex>

/**************************************************
 * Globals
 **************************************************/

// ref char* used in splitString, function loses scope from exec call
char *deleteMe;

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
  ExecWithParam = 4,
  SetPath = 5,
  SetHome = 6,
  Cd = 7,
};

/**************************************************
 * END Globals
 **************************************************/

/**************************************************
 * Generic Helper Functions
 **************************************************/

char *splitString(std::string input, char **outArr, std::string delimiters)
{
  char *cstr_input = new char[input.length() + 1];
  strcpy(cstr_input, input.c_str());

  char *pos;
  int index = 0;
  pos = strtok(cstr_input, delimiters.c_str());
  while (pos != NULL)
  {
    outArr[index] = pos;
    pos = strtok(NULL, delimiters.c_str());
    index++;
  }
  outArr[index] = NULL;

  return cstr_input;
}

// trim pwd to current directory and append to prompt prefix
std::string getPromptPrefix()
{
  char c_str[PATH_MAX];
  std::string pwd = getcwd(c_str, sizeof(c_str));
  auto lastSlashIndex = pwd.find_last_of("/");
  if (lastSlashIndex != 0)
  {
    pwd = pwd.substr(lastSlashIndex + 1, pwd.length());
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
 * END Generic Helper Functions
 **************************************************/

/**************************************************
 * Quash Process Input Parsing
 **************************************************/

bool isChangeDir(std::string in)
{
  if (std::regex_match(in, std::regex("cd .*")) || in == "cd")
  {
    return true;
  }

  return false;
}

bool isSetHome(std::string in)
{
  if (std::regex_match(in, std::regex("set HOME=\".*\"")))
  {
    return true;
  }

  return false;
}

bool isSetPath(std::string in)
{
  if (std::regex_match(in, std::regex("set PATH=\".*\"")))
  {
    return true;
  }

  return false;
}

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

  // valid program path w/ no parameters has no spaces.
  // ExecNoParam does not apply to commands with parameters.
  if (len < 1 || in.find(' ') != std::string::npos)
  {
    return false;
  }

  // command is in form of './...', '/...', or 'command'
  return true;
}

bool isExecWithParam(std::string in)
{
  int len = 0;
  while (in[len] != '\0')
  {
    len += 1;
  }

  if (len < 1 || in.find(' ') == std::string::npos)
  {
    return false;
  }

  // command is in form of './... ...', '/... ...' or 'command ...'
  return true;
}

/**************************************************
 * END Quash Process Input Parsing
 **************************************************/

/**************************************************
 * Quash Process Operation Handling
 **************************************************/

void handleChangeDir(std::string input)
{
  int success;

  if (input == "cd")
  {
    auto home = getenv("HOME");
    success = chdir(home);
    if (success == -1)
    {
      echo("cd: no such file or directory: " + std::string(home) + '\n');
    }
  }
  else
  {
    auto firstQuoteAndRest = input.substr(input.find_first_of(' ') + 1, std::string::npos);
    auto newDir = firstQuoteAndRest.substr(0, firstQuoteAndRest.find_first_of(' '));
    success = chdir(newDir.c_str());
    if (success == -1)
    {
      echo("cd: no such file or directory: " + newDir + '\n');
    }
  }
}

void handleSetHome(std::string input)
{
  auto firstQuoteAndRest = input.substr(input.find_first_of('\"') + 1, std::string::npos);
  auto newHome = firstQuoteAndRest.substr(0, firstQuoteAndRest.find_first_of('\"'));

  if (!(setenv("HOME", newHome.c_str(), 1) == 0))
  {
    echo("quash: set: Error setting home.\n");
  }
}

void handleSetPath(std::string input)
{
  auto firstQuoteAndRest = input.substr(input.find_first_of('\"') + 1, std::string::npos);
  auto newPath = firstQuoteAndRest.substr(0, firstQuoteAndRest.find_first_of('\"'));

  if (!(setenv("PATH", newPath.c_str(), 1) == 0))
  {
    echo("quash: set: Error setting path.\n");
  }
}

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

void handleExecNoParam(std::string input)
{
  auto pid = fork();

  // if error
  if (pid < 0)
  {
    echo("Error creating process.\n");
    return;
  }

  // if child
  if (pid == 0)
  {
    int error = 0;

    auto path = input;
    auto executableName = input.substr(input.find_last_of('/') + 1, input.length());
    error = execlp(path.c_str(), executableName.c_str(), NULL);

    if (error < 0)
    {
      echo("quash: no such file or directory: '" + path + "'\n");
    }

    exit(0);
  }
  // if parent
  else
  {
    waitpid(pid, NULL, 0);
  }
}

void handleExecWithParam(std::string input)
{
  auto pid = fork();

  if (pid < 0)
  {
    echo("Error creating process.\n");
    return;
  }

  if (pid == 0)
  {
    char *args[100];
    deleteMe = splitString(input, args, " ");

    auto path = input.substr(0, input.find_first_of(' '));
    auto executableName = path.substr(input.find_last_of('/') + 1, input.length());
    auto error = execvp(path.c_str(), args);

    if (error < 0)
    {
      echo("quash: no such file or directory: '" + path + "'\n");
    }

    exit(0);
  }
  else
  {
    waitpid(pid, NULL, 0);
  }

  delete[] deleteMe;
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
  if (isSetPath(in))
  {
    return SetPath;
  }
  else if (isSetHome(in))
  {
    return SetHome;
  }
  else if (isChangeDir(in))
  {
    return Cd;
  }
  else if (isExitCommand(in))
  {
    return Exit;
  }
  else if (isExecWithParam(in))
  {
    return ExecWithParam;
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
  case Init:
    echo("Something's broken in the run loop.\n");
    break;

  case Error:
    handleError(input);
    break;

  case Exit:
    echo("Exiting...\n");
    break;

  case ExecWithParam:
    handleExecWithParam(input);
    break;

  case ExecNoParam:
    handleExecNoParam(input);
    break;

  case SetPath:
    handleSetPath(input);
    break;

  case SetHome:
    handleSetHome(input);
    break;

  case Cd:
    handleChangeDir(input);
    break;
  }
}

int main(int argc, char **argv, char **envp)
{
  QuashOperation op = Init;
  echo(TITLE + "\n\n\n\n\n\n");
  echo(getenv("HOME"));

  // The run loop. Get input, determine what operation was specified, run (handle) the operation.
  while (op != Exit)
  {
    std::string uin = prompt();
    op = getOp(uin);
    runOp(op, uin);
  }

  return 0;
}
