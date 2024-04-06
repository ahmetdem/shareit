#ifndef CLIENT_H
#define CLIENT_H

#include "./Global.h"
#include <filesystem>
#include <iostream>
#include <string>

class Client {
public:
  Client(const std::string &serverAddress, const std::string &name);
  ~Client();

  bool setupConn(int port, ConnectionType type);
  void closeConn();
  bool isConnected();

  void receiveClientsList();

  fs::path sendFile(int clientSocket, const fs::path &filePath);

  void sendFileToClient(int clientSocket, const fs::path &filePath);
  bool receiveFileFromServer();

private:
  std::string m_serverAddress;

  int m_port;
  int m_clientSocket;

  std::string m_uniqueName;
  bool m_isConnected;
};

#endif
