#include "../include/Server.h"
#include "../include/Global.h"
#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
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

  // Add client to the map
  addClient(clientName, clientSocket);

  // Handle client
  handleClient(clientSocket, clientIp, clientName, connectionType);

  this->getClients();

  return true;
}

void Server::receiveFile(int clientSocket) {
  const int bufferSize = 1024; // Buffer size for receiving data
  char buffer[bufferSize];

  // Receive file data in a loop, handling potential partial reads
  std::string fileData;
  while (true) {
    ssize_t bytesRead = recv(clientSocket, buffer, bufferSize, 0);
    std::cout << "Server: " << bytesRead << "\n";

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

  // Parse client socket and filename from the received data
  size_t pos = fileData.find('\n');
  if (pos == std::string::npos) {
    std::cerr << "Error: Invalid data format\n";
    return;
  }

  std::string clientSocketStr = fileData.substr(0, pos);
  [[maybe_unused]] int clientSocketOfReceiver = std::stoi(clientSocketStr);

  fileData = fileData.substr(pos + 1); // Remove the client socket part

  // Extract filename from the beginning of the file data
  pos = fileData.find('\n');
  if (pos == std::string::npos) {
    std::cerr << "Error: Invalid data format\n";
    return;
  }

  std::string filename = fileData.substr(0, pos);
  fileData = fileData.substr(pos + 1); // Get remaining file data

  std::ofstream outputFile(filename, std::ios::binary);
  if (!outputFile.is_open()) {
    std::cerr << "Error: Failed to create output file: " << filename
              << std::endl;
    // Additional debugging output
    std::cerr << "Current working directory: "
              << std::filesystem::current_path() << std::endl;
    return;
  }

  // Write file data to file
  outputFile << fileData;

  outputFile.close(); // Ensure file is closed
  std::cout << "File received successfully: " << filename << std::endl;

  this->removeClient(clientSocket);

  // Call the function to send the file to the client with the given socket
  this->sendFileToClient(clientSocketOfReceiver, filename);
}

bool Server::sendFileToClient(const int clientSocket,
                              const fs::path &filePath) {
  // Open the file
  std::ifstream file(filePath, std::ios::binary);
  if (!file) {
    std::cerr << "Error: Failed to open file: " << filePath << std::endl;
    return false;
  }

  // Read file contents into buffer
  std::ostringstream buffer;
  buffer << file.rdbuf();
  std::string fileData = buffer.str();

  // Construct message with filename, client socket, and file data
  std::ostringstream message;
  message << filePath.filename().string() << '\n';
  message << fileData << EOF_MARKER;

  // Send message to the client
  ssize_t bytesSent =
      send(clientSocket, message.str().c_str(), message.str().size(), 0);
  if (bytesSent == -1) {
    std::cerr << "Error: Failed to send file\n";
    return false;
  }

  file.close();
  std::cout << "File Sent Successfully to client: " << clientSocket << "\n";

  this->removeClient(clientSocket);
  return true;
}

void Server::addClient(const std::string &name, int clientSocket) {
  connectedClients[name] = clientSocket;
}

void Server::removeClient(const int clientSocket) {
  for (auto it = connectedClients.begin(); it != connectedClients.end(); ++it) {
    if (it->second == clientSocket) {
      connectedClients.erase(it);
      return;
    }
  }
}

int Server::getClientSocket(const std::string &name) {
  auto it = connectedClients.find(name);
  if (it != connectedClients.end()) {
    return it->second; // Return the client socket
  }
  return -1; // Client not found
}

void Server::getClients() {
  for (const auto &pair : connectedClients) {
    std::cout << pair.first << "\n";
  }
}

void Server::sendClientsToClient(const int clientSocket) {
  // Serialize the list of client names and sockets into a string
  std::ostringstream clientList;
  for (const auto &pair : connectedClients) {
    clientList << pair.first << ":" << pair.second << "\n";
  }
  std::string serializedClientList = clientList.str();

  // Send the serialized list to the client
  ssize_t bytesSent = send(clientSocket, serializedClientList.c_str(),
                           serializedClientList.size(), 0);
  if (bytesSent == -1) {
    std::cerr << "Error: Failed to send client list to client\n";
    return;
  }
}

void Server::handleClient(int clientSocket, const char clientIp[],
                          const std::string &clientName,
                          ConnectionType connType) {
  std::cout << "Client '" << clientName
            << "' connected with connection type: " << connType
            << " with ip: " << clientIp << std::endl;

  // Create a new thread to handle this client
  std::thread clientThread([this, clientSocket, clientName, connType]() {
    switch (connType) {
    case ConnectionType::Normal:
      // Handle normal connection
      break;

    case ConnectionType::FileTransfer:
      // Handle file transfer
      this->sendClientsToClient(clientSocket);
      this->receiveFile(clientSocket);
      break;

    default:
      std::cout << "Error: Unknown Operation." << std::endl;
    }
  });

  // Detach the thread so that it can run independently
  clientThread.detach();
}
