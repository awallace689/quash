#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

int main(void) {
    int c;
    FILE *file;
    char *filename;
    filename = (char *)malloc(200 * sizeof(char));
    read(STDIN_FILENO, filename, 200);
    cout<<filename<<endl;
    return(0);
}
