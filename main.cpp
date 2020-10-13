#include <string>
#include <iostream>


std::string PROMPT_PREFIX = "> ";


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


// compile with 'make quash'
int main()
{
  echo("Hello, " + prompt() + "!\n");

  // do stuff

  return 0;
}