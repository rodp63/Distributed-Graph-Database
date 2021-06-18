#ifndef HEADER_DGDB
#define HEADER_DGDB
  
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <iostream>
#include <string>
#include <thread>
#include <map>
#include <vector>

using namespace std;

class DGDB
{
private:
    int port;
    string ip;
    struct sockaddr_in stSockAddr;
    int connection;
    int server;
    int socketServer;
    int socketCliente;
    int socketRepository; // se usa en el servidor respositorio
    char mode;
    int numberRepositories;
    int repository;
    
    int mainPort;
    string mainIp;
 
    std::map<int,std::string>   connections;
    std::vector<int> socketRepositories; // se usa en el maestro
    //Repository_port  Repository_ip

    void runMainServer();
    
    void runConnection(int Pconnection);
    void connMasterRepository(int pPort, string pIp);

public:
    void runRepository();
    DGDB(char Pmode='S'){
        mode = Pmode;
        ip="127.0.0.1";
        port=50000;
        connection=0;
        server=0;
        numberRepositories = 0;
        repository=0;
        socketRepositories.push_back(0);
    }
    void setMainIp(string ip){mainIp = ip;};
    void setMainPort(int pport=50000){mainPort = pport;};

    void setServer();
    void setClient();
    void setRepository();
    void closeServer();
    void closeClient();
    void closeRepository();
    void runServer();
    void setMode(char Pmode='S') { mode = Pmode;}
    void setPort(int Pport=50000) { port = Pport;}
    void setIp(string Pip="127.0.0.1") { ip = Pip;}
    void setNumberRepositories(int r){numberRepositories = r;}
    void registerRepository();
    // CRUD DGDB
    void setNode(string name);
    void setRelation(string nameA,string nameB);
    void createRelation(string nameA,string nameB,int conn=0);
    ///  pritocolo
    void createNode(string name,int conn=0);
    void createNodeAttrite(string name,int conn,string attriteName,string attriteValue);

};

#endif