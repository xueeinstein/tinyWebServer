#ifndef LOADBALANCER_H
#define LOADBALANCER_H

#include <iostream>
#include <list>
#include <ctype.h>
#include <errno.h>
#include "server.h"

#define LOG(lf, msg) fprintf(lf, "%f\n", (msg))

using namespace std;

class LoadBalancer : public server
{
private:
	int max_visual_server; // record the max num of visual server

public:
	LoadBalancer(int max, char* bindingport, int backlog, int ifverbose);

	~LoadBalancer();

	list<server_info> server_cluster; // record the avariable servers

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
		if (strncmp(client_buf, "GET",3) == 0){
			FILE *log = fopen("log.log", "a");
			clock_t t1, t2;
			t1 = clock();
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
			// cout << errno << endl;
		
			// send the query from client to the selected server
			send(proxy_socketid, client_buf, sizeof(client_buf), 0);

			cout << "LB handled query, servers status is: " << endl;
			char server_buf[4096]; 
			while(recv(proxy_socketid, server_buf, sizeof(server_buf), 0)>0){
				send(client_socketid, server_buf, sizeof(server_buf), 0);
			}
			// calculate response time and log
			t2 = clock();
			float diff = (((float)t2 - (float)t1) / 1000000.0F ) * 1000;   
			cout << diff << endl;
			LOG(log, diff);
			fclose(log);
			close(client_socketid);
			close(proxy_socketid);
		}
		else{
			// cout << "No get, goes to here " << client_buf << endl;
			char server_nm[10];
			char act_str[5];
			char *p1=strchr(client_buf,' ');
			// cout << "p1: " << p1 << endl;
			strncpy(server_nm, client_buf, p1-client_buf);
			// cout << "No get, goes to here, server_nm" << server_nm <<endl;
			char *p2=client_buf + strlen(client_buf);
			strncpy(act_str, p1+1, p2-p1-1);
			// strcpy(act_str, p1);
			cout << "server_nm: " << server_nm << " act_str: " << act_str <<endl;

			list<server_info>::iterator it;
        	for(it=pLB->server_cluster.begin(); strcmp(it->Port, server_nm) != 0; it++);
        	it->active_con = atoi(act_str);
			pLB->printStatus();
			close(client_socketid);
		}	
	}
	pthread_exit(NULL);
	return NULL;
}
LoadBalancer::LoadBalancer(int max, char* bindingport, int backlog, int ifverbose) : 
	server(bindingport, backlog, 0, 0, ifverbose)
{
	max_visual_server = max;
	
}

void LoadBalancer::serverWorking(){
	struct loadBalanceArg loadBalanceArg;
	char s[INET6_ADDRSTRLEN];
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	while (1){
		sin_size = sizeof(their_addr);
		loadBalanceArg.client_socketid = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		loadBalanceArg.pLB = this;
		if (loadBalanceArg.client_socketid== -1){
			perror("accept");
			continue;
		}
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		// cout << "server: got connection from " << s << endl;
		pthread_t id;
		pthread_create(&id, NULL, loadBalance, &loadBalanceArg);
	}
	close(sockfd);
	return;
}

void LoadBalancer::bootServers(){
	int i, port;
	port = atoi(Port);
	char con_port[10];
	sprintf(con_port, "%d", ++port);
	char p[10];
	struct server_info server_info;
	char command[30];
	// sprintf(command, sizeof(command), "./node -p ");
	for (i=0; i<max_visual_server; i++){
		strcpy(command, "./node -p ");
		sprintf(p, "%d", ++port);
		strcat(command, p);
		strcat(command, " -c ");
		// strcat(command, con_port);
		strcat(command, Port);
		server_info.pfile = popen(command, "r");
		if (server_info.pfile == NULL){
			i--;
			break;
		}
		cout << "boot servers excute command: " << command << endl;
		strcpy(server_info.Port, p);
		server_info.active_con = 0;
		server_info.ability = 1;
		server_cluster.push_back(server_info);
	}
	return;
}
int LoadBalancer::selectServer(){
	int i;
	char* p;
	list<server_info>::iterator it;
	// updateStatus();
	// cout << "updateStatus OK!" << endl;
	for (it=server_cluster.begin(), i=it->active_con, p=it->Port; it!=server_cluster.end(); it++){
		if (i > it->active_con){
			i = it->active_con;
			p = it->Port;
		}
	}
	cout << "selected server: " << p << endl;
	return atoi(p);
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