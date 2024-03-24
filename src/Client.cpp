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

fs::path Client::sendFile(int clientSocket, const fs::path &filePath) {
  // Open the file
  std::ifstream file(filePath, std::ios::binary);
  if (!file) {
    std::cerr << "Error: Failed to open file: " << filePath << std::endl;
    return "";
  }

  // Read file contents into buffer
  std::ostringstream buffer;
  buffer << file.rdbuf();
  std::string fileData = buffer.str();

  // Construct message with filename and file data
  std::ostringstream message;
  message << clientSocket << "\n";
  message << filePath.filename().string() << '\n';
  message << fileData << EOF_MARKER;

  // Send message to the server
  ssize_t bytesSent = send(this->m_clientSocket, message.str().c_str(),
                           message.str().size(), 0);
  if (bytesSent == -1) {
    std::cerr << "Error: Failed to send file\n";
    return "";
  }

  file.close();
  std::cout << "File Sent Successfully\n";

  // Assuming the server saves files in a directory named "client_files"
  // Construct the path where the file should be saved on the server
  fs::path serverFilePath = "client_files" / filePath.filename();

  return serverFilePath;
}

bool Client::receiveFileFromServer() {
  const int bufferSize = 1024; // Buffer size for receiving data
  char buffer[bufferSize];

  // Receive file data in a loop, handling potential partial reads
  std::string fileData;
  while (true) {
    ssize_t bytesRead = recv(m_clientSocket, buffer, bufferSize, 0);

    if (bytesRead > 0) {
      fileData.append(buffer, bytesRead);
    } else if (bytesRead == 0) {
      // Connection closed by the server, but EOF marker not received
      std::cerr << "Error: Connection closed unexpectedly\n";
      return false;
    } else {
      // Error occurred during receive
      std::cerr << "Error: Failed to receive data\n";
      return false;
    }

    // Check if the EOF marker is found
    if (fileData.find(EOF_MARKER) != std::string::npos) {
      // Extract client socket and filename
      size_t pos = fileData.find('\n');
      if (pos == std::string::npos) {
        std::cerr << "Error: Invalid data format\n";
        return false;
      }
      fileData = fileData.substr(pos + 1); // Remove the client socket part

      // Extract filename from the beginning of the file data
      pos = fileData.find('\n');
      if (pos == std::string::npos) {
        std::cerr << "Error: Invalid data format\n";
        return false;
      }
      std::string filename = fileData.substr(0, pos);
      fileData = fileData.substr(pos + 1); // Get remaining file data

      // Save the file to the "received_client_files" directory
      std::string directoryName = "received_client_files";

      // Create the directory if it doesn't exist
      if (!std::filesystem::exists(directoryName)) {
        std::filesystem::create_directory(directoryName);
      }

      std::string filePath = directoryName + "/" + filename;
      std::ofstream outputFile(filePath, std::ios::binary);
      if (!outputFile.is_open()) {
        std::cerr << "Error: Failed to create output file: " << filePath
                  << std::endl;
        return false;
      }

      // Write file data to file
      outputFile << fileData;

      outputFile.close(); // Ensure file is closed
      std::cout << "File received successfully: " << filePath << std::endl;

      return true;
    }
  }

  return true; // Not reached, added to suppress compiler warning
}

void Client::receiveClientsList() {
  char buffer[1024]; // Assuming a maximum buffer size
  ssize_t bytesReceived = recv(m_clientSocket, buffer, sizeof(buffer), 0);
  if (bytesReceived <= 0) {
    std::cerr << "Error: Failed to receive client list from server\n";
    return;
  }

  // Parse the received data into individual client names and sockets
  std::istringstream iss(std::string(buffer, bytesReceived));
  std::string clientInfo;
  while (std::getline(iss, clientInfo, '\n')) {
    // Split the client info into name and socket
    size_t colonPos = clientInfo.find(':');
    if (colonPos != std::string::npos) {
      std::string clientName = clientInfo.substr(0, colonPos);
      int clientSocket = std::stoi(clientInfo.substr(colonPos + 1));
      // Display client name and socket
      std::cout << "Client: " << clientName << ", Socket: " << clientSocket
                << std::endl;
    }
  }
}

void Client::closeConn() {
  if (m_clientSocket != -1) {
    close(m_clientSocket);

    this->m_clientSocket = -1;
    this->m_isConnected = false;
  }
}
