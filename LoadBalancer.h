#ifndef LOADBALANCER_H
#define LOADBALANCER_H

#include <iostream>
#include <list>
#include <ctype.h>
#include <errno.h>
#include "server.h"

using namespace std;

#define DEST_IP "127.0.0.1"
// using server_info to record the server ability and binded port
struct server_info
{
	FILE* pfile;
	char Port[10];
	int active_con;
	int ability;
};

class LoadBalancer : public server
{
private:
	int max_visual_server; // record the max num of visual server

	list<server_info> server_cluster; // record the avariable servers

	void updateStatus(); // update server cluster status

	void strRev(char* s); // reverse the string

public:
	LoadBalancer(int max, char* bindingport, int backlog, int ifverbose);
	~LoadBalancer();

	void bootServers();
	
	// for load balancer, we need overload the serverWorking function
	void serverWorking();

	// according to the least connection principle, to select server
	// return the selected server port
	int selectServer(); 

	// print server cluster list info
	void printStatus();

};

struct loadBalanceArg
{
	LoadBalancer* pLB;
	int client_socketid;
};
void *loadBalance(void* param){
	LoadBalancer* pLB = ((loadBalanceArg*) param)->pLB;
	int client_socketid = ((loadBalanceArg*) param)->client_socketid;
	char client_buf[1024]; // buffer to save the query
	if(recv(client_socketid, client_buf, sizeof(client_buf), 0)>0) { 
		// select the server with the least connection
		// fill the destination server address
		cout << "LB get query, servers status is: " << endl;
		pLB->printStatus();
		sockaddr_in server_addr;
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = inet_addr(DEST_IP);
		server_addr.sin_port = htons(pLB->selectServer());
		// bzero(&(server_addr.sin_zero), 8);

		int proxy_socketid = socket(AF_INET, SOCK_STREAM, 0);
		if (proxy_socketid == -1)
			cout << "proxy_socketid error!" << endl;
		cout << "LB begin to proxy" << endl;
		// here error!!! unable to 
		int test = connect(proxy_socketid, (struct sockaddr*)&server_addr, sizeof(struct sockaddr));

		if (test == -1)
			cout << "LB connect server error!" << endl;
		cout << errno << endl;
		
		// send the query from client to the selected server
		send(proxy_socketid, client_buf, sizeof(client_buf), 0);

		cout << "LB handled query, servers status is: " << endl;
		pLB->printStatus();
		char server_buf[4096]; 
		while(recv(proxy_socketid, server_buf, sizeof(server_buf), 0)>0){
			send(client_socketid, server_buf, sizeof(server_buf), 0);
		}
		close(proxy_socketid);
	}
}
LoadBalancer::LoadBalancer(int max, char* bindingport, int backlog, int ifverbose) : 
	server(bindingport, backlog, ifverbose)
{
	max_visual_server = max;
	// bootServers();
}

void LoadBalancer::serverWorking(){
	struct loadBalanceArg loadBalanceArg;
	char s[INET6_ADDRSTRLEN];
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	while (1){
		// main accept() loop
		// pthread_mutex_lock(&mutex);
		sin_size = sizeof(their_addr);
		loadBalanceArg.client_socketid = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		loadBalanceArg.pLB = this;
		if (loadBalanceArg.client_socketid== -1){
			perror("accept");
			continue;
		}
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		cout << "server: got connection from " << s << endl;

        printStatus();
		pthread_t id;
        // active_connect++;
		pthread_create(&id, NULL, loadBalance, &loadBalanceArg);
		// char client_buf[4096];
		// int len = recv(passedArg.new_fd, buf, sizeof(buf), 0);
		// if(fork() == 0){
			// in sub process
			// create a new socket and conncet to selected server
			// redict the client query to the selected server
      		
			// int sockfd_p;
			// struct sockaddr_in dest_addr;
			// sockfd_p = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			// if (sockfd_p == -1){
				// perror("socket()");
				// exit(1);
			// }

			// setup connection info
			// dest_addr.sin_family = AF_INET;              
  			// dest_addr.sin_port = htons(selectServer());       
  			// dest_addr.sin_addr.s_addr = inet_addr(DEST_IP); 
  			// bzero(&(dest_addr.sin_zero), 8); 

  			// if (connect(sockfd_p, (struct sockaddr*)&dest_addr, sizeof(struct sockaddr)) == -1){
  				// perror("connect error!");
  				// exit(1);
  			// }
  			// send(sockfd_p, buf, sizeof(buf), 0);
  			// while(1){
  				// if (recv(sockfd_p, buf, sizeof(buf), 0) > 0){
  					// send(passedArg.new_fd, buf, sizeof(buf), 0);
  				// }
  			// }
  			// close(sockfd_p);
		// }
        // (*passedArg.pActive)--;
	}
	close(sockfd);
	return;
}

void LoadBalancer::bootServers(){
	int i, port;
	port = atoi(Port);
	char p[10];
	struct server_info server_info;
	char command[30];
	// sprintf(command, sizeof(command), "./node -p ");
	strcpy(command, "./node -p");
	for (i=0; i<max_visual_server; i++){
		sprintf(p, "%d", ++port);
		strcat(command, p);
		server_info.pfile = popen(command, "r");
		if (server_info.pfile == NULL){
			i--;
			break;
		}
		strcpy(server_info.Port, p);
		server_info.active_con = 0;
		server_info.ability = 1;
		server_cluster.push_back(server_info);
		strcpy(command, "./node -p");
	}
	return;
}
int LoadBalancer::selectServer(){
	int i;
	char* p;
	list<server_info>::iterator it;
	updateStatus();
	cout << "updateStatus OK!" << endl;
	for (it=server_cluster.begin(), i=it->active_con, p=it->Port; it!=server_cluster.end(); it++){
		if (i < it->active_con){
			i = it->active_con;
			p = it->Port;
		}
	}
	cout << "selected server: " << p << endl;
	return atoi(p);
}

void LoadBalancer::updateStatus(){
	list<server_info>::iterator it;
	cout << "begin updateStatus.." << endl;
	for (it=server_cluster.begin(); it!=server_cluster.end(); it++){
		fseek(it->pfile, 0, SEEK_END);
		char c;
		long last = ftell(it->pfile);
		long count;
		char active[8] = {'\0'};
		for(count = 1L; count <= last; count++)
		{
			fseek(it->pfile, -count, SEEK_END);
			c = getc(it->pfile);
			// if (isdigit(c)){
				strncat(active, &c, 1);
			// }
			// if (c == ' ')
				// break;
		}
		// reverse the active
		strRev(active);
		cout << it->Port << " active: " << active << endl;
		it->active_con = atoi(active);
		cout << it->Port << " active_num: " << it->active_con << endl;
	}
	return;
}

void LoadBalancer::printStatus(){
	list<server_info>::iterator it;
	for (it=server_cluster.begin(); it!=server_cluster.end(); it++){
		printf("%s/%d ", it->Port, it->active_con);
	}
	printf("\n");
	return;
}
void LoadBalancer::strRev(char* s){
	char tmp, *end = s + strlen(s) -1;
	while(end > s){
		tmp = *s;
		*s = *end;
		*end = tmp;
		--end;
		++s;
	}
	return;
}
#endif