
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <iostream>
#include <string>
using namespace std;


main ()
{
  string arr[100];
  arr[1] = "Hello ";
  arr[2] = "world";
  cout<<arr[1]<<arr[2]<<arr[3]<<endl;
}
