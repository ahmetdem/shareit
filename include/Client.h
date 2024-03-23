#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

class Client {
public:
  Client(const std::string &serverAddress);
  ~Client();

  bool setupConn(int port);
  void closeConn();
  bool isConnected();

  void sendFile(const fs::path& filePath);

private:
  std::string m_serverAddress;

  int m_port;
  int m_clientSocket;

  bool m_isConnected;
};

