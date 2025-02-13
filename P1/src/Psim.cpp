#include <deque>
#include <functional>
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
const int INSTR_MEM_SIZE = 16;
const int REG_FILE_SIZE  = 8;
const int DAM_SIZE       = 8;

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

// format <Reg name, val>
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
class OpToken : public virtual Token {
protected:
  // inherited place1+2 are used as source1, source2
  int destination;
  string opcode;

public:
  virtual string printToken() override {
    return begining + opcode + ",R" + to_string(destination) + ",R" + to_string(place1) + ",R" + to_string(place2) + ">";
  }

  OpToken(string op, int dest, int src1, int src2) {
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
class LitOpToken : public OpToken, public virtual Token {
public:
  string printToken() override {
    return begining + opcode + "R" + to_string(destination) + "," + to_string(place1) + "," + to_string(place2) + ">";
  }

  LitOpToken(string op, int dest, int src1, int src2)
      : OpToken {op, dest, src1, src2} { }
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
  string name = "output node";

public:
  // input nodes need a name for displaying
  Node(string n) {
    input = nullptr;
    name  = n;
  }

  // output nodes do not need a name (and it's annoying to list them)
  Node(Node* in) {
    input = in;
    // name  = n;
  }

  // to prevent errors from tests
  Node(string n, Node* in) {
    input = in;
    name  = n;
  }

  // return true if the queue for the node is empty
  bool nodeIsEmpty() { return this->tokenQueue.empty(); }

  // return the front token of the queue for reading
  Token* peekFront() { return this->tokenQueue.front(); }

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

// #beginregion --- Transition Class Declaration ---

// recieves token from input class and outputs a token to output class
// takes a decider function that will convert the input token to an output token
// the decider function must return a Token* pointer if successfull
// or a Token* nullptr if not successfull
class Transition {
protected:
  bool canOperate = true;
  Token* input1;
  Token* output1;
  Node* nInput;
  Node* nOutput;
  function<Token*(Token*)> deciderFunction;

public:
  Transition(Node* in, Node* out, function<Token*(Token*)> f) {
    nInput          = in;
    nOutput         = out;
    input1          = nullptr;
    output1         = nullptr;
    deciderFunction = f;
  }

  // sink mode, will only consume and not output
  Transition(Node* in, function<Token*(Token*)> f) {
    nInput          = in;
    nOutput         = nullptr;
    input1          = nullptr;
    output1         = nullptr;
    deciderFunction = f;
  }

  void moveTokens() {
    // check if an output is ready and the transition can operate
    if ((output1 != nullptr) && (canOperate == true)) {

      // put the output token to the output node
      if (nOutput != nullptr) nOutput->pushToken(output1);

      // remove the input token from the input node
      input1 = nInput->popToken();

      // remove the input token
      // free(input1);
      // delete input1; //WARN: There will probably be memory leaks from this
      // Can't be bothered to make a class deconstructor
      // for all the tokens on a time crunch
      input1 = nullptr;

      // remove the reference to the output token
      output1 = nullptr;

      // mark the transition as operated
      canOperate = false;
    }
  }

  void setOutputToken(Token* target) { this->output1 = target; }

  void readToken() { input1 = nInput->peekFront(); }

  bool compute() {

    // don't start computaiton if it has already ran
    if (canOperate == false) { return false; }

    // get the next token from the front of the input queue if any
    if (nInput->nodeIsEmpty() == true) { return false; }
    readToken();

    // run the input function to get an output function if any
    setOutputToken(deciderFunction(input1));

    // skip running output if there was no output token
    // and if the transition is not a sink
    if ((output1 == nullptr) && (nOutput != nullptr)) { return false; }

    // trigger move tokens if an output token was found
    moveTokens();

    return true;
  }
};

// #endregion

// #beginregion --- Global Class Variables ---

deque<Node*> inputNodes;       // dynamic itterable queue of all input nodes
deque<Node*> outputNodes;      // dynamic itterable queue of all output nodes
Node INM("INM");               // instruction memory
REGToken* RGF[REG_FILE_SIZE];  // Register File
DAMToken* DAM[DAM_SIZE];       // Data Memory
deque<Transition> transitions; // dynamic itterable of all transitions

// #endregion

// #beginregion --- Global Function Declarations ---

// itterate over a generic array of tokens
string printTokenArray(Token** arry, int length) {
  string output;
  // itterate over array
  for (int i = 0; i < length; i++) {

    // skip if array spot is empty
    if (arry[i] == nullptr) continue;

    // add token output
    output.append(arry[i]->printToken());

    // skip the comma if at last member
    if (i + 1 == length) break;
    output.append(",");
  } // END FOR LOOP
  return output;
}

// append the current cycle to the fileOutput queue
void printCycle(int stepNumber) {

  // cycle header
  string output = "STEP" + to_string(stepNumber) + ":\n";

  // node itterator
  auto it = inputNodes.begin();

  // itterate over input nodes
  while (it != inputNodes.end()) {

    // concat all the input nodes as a string
    output.append((*it)->printTokenQueue());
    output.append("\n");

    // continue to next value
    it++;
  }

  // Add the RGF and DAM
  output.append("RGF:");
  output.append(printTokenArray((Token**) RGF, REG_FILE_SIZE));
  output.append("\n");

  output.append("DAM:");
  output.append(printTokenArray((Token**) DAM, DAM_SIZE));
  output.append("\n");

  // push onto the fileOutput queue
  fileOutput.push_back(output);
}

// #endregion

// takes a REG token as input and outputs an OP token
Token* computationTest(Token* DAMTokenInput) {
  // downcast from the base class to the member class
  REGToken* temp = dynamic_cast<REGToken*>(DAMTokenInput);

  // create a new token as an example (conversion)
  static OpToken output("MULT", temp->getReg(), temp->getVal(), 1);

  // up-cast from specific token to generic token
  return dynamic_cast<Token*>(&output);
}

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
  OpToken example2(string("ADD"), 3, 3, 2);
  LitOpToken example3(string("ADD"), 3, 3, 2);

  // cout << printToken(&example) << endl;
  // cout << printToken(&example1) << endl;
  // cout << printToken(&example2) << endl;
  // cout << printToken(&example3) << endl;

  // cout << "node test" << endl;

  Node Ainput("A Input");            // creat an input node
  Node Aoutput("A output", &Ainput); // put the input at the output node
  Aoutput.pushToken(&example1);
  Aoutput.pushToken(&example2);
  cout << Aoutput.printTokenQueue() << endl; // Aoutput --> Ainput

  cout << "commit top data" << endl;
  Aoutput.commit(); // move data
  cout << Ainput.printTokenQueue() << endl;
  cout << Aoutput.printTokenQueue() << endl;

  // transition test
  cout << "transfer via transition" << endl;
  // create a transition of Ainput --> transition --> Aoutput
  Transition exampleT(&Ainput, &Aoutput, &computationTest);

  // trigger the transition
  exampleT.compute();

  // display in terminal
  cout << Ainput.printTokenQueue() << endl;
  cout << Aoutput.printTokenQueue() << endl;

  return 0;
}

// #beginregion --- Transition computational functions ---
// these functions will be given to different transitions as objects
// each function will take a generic token expecting to be able
// to convert the token to a type of useable token
// if the function is able to process the token, it will return a pointer to an output token
// if the function cannot proccess a token, it will return a nullptr

// takes a OpToken from INM, checks the RGF, and
// returns a LitOpToken with the resolved values
Token* ReadAndDecode(Token* in) {
  // input token will be from the INM as an OpToken
  OpToken* input = dynamic_cast<OpToken*>(in);

  // get the operands from the token
  int source1 = input->getSrc1();
  int source2 = input->getSrc2();

  // check if the operands from RGF are available
  bool src1Avail = (RGF[source1] != nullptr);
  bool src2Avail = (RGF[source2] != nullptr);
  bool RGFAvail  = (src1Avail && src2Avail);

  // exit if the RGF is not available
  if (!RGFAvail) return nullptr;

  // get the values from RGF
  int val1 = RGF[source1]->getVal();
  int val2 = RGF[source2]->getVal();

  // create the output token
  LitOpToken* output = new LitOpToken(input->getOpcode(), input->getDest(), val1, val2);

  // return the pointer to the output token
  return output;
} // END ReadAndDecode()

// checks the LitOpToken from the INB
// returns the same token to the AIB if it's not a LD instr
Token* Issue1(Token* in) {
  // input token will be from the INB as a LitOpToken
  LitOpToken* input = dynamic_cast<LitOpToken*>(in);

  // check if the op is a load instruction
  bool isLoad = (input->getOpcode() == string("LD"));

  // stop if the token is a LD instruction
  if (isLoad) return nullptr;

  // move the input otherwise
  else return input;
} // END Issue1()

// checks the LitOpToken from the INB
// returns the same token to the LIB if it is a LD instr
Token* Issue2(Token* in) {
  // input token will be from the INB as a LitOpToken
  LitOpToken* input = dynamic_cast<LitOpToken*>(in);

  // check if the op is a load instruction
  bool isLoad = (input->getOpcode() == string("LD"));

  // stop if the token is a LD instruction
  if (isLoad) return input;

  // move the input otherwise
  else return nullptr;
} // END Issue2()

// helper for the string switch case
enum class StringCode { ADD, SUB, AND, OR, LD, UNKOWN };

// turn the strings into something useable for a switch case statment
StringCode hashString(const string& str) {
  if (str == string("ADD")) return StringCode::ADD;
  if (str == string("SUB")) return StringCode::SUB;
  if (str == string("AND")) return StringCode::AND;
  if (str == string("OR")) return StringCode::OR;
  if (str == string("LD")) return StringCode::LD;
  return StringCode::UNKOWN;
}

// used for both the ALU and ADDR
// takes a LitOpToken and turns
// it into a REGToken
Token* LogicUnit(Token* in) {
  // input token will be from the AIB or LIB as a LitOpToken
  LitOpToken* input = dynamic_cast<LitOpToken*>(in);
  int arg1          = input->getSrc1();
  int arg2          = input->getSrc2();
  int result        = 0;

  switch (hashString(input->getOpcode())) {
  // ALU cases
  case StringCode::ADD:
    result = arg1 + arg2;
    return new REGToken(input->getDest(), result);
  case StringCode::SUB:
    result = arg1 - arg2;
    return new REGToken(input->getDest(), result);
  case StringCode::AND:
    result = arg1 && arg2;
    return new REGToken(input->getDest(), result);
  case StringCode::OR:
    result = arg1 || arg2;
    return new REGToken(input->getDest(), result);
  case StringCode::LD: // ADDR case
    result = arg1 + arg2;
    return new REGToken(input->getDest(), result);
  case StringCode::UNKOWN: // exception handling
    return nullptr;
  }

  return nullptr;
} // END LogicUnit()

// takes a REGToken from the ADB and returns
// a REGToken with the resolved memory value
// from the DAM
Token* Loader(Token* in) {
  // Expects a REGToken as input from ADB
  REGToken* input = dynamic_cast<REGToken*>(in);

  // gets the data from the index in memory
  int result = DAM[input->getVal()]->getVal();

  // create a new token with the result
  REGToken* output = new REGToken(input->getReg(), result);

  // return the output token
  return output;
}

// takes a REGToken and writes it to the RGF
Token* Writer(Token* in) { }

// #endregion

// #beginregion --- Class Initialization functions ---

void initHardware() {
  // --- init Nodes ---
  static Node INBin("INB");
  static Node INBout(&INBin);

  static Node AIBin("AIB");
  static Node AIBout(&AIBin);

  static Node LIBin("LIB");
  static Node LIBout(&LIBin);

  static Node ADBin("ADB");
  static Node ADBout(&ADBin);

  static Node REBin("REB");
  static Node REBout(&REBin);

  // add nodes to input queue
  inputNodes.push_back(&INM); // global instruction queue
  inputNodes.push_back(&INBin);
  inputNodes.push_back(&AIBin);
  inputNodes.push_back(&LIBin);
  inputNodes.push_back(&ADBin);
  inputNodes.push_back(&REBin);

  // add nodes output queue
  outputNodes.push_back(&INBout);
  outputNodes.push_back(&AIBout);
  outputNodes.push_back(&LIBout);
  outputNodes.push_back(&ADBout);
  outputNodes.push_back(&REBin);

  // init transitions
}

// #endregion
