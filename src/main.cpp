#include "DGDB.h"
#include "tools.h"

int ExecuteDGDB(std::vector<std::string> argv, mode &run_mode) {
  // TODO: default contructor shouldn't define some variables
  DGDB db;

  std::vector<std::string> args(argv.begin()+1, argv.end());

  // CRUD request
  if (run_mode == mode::kClient) {
    // TODO: these 4 following variables must be args
    // Setting main server
    std::string server_ip("127.0.0.1");
    int server_port = 50000;
    
    // Setting client
    std::string client_ip("127.0.0.1");
    int client_port = 50000;
    char mode;
    strcpy(&mode, argv[0].c_str());

    db.SetMode(mode);
    db.SetClient(client_ip, client_port); // TODO: add parameters client_ip, client_port
    db.SetMainIp(server_ip); 
    db.SetMainPort(server_port);

    if (argv[0] == "C")
      db.SetNode(args);
    else if (argv[0] == "R")
      db.SetQuery(args);
    else if (argv[0] == "U")
      db.SetUpdate(args);
    else if (argv[0] == "D")
      db.SetDelete(args);
  }

  // SERVER request
  else if (run_mode == mode::kServer) {

    std::string main_or_repo = argv[1];

    // Main server
    if (main_or_repo == "--server") {

      int server_port;
      
      if (argv.size() == 2) {
        server_port = 50000; // default setting
      } else { // 4 arguments
        server_port = stoi(argv[3]);
      }

      system("clear");
      PrintMainServer();

      db.SetPort(server_port);
      db.SetMode('S');
      db.SetServer();
      db.RunServer();
    }
    // Repository server
    else if (main_or_repo == "--repository") {
      
      // Setting main server
      int repo_port = stoi(argv[3]);
      std::string main_ip;
      int main_port;
      // TODO: this must be an arg
      std::string repo_ip("127.0.0.1");

      if (argv.size() == 6) {
        main_ip = argv[4];
        main_port = stoi(argv[5]);
      } else { // default setting
        main_ip = "127.0.0.1";
        main_port = 50000;
      }

      db.SetPort(repo_port);
      db.SetIp(repo_ip);
      db.SetMainIp(main_ip);
      db.SetMainPort(main_port);
      db.SetMode('E');
      db.SetRepository();
      
      std::cout << "[DGDB] Running Repository..." << std::endl;
      std::this_thread::sleep_for (std::chrono::seconds(2)); // seems pro
      system("clear");
      PrintRepository();

      db.RunServer();
    }
  }
  else std::cout << "[BUG DETECTED] ExecuteDGDB" << std::endl;

  return 1;
}

void RunDGDB() {
    char *input_user;
    std::vector<std::string> args;
    int status_dgdb;
    bool correct_input = false;
    mode run_mode;

    do {  
      printf("DGDB >> ");
      input_user = ReadInputConsole();
      args = SplitInput(input_user);
      correct_input = VerifyInput(args, run_mode);
      if (correct_input) status_dgdb = ExecuteDGDB(args, run_mode);
      free(input_user);
    } while(status_dgdb);
}

int main() {
    RunDGDB();
    return EXIT_SUCCESS;
}

/*
int action; // 1B CRUD                  C
int name_node_size;// 3B                3
char name_node[255];// VB               15151515 (DNI)
int number_of_attributes; // 2B         1
int number_of_relations;  // 3B         0

// attributes are optional
int name_attribute_size;  //  3B        13
char name_attribute[255]; //  VB        Primer Nombre
int value_attribute_size; //  3B        5
char value_attribute[255]; // VB        Julio

// relations are optional
int node_relation_size;  // 3B
char node_relations[255];// VB

insert
tabla de Nodos
Julio
Fabiola

tabla de Relaciones
Julio  |  Fabiola

tabla atributos
Julio  | edad | 48
Julio  | grad Aca | Phd
Julio  | Est. Civ. | Casado



9B  VB ?A  6B VB ?R 3B VB
int action; // 1B CRUD                  C
int name_node_size;// 3B                3
int number_of_attributes; // 2B         1
int number_of_relations;  // 3B         0
char name_node[255];// VB               15151515 (DNI)

// attributes are optional
int name_attribute_size;  //  3B        13
int value_attribute_size; //  3B        5
char name_attribute[255]; //  VB        Primer Nombre
char value_attribute[255]; // VB        Julio

// relations are optional
int node_relation_size;  // 3B
char node_relations[255];// VB

////////////////////////////////////////////

int action; //       1B CRUD                  R
int query_node_size ; //  2B                  0
char query_node[99];  //  VB                  0
int  deep; //        1B                       3
int  leaf; // on/off 1B                       off
int attributes; // on/off 1B                  1       //  se quiere extraer los atributos

int number_of_conditions;  // 2B              3

int  query_name_attribute_size;  //  3B       15
char query_name_attribute[255]; //  VB        Grado academico
int  operador; // 1=, 2>, 3<, ..... 4like     4
int  query_value_attribute_size; //  3B       2
char query_value_attribute[255]; // VB        MSc*
int  is_and; on/off // 1B                     AND

int  query_name_attribute_size;  //  3B       15
char query_name_attribute[255]; //  VB        Grado academico
int  operador; // 1=, 2>, 3<, ..... 4like     4
int  query_value_attribute_size; //  3B       2
char query_value_attribute[255]; // VB        Dr
int  is_and; on/off // 1B                     OR

int  query_name_attribute_size;  //  3B       15
char query_name_attribute[255]; //  VB        Grado academico
int  operador; // 1=, 2>, 3<, ..... 4like     4
int  query_value_attribute_size; //  3B       2
char query_value_attribute[255]; // VB        Dra
int  is_and; on/off // 1B



////////////////////////////////////////////
int action; //       1B CRUD                      U
int node_or_attribute; // on:node off:attibute    1
int  query_node_size ; //  2B                     5
char query_value_node[99];  //  VB                      Julio

int  set_node_size ; //  3B                       4
char set_value_node[99];  //  VB                        Omar

int query_attribute_size;  //          3B        15
int query_value_attribute_size; //  3B           Grado academico
int set_attribute_size;  //  3B        13        2
int set_value_attribute_size; //  3B             Dr

////////////////////////////////////////////
int action; //       1B CRUD                      D
int node_or_attribute; // 0:node 1:attibute 2:R   1
int  query_node_size ; //  2B                     5
char query_value_node[99];  //  VB                      Julio

int query_attribute_o_R_size;  //          3B        15
int query_value_attribute_o_R_size; //  3B           Grado academico
*/
