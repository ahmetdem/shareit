#ifndef CLIENT_H
#define CLIENT_H

#include <filesystem>
#include <iostream>
#include <string>
#include "./Global.h"

namespace fs = std::filesystem;

class Client {
public:
  Client(const std::string &serverAddress, const std::string &name);
  ~Client();

  bool setupConn(int port, ConnectionType type);
  void closeConn();
  bool isConnected();

  void sendFile(const fs::path &filePath);

private:
  std::string m_serverAddress;

  int m_port;
  int m_clientSocket;

  std::string m_uniqueName;
  bool m_isConnected;
};

#endif
