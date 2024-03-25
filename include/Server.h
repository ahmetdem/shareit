#ifndef SERVER_H
#define SERVER_H

#include "Global.h"
#include <iostream>
#include <map>
#include <string>

class Server {
public:
  Server(int port);
  ~Server();

  void start();
  bool stop();

  bool acceptConn();

  void receiveFile(int clientSocket);
  bool sendFileToClient(int clientSocket, const fs::path &filePath);

  void addClient(const std::string &name, int clientSocket);
  void removeClient(const int clientSocket);
  int getClientSocket(const std::string &name);

  void getClients();
  void sendClientsToClient(int clientSocket);

private:
  int m_port;
  int m_serverSocket;

  std::map<std::string, int> connectedClients;

  void handleClient(int clientSocket, const char clientIp[],
                    const std::string &name, ConnectionType connType);
};

#endif // !SERVER_H
