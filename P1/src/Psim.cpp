#include <deque>
#include <iostream>
#include <string>

// can only use std on this project, so it's okay to use this
// would not use this under normal circumstances
using namespace std;

// #beginregion --- Global variables ---

string InstructionsPath = "instructions.txt";
string RegistersPath    = "registers.txt";
string DataMemoryPath   = "datamemory.txt";
string OutputFilePath   = "simulation.txt";
deque<string> fileOutput; // contains the cycles for the file output

// #endregion

// argc is # of arguments including program execution
// argv is the array of strings of every argument including execution
int main(int argc, char* argv[]) {
  cout << "Hello World!" << endl;

  // change path locations if arguments set
  if (argc >= 4) {
    InstructionsPath = argv[1];
    RegistersPath    = argv[2];
    DataMemoryPath   = argv[3];
  }
  // optionally override output filepath if set
  if (argc == 5) { OutputFilePath = argv[4]; }

  return 0;
}

// #beginregion --- Class Declarations ---

class Token {
public:
  int place1, place2;
  virtual string printToken() { return string("<") + to_string(place1) + "," + to_string(place2) + ">"; };
};

class opToken : virtual Token {
public:
  // inherited place1+2 are used as source1, source2
  int destination;
  string opcode;
};

/*
class Node {
  public:
    deque<>
};*/

// #endregion