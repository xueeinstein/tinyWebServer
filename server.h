#include <iostream>
#include <sys/stat.h> // POSIX system call, return file attributes
#include <sys/socket.h>
#include <sys/types.h>
#include <cstring>
#include <stdio.h>
#include <error.h>
#include <unistd.h> // contains close(), system call?
#include <netdb.h>
#include <arpa/inet.h> // inet_ntop()
#include <pthread.h>
#include <fcntl.h> // define open(), fcntl() 
#include <time.h>

using namespace std;

const char PATH[] = "./www"; // set www path

struct passedArg
{
    /* data */
    int new_fd;
    int* pActive;
};

void *respHttpQuery(void *param) 
{

    int fd2 = ((passedArg*) param)->new_fd;     //the client sockid
    int* pActive = ((passedArg*) param)->pActive;
    (*pActive)++;
    char buf[1024]; // buffer to save the query
    if(recv(fd2, buf, sizeof(buf), 0)>0) {  
    	time_t timep;
		time (&timep);
		int isNotFound = 0; 
        cout<<"got a HTTP query, content is: "<<endl<<buf<<endl;
        if (strncmp(buf, "GET",3) == 0) {
            char webname[100];
            char *p1=strchr(buf,' ');
            char *p2=strchr(p1+1,' ');
            strncpy(webname,p1+1,p2-p1-1);       // get the filename that client query, may contains direction
            cout << "query:" <<webname << " at:" << ctime(&timep) << endl;
            // check wheather webname contains GET var, like ?a=
            // using the GET var ?sleep= to descide the sleep seconds 
            char get = '?';
            if (strstr(webname, &get) != NULL){
            	cout << "Here contains GET var" << endl;
            	char trueWebName[100];
            	char sleep[6];
            	char *p1=strchr(webname, '?');
            	strncpy(trueWebName, webname, p1-webname);
            	// get GET var

            	char *p2=strstr(webname, "sleep=");
            	char *p3=strchr(p1+1, '?');
            	if (p3 == NULL){
            		p3 = webname + (int)strlen(webname);
            	}
            	if (p2 == NULL)
            		cout << "get p2 error" << endl;
            	int len = strlen(webname) - 1;
            	strncpy(sleep, p2+6, p3-p2-6);
            	int sleep_time = atoi(sleep);
            	// cout << "p2: " << p2 << endl;
            	// cout << "p3: " << p3 << endl;
            	// cout << "predict sleep time is: " << sleep << "&"<< sleep_time << endl;
            	// sleep(sleep_time);
            	// wait(3);
            	// clock_t start, end;
            	// double elapsed;
            	// start = clock();
            	// while(1){
            		// len = 1;
            		// end = clock();
            		// elapsed = (double)(end - start);
            		// cout << "elapsed: " << elapsed << endl;
            		// if (elapsed > sleep_time * 100000)
            			// break;
            	// }
                usleep(sleep_time * 1000000);
            	strcpy(webname, trueWebName);
            }
            struct stat st; // to save the struct stat buffer
            char filename[1024]={'\0'};
            strcpy(filename,PATH);
            strcat(filename,webname);
            int file_status = stat(filename,&st);
            if (file_status == -1){
            	// the query file doesn't exist in www
            	// redirect to 404.html
            	isNotFound = 1;
            	*webname = '\0';
            	*filename = '\0';
            	strcpy(webname, "/404.html");
            	strcpy(filename, PATH);
            	strcat(filename, webname);
            	file_status = stat(filename, &st);
            }
            char protocol[2048]={'\0'};
            char content_type[100]={'\0'};
            if(strncmp(webname,".png",4)==0) // temporally it only support .png picture               
            {
                strncpy(content_type,"Content-Type: image/png\r\n",strlen("Content-Type: image/png\r\n"));
            }else
            {
                strncpy(content_type,"Content-Type: text/html\r\n",strlen("Content-Type: text/html\r\n"));
            }
            if (isNotFound){
            	sprintf(protocol,"HTTP/1.1 404 Not Found\r\n"
                    "Server: tinyWebServer/1.0\r\n"
                    "%s"
                    "Content-Length: %d bytes\r\n"
                    "Connection: keep-alive\r\n\r\n",content_type,(int)st.st_size);
            }
            else{
            	sprintf(protocol,"HTTP/1.1 200 OK\r\n"
                    "Server: tinyWebServer/1.0\r\n"
                    "%s"
                    "Content-Length: %d bytes\r\n"
                    "Connection: keep-alive\r\n\r\n",content_type,(int)st.st_size);
            }
            cout << filename << endl;
            cout<<protocol<<endl;
            send(fd2,protocol,strlen(protocol),0); // send protocol header          
            size_t len=0;
            char fbuf[1024]={'\0'};   
            int fd=open(filename,O_RDONLY,0777); // 
            while((len=read(fd,fbuf,sizeof(fbuf)))>0){
                send(fd2,fbuf,len,0);
            }
            close(fd);
            close(fd2);
        }
    }
    pthread_exit(NULL); // noted this to keep server living
    (*pActive)--;
    return NULL;
}

class server
{
private:
	int sockfd;	// the socket file descriptor

	char* Port; // server binded port

	int Backlog; // the num of connections allowed on the incoming queue

	// pthread_mutex_t mutex;	

    int active_connect; // the active connect , reflect the work load of this server

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

    // get the number of active connection
    int getActiveCon();

	void test();
};

server::server(char* bindingPort, int backlog)
{
	Port = bindingPort;
	Backlog = backlog;
	// pthread_mutex_init(&mutex, NULL);
    active_connect = 0;
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
	// int new_fd;
    struct passedArg passedArg;
	char s[INET6_ADDRSTRLEN];
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	while (1){
		// main accept() loop
		// pthread_mutex_lock(&mutex);
		sin_size = sizeof(their_addr);
		passedArg.new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        passedArg.pActive = &active_connect;
		if (passedArg.new_fd == -1){
			perror("accept");
			continue;
		}
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		cout << "server: got connection from " << s << endl;
		pthread_t id;
        // active_connect++;
		pthread_create(&id, NULL, respHttpQuery, &passedArg);
        // active_connect--;
		// pthread_mutex_unlock(&mutex);
        cout << "Now num. of working is: " << this->getActiveCon() << endl;
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

int server::getActiveCon()
{
    return active_connect;
}
