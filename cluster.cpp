#include <iostream>
#include <stdlib.h>
#include "LoadBalancer.h"

using namespace std;

int main(){
	int i;
	char p[20];
	cout << "Please input the port that you want to bind: ";
	cin >> p;
	LoadBalancer* LB = new LoadBalancer(2, p, 5, 1);
	int port_num = atoi(p);
	char port_str[10];
	sprintf(port_str, "%d", ++port_num);
	cout << "collector binding port: " << port_str << endl;
	// LB->pcollector = new server(port_str, 5);
	LB->pcollector = new server(port_str, 5);
	LB->pcollector->serverGetaddrinfo();
	LB->pcollector->serverBinding();
	if(LB->pcollector->serverListening() != 0)
		cout << "collector works" << endl;
	LB->bootServers();
	if ((i = LB->serverGetaddrinfo()) != 0)
		return 0;
	else
	{
		cout << "Load Balancer getaddrinfo OK.." << endl;
		if ((i = LB->serverBinding()) != 0)
			return 0;
		else{
			cout << "Load Balancer get socket file descriptor & bind OK..." << endl;
			if ((i = LB->serverListening()) != 0)
				return 0;
			else
				cout << "Load Balancer listening OK..." << endl;
		}
	}
	cout << "press Ctrl + C to exit server." << endl;
	// if (fork() == 0)
	LB->serverWorking();
	return 0;
}