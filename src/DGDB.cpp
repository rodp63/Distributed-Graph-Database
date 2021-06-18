#include "DGDB.h"
#include "tools.h"

using namespace std;

void DGDB::runConnection(int Pconnection)
{
  int n;
  int s, ss;
  char buffer[1024];
  char bufferB[1024];
  connections[Pconnection] = "";
  string data;
  bool existeRelaciones = false;
  bool existeAtributos = false;

  cout << "wwww" << endl;
  while (server || repository)
  {
    n = read(Pconnection, buffer, 1);
    if (n < 0)
      perror("ERROR reading from socket");
    else if (n == 0)
    {
    }
    else if (n > 0)
    {
      printf("%s\n", buffer);
    }
    if (buffer[0] == 'C')
    {
      cout << "Action:C\n";
      n = read(Pconnection, buffer, 3);
      if (n < 3)
      {
        perror("ERROR reading size\n");
      }
      else
      {
        buffer[n + 1] = '\0';
        s = atoi(buffer) + 2;
      }
      n = read(Pconnection, buffer, s);
      if (n < s)
      {
        perror("ERROR reading size\n");
      }
      else
      {
        int r;
        if (buffer[s - 1] == '0') // no existen relaciones
        {
          cout << "No se envio relaciones" << endl;
          buffer[s - 1] = '\0';
        }
        else if (buffer[s - 1] - '0' > 0)
        { // exite almenos una relacion
          existeRelaciones = true;
          n = read(Pconnection, bufferB, 3);
          bufferB[3] = '\0';
          ss = atoi(bufferB);
          n = read(Pconnection, bufferB, ss);
          bufferB[n] = '\0';

          cout << "nodeB:" << bufferB << endl;
        }
        if (buffer[s - 2] == '0') // no existen atributos
        {
          cout << "No se envio atributos" << endl;
          buffer[s - 2] = '\0';
        }
        cout << "data:[" << buffer << "]" << endl;
        data = buffer;
        if (socketRepositories.size() > 1)
        {
          // enviar al  Repository
          // determinar que repository
          if (socketRepositories.size() < 2)
          {
            cout << "Repository has not been attached." << endl;
            return;
          }
          else
            r = buffer[0] % (socketRepositories.size() - 1);
          r++;
          cout << "xxxx--:" << socketRepositories.size() << endl;
          cout << "xxxx-r:" << r << endl;
          cout << "xxxx-s:" << socketRepositories[r] << endl;
          cout << "xxxx-d:" << data << endl;
          if (existeRelaciones && !existeAtributos)
          {
            createRelation(data, bufferB, socketRepositories[r]);
          }
          else if (existeRelaciones && existeAtributos)
          {
            //
          }
          else if (!existeRelaciones && existeAtributos)
          {
            //
          }
          else if (!existeRelaciones && !existeAtributos)
          {
            createNode(data, socketRepositories[r]);
          }

          cout << "xxxx" << endl;
        }
        else if (repository)
        {
          cout << "Store:" << data << "-" << bufferB << endl;
        }
      }
    }
    if (buffer[0] == 'R')
    {
    }
    if (buffer[0] == 'U')
    {
    }
    if (buffer[0] == 'D')
    {
    }
    if (buffer[0] == 'R')
    {
      n = read(Pconnection, buffer, 21);
      if (n < 21)
      {
        perror("ERROR reading size\n");
      }
      else
      {
        char vIp[17];
        char vPort[6];
        int vPort_int;
        char *pbuffer;
        strncpy(vPort, buffer, 5);
        vPort_int = atoi(vPort);
        pbuffer = &buffer[5];
        strncpy(vIp, pbuffer, 16);
        buffer[16] = '\0';
        string vIp_string = vIp;
        trim(vIp_string);
        connMasterRepository(vPort_int, vIp_string);
        if (repository)
        {
          socketRepositories.push_back(socketRepository);
          cout << "Repository registed." << endl;
        }
        else
          perror("ERROR no se pudo registrar Repositorio\n");
      }
    }

    //n = write(ConnectFD,"I got your message",18);
    //if (n < 0) perror("ERROR writing to socket");
    /* perform read write operations ... */
  }
  connections.erase(Pconnection);
  shutdown(Pconnection, SHUT_RDWR);
  close(Pconnection);
}

void DGDB::runMainServer()
{
  for (;;)
  {
    int newConnection = accept(socketServer, NULL, NULL);
    if (0 > newConnection)
    {
      perror("error accept failed");
      close(newConnection);
      exit(EXIT_FAILURE);
    }
    std::thread(&DGDB::runConnection, this, newConnection).detach();
  }
}

void DGDB::runServer()
{
  if (mode == 'S')
  {
    std::thread runThread(&DGDB::runMainServer, this);
    runThread.join();
  }
  else if (mode == 'R')
  {
    cout << "RRRRR" << endl;
    std::thread runThread(&DGDB::runRepository, this);
    runThread.join();
  }
}
void DGDB::setClient()
{
  int Res;
  socketCliente = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (-1 == socketCliente)
  {
    perror("cannot create socket");
    exit(EXIT_FAILURE);
  }
  //memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(port);
  Res = inet_pton(AF_INET, ip.c_str(), &stSockAddr.sin_addr);
  if (0 > Res)
  {
    perror("error: first parameter is not a valid address family");
    close(socketCliente);
    exit(EXIT_FAILURE);
  }
  else if (0 == Res)
  {
    perror("char string (second parameter does not contain valid ipaddress");
    close(socketCliente);
    exit(EXIT_FAILURE);
  }
  Res == connect(socketCliente, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in));
  if (Res == -1)
  {
    perror("connect failed");
    close(socketCliente);
    exit(EXIT_FAILURE);
  }
  connection = 1;
}
void DGDB::closeClient()
{
  connection = 0;
  shutdown(socketCliente, SHUT_RDWR);
  close(socketCliente);
}

void DGDB::setServer()
{
  int Res;
  socketServer = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (-1 == socketServer)
  {
    perror("can not create socket");
    exit(EXIT_FAILURE);
  }
  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(port);
  stSockAddr.sin_addr.s_addr = INADDR_ANY;
  Res = bind(socketServer, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in));
  if (-1 == Res)
  {
    perror("error bind failed");
    close(socketServer);
    exit(EXIT_FAILURE);
  }
  Res = listen(socketServer, 10);
  if (-1 == Res)
  {
    perror("error listen failed");
    close(socketServer);
    exit(EXIT_FAILURE);
  }
  server = 1;
}

void DGDB::closeServer()
{
  server = 0;
  close(socketServer);
}

void DGDB::setRepository()
{

  cout << "setRepository" << endl;
  int Res;
  socketRepository = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (-1 == socketRepository)
  {
    perror("can not create socket");
    exit(EXIT_FAILURE);
  }
  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(port);
  stSockAddr.sin_addr.s_addr = INADDR_ANY;
  Res = bind(socketRepository, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in));
  if (-1 == Res)
  {
    perror("error bind failed");
    close(socketServer);
    exit(EXIT_FAILURE);
  }
  Res = listen(socketRepository, 10);
  if (-1 == Res)
  {
    perror("error listen failed");
    close(socketRepository);
    exit(EXIT_FAILURE);
  }

  registerRepository();
  repository = 1;
  mode = 'R';
}

void DGDB::setNode(string name)
{
  createNode(name, socketCliente);
}

void DGDB::setRelation(string nameA, string nameB)
{
  createRelation(nameA, nameB, socketCliente);
}
void DGDB::createRelation(string nameA, string nameB, int conn ){

  //string tmp = nameA + "-" + nameB;
  //int n = write(socketCliente,tmp.c_str(),tmp.length());

  /*
int action; // 1B CRUD                  C
int name_node_size;// 3B                3
char name_node[255];// VB               15151515 (DNI)
int number_of_attributes; // 2B         0
int number_of_relations;  // 3B         1

// relations are optional
int node_relation_size;  // 3B
char node_relations[255];// VB

*/

  char tamano[4];
  sprintf(tamano, "%03d", nameA.length());
  string buffer;
  string tmp = tamano;
  buffer = "C" + tmp + nameA + "01";

  sprintf(tamano, "%03d", nameB.length());
  tmp = tamano;
  buffer = buffer + tmp + nameB;
  //C004julio01004UCSP

  int n;
  //cout << "*" << buffer.c_str() << "*" << "endl";
  n = write(conn, buffer.c_str(), buffer.length());
  if (n < 0)
  {
    perror("error listen failed");
    close(conn);
    exit(EXIT_FAILURE);
  }
  else if (n > 0 && n != buffer.length())
  {
    /* code */
    perror("error listen failed\n");
  }
}

/// Protocolo

void DGDB::createNode(string name, int conn)
{

  /*
int action; // 1B CRUD                  C
int name_node_size;// 3B                3
char name_node[255];// VB               15151515 (DNI)
int number_of_attributes; // 2B         1
int number_of_relations;  // 3B         0
*/

  char tamano[4];
  sprintf(tamano, "%03d", name.length());
  string buffer;
  string tmp = tamano;
  buffer = "C" + tmp + name + "00";
  int n;
  //cout << "*" << buffer.c_str() << "*" << "endl";
  n = write(conn, buffer.c_str(), buffer.length());
  if (n < 0)
  {
    perror("error listen failed");
    close(socketCliente);
    exit(EXIT_FAILURE);
  }
  else if (n > 0 && n != buffer.length())
  {
    /* code */
  }
}

void DGDB::registerRepository()
{

  /*
int action; // X
int port; // 5B
char ip[16] VB
*/
  cout << ip << endl;
  string buffer;
  char vip[17];
  char vport[6];
  sprintf(vport, "%05d", port);
  vport[5] = '\0';
  sprintf(vip, "%016s", ip.c_str());
  cout << vip << endl;
  string sport = vport;
  string sip = vip;
  buffer = "R" + sport + sip;
  cout << "*" << buffer.c_str() << "*" << endl;
  // Set conn to Main
  int Res;
  int socketMain = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (-1 == socketMain)
  {
    cout << "cannot create socket" << endl;
    exit(EXIT_FAILURE);
  }
  //memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
  cout << "*1*" << endl;
  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(mainPort);
  Res = inet_pton(AF_INET, mainIp.c_str(), &stSockAddr.sin_addr);
  if (0 > Res)
  {
    cout << "error: first parameter is not a valid address family" << endl;
    close(socketMain);
    exit(EXIT_FAILURE);
  }
  else if (0 == Res)
  {
    cout << "char string (second parameter does not contain valid ipaddress" << endl;
    close(socketMain);
    exit(EXIT_FAILURE);
  }
  cout << "*2*" << endl;
  Res == connect(socketMain, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in));
  if (Res == -1)
  {
    cout << "connect failed" << endl;
    close(socketMain);
    exit(EXIT_FAILURE);
  }
  //
  cout << "*3*" << endl;
  int n = write(socketMain, buffer.c_str(), buffer.length());
  cout << "*4*" << endl;
  if (n < 0)
  {
    cout << "error listen failed" << endl;
    close(socketMain);
    exit(EXIT_FAILURE);
  }
  else if (n > 0 && n != buffer.length())
  {
    cout << "Registing Repository:[" << buffer << "]" << endl;
  }
  cout << n << endl;
}

void DGDB::runRepository()
{

  for (;;)
  {
    int newConnection = accept(socketRepository, NULL, NULL);
    if (0 > newConnection)
    {
      perror("error accept failed");
      close(newConnection);
      exit(EXIT_FAILURE);
    }
    std::thread(&DGDB::runConnection, this, newConnection).detach();
  }
}
void DGDB::connMasterRepository(int pPort, string pIp)
{
  cout << pPort << "-" << pIp << endl;
  int Res;
  socketRepository = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (-1 == socketRepository)
  {
    perror("cannot create socket");
    exit(EXIT_FAILURE);
  }
  //memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(pPort);
  Res = inet_pton(AF_INET, pIp.c_str(), &stSockAddr.sin_addr);
  if (0 > Res)
  {
    perror("error: first parameter is not a valid address family");
    close(socketRepository);
    exit(EXIT_FAILURE);
  }
  else if (0 == Res)
  {
    perror("char string (second parameter does not contain valid ipaddress");
    close(socketRepository);
    exit(EXIT_FAILURE);
  }
  Res == connect(socketRepository, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in));
  if (Res == -1)
  {
    perror("connect failed");
    close(socketRepository);
    exit(EXIT_FAILURE);
  }
  repository = 1;
}