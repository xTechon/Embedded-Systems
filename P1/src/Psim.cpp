#include <cstddef>
#include <deque>
#include <iostream>
#include <ostream>
#include <queue>
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

// #beginregion --- Token Class Dclarations ---

// default format <int, int>
class Token {
protected:
  int place1, place2;
  string begining = "<";

public:
  virtual string printToken() { return begining + to_string(place1) + "," + to_string(place2) + ">"; };
}; // END CLASS

// format <addr, val>
class DAMToken : public virtual Token {
public:
  DAMToken(int addr, int val) {
    place1 = addr;
    place2 = val;
  }

  int getAddr() { return place1; }

  int getVal() { return place2; }
}; // END CLASS

class REGToken : public virtual Token {
public:
  REGToken(int reg, int val) {
    place1   = reg;
    place2   = val;
    begining = "<R";
  }

  virtual string printToken() { return begining + to_string(place1) + "," + to_string(place2) + ">"; };

  int getReg() { return place1; }

  int getVal() { return place2; }
};

// format <OPCODE, Dest Reg, Src Reg, Src Reg>
class opToken : public virtual Token {
protected:
  // inherited place1+2 are used as source1, source2
  int destination;
  string opcode;

public:
  virtual string printToken() override {
    return begining + opcode + "R" + to_string(destination) + ",R" + to_string(place1) + ",R" + to_string(place2) + ">";
  }

  opToken(string op, int dest, int src1, int src2) {
    opcode      = op;
    destination = dest;
    place1      = src1;
    place2      = src2;
  }

  string getOpcode() { return opcode; }

  int getDest() { return destination; }

  int getSrc1() { return place1; }

  int getSrc2() { return place2; }
}; // END CLASS

// format <OPCODE, Dest Reg, VALUE, VALUE>
class litOpToken : opToken, public virtual Token {
public:
  string printToken() override {
    return begining + opcode + "R" + to_string(destination) + "," + to_string(place1) + "," + to_string(place2) + ">";
  }

  litOpToken(string op, int dest, int src1, int src2)
      : opToken {op, dest, src1, src2} { }
}; // END CLASS

// #endregion

// #beginregion --- Node Class Declaration ---

// input node --consumed by--> transition  || Phase 1
// transitions --write to--> output node   || Phase 1
// output node --commit to--> input node   || Phase 2
class Node {
public:
  queue<Token*> tokenQueue;
  Node* input;

  Node() { input = nullptr; }

  Node(Node* in) { input = in; }

  // push the top of the queue to the input queue
  // as long as the queue is not empty
  void commit() {
    if (!(this->tokenQueue.empty()) && (input != nullptr)) {
      Token* temp = this->tokenQueue.front();
      input->tokenQueue.push(temp);
      this->tokenQueue.pop(); // remove element from this queue
    }
  };
};

// #endregion

// example usage of inheritance
string printToken(Token* foo) { return foo->printToken(); }

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

  // example usage of inheritance
  DAMToken example(5, 10);
  REGToken example1(5, 10);
  opToken example2(string("ADD"), 3, 3, 2);
  litOpToken example3(string("ADD"), 3, 3, 2);

  cout << printToken(&example) << endl;
  cout << printToken(&example1) << endl;
  cout << printToken(&example2) << endl;
  cout << printToken(&example3) << endl;

  cout << "node test" << endl;

  Node Ainput;           // creat an input node
  Node Aoutput(&Ainput); // put the input at the output node
  Aoutput.tokenQueue.push(&example2);
  cout << "A output: " << Aoutput.tokenQueue.front()->printToken() << endl;

  Aoutput.commit(); // move data
  cout << "A input: " << Ainput.tokenQueue.front()->printToken() << endl;

  return 0;
}