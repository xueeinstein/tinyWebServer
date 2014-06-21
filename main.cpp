#include <iostream>
#include <stdlib.h>
#include "server.h"

using namespace std;

int main(){
	int i;
	char p[20];
	cout << "Please input the port that you want to bind: ";
	cin >> p;
	server* s = new server(p, 5);
	//s->test();
	if ((i = s->serverGetaddrinfo()) != 0)
		return 0;
	else
	{
		cout << "server getaddrinfo OK.." << endl;
		if ((i = s->serverBinding()) != 0)
			return 0;
		else{
			cout << "server get socket file descriptor & bind OK..." << endl;
			if ((i = s->serverListening()) != 0)
				return 0;
			else
				cout << "server listening OK..." << endl;
		}
	}
	cout << "press Ctrl + C to exit server." << endl;
	// if (fork() == 0)
		s->serverWorking();
	// else{
		// while(1){
			// system("curl http://localhost:8088/index.html?sleep=5");
			// cout << "Active Connection Num: " << s->getActiveCon() << endl;
			// sleep(1);
		// }
	// }

	//generate simulated client processes
	return 0;
}