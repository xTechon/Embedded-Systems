#include <deque>
#include <iostream>
#include <ostream>
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
public:
  virtual string printToken() { return string("<") + to_string(place1) + "," + to_string(place2) + ">"; };
}; // END CLASS



// format <addr, val>
class DAMToken : public virtual Token {
  public:
  DAMToken(int addr, int val) {
    place1 = addr;
    place2 = val;
  }
}; // END CLASS
    


// format <OPCODE, Dest Reg, Src Reg, Src Reg>
class opToken : public virtual Token {
protected:
  // inherited place1+2 are used as source1, source2
  int destination;
  string opcode;

public:
  virtual string printToken() override {
    return string("<") + opcode + "R" + to_string(destination) + ",R" + to_string(place1) + ",R" + to_string(place2) + ">";
  }
  opToken(string op, int dest, int src1, int src2) {
    opcode = op;
    destination = dest;
    place1 = src1;
    place2 = src2;
  }
  string getOpcode() {return opcode;}
  int getDest() {return destination;}
  int getSrc1(){return place1;}
  int getSrc2(){return place2;}
}; // END CLASS



//format <OPCODE, Dest Reg, VALUE, VALUE>
class litteralOpToken : opToken, public virtual Token {
public:
  string printToken() override {
    return string("<") + opcode + "R" + to_string(destination) + "," + to_string(place1) + "," + to_string(place2) + ">";
  }
  litteralOpToken(string op, int dest, int src1, int src2) : opToken{op, dest, src1, src2} {}
}; // END CLASS

// #endregion

// example usage of inheritance
string printToken(Token* foo) {
  return foo->printToken(); 
}

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
  DAMToken example(5,10);
  opToken example2(string("ADD"), 3,3,2);
  litteralOpToken example3(string("ADD"), 3,3,2);
  
  cout << printToken(&example) << endl;
  cout << printToken(&example2) << endl;
  cout << printToken(&example3) << endl;


  return 0;
}