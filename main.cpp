/*
 * Compile with 'make quash'.
 */

#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**************************************************
 * Globals
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
  ExecWithParam = 4,
};

// I don't use this anywhere yet, but this copies the shell's PATH.
std::string PATH = std::string(getenv("PATH"));

/**************************************************
 * END Globals
 **************************************************/

/**************************************************
 * Generic Helper Functions
 **************************************************/

void splitString(std::string input, char **outArr, std::string delimiters)
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
}

// trim pwd to current directory and append to prompt prefix
std::string getPromptPrefix()
{
  std::string pwd = getenv("PWD");
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

// TODO: change to 'execl' instead of 'execlp' and manually search PATH after it is implemented.
// bash/zsh built-in shell commands will not throw error but have no effect. exec_p will not fail
// for these builtin commands but they occur in child process, which exits).
// (try typing 'jobs' or 'cd', then 'asdf' to see the difference).
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
    auto path = input;
    auto executableName = input.substr(input.find_last_of('/') + 1, input.length());
    auto error = execlp(path.c_str(), executableName.c_str(), NULL);

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

// TODO: change to 'execv' instead of 'execvp' and manually search PATH after it is implemented.
// bash/zsh built-in shell commands will not throw error but have no effect. exec_p will not fail
// for these builtin commands but they occur in child process, which exits).
// (try typing 'cd ..' then 'asdf -a' to see the difference).
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
    splitString(input, args, " ");

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