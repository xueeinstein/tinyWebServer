#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstring>
#include <stdio.h>
#include <unistd.h> // contains close(), system call?
#include <netdb.h>
#include <arpa/inet.h> // inet_ntop()
#include <pthread.h>

using namespace std;

class server
{
private:
	int sockfd;	// the socket file descriptor

	char* Port; // server binded port

	int Backlog; // the num of connections allowed on the incoming queue

	// socket info. port, IP...
	struct addrinfo hints;  

	// server socket info. port, IP...
	struct addrinfo *serverInfo;

	// get sockaddr, IPv4 or IPv6:
	void *get_in_addr(struct sockaddr *sa);

	// handle query
	// void *respQuery(void * param){
		// return;
	// }

public:
	// constructor, initial Port, Backlog, serveraddr 
	server(char* bindingPort, int backlog);

	// free the addrinfo
	~server();

	// getaddrinfo, if success, return 0, else return 1
	int serverGetaddrinfo();

	//get socket file despriser & binding port, if success, return 0, else return 1
	int serverBinding();

	// listening port, if success, return 0, else return -1
	int serverListening();

	// coninuous working, to accept the connection from clients, send and receive msg
	void serverWorking();

	void test();
};

server::server(char* bindingPort, int backlog)
{
	Port = bindingPort;
	Backlog = backlog;
}

int server::serverGetaddrinfo()
{
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_STREAM; // set socket type as stream socket
	hints.ai_flags = AI_PASSIVE; // fill in server host ip.
	int r = getaddrinfo(NULL, const_cast<char*>(Port), &hints, &serverInfo);
	if (r != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(r));
	}
	return r;
}

int server::serverBinding()
{
	struct addrinfo *p;

	// find the first available ip, to bind.
	for (p = serverInfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			perror("server: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)
	{
		fprintf(stderr, "server: failed to bind socket.\n");
		return 1;
	}
	freeaddrinfo(serverInfo);
	return 0;
}

int server::serverListening()
{
	return listen(sockfd, Backlog);
}

void server::serverWorking()
{
	int new_fd;
	char s[INET6_ADDRSTRLEN];
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	while (1){
		// main accept() loop
		sin_size = sizeof(their_addr);
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1){
			perror("accept");
			continue;
		}
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		cout << "server: got connection from " << s << endl;
		pthread_t id;
		//pthread_create(&id, NULL, respQuery, &new_fd);
	}
	close(sockfd);
	return;
}

void server::test()
{
	cout << Port << endl;
	cout << Backlog << endl;
}

// get sockaddr, IPv4 or IPv6:
void* server::get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
main(){
	char p[20];
	cin >> p;
	server* s = new server(p, 5);
	s->test();
	printf("hello\n");
	s->serverGetaddrinfo();
	s->serverBinding();
	s->serverListening();
	s->serverWorking();
}