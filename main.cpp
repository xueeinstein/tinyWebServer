#include <iostream>
#include "server.h"

using namespace std;

int main(){
	int i;
	char p[20];
	cin >> p;
	server* s = new server(p, 5);
	s->test();
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
	s->serverWorking();
	return 0;
}