#ifndef DGDB_TOOLS_H_
#define DGDB_TOOLS_H_

inline std::string trim(std::string& str)
{
    str.erase(0, str.find_first_not_of(' '));       //prefixing spaces
    str.erase(str.find_last_not_of(' ')+1);         //surfixing spaces
    return str;
}

inline bool is_number(const std::string &s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

void TryCommands() {
  std::cout << "-------------------------------------------------------------\n";
  std::cout << "               THESE COMMANDS ARE AVAILABLE:\n";
  std::cout << "-------------------------------------------------------------\n";
  std::cout << "---------------------- Main commands ------------------------\n";
  std::cout << "-------------------------------------------------------------\n";
  std::cout << " To run main server:\n";
  std::cout << "\tS --server (default port 50000)\n";
  std::cout << "\tS --server -p <port>\n";
  std::cout << " To run a repository:\n";
  std::cout << "\tS --repository -p <port> (default main ip and port)\n";
  std::cout << "\tS --repository -p <port> <ip_main> <port_main>\n";
  std::cout << "-------------------------------------------------------------\n";
  std::cout << "---------------------- CRUD commands ------------------------\n";
  std::cout << "-------------------------------------------------------------\n";
  std::cout << " Create:\n";
  std::cout << "\tC <node_name> [-a \"<atribute_name>=<atribute_value>\"]*\n"; 
  std::cout << "\t[-r \"<relation>\"]*\n";
  std::cout << " Request:\n";
  std::cout << "\tR <node> [--leaf] [--attr] [-d <depth>]\n";
  std::cout << "\t[-c \"<key> <op> <value> <logical_op>\"]*\n";
  std::cout << " Update:\n";
  std::cout << "\tU <node> --name <new_name>\n";
  std::cout << "\tU <node> --attr <new_value>\n";
  std::cout << " Delete:\n";
  std::cout << "\tD <node>\n";
  std::cout << "\tD <node> --relation <node>\n";
  std::cout << "\tD <node> --attr <attribute>\n";
  std::cout << "-------------------------------------------------------------" << std::endl;
}

void PrintRepository() {
  std::cout << "+-----------------------------------------------------------+\n";
  std::cout << "|               RUNNING DGDB REPOSITORY                     |\n";
  std::cout << "+-----------------------------------------------------------+\n";
}

void PrintMainServer() {
  std::cout << "+-----------------------------------------------------------+\n";
  std::cout << "|               RUNNING DGDB MAIN SERVER                    |\n";
  std::cout << "+-----------------------------------------------------------+\n";
}

enum class mode{
  kServer,
  kClient,
};

char *ReadInputConsole() {
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

std::vector<std::string> SplitInput(char *input_user_to_split){
  std::vector<std::string> args;
  char *token;

  token = strtok(input_user_to_split, " ");
  while(token != NULL){
    args.push_back(token);
    token = strtok(NULL, " ");
  }

  return args;
}

bool VerifyInput(std::vector<std::string> &input_user, mode &run_mode) {
  // only enter
  if (input_user.size() == 0) return false;
  
  // Help enter
  if (input_user[0] == "help" || input_user[0] == "HELP" || input_user[0] == "Help") {
    TryCommands();
    return false;
  }
  
  if ( input_user.size() == 1) {
    if (input_user[0] == "C" || input_user[0] == "R" ||
        input_user[0] == "U" || input_user[0] == "D") {
      std::cout << "[ERROR] Incomplete arguments." << std::endl;
      return false;
    } else {
      std::cout << "[ERROR] First argument doesn't exist." << std::endl;
      return false;
    }
  }

  // Server verifying
  if (input_user[0] == "S") {
    // Server
    if ( input_user[1] == "--server") {
      if (input_user.size() != 2 && input_user.size() != 4) {
        std::cout << "[ERROR] Incomplete arguments." << std::endl;
        return false;
      }
      if (input_user.size() == 2) {
        std::cout << "Running default main server..." << std::endl;
        run_mode = mode::kServer;
        return true;
      }
      if (input_user.size() > 4) {
        std::cout << "[ERROR] Too many argument. try command \"help\"" << std::endl;
        return false;
      }
      if (input_user[2] != "-p") {
        std::cout << "[ERROR] "<< input_user[2]<< " command, doesn't exist" << std::endl;
        return false;
      }
      if (!is_number(input_user[3])) {
        std::cout << "[ERROR] Port isn't number." << std::endl;
        return false;
      }
      std::cout << "Running main server..." << std::endl;
      return true;

    // Repository
    } else if (input_user[1] == "--repository") {
      if (input_user.size() != 4 && input_user.size() != 6) {
        std::cout << "[ERROR] Incomplete arguments." << std::endl;
        return false;
      }
      if (input_user[2] != "-p") {
        std::cout << "[ERROR] "<< input_user[2]<< " command, doesn't exist" << std::endl;
        return false;
      }
      if (!is_number(input_user[3])) {
        std::cout << "[ERROR] Port isn't number." << std::endl;
        return false;
      }
      if (input_user.size() == 6) {
        if (!is_number(input_user[5])) {
          std::cout << "[ERROR] Main port isn't number." << std::endl;
          return false;
        }
        std::cout << "Running repository..." << std::endl;
        run_mode = mode::kServer;
        return true;
      }
      std::cout << "Running default repository..." << std::endl;
      run_mode = mode::kServer;
      return true;
    } else {
      std::cout << "[ERROR] 2nd argument doesn't exist. Try help" << std::endl;
      return false;
    }
  // Client verifying
  } else if ( input_user[0] == "C" || input_user[0] == "R" ||
              input_user[0] == "U" || input_user[0] == "D") {
    if (input_user.size() == 1) {
      std::cout << "[ERROR] Incomplete arguments." << std::endl;
      return false;
    }
    run_mode = mode::kClient;
    return true;
  } else {
    std::cout << "[ERROR] First argument doesn't exist" << std::endl;
    return false;
  }

  return true;

}

#endif // DGDB_TOOLS_H_