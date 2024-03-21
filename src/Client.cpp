#include "../include/Client.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

Client::Client(const std::string &serverAdress, int port)
    : m_serverAddress(serverAdress), m_port(port), m_clientSocket(0) {}

Client::~Client() { this->closeConn(); }

bool Client::setupConn() {
  // Create socket
  m_clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (m_clientSocket == -1) {
    std::cerr << "Error: Failed to create socket\n";
    return false;
  }

  // Set up server address structure
  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(m_port);
  if (inet_pton(AF_INET, m_serverAddress.c_str(), &serverAddr.sin_addr) <= 0) {
    std::cerr << "Error: Invalid address\n";
    return false;
  }

  // Connect to server
  if (connect(m_clientSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) ==
      -1) {
    std::cerr << "Error: Failed to connect to server\n";
    close(m_clientSocket);
    return false;
  }

  return true;
}

void Client::closeConn() {
  if (m_clientSocket != -1) {
    close(m_clientSocket);
    m_clientSocket = -1;
  }
}
