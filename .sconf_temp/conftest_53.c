
#include <stdio.h>    
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>    
volatile int test;
void sig_handler(int sg)
{ 
    test=1;
}
int main()
{
    signal(SIGUSR1,sig_handler);
    test=0;
    kill(getpid(),SIGUSR1);
    sleep(1);
    if(test==0) {
        printf("Signals ARE BROKEN!\n");
        kill(0,SIGQUIT);
    }
    test=0;
    kill(getpid(),SIGUSR1);
    if(test==1)
        exit(0);
    else
        exit(1);
}
