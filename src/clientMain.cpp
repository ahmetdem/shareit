#include "../include/Client.h"
#include "../include/Global.h"
#include "../include/Parser.h"
#include <iostream>
#include <string>
#include <thread>

int main(int argc, char const *argv[]) {
  std::string name;

  std::cout << "Enter a Nickname: ";
  std::cin >> name;
  std::cout << std::endl;

  Client client("127.0.1.1", name);
  Parser parser;

  Parser::Option sendFileToClientOption(
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

        if (!client.setupConn(port, ConnectionType::FileTransfer)) {
          std::cerr << "Error: Failed to set up connection\n";
          return;
        }

        std::cout << "Connection established\n";

        fs::path path = argv[3];

        if (!fs::exists(path)) {
          std::cerr << "Error: Path does not exist\n";
          return;
        }

        client.receiveClientsList();

        int clientSocket;
        std::cout << "\n\nWrite the Socket number of The Client You want to "
                     "send File: ";
        std::cin >> clientSocket;
        std::cout << std::endl;

        client.sendFile(clientSocket, path);
      });

  Parser::Option receiveFileOption(
      "-r", "Receive files that another Client sends.",
      [argv, argc, &client]() {
        if (argc != 3) {
          std::cerr << "Usage: <program_name> -r <port_number>\n";
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

        if (!client.setupConn(port, ConnectionType::FileTransfer)) {
          std::cerr << "Error: Failed to set up connection\n";
          return;
        }

        std::cout << "Connection established, waiting for file transfer\n";

        while (true) {
          if (client.receiveFileFromServer()) {
            break;
          }
          // add a delay here to avoid busy waiting
          std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
      });

  Parser::Option connectOption(
      "-c", "Just connect to the server.", [argv, &client]() {
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

        if (!client.setupConn(port, ConnectionType::Normal)) {
          std::cerr << "Error: Failed to set up connection\n";
          return;
        }

        std::cout << "Connection established\n";
      });

  parser.add_custom_option(sendFileToClientOption);
  parser.add_custom_option(connectOption);
  parser.add_custom_option(receiveFileOption);

  parser.parse(argc, argv);

  return 0;
}
