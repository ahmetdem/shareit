#include <iostream>

class Client {
public:
  Client(const std::string &serverAddress, int port);
  ~Client();

  bool setupConn();
  void closeConn();

  // void readFile();
  // void sendFile();

private:
  std::string m_serverAddress;

  int m_port;
  int m_clientSocket;
};

