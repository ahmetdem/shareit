#include "../include/Server.h"

int main() {
  // Create server object
  Server server(8080); // Port number
  // Start server
  server.start();
  // Accept connections
  if (!server.acceptConn()) {
    std::cerr << "Error: Failed to accept connection\n";
    return 1;
  }

  server.stop();

  return 0;
}
