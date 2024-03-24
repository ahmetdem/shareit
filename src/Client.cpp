#include "../include/Client.h"
#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

const std::string EOF_MARKER = "##EOF##";

Client::Client(const std::string &serverAdress, const std::string &name)
    : m_serverAddress(serverAdress), m_port(0), m_clientSocket(0),
      m_uniqueName(name) {}

Client::~Client() { this->closeConn(); }

bool Client::setupConn(int port, ConnectionType type) {
  this->m_port = port;

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

  // Send client name and connection type to server
  std::string message = m_uniqueName + ":" + std::to_string(type);
  ssize_t bytesSent = send(m_clientSocket, message.c_str(), message.size(), 0);
  if (bytesSent < 0) {
    std::cerr
        << "Error: Failed to send client name and connection type to server."
        << std::endl;
    close(m_clientSocket);
    return false;
  }
  this->m_isConnected = true;
  return true;
}

bool Client::isConnected() { return this->m_isConnected; }

void Client::sendFile(const fs::path &filePath) {
  // Open the file
  std::ifstream file(filePath, std::ios::binary);
  if (!file) {
    std::cerr << "Error: Failed to open file: " << filePath << std::endl;
    return;
  }

  // Read file contents into buffer
  std::ostringstream buffer;
  buffer << file.rdbuf();
  std::string fileData = buffer.str();

  // Construct message with filename and file data
  std::ostringstream message;
  message << filePath.filename().string() << '\n';
  message << fileData << EOF_MARKER;

  // Send message to the server
  ssize_t bytesSent = send(this->m_clientSocket, message.str().c_str(),
                           message.str().size(), 0);
  if (bytesSent == -1) {
    std::cerr << "Error: Failed to send file\n";
    return;
  }

  file.close();
  std::cout << "File Sent Successfully\n";
}

void Client::closeConn() {
  if (m_clientSocket != -1) {
    close(m_clientSocket);
    this->m_clientSocket = -1;
    this->m_isConnected = false;
  }
}
