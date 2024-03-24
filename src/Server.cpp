#include "../include/Server.h"
#include "../include/Global.h"
#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
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
  struct sockaddr_in clientAddr;
  int clientSocket;
  socklen_t clientAddrLen = sizeof(clientAddr);

  // Accept connection
  clientSocket =
      accept(m_serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
  if (clientSocket < 0) {
    std::cerr << "Error: Failed to accept connection." << std::endl;
    return false;
  }

  std::string clientName;
  ConnectionType connectionType;
  // Receive client name and connection type from client
  char buffer[1024];
  ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
  if (bytesReceived < 0) {
    std::cerr << "Error: Failed to receive client name and connection type."
              << std::endl;
  } else {
    buffer[bytesReceived] = '\0';
    std::string dataReceived(buffer);

    // Parse client name and connection type from received data
    size_t pos = dataReceived.find(':');
    clientName = dataReceived.substr(0, pos);
    std::string connectionTypeStr = dataReceived.substr(pos + 1);
    connectionType = static_cast<ConnectionType>(std::stoi(connectionTypeStr));
  }

  // Get client IP address
  char clientIp[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIp, INET_ADDRSTRLEN);

  // Handle client
  handleClient(clientSocket, clientIp, clientName, connectionType);

  return true;
}

void Server::receiveFile(int clientSocket) {
  const int bufferSize = 1024; // Buffer size for receiving data
  char buffer[bufferSize];

  // Receive file data in a loop, handling potential partial reads
  std::string fileData;
  while (true) {
    ssize_t bytesRead = recv(clientSocket, buffer, bufferSize, 0);

    if (bytesRead == 0) {

      if (fileData.find("EOF") != std::string::npos) {
        fileData.erase(fileData.find("EOF"), 3); // Remove "EOF" marker
        break;
      } else {
        std::cerr
            << "Warning: Unexpected connection closure without EOF marker\n";
        break;
      }
    } else if (bytesRead < 0) {
      std::cerr << "Error: Failed to receive data\n";
      return;
    }

    fileData += std::string(buffer, bytesRead);
  }

  // Extract filename from the beginning of the file data
  std::string filename = fileData.substr(0, fileData.find('\n'));
  fileData = fileData.substr(filename.length() + 1); // Get remaining file data

  // Open file for writing
  std::ofstream outputFile(filename, std::ios::binary);
  if (!outputFile.is_open()) {
    std::cerr << "Error: Failed to create output file: " << filename
              << std::endl;
    return;
  }

  // Write file data to file
  outputFile << fileData;

  outputFile.close(); // Ensure file is closed
  std::cout << "File received successfully: " << filename << std::endl;
}

void Server::handleClient(int clientSocket, char clientIp[],
                          std::string clientName, ConnectionType connType) {

  std::cout << "Client '" << clientName
            << "' connected with connection type: " << connType
            << " with ip: " << clientIp << std::endl;

  switch (connType) {
  case ConnectionType::Normal:
    break;

  case ConnectionType::FileTransfer:
    this->receiveFile(clientSocket);
    break;

  default:
    std::cout << "Error: Unknown Operation." << std::endl;
  }

  this->stop();
}
