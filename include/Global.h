#ifndef GLOBAL_H
#define GLOBAL_H

#include <filesystem>

namespace fs = std::filesystem;

const std::string EOF_MARKER = "##EOF##";

enum ConnectionType { Normal, FileTransfer };


#endif // !GLOBAL_H
