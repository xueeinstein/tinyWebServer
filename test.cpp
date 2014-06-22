#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>

int main()
{
    int i;
    pid_t process[3];
    int ret[3];
    // for (i=0; i<6; i++)
    	// system("curl http://localhost:8088/index.html");
    // execl("/bin/curl", "curl", "http://localhost:8088/index.html", NULL);
    process[0] = fork();
    process[1] = fork();
    process[2] = fork();
    if (process[0] == 0){
        // execl("/bin/curl", "curl", "http://localhost:8088/index.html", NULL);
        while(1){
            ret[0] = system("curl http://localhost:8088/index.html?sleep=1");
            if (WIFSIGNALED(ret[0]) && 
            (WTERMSIG(ret[0]) == SIGINT || WTERMSIG(ret[0]) == SIGQUIT || WTERMSIG(ret[0]) == SIGKILL))
                break;
            // sleep(1);
        }
    }
    if (process[1] == 0){
        // execl("/bin/curl", "curl", "http://localhost:8088/index.html", NULL);
        while(1){
            ret[1] = system("curl http://localhost:8088/index.html?sleep=3");
            if (WIFSIGNALED(ret[1]) && 
            (WTERMSIG(ret[1]) == SIGINT || WTERMSIG(ret[1]) == SIGQUIT || WTERMSIG(ret[1]) == SIGKILL))
                break;
            // sleep(1);
        }
    }
    if (process[2] == 0){
        // execl("/bin/curl", "curl", "http://localhost:8088/index.html", NULL);
        while(1){
            ret[2] = system("curl http://localhost:8088/index.html?sleep=5");
            if (WIFSIGNALED(ret[2]) && 
            (WTERMSIG(ret[2]) == SIGINT || WTERMSIG(ret[2]) == SIGQUIT || WTERMSIG(ret[2]) == SIGKILL))
                break;
            // sleep(1);
        }
    }

    char c;
    while((c = getchar()) != 'q');

    kill(0, SIGKILL);
    return 0;
}