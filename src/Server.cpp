#include "../include/Server.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

Server::Server(const int port) : m_port(port), m_serverSocket(-1) {}
Server::~Server() { this->stop(); }

void Server::start() {
  // Create socket
  m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (m_serverSocket == -1) {
    std::cerr << "Error: Failed to create socket\n";
    return;
  }

  // Set up server address structure
  sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(m_port);

  // Bind socket to address
  if (bind(m_serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
    std::cerr << "Error: Bind failed\n";
    return;
  }

  // Listen for incoming connections
  if (listen(m_serverSocket, SOMAXCONN) == -1) {
    std::cerr << "Error: Listen failed\n";
    return;
  }

  std::cout << "Server started\n";
}

bool Server::stop() {
  if (m_serverSocket != -1) {
    if (close(m_serverSocket) == -1) {
      std::cerr << "Error: Failed to close server socket\n";
      return false;
    }
    m_serverSocket = -1;
    std::cout << "Server closed\n";
  }
  return true;
}

bool Server::acceptConn() {
  sockaddr_in clientAdrr;
  socklen_t clientAdrrLen = sizeof(clientAdrr);

  int clientSocket =
      accept(this->m_serverSocket, (sockaddr *)&clientAdrr, &clientAdrrLen);

  if (clientSocket == -1) {
    std::cerr << "Error: Accept failed\n";
    return false;
  }

  // Handle client connection in a separate thread or process
/*   this->handleClient(clientSocket); */

  return true;
}

// void Server::receiveFile() {}
// void Server::saveFile() {}

// void Server::handleClient(int clientSocket) {}
