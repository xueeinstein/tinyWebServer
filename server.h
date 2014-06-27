#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <list>
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

#define DEST_IP "127.0.0.1"
using namespace std;

const char PATH[] = "./www"; // set www path
// using server_info to record the server ability and binded port

struct server_info
{
    FILE* pfile;
    char Port[10];
    int active_con;
    int ability;
};

class server
{
private:
    int Backlog; // the num of connections allowed on the incoming queue

    // socket info. port, IP...
    struct addrinfo hints;  

    // server socket info. port, IP...
    struct addrinfo *serverInfo;

protected:
    
    int active_connect; // the active connect , reflect the work load of this server

    int sockfd; // the socket file descriptor

    int verbose; // wheter to display verbose messages

    // get sockaddr, IPv4 or IPv6:
    void *get_in_addr(struct sockaddr *sa);

public:
    char* Port; // server binded port
    int connect_port; // the port to connect the Load Balancer
    int connect; // wheter to connect the Load Balancer
    // constructor, initial Port, Backlog, serveraddr 
    server(char* bindingPort, int backlog, int ifconnect=0, int cp=0, int ifverbose=0);

    // free the addrinfo
    ~server(){};

    // getaddrinfo, if success, return 0, else return 1
    int serverGetaddrinfo();

    //get socket file despriser & binding port, if success, return 0, else return 1
    int serverBinding();

    // listening port, if success, return 0, else return -1
    int serverListening();

    // coninuous working, to accept the connection from clients, send and receive msg
    void serverWorking();

    void changeServerInfoList(list<server_info> *plist);

    // get the number of active connection
    int getActiveCon();

    void sendAciveCon();

    void test();
};

struct passedArg
{
    int new_fd;
    int* pActive;
    int verbose;
    server* pserver;
};

void *respHttpQuery(void *param) 
{
        
    int fd2 = ((passedArg*) param)->new_fd;     //the client sockid
    int* pActive = ((passedArg*) param)->pActive;
    int verbose = ((passedArg*) param)->verbose;
    server* pserver = ((passedArg*) param)->pserver;

   
    (*pActive)++;
    pserver->sendAciveCon();
    cout << "Now num. of working is: " << pserver->getActiveCon() << endl;
    char buf[1024]; // buffer to save the query
    if(recv(fd2, buf, sizeof(buf), 0)>0) {  
    	time_t timep;
		time (&timep);
		int isNotFound = 0; 
        if (verbose)
            cout<<"got a HTTP query, content is: "<<endl<<buf<<endl;
        if (strncmp(buf, "GET",3) == 0) {
            char webname[100];
            char *p1=strchr(buf,' ');
            char *p2=strchr(p1+1,' ');
            strncpy(webname,p1+1,p2-p1-1);       // get the filename that client query, may contains direction
            if (verbose)
                cout << "query:" <<webname << " at:" << ctime(&timep) << endl;
            // check wheather webname contains GET var, like ?a=
            // using the GET var ?sleep= to descide the sleep seconds 
            char get = '?';
            if (strstr(webname, &get) != NULL){
                if (verbose)
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
            	if (p2 == NULL && verbose)
            		cout << "get p2 error" << endl;
            	int len = strlen(webname) - 1;
            	strncpy(sleep, p2+6, p3-p2-6);
            	int sleep_time = atoi(sleep);
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
            if (verbose){
                cout << filename << endl;
                cout<<protocol<<endl;
            }
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
    (*pActive)--;
    pserver->sendAciveCon();
    cout << "Now num. of working is: " << pserver->getActiveCon() << endl;
    pthread_exit(NULL); // noted this to keep server living
    return NULL;
}

server::server(char* bindingPort, int backlog, int ifconnect, int cp, int ifverbose)
{
	Port = bindingPort;
	Backlog = backlog;
    connect = ifconnect;
    connect_port = cp;
    verbose = ifverbose;
	// pthread_mutex_init(&mutex, NULL);
    active_connect = 0;
    if (connect){
       
    }
}

int server::serverGetaddrinfo()
{
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_STREAM; // set socket type as stream socket
	hints.ai_flags = AI_PASSIVE; // fill in server host ip.
	int r = getaddrinfo(NULL, const_cast<char*>(Port), &hints, &serverInfo);
    if ((r != 0) && verbose){
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
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1 && verbose)
		{
			perror("server: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1 && verbose)
		{
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL && verbose)
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
    struct passedArg passedArg;
	char s[INET6_ADDRSTRLEN];
	struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    int act = this->getActiveCon();
    cout << "Now num. of working is: " << act << endl;

	while (1){
		// main accept() loop
		// pthread_mutex_lock(&mutex);
		sin_size = sizeof(their_addr);
		passedArg.new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        passedArg.pActive = &active_connect;
        passedArg.verbose = verbose;
        passedArg.pserver = this;
		if (passedArg.new_fd == -1 && verbose){
			perror("accept");
			continue;
		}
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        if (verbose)
		    cout << "server: got connection from " << s << endl;
		pthread_t id;
        // active_connect++;
		pthread_create(&id, NULL, respHttpQuery, &passedArg);
        // active_connect--;
		// pthread_mutex_unlock(&mutex);
	}
	close(sockfd);
	return;
}

void server::changeServerInfoList(list<server_info> *plist)
{
    int new_fd;
    char s[INET6_ADDRSTRLEN];
    char buf[1024];
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;

    sin_size = sizeof(their_addr);
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1 && verbose){
        perror("accept");
    }
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
    if (verbose)
        cout << "server: got connection from " << s << endl;

    FILE* ptmp; // using tmp file to make this easier
    ptmp = fopen("tmp.data", "w+");
    while(recv(new_fd, buf, sizeof(buf), 0) > 0){
        cout << "collector: " << buf << endl;
        fprintf(ptmp, "%s", buf);
    }
    fclose(ptmp);
    char port_str[10];
    int active;
    ptmp = fopen("tmp.data", "r");
    while(!feof(ptmp)){
        fscanf(ptmp, "%s %d\n", port_str, &active);
        // find the port_str in the list
        list<server_info>::iterator it;
        for(it=plist->begin(); !strcmp(it->Port, port_str); it++);
        it->active_con = active;
    }
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

void server::sendAciveCon()
{
    if (connect){
    int side_socket;
    sockaddr_in LB_addr;
        side_socket = socket(AF_INET, SOCK_STREAM, 0);
       
        LB_addr.sin_family = AF_INET;
        LB_addr.sin_addr.s_addr = inet_addr(DEST_IP);
        LB_addr.sin_port = htons(connect_port);
        ::connect(side_socket, (struct sockaddr*)&LB_addr, sizeof(struct sockaddr));
        int test;
        if (test == -1)
            cout << "side control error!" << endl;
        char buf[40];
        strcpy(buf, Port);
        char act[20];
        sprintf(act, "%d", this->getActiveCon());
        strncat(buf, " ", 1);
        strncat(buf, act, strlen(act));
        send(side_socket, buf, sizeof(buf), 0);
    }
    return;
}
#endif