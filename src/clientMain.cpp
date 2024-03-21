#include "../include/Client.h"

int main() {
  // Create client object
  Client client("127.0.0.1", 8080); // Server address and port
  // Set up connection
  if (!client.setupConn()) {
    std::cerr << "Error: Failed to set up connection\n";
    return 1;
  }
  std::cout << "Connection established\n";
  // Close connection
  client.closeConn();
  return 0;
}
