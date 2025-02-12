#include <deque>
#include <iostream>
#include <ostream>
#include <string>

// can only use std on this project, so it's okay to use this
// would not use this under normal circumstances
using namespace std;

// #beginregion --- Global Variables ---

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
private:
  deque<Token*> tokenQueue;
  Node* input;
  string name = "unnamed";

public:
  Node(string n) {
    input = nullptr;
    name  = n;
  }

  Node(string n, Node* in) {
    input = in;
    name  = n;
  }

  // insert a token to the beginning of the queue
  void pushToken(Token* temp) { this->tokenQueue.push_back(temp); }

  // return the token from the front of the queue and pop it
  Token* popToken() {
    static Token* temp = this->tokenQueue.front();
    this->tokenQueue.pop_front();
    return temp;
  }

  string printFrontToken() { return this->tokenQueue.front()->printToken(); }

  string printTokenQueue() {
    // start the output with the [name]: of the node
    string output = name + ":";

    // queue itterator
    auto it = this->tokenQueue.begin();

    // itterate over entire queue
    while (it != this->tokenQueue.end()) {

      // add the token to the line
      output.append((*it)->printToken());

      // only add commas if this token is not the last one
      if ((it + 1 != this->tokenQueue.end())) output.append(",");

      // continue to next value
      it++;
    }
    return output;
  }

  // push the top of the queue to the input queue
  // as long as the queue is not empty
  void commit() {
    if (!(this->tokenQueue.empty()) && (input != nullptr)) {
      // Token* temp = this->tokenQueue.front();
      input->pushToken(this->popToken());
      // this->tokenQueue.pop_front(); // remove element from this queue
    }
  }
};

// #endregion

// #beginregion -- Global Class Variables

deque<Node> inputNodes;  // itterable queue of all input nodes
deque<Node> outputNodes; // itterable queue of all output nodes
Token* INM[16];          // instruction memory
Token* RGF[8];           // Register File
Token* DAM[8];           // Data Memory

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

  // cout << printToken(&example) << endl;
  // cout << printToken(&example1) << endl;
  // cout << printToken(&example2) << endl;
  // cout << printToken(&example3) << endl;

  // cout << "node test" << endl;

  Node Ainput("A Input");            // creat an input node
  Node Aoutput("A output", &Ainput); // put the input at the output node
  Aoutput.pushToken(&example1);
  Aoutput.pushToken(&example2);
  cout << Aoutput.printTokenQueue() << endl;

  cout << "commit top data" << endl;
  Aoutput.commit(); // move data
  cout << Ainput.printTokenQueue() << endl;
  cout << Aoutput.printTokenQueue() << endl;

  return 0;
}