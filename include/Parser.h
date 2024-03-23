#ifndef PARSER_H
#define PARSER_H

#include <functional>
#include <iostream>
#include <vector>

class Parser {
public:
  // Define a structure to represent a option to parse.
  struct Option {
    const std::string name;
    const std::string description;
    std::function<void()> function;

    // Constructor overload for functions without arguments
    Option(const std::string &n, const std::string &desc,
           std::function<void()> func)
        : name(n), description(desc), function(func) {}
  };

  /* Option to add custom functions. */
  void add_custom_option(const Option &option) { options.push_back(option); }

  // Method to parse command-line arguments and execute associated functions
  void parse(const int argc, const char *argv[]) {
    bool parsed = false;

    for (int i = 1; i < argc; ++i) {
      for (const Option &op : options) {
        if (argv[i] == op.name) {
          op.function();
          parsed = true;
        }
      }
    }

    if (!parsed) {
      this->help();
    }
  }

  void help() {
    std::cout << "You did something wrong, the commands you can use are: \n\n";

    for (const Option &op : options) {
      std::cout << op.name << ": " << op.description << "\n";
    }
  }

private:
  Option OptionFromName(const char *name) {
    for (Option op : options) {
      if (op.name == name) {
        return op;
      }
    }
    throw std::runtime_error("Option not found");
  }

  std::string stripLastWord(const std::string &path) {

    int index{0};
    for (size_t i = path.size() - 1; i > 0; i--) {
      if (path[i] == '/') {
        index = i;
        break;
      }
    }
    return path.substr(index, path.size());
  }

  std::vector<Option> options{};
};

#endif // PARSER_H
