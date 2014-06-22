#ifndef LOADBALANCER_H
#define LOADBALANCER_H

#include <iostream>
#include <list>
#include "server.h"

using namespace std;

#define DEST_IP "localhost"
// using server_info to record the server ability and binded port
struct server_info
{
	server* s;
	char Port[10];
	int active_con;
	int ability;
};

class LoadBalancer : public server
{
private:
	int max_visual_server; // record the max num of visual server

	list<server_info> server_cluster; // record the avariable servers

	// according to the least connection principle, to select server
	// return the selected server port
	int selectServer(); 

	void updateStatus(); // update server cluster status

public:
	LoadBalancer(int max, char* bindingport, int backlog);
	~LoadBalancer();

	void bootServers();
	
	// for load balancer, we need overload the serverWorking function
	void serverWorking();

	// print server cluster list info
	void printStatus();

};

LoadBalancer::LoadBalancer(int max, char* bindingport, int backlog) : 
	server(bindingport, backlog)
{
	max_visual_server = max;
	// bootServers();
}

void LoadBalancer::serverWorking(){
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
        (*passedArg.pActive)++;
		if (passedArg.new_fd == -1){
			perror("accept");
			continue;
		}
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		cout << "server: got connection from " << s << endl;

		char buf[1024];
		recv(passedArg.new_fd, buf, sizeof(buf), 0);
		if(fork() == 0){
			// in sub process
			// create a new socket and conncet to selected server
			// redict the client query to the selected server
      		
			int sockfd_p;
			struct sockaddr_in dest_addr;
			sockfd_p = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (sockfd_p == -1){
				perror("socket()");
				exit(1);
			}

			// setup connection info
			dest_addr.sin_family = AF_INET;              
  			dest_addr.sin_port = htons(selectServer());       
  			dest_addr.sin_addr.s_addr = inet_addr(DEST_IP); 
  			bzero(&(dest_addr.sin_zero), 8); 

  			if (connect(sockfd_p, (struct sockaddr*)&dest_addr, sizeof(struct sockaddr)) == -1){
  				perror("connect error!");
  				exit(1);
  			}
  			send(sockfd_p, buf, sizeof(buf), 0);
  			while(1){
  				if (recv(sockfd_p, buf, sizeof(buf), 0) > 0){
  					send(passedArg.new_fd, buf, sizeof(buf), 0);
  				}
  			}
  			close(sockfd_p);
		}
        (*passedArg.pActive)--;
        printStatus();
	}
	close(passedArg.new_fd);
	return;
}

void LoadBalancer::bootServers(){
	int i, port;
	port = atoi(Port);
	char p[10];
	struct server_info server_info;
	server* ps[max_visual_server];
	for (i=0; i<max_visual_server; i++){
		sprintf(p, "%d", ++port);

		// ps[i] = new server(p, 5);
		// if ((ps[i]->serverGetaddrinfo()) != 0){
			// i--;
			// break;
		// }
		// if ((ps[i]->serverBinding()) != 0){
			// i--;
			// break;
		// }
		// if ((ps[i]->serverListening()) != 0){
			// i--;
			// break;
		// }
		server_info.s = ps[i];
		strcpy(server_info.Port, p);
		server_info.active_con = 0;
		server_info.ability = 1;
		server_cluster.push_back(server_info);
	}
}
int LoadBalancer::selectServer(){
	int i;
	char* p;
	list<server_info>::iterator it;
	updateStatus();
	for (it=server_cluster.begin(), i=it->active_con, p=it->Port; it!=server_cluster.end(); it++){
		if (i < it->active_con){
			i = it->active_con;
			p = it->Port;
		}
	}
	return atoi(p);
}

void LoadBalancer::updateStatus(){
	list<server_info>::iterator it;
	for (it=server_cluster.begin(); it!=server_cluster.end(); it++){
		it->active_con = it->s->getActiveCon();
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
#endif