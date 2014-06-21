#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    int i;
    pid_t process[6];
    // for (i=0; i<6; i++)
    	system("curl http://localhost:8088/index.html");
    // execl("/bin/curl", "curl", "http://localhost:8088/index.html", NULL);
    process[0] = fork();
    process[1] = fork();
    process[2] = fork();
    if (process[0] == 0){
        // execl("/bin/curl", "curl", "http://localhost:8088/index.html", NULL);
        while(1){
            system("curl http://localhost:8088/index.html?sleep=11");
            // sleep(1);
        }
    }
    if (process[1] == 0){
        // execl("/bin/curl", "curl", "http://localhost:8088/index.html", NULL);
        while(1){
            system("curl http://localhost:8088/index.html?sleep=13");
            // sleep(1);
        }
    }
    if (process[2] == 0){
        // execl("/bin/curl", "curl", "http://localhost:8088/index.html", NULL);
        while(1){
            system("curl http://localhost:8088/index.html?sleep=15");
            // sleep(1);
        }
    }
    return 0;
}