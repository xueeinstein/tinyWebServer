#include <iostream>
#include <stdlib.h>
#include <getopt.h>
#include "server.h"

using namespace std;

// the name of this program
const char* program_name;
// print usage information for this program
// STREAM typically stdout or stderr
void print_usage(FILE* stream, int exit_code)
{
	fprintf(stream, "Usage: %s options [ params..]\n", program_name);
	fprintf(stream, 
		"	-h --help				Display this usage information.\n"
		"	-p --port port_num			The port to bind.\n"
		"	-o --output filename 			Write output to file.\n"
		"	-v --verbose				Print verbose messages.\n"
		"	-c --connect LB				connect the Load Balancer.\n");
	exit(exit_code);
}
int main(int argc, char* argv[]){
	int i;
	int next_option;
	// a string listing valid short options letters
	const char* const short_options = "hp:o:vc:";
	// an array describing valid long options
	const struct option long_options[] = {
		{"help", 0, NULL, 'h'},
		{"port", 1, NULL, 'p'},
		{"output", 1, NULL, 'o'},
		{"verbose", 0, NULL, 'v'},
		{"connect", 1, NULL, 'c'},
		{NULL, 0, NULL, 0}
	};
	char* binding_port = NULL;
	const char* output_filename = NULL;
	int verbose = 0; // wheter to display verbose messages
	int LB_port = 0;
	int ifconnect = 0;
	program_name = argv[0];

	do{
		next_option = getopt_long(argc, argv, short_options, long_options, NULL);
		switch(next_option){
			case 'h': 
				print_usage(stdout, 0);
				break;
			case 'p':
				binding_port = optarg;
				break;
			case 'o':
				output_filename = optarg;
				break;
			case 'v':
				verbose = 1;
				break;
			case 'c':
				ifconnect = 1;
				LB_port = atoi(optarg);
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
	cout << "LB_port: " << LB_port << " ifconnect: " << ifconnect << endl;
	server* s = new server(binding_port, 5, ifconnect, LB_port, verbose);
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