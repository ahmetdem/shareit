#include "../include/Server.h"

int main() {
  // Create server object
  Server server(8000); // Port number
  // Start server
  server.start();

  while (true) {
    // Accept connections
    if (!server.acceptConn()) {
      std::cerr << "Error: Failed to accept connection\n";
      break; // Exit the loop if an error occurs
    }
  }

  server.stop();

  return 0;
}
