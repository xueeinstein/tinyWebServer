#include <iostream>
#include <stdlib.h>
#include <getopt.h>
#include "LoadBalancer.h"

using namespace std;

// the name of this program
const char* program_name;
// print usage information for this program
// STREAM typically stdout or stderr
void print_usage(FILE* stream, int exit_code)
{
	fprintf(stream, "Usage: %s options [ params..]\n", program_name);
	fprintf(stream, 
		"	-h --help				Display this usage information(default is 8088).\n"
		"	-p --port port_num			The port to bind.\n"
		"	-n --num-of-servers num 		the num of servers to boot(default is 2).\n");
	exit(exit_code);
}

int main(int argc, char* argv[]){
	int i;
	int num = 2;
	int next_option;
	char* binding_port = NULL;
	program_name = argv[0];
	const char* const short_options = "hp:n:";
	// an array describing valid long options
	const struct option long_options[] = {
		{"help", 0, NULL, 'h'},
		{"port", 1, NULL, 'p'},
		{"num-of-server", 1, NULL, 'n'},
		{NULL, 0, NULL, 0}
	};
	do{
		next_option = getopt_long(argc, argv, short_options, long_options, NULL);
		switch(next_option){
			case 'h': 
				print_usage(stdout, 0);
				break;
			case 'p':
				binding_port = optarg;
				break;
			case 'n':
				num= atoi(optarg);
				break;
			case '?':
				print_usage(stdout, 1);
				break;
			case -1:
				break;
			default:
				abort();
		}

	}while(next_option != -1);

	if (binding_port == NULL){
		sprintf(binding_port, "%s", "8088");
	}
	LoadBalancer* LB = new LoadBalancer(num, binding_port, 5, 1);
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