#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <string>
#include <iostream>
using namespace std;
int main()
{
  struct jobStruct {int a; char* b;};
  int pid = 5;
  struct jobStruct to_string(pid)  = {1,"asdf"};
  cout<<to_string(pid).a;
  return 0;
}
