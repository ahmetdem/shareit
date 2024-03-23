#include <iostream>
#include <string>
#include <vector>

class Server {
public:
  Server(int port);
  ~Server();

  void start();
  bool stop();

  bool acceptConn();
  void receiveFile(int clientSocket);

private:
  int m_port;
  int m_serverSocket;

  void handleClient(int clientSocket);
};
