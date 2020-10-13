/*
 * Compile with 'make quash'.
 */


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


int main()
{
  echo("Hello...\n");
  auto name = prompt();
  echo(name + "!\n");

  // do stuff

  return 0;
}