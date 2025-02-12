#include <iostream>
#include <string>

// Global variables

std::string InstructionsPath = "instructions.txt";
std::string RegistersPath    = "registers.txt";
std::string DataMemoryPath   = "datamemory.txt";
std::string OutputFilePath   = "simulation.txt";

// argc is # of arguments including program execution
// argv is the array of strings of every argument including execution
int main(int argc, char* argv[]) {
  std::cout << "Hello World!" << std::endl;

  // change path locations if arguments set
  if (argc >= 4) {
    InstructionsPath = argv[1];
    RegistersPath    = argv[2];
    DataMemoryPath   = argv[3];
  }
  // optionally override output filepath if set
  if (argc == 5) {
    OutputFilePath = argv[4];
  }
    std::cout << InstructionsPath << std::endl;
    std::cout << RegistersPath << std::endl;
    std::cout << DataMemoryPath << std::endl;
    std::cout << OutputFilePath << std::endl;

  return 0;
}