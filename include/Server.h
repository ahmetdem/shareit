#include <iostream>
#include <vector>

class Server {
public:
  Server(int port);
  ~Server();

  void start();
  bool stop();

  bool acceptConn();

  // void receiveFile();
  // void saveFile();

private:
  int m_port;
  int m_serverSocket;

  void handleClient(int clientSocket);
};
