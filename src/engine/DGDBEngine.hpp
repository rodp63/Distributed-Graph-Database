#ifndef DGDB_ENGINE_EDGDBEngine_HPP_
#define DGDB_ENGINE_EDGDBEngine_HPP_

#include "DGDB.h"
#include "tools.h"

enum class DGDBEngineError {
  not_found = 1,
  type_is_not_mapped_to_storage,
  trying_to_dereference_null_iterator,
  too_many_tables_specified,
  incorrect_set_fields_specified,
  column_not_found,
  table_has_no_primary_key_column,
  cannot_start_a_transaction_within_a_transaction,
  no_active_transaction,
  incorrect_journal_mode_string,
  invalid_collate_argument_enum,
  failed_to_init_a_backup,
  unknown_member_value,
  incorrect_order,
};

enum class mode{
  kServerMain,
  kServerRepository,
  kClient,
  kError,
  kNotArguments,
  kShell,
};

static const char* const running_joke[] = { "[DGDB] Running Repository.",
                                            "[DGDB] Running Repository..",
                                            "[DGDB] Running Repository...",
                                            "[DGDB] Running Repository...."};

class DGDBEngine {

 private:

  std::vector<std::string> args_first_input;
  bool shell_on = true;

 public:

  mode run_mode;
  DGDBEngine(){}

  void Init(int, char**);
  void PrintError();
  void PrintServerTitle(std::string);
  void PrintMainUsage();
  void PrintMainUsageFirstArg();
  char* ReadInputConsole();
  std::string PrintError(int);
  std::vector<std::string> SplitInput(char *);
  bool VerifyInput(std::vector<std::string>);
  bool RunServerMain();
  bool RunServerRepository();
  bool RunClient(std::vector<std::string>, std::string);
  void SetEngineMode(std::string);
  bool Shell();
};

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void DGDBEngine::Init(int argc, char** argv) {

  if (argc == 1) {
    this->run_mode = mode::kNotArguments;
    return;
  }

  while (argc > 1) {
    args_first_input.push_back(argv[argc-1]);
    argc--;
  }
  std::reverse(args_first_input.begin(), args_first_input.end());
  SetEngineMode(args_first_input[0]);
}

void DGDBEngine::SetEngineMode(std::string string_mode) {
  if (string_mode == "server") {
    this->run_mode = mode::kServerMain;
  } else if (string_mode == "repository") {
    this->run_mode = mode::kServerRepository;
  } else if (string_mode == "shell") {
    this->run_mode = mode::kShell;
  } else {
    this->run_mode = mode::kError;
  }
}

bool DGDBEngine::RunServerMain() {

  bool correct_input = true;
  correct_input = VerifyInput(args_first_input);
  if (!correct_input) return false;

  DGDB db;

  int server_port;
  std::string type_server;

  if (args_first_input.size() == 2) {
    server_port = 50000;
    type_server = "RUNNING DEFAULT MAIN SERVER | port = 50000";
  } else if (args_first_input.size() == 3) {
    server_port = std::stoi(args_first_input[2]);
    type_server = "RUNNING NORMAL MAIN SERVER | port = " + args_first_input[2];
  } else {
    std::cout << "[BUG DETECTED] RunServerMain" << std::endl;
  }

  db.setPort(server_port);
  db.setMode('S');
  db.setServer();
  
  PrintServerTitle(type_server);

  db.runServer();
    
  return true;
}

bool DGDBEngine::RunServerRepository() {

  bool correct_input = true;
  correct_input = VerifyInput(args_first_input);
  if (!correct_input) return false;

  DGDB db;

  int repo_port = stoi(args_first_input[2]);
  std::string main_ip;
  int main_port;
  std::string type_server;

  if (args_first_input.size() == 5) {
    main_ip = args_first_input[3];
    main_port = stoi(args_first_input[4]);
    type_server = "Running repository";
  } else if (args_first_input.size() == 3){
    main_ip = "35.240.132.238";
    main_port = 50000;
    type_server = "Running default repository";
  } else {
    std::cout << "[BUG DETECTED] RunServerRepository" << std::endl;
  }

  // TODO: delete this arg
  std::string repo_ip("127.0.0.1");

  db.setPort(repo_port);
  db.setIp(repo_ip);
  db.setMainIp(main_ip);
  db.setMainPort(main_port);
  db.setMode('E');
  db.setRepository();
  
  // OPTIONAL: Seems pro
  int joke = 0;
  while (joke++ < 12) {
    std::cout << running_joke[joke%4] << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    system("clear");
  }

  PrintServerTitle(type_server);

  db.runServer();

  return true;
}

bool DGDBEngine::RunClient(std::vector<std::string> args_in, 
                          std::string ip_main_server) {
  
  DGDB db;

  std::string server_ip = ip_main_server;
  std::string crud_mode = args_in[0];
  std::vector<std::string> args(args_in.begin()+1, args_in.end());

  // TODO: these 4 following variables must be args
  // Setting main server
  int server_port = 50000;
  
  // Setting client
  std::string client_ip("127.0.0.1");
  int client_port = 40000;
  char mode;
  strcpy(&mode, crud_mode.c_str());

  db.setMode(mode);
  db.setClient();
  db.setMainIp(server_ip);
  db.setMainPort(server_port);

  bool st;

  if (crud_mode == "create")
    st = db.setNode(args);
  else if (crud_mode == "read")
    st = db.setQuery(args);
  else if (crud_mode == "update")
    st = db.setUpdate(args);
  else if (crud_mode == "delete")
    st = db.setDelete(args);
  else std::cout << "[BUG DETECTED] RunClient" << std::endl;

  if (st)
    db.WaitResponse();
  
  return true;
}

bool DGDBEngine::Shell() {

  bool correct_in = true;
  correct_in = VerifyInput(args_first_input);
  if (!correct_in) return false;

  this->run_mode = mode::kClient;
  std::string server_ip;
  if (args_first_input[1] != "--default") {
    server_ip = args_first_input[1];
  } else server_ip = "127.0.0.1";
  
  char *input_user;
  std::vector<std::string> args;
  bool status_dgdb;
  this->shell_on = true;
  bool correct_input = false;

  do {
    printf("DGDB >> ");
    input_user = ReadInputConsole();
    args = SplitInput(input_user);
    correct_input = VerifyInput(args);
    
    if (correct_input) {
      status_dgdb = RunClient(args, server_ip);
    }
    
    free(input_user);
  } while(shell_on);

  return true;
}

std::string DGDBEngine::PrintError(int c) {
  switch(static_cast<DGDBEngineError>(c)) {
    case DGDBEngineError::not_found:
      return "Not found";
    case DGDBEngineError::type_is_not_mapped_to_storage:
      return "Type is not mapped to storage";
    case DGDBEngineError::trying_to_dereference_null_iterator:
      return "Trying to dereference null iterator";
    case DGDBEngineError::too_many_tables_specified:
      return "Too many tables specified";
    case DGDBEngineError::incorrect_set_fields_specified:
      return "Incorrect set fields specified";
    case DGDBEngineError::column_not_found:
      return "Column not found";
    case DGDBEngineError::table_has_no_primary_key_column:
      return "Table has no primary key column";
    case DGDBEngineError::cannot_start_a_transaction_within_a_transaction:
      return "Cannot start a transaction within a transaction";
    case DGDBEngineError::no_active_transaction:
      return "No active transaction";
    case DGDBEngineError::invalid_collate_argument_enum:
      return "Invalid collate_argument enum";
    case DGDBEngineError::failed_to_init_a_backup:
      return "Failed to init a backup";
    case DGDBEngineError::unknown_member_value:
      return "Unknown member value";
    case DGDBEngineError::incorrect_order:
      return "Incorrect order";
    default:
      return "unknown error";
  }
}

void DGDBEngine::PrintMainUsage() {
  std::cout << "\nUSAGE: dgdb [COMMAND]\n\n";
  std::cout << "  COMMAND \n";
  std::cout << "     server      To init main server\n";
  std::cout << "     repository  To start one repository\n";
  std::cout << "     shell       To start client shell" << std::endl;
}

void DGDBEngine::PrintMainUsageFirstArg() {
  std::cout << "\n[ERROR: \""<< args_first_input[0] <<"\" first Argument doesn't exist]\n";
  std::cout << "USAGE: dgdb [COMMAND]\n\n";
  std::cout << "  COMMAND \n";
  std::cout << "     server      To init main server\n";
  std::cout << "     repository  To start one repository\n";
  std::cout << "     shell       To start client shell" << std::endl;
}

void DGDBEngine::PrintServerTitle(std::string title) {
  system("clear");
  std::cout << "+-----------------------------------------------------------+\n";
  std::cout << "          " << title << "                    \n";
  std::cout << "+-----------------------------------------------------------+\n";
}

char* DGDBEngine::ReadInputConsole() {
  int position = 0;
  char *buffer = (char*)malloc(sizeof(char) * 1024);
  int c;

  if(!buffer){
    fprintf(stderr, "DGDB: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while(1){
    c = getchar();
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;
  }
  return buffer;
}

std::vector<std::string> DGDBEngine::SplitInput(char *input_user_to_split){
  std::vector<std::string> args;
  char *token;

  token = strtok(input_user_to_split, " ");
  while(token != NULL){
    args.push_back(token);
    token = strtok(NULL, " ");
  }

  return args;
}

bool DGDBEngine::VerifyInput(std::vector<std::string> input_user) {
  
  if (this->run_mode == mode::kClient) {
    if (input_user[0] == "create") {
      if (input_user.size() == 1) {
        std::cout << "\n[ERROR] Not enought arguments\n";
        std::cout << "USAGE: create [REQUIRED] [OPTIONALS]*\n\n";
        std::cout << "  REQUIRED\n";
        std::cout << "     <node_name> \n";
        std::cout << "  OPTIONALS\n";
        std::cout << "     -a \"<atribute_name>=<atribute_value>\"   To add atributes\n";
        std::cout << "     -r \"<relation>\"                         To add relations\n";
        std::cout << "     *                                         Its posible multiple times\n";
        return false;
      } else {
        return true;
      }
    } else if (input_user[0] == "read") {
      if (input_user.size() == 1) {
        std::cout << "\n[ERROR] Not enought arguments\n";
        std::cout << "USAGE: read <node_name> [--leaf] [--attr] [-d <depth>] ";
        std::cout << "[-c \"<key> <op> <value> <logical_op>\"]*\n";
        std::cout << "  ARGUMENTS EXPLAINED\n";
        std::cout << "     <node_name>       Node's name object of request\n";
        std::cout << "     --leaf            Set this retrieve only leaf of search according depth (optional)\n";
        std::cout << "     --attr            Set this retrieve attributes (optional)\n";
        std::cout << "     -d                To set depth of search (required)\n";
        std::cout << "     <depth>           Number of depth of search\n";
        std::cout << "     -c                To set a condition\n";
        std::cout << "     <key>             Attribute name\n";
        std::cout << "     <op>              operator that could be > < =\n";
        std::cout << "     <value>           Value of attribute\n";
        std::cout << "     <logical_op>      To set a logical union between condition. Could be & |\n";
        std::cout << "                       (optional if last condition, required if not)\n";
        std::cout << "     *                 Its posible multiple times\n";
        return false;
      } else {
        return true;
      }
    } else if (input_user[0] == "update") {
      if (input_user.size() == 1) {
        std::cout << "\n[ERROR] Not enought arguments\n";
        std::cout << "USAGE: update <node_name> [ARGUMENT]\n\n";
        std::cout << "  ARGUMENT\n";
        std::cout << "     --name <new_name>       To change name of node or\n";
        std::cout << "     --attr <new_value>      To add one attribute\n";
        return false;
      } else {
        return true;
      }
    } else if (input_user[0] == "delete") {
      if (input_user.size() == 1) {
        std::cout << "\n[ERROR] Not enought arguments\n";
        std::cout << "USAGE: delete <node_name> [ARGUMENT]\n\n";
        std::cout << "  ARGUMENT\n";
        std::cout << "     --relation <name_relation>      To delete a relation or\n";
        std::cout << "     --attr <name_attribute>         To delete an attribute\n";
        return false;
      }  else {
        return true;
      }
    } else if (input_user[0] == "exit") {
        this->shell_on = false;
        return false;
    } else {
      std::cout << "\n[ERROR] \""<< input_user[0] <<"\" is not an valid command\n";
      std::cout << "USAGE: [COMMAND]\n\n";
      std::cout << "  COMMAND \n";
      std::cout << "     create     To create a new node\n";
      std::cout << "     read       To retrieve information from data base\n";
      std::cout << "     update     To update node's information\n";
      std::cout << "     delete     To delete a node or relation\n";
      std::cout << "     exit       To exit shell\n";
      return false;
    }
  } else if (this->run_mode == mode::kShell) {
    if (input_user.size() == 1) {
      std::cout << "\n[ERROR: Not enought arguments. Define main server ip address]\n";
      std::cout << "USAGE: dgdb shell [OPTION]\n\n";
      std::cout << "  OPTION \n";
      std::cout << "     <ip_main_server>     To run no local, set main server ip address\n";
      std::cout << "     --default            main server ip = 127.0.0.1 \n";
      return false;
    }
    if (input_user.size() > 2) {
      std::cout << "\n[ERROR: Too many arguments]\n";
      std::cout << "USAGE: dgdb shell [OPTION]\n\n";
      std::cout << "  OPTION \n";
      std::cout << "     <ip_main_server>     To run no local, set main server ip address\n";
      std::cout << "     --default            main server ip = 127.0.0.1 \n";
      return false;
    }
    return true;
  } else if (this->run_mode == mode::kServerRepository) { // REPOSITORY VERIFYING
    if (input_user.size() == 1) {
      std::cout << "\n[ERROR: Not enought arguments]\n";
      std::cout << "USAGE: dgdb repository [OPTION]\n\n";
      std::cout << "  OPTION \n";
      std::cout << "     -p <port_value>\n";
      std::cout << "             ip and port main server => 127.0.0.1 | 50000\n";
      std::cout << "     -p <port_value> \"<ip_main_server>\" <port_main_server>\n";
      std::cout << "             To personalize main server port and ip\n";
      return false;
    } else if (input_user.size() == 2) {
      if (input_user[1] == "-p") {
        std::cout << "\n[ERROR: Not enought arguments]\n";
        std::cout << "USAGE: dgdb repository [OPTION]\n\n";
        std::cout << "  OPTION \n";
        std::cout << "     -p <port_value>\n";
        std::cout << "             ip and port main server => 127.0.0.1 | 50000\n";
        std::cout << "     -p <port_value> \"<ip_main_server>\" <port_main_server>\n";
        std::cout << "             To personalize main server port and ip\n";
        return false;
      } else {
        std::cout << "\n[ERROR: \""<< input_user[1] <<"\" isn't a option and not enought arguments]\n";
        std::cout << "USAGE: dgdb repository [OPTION]\n\n";
        std::cout << "  OPTION \n";
        std::cout << "     -p <port_value>\n";
        std::cout << "             ip and port main server => 127.0.0.1 | 50000\n";
        std::cout << "     -p <port_value> \"<ip_main_server>\" <port_main_server>\n";
        std::cout << "             To personalize main server port and ip\n";
        return false;
      }
    } else if (input_user.size() == 3) {
      if (input_user[1] != "-p") {
        std::cout << "\n[ERROR: \""<< input_user[1] <<"\" isn't a option]\n";
        std::cout << "USAGE: dgdb repository [OPTION]\n\n";
        std::cout << "  OPTION \n";
        std::cout << "     -p <port_value>\n";
        std::cout << "             ip and port main server => 127.0.0.1 | 50000\n";
        std::cout << "     -p <port_value> \"<ip_main_server>\" <port_main_server>\n";
        std::cout << "             To personalize main server port and ip\n";
        return false;
      }
      if (!is_number(input_user[2])) {
        std::cout << "\n[ERROR: port_value isn't a number]\n";
        std::cout << "USAGE: dgdb repository [OPTION]\n\n";
        std::cout << "  OPTION \n";
        std::cout << "     -p <port_value>\n";
        std::cout << "             ip and port main server => 127.0.0.1 | 50000\n";
        std::cout << "     -p <port_value> \"<ip_main_server>\" <port_main_server>\n";
        std::cout << "             To personalize main server port and ip\n";
        return false;
      }
      std::cout << "Running default repository" << std::endl;
      return true;
    } else if (input_user.size() == 4) {
      if (input_user[1] != "-p") {
        std::cout << "\n[ERROR: \""<< input_user[1] <<"\" isn't a option]\n";
        std::cout << "USAGE: dgdb repository [OPTION]\n\n";
        std::cout << "  OPTION \n";
        std::cout << "     -p <port_value>\n";
        std::cout << "             ip and port main server => 127.0.0.1 | 50000\n";
        std::cout << "     -p <port_value> \"<ip_main_server>\" <port_main_server>\n";
        std::cout << "             To personalize main server port and ip\n";
        return false;
      }
      if (!is_number(input_user[2])) {
        std::cout << "\n[ERROR: port_value isn't a number]\n";
        std::cout << "USAGE: dgdb repository [OPTION]\n\n";
        std::cout << "  OPTION \n";
        std::cout << "     -p <port_value>\n";
        std::cout << "             ip and port main server => 127.0.0.1 | 50000\n";
        std::cout << "     -p <port_value> \"<ip_main_server>\" <port_main_server>\n";
        std::cout << "             To personalize main server port and ip\n";
        return false;
      }
      if (is_number(input_user[3])) {
        std::cout << "\n[ERROR: Ip shouldn't be a number]\n";
        std::cout << "USAGE: dgdb repository [OPTION]\n\n";
        std::cout << "  OPTION \n";
        std::cout << "     -p <port_value>\n";
        std::cout << "             ip and port main server => 127.0.0.1 | 50000\n";
        std::cout << "     -p <port_value> \"<ip_main_server>\" <port_main_server>\n";
        std::cout << "             To personalize main server port and ip\n";
        return false;
      }
      std::cout << "\n[ERROR: Incomplete arguments]\n";
      std::cout << "USAGE: dgdb repository [OPTION]\n\n";
      std::cout << "  OPTION \n";
      std::cout << "     -p <port_value>\n";
      std::cout << "             ip and port main server => 127.0.0.1 | 50000\n";
      std::cout << "     -p <port_value> \"<ip_main_server>\" <port_main_server>\n";
      std::cout << "             To personalize main server port and ip\n";
      return false;

    } else if (input_user.size() == 5) {
      if (input_user[1] != "-p") {
        std::cout << "\n[ERROR: \""<< input_user[1] <<"\" isn't a option]\n";
        std::cout << "USAGE: dgdb repository [OPTION]\n\n";
        std::cout << "  OPTION \n";
        std::cout << "     -p <port_value>\n";
        std::cout << "             ip and port main server => 127.0.0.1 | 50000\n";
        std::cout << "     -p <port_value> \"<ip_main_server>\" <port_main_server>\n";
        std::cout << "             To personalize main server port and ip\n";
        return false;
      }
      if (!is_number(input_user[2])) {
        std::cout << "\n[ERROR: port_value isn't a number]\n";
        std::cout << "USAGE: dgdb repository [OPTION]\n\n";
        std::cout << "  OPTION \n";
        std::cout << "     -p <port_value>\n";
        std::cout << "             ip and port main server => 127.0.0.1 | 50000\n";
        std::cout << "     -p <port_value> \"<ip_main_server>\" <port_main_server>\n";
        std::cout << "             To personalize main server port and ip\n";
        return false;
      }
      if (is_number(input_user[3])) {
        std::cout << "\n[ERROR: Ip shouldn't be a number]\n";
        std::cout << "USAGE: dgdb repository [OPTION]\n\n";
        std::cout << "  OPTION \n";
        std::cout << "     -p <port_value>\n";
        std::cout << "             ip and port main server => 127.0.0.1 | 50000\n";
        std::cout << "     -p <port_value> \"<ip_main_server>\" <port_main_server>\n";
        std::cout << "             To personalize main server port and ip\n";
        return false;
      }
      if (!is_number(input_user[4])) {
        std::cout << "\n[ERROR: Main server port_value isn't a number]\n";
        std::cout << "USAGE: dgdb repository [OPTION]\n\n";
        std::cout << "  OPTION \n";
        std::cout << "     -p <port_value>\n";
        std::cout << "             ip and port main server => 127.0.0.1 | 50000\n";
        std::cout << "     -p <port_value> \"<ip_main_server>\" <port_main_server>\n";
        std::cout << "             To personalize main server port and ip\n";
        return false;
      }
      std::cout << "Running repository" << std::endl;
      return true;
    } else if (input_user.size() > 5) {
      std::cout << "\n[ERROR: Too many arguments]\n";
      std::cout << "USAGE: dgdb repository [OPTION]\n\n";
      std::cout << "  OPTION \n";
      std::cout << "     -p <port_value>\n";
      std::cout << "             ip and port main server => 127.0.0.1 | 50000\n";
      std::cout << "     -p <port_value> \"<ip_main_server>\" <port_main_server>\n";
      std::cout << "             To personalize main server port and ip\n";
      return false;

    } else {
      std::cout << "[BUG DETECTED] VerifyInput REPOSITORY" << std::endl;
    }


  } else if (this->run_mode == mode::kServerMain) { // SERVER VERIFYING
    if (input_user.size() == 1) {
      std::cout << "\n[ERROR: Enought arguments]\n";
      std::cout << "USAGE: dgdb server [OPTION]\n\n";
      std::cout << "  OPTION \n";
      std::cout << "     --default          This set port = 50000\n";
      std::cout << "     -p <port_value>    To personalize port\n";
      return false;
    } else if (input_user.size() > 3) {
      std::cout << "\n[ERROR: Too many arguments]\n";
      std::cout << "USAGE: dgdb server [OPTION]\n\n";
      std::cout << "  OPTION \n";
      std::cout << "     --default          This set port = 50000\n";
      std::cout << "     -p <port_value>    To personalize port\n";
      return false;
    } else if (input_user.size() == 2) {
      if (input_user[1] == "--default") {
      std::cout << "Running default main server..." << std::endl;
      return true;
      } else if(input_user[1] == "-p") {
        std::cout << "\n[ERROR: You must define port_value or try --default]\n";
        std::cout << "USAGE: dgdb server [OPTION]\n\n";
        std::cout << "  OPTION \n";
        std::cout << "     --default          This set port = 50000\n";
        std::cout << "     -p <port_value>    To personalize port\n";
        return false;
      } else {
        std::cout << "\n[ERROR: \""<< input_user[1] <<"\" isn't a option]\n";
        std::cout << "USAGE: dgdb server [OPTION]\n\n";
        std::cout << "  OPTION \n";
        std::cout << "     --default          This set port = 50000\n";
        std::cout << "     -p <port_value>    To personalize port\n";
        return false;
      }
    } else if (input_user.size() == 3) {
      if (input_user[1] != "-p") {
        std::cout << "\n[ERROR: \""<< input_user[1] <<"\" isn't a option]\n";
        std::cout << "USAGE: dgdb server [OPTION]\n\n";
        std::cout << "  OPTION \n";
        std::cout << "     --default          This set port = 50000\n";
        std::cout << "     -p <port_value>    To personalize port\n";
        return false;
      } else if (!is_number(input_user[2])) {
        std::cout << "\n[ERROR: port_value \""<< input_user[2] <<"\" isn't a number]\n";
        std::cout << "USAGE: dgdb server [OPTION]\n\n";
        std::cout << "  OPTION \n";
        std::cout << "     --default          This set port = 50000\n";
        std::cout << "     -p <port_value>    To personalize port\n";
        return false;
      } else {
        std::cout << "Running main server" << std::endl;
        return true;
      }
    } else {
      std::cout << "[BUG DETECTED] VerifyInput SERVER" << std::endl;
      return false;
    }
    return true;
  }  else{
    std::cout << "[BUG DETECTED] VerifyInput" << std::endl;
  }

  return true;
}

#endif // DGDB_ENGINE_EDGDBEngine_HPP_
