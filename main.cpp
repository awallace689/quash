/*
 * Compile with 'make quash'.
 */

#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <regex>
#include<linux/limits.h> //Used because my compiler was failing to find library for PATH_MAX
#include <sys/wait.h> //Compiler wasnt se
using namespace std;
/**************************************************
 * Globals
 **************************************************/

// ref char* used in splitString, function loses scope from exec callv
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

  cout<<getPromptPrefix();
  std::getline(std::cin, user_in);

  return user_in;
}

void echo(std::string s)
{
  cout<<s;
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

//PIPE HANDLING
bool isPiped(string input)
{
  size_t found = input.find("|");
  if(found != string::npos)
  {
    return true;
  }
  return false;
}
string pipeFirstCommand(string input)
{
  auto firstCommand = input.substr(0, input.find_first_of("|") -1);
  return firstCommand;
}
string pipeSecondCommand(string input)
{
  auto secondCommand = input.substr(input.find_first_of("|")+2,input.length());
  return secondCommand;
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

void handlePiped(string uin)
{
  QuashOperation op;
  pid_t pid_1, pid_2;

  int p1[2];
  pipe(p1);

  pid_1 = fork();
  if(pid_1 == 0){
    //do first operation and write to write end of pipe
    //get first op
    op = getOp(pipeFirstCommand(uin));
    dup2(p1[1], STDOUT_FILENO);
    close(p1[0]);
    runOp(op, pipeFirstCommand(uin));
    exit(0);
  }

  pid_2 = fork();
  if(pid_2 == 0){
    //get second op
    op = getOp(pipeSecondCommand(uin));
    //do second operation with stdin from read end of pipe
    dup2(p1[0], STDIN_FILENO);
    close(p1[1]);
    runOp(op, pipeSecondCommand(uin));
    exit(0);
  }

  close(p1[0]);
  close(p1[1]);
  waitpid(pid_1,NULL,0);
  waitpid(pid_2,NULL,0);
}
string getOutfile(string input)
{
  size_t found = input.find("> ");
  size_t found1 = input.find(">");
  if(found != string::npos)
  {
     auto filename = input.substr(input.find_first_of('>')+2, input.find_last_of('.')+4);
     return filename;
  }
  else if(found1 != string::npos)
  {
    auto filename = input.substr(input.find_first_of('>')+1, input.find_last_of('.')+4);
    return filename;
  }
  else
  {
    return "-1";
  }
}

string commandToOutfile(string input)
{
     auto command = input.substr(0, input.find_first_of(">") -1 );
     return command;
}


int main(int argc, char **argv, char **envp)
{
  QuashOperation op = Init;
  echo(TITLE + "\n\n\n\n\n\n");
  echo(getenv("HOME"));
  //PRESERVE initial file i/o location
  int saved_stdout = dup(STDOUT_FILENO);
  int saved_stdin = dup(STDIN_FILENO);
  // The run loop. Get input, determine what operation was specified, run (handle) the operation.
  while (op != Exit)
  {
    //Assigns the user input to a string
    std::string uin = prompt();

    //If users input contains a | this if statement handles it
    if(isPiped(uin)){
      handlePiped(uin);
      dup2(saved_stdout, STDOUT_FILENO);
      dup2(saved_stdin,STDIN_FILENO);
    }

    //Checks if the user has designated an output file, sets STDOUT to that file, runs op, then resets STDOUT_FILENO
    else if(getOutfile(uin) != "-1")
    {
      {
        fflush(stdout);
        freopen(getOutfile(uin).c_str(),"w",stdout);
        op = getOp(commandToOutfile(uin));
        runOp(op, commandToOutfile(uin));
        fflush(stdout);
        dup2(saved_stdout, STDOUT_FILENO);
      }
    }

    else{
      op = getOp(uin);
      runOp(op, uin);
    }
  }

  return 0;
}
