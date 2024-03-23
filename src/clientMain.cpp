#include "../include/Client.h"
#include "../include/Parser.h"
#include <iostream>
#include <string>

int main(int argc, char const *argv[]) {
  Client client("127.0.0.1");
  Parser parser;

  Parser::Option sendFileOption(
      "-s", "Send Files to the server.", [argv, argc, &client]() {
        if (argc != 4) {
          std::cerr << "Usage: <program_name> -s <port_number> <file_path>\n";
          return;
        }

        int port;
        try {
          port = std::stoi(argv[2]);
        } catch (const std::invalid_argument &e) {
          std::cerr << "Invalid port number: " << argv[2] << std::endl;
          return;
        } catch (const std::out_of_range &e) {
          std::cerr << "Port number out of range: " << argv[2] << std::endl;
          return;
        }

        if (!client.setupConn(port)) {
          std::cerr << "Error: Failed to set up connection\n";
          return;
        }

        std::cout << "Connection established\n";

        fs::path path = argv[3];

        if (!fs::exists(path)) {
          std::cerr << "Error: Path does not exist\n";
          return;
        }

        client.sendFile(path);
      });

  parser.add_custom_option(sendFileOption);
  parser.parse(argc, argv);

  return 0;
}
