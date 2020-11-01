#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

void signalHandler(int signal)
{
	printf("Cought signal %d!\n",signal);
	if (signal==SIGCHLD) {
		printf("Child ended\n");
		wait(NULL);
	}
}

int main()
{
	signal(SIGALRM,signalHandler);
	signal(SIGUSR1,signalHandler);
	signal(SIGCHLD,signalHandler);

	if (!fork()) {
		printf("Child running...\n");
		sleep(2);
		printf("Child sending SIGALRM...\n");
		kill(getppid(),SIGALRM); /*send alarm signal to parent*/
		sleep(5);
		printf("Child exitting...\n");
		return 0;
	}
	printf("Parent running, PID=%d. Press ENTER to exit.\n",getpid());
	getchar();
	printf("Parent exitting...\n");
	return 0;
}
