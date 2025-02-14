#include <deque>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <ostream>
#include <regex>
#include <string>
#include <vector>

// can only use std on this project, so it's okay to use this
// would not use this under normal circumstances
using namespace std;

// TODO: Step 5 has the order of R4 and R5 Reversed
// TODO: the function of AND is ambiguous
// TODO: the function of OR is ambiguous
// TODO: Extra empty cycle at the end of the simulation

// #beginregion --- Global Variables ---

string InstructionsPath = "instructions.txt";
string RegistersPath    = "registers.txt";
string DataMemoryPath   = "datamemory.txt";
string OutputFilePath   = "simulation.txt";
vector<string> fileOutput; // contains the cycles for the file output
const int INSTR_MEM_SIZE = 16;
const int REG_FILE_SIZE  = 8;
const int DAM_SIZE       = 8;

// #endregion

// #beginregion --- Main Function Declarations ---

void initHardware();
void Simulator();
void parseInstructionFile(string, string);
void parseRegisterFile(string, string);
void parseDataMemoryFile(string, string);
string printCycle(int);
void outputFile(string);

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

  // load data from the files
  parseInstructionFile(InstructionsPath, "[<>]|(,R)");
  parseRegisterFile(RegistersPath, "(<R)|,|>");
  parseDataMemoryFile(DataMemoryPath, "[<>]|,");

  // initalize the hardware
  initHardware();

  cout << printCycle(0) << endl;

  // run the simulation
  Simulator();
  
  // output the file
  outputFile(OutputFilePath);

  return 0;
}

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
class DAMToken : public Token {
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
    return begining + opcode + ",R" + to_string(destination) + "," + to_string(place1) + "," + to_string(place2) + ">";
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
    Token* temp = this->tokenQueue.front();
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
    // make sure there is an input to push to
    if (input != nullptr) {
      // flush everything from this output node
      while (!(this->tokenQueue.empty())) {

        // remove an element from this queue
        input->pushToken(this->popToken());
      }
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
    if (canOperate == true) {

      // put the output token to the output node
      if ((output1 != nullptr) && (nOutput != nullptr)) nOutput->pushToken(output1);

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

  bool canMove() { return canOperate; }

  void reset() { canOperate = true; }
};

// #endregion

// #beginregion --- Global Class Variables ---

deque<Node*> inputNodes;        // dynamic itterable queue of all input nodes
deque<Node*> outputNodes;       // dynamic itterable queue of all output nodes
Node INM("INM");                // instruction memory
REGToken* RGF[REG_FILE_SIZE];   // Register File
DAMToken* DAM[DAM_SIZE];        // Data Memory
deque<Transition*> transitions; // dynamic itterable of all transitions

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
string printCycle(int stepNumber) {

  // cycle header
  string output = "STEP " + to_string(stepNumber) + ":\n";

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

  return output;
}

// generic token parsers as a helper function for the more specifc token generators
vector<vector<string>> parser(string filePath, string delimiterRegex) {

  // open the file
  ifstream file(filePath);
  // set the regex
  regex del(delimiterRegex);

  // create the file itterator
  istream_iterator<string> fItterator(file);
  istream_iterator<string> fEnd;

  vector<vector<string>> output; // output variable

  // itterate over the file
  while (fItterator != fEnd) {

    // split the line of the file by the regex delimiter
    sregex_token_iterator it((*fItterator).begin(), (*fItterator).end(), del, -1);
    sregex_token_iterator end;

    // make vector to contain token
    vector<string> parsedToken;

    // itterate over the line of the file
    while (it != end) {

      // make sure to remove any empty strings
      if (*it != "") {
        // Add the block to the output vector
        parsedToken.push_back(*it);
      }

      ++it;
    } // END While

    // add the token to the output
    output.push_back(parsedToken);
    ++fItterator;
  } // END While

  // make sure to close the file when finished
  file.close();

  return output;
} // END parser()

// parse the input files
void parseInstructionFile(string filePath, string delimiterRegex) {
  // parses the instruction file and for ever line,
  // creates OpTokens,
  // Then, places the tokens on the insturction queue

  // get the output string from the parser
  vector<vector<string>> parsedTokens = parser(filePath, delimiterRegex);

  // itterate over the list of parsedTokens
  for (auto baseToken : parsedTokens) {
    // define the variables
    string opcode = baseToken[0];
    int dest      = stoi(baseToken[1]);
    int src1      = stoi(baseToken[2]);
    int src2      = stoi(baseToken[3]);

    // generate a new token for the INM
    OpToken* output = new OpToken(opcode, dest, src1, src2);

    // put the token on the INM
    INM.pushToken(dynamic_cast<Token*>(output));
  }

  return;
}

void parseRegisterFile(string filePath, string delimiterRegex) {
  // get the output string from the parser
  vector<vector<string>> parsedTokens = parser(filePath, delimiterRegex);

  // itterate over the list of parsedTokens
  for (auto baseToken : parsedTokens) {
    // define the variables
    int regLoc = stoi(baseToken[0]);
    int val    = stoi(baseToken[1]);

    // generate a new token for the RGF
    REGToken* output = new REGToken(regLoc, val);

    // put the token on the RGF
    RGF[regLoc] = output;
  }

  return;
}

void parseDataMemoryFile(string filePath, string delimiterRegex) {
  // get the output string from the parser
  vector<vector<string>> parsedTokens = parser(filePath, delimiterRegex);

  // itterate over the list of parsedTokens
  for (auto baseToken : parsedTokens) {
    // define the variables
    int damLoc = stoi(baseToken[0]);
    int val    = stoi(baseToken[1]);

    // generate a new token for the DAM
    DAMToken* output = new DAMToken(damLoc, val);

    // put the token on the DAM
    DAM[damLoc] = output;
  }

  return;
}

void outputFile(string filePath) {

  ofstream outFile;
  outFile.open(filePath);
  
  for (auto item : fileOutput) {
    // print to file
    outFile << item << "\n";
  }
  
  outFile.close();
  return;
}

// #endregion

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
} // END Loader()

// takes a REGToken and writes it to the RGF
Token* Writer(Token* in) {
  // Expects a REG token as input from REB
  REGToken* input = dynamic_cast<REGToken*>(in);

  // set the RGF index to the input pointer
  RGF[input->getReg()] = input;

  // does not need to return anything other than an nullptr
  // as this function is for a sink
  return nullptr;
} // END Writer()

// #endregion

// #beginregion --- Class Initialization functions ---

void initHardware() {
  // --- init Nodes ---
  static Node INBin("INB");
  static Node INBout("INB Out", &INBin);

  static Node AIBin("AIB");
  static Node AIBout("AIB Out", &AIBin);

  static Node LIBin("LIB");
  static Node LIBout("LIB Out", &LIBin);

  static Node ADBin("ADB");
  static Node ADBout("ADB Out", &ADBin);

  static Node REBin("REB");
  static Node REBout("REB Out", &REBin);

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
  outputNodes.push_back(&REBout);

  // --- init transitions ---
  // INM --> Decoder --> INBout
  static Transition decdoder(&INM, &INBout, &ReadAndDecode);

  // INBin --> Issue1 --> AIBout
  static Transition issuer1(&INBin, &AIBout, &Issue1);

  // INBin --> Issue2 --> LIBout
  static Transition issuer2(&INBin, &LIBout, &Issue2);

  // LIBin --> ADDR --> ADBout
  static Transition addr(&LIBin, &ADBout, &LogicUnit);

  // AIBin --> ALU --> REBout
  static Transition alu(&AIBin, &REBout, &LogicUnit);

  // ADBin --> LOAD --> REBout
  static Transition load(&ADBin, &REBout, &Loader);

  // REBin --> WRITE --> sink
  static Transition write(&REBin, &Writer);

  // put transitions on global queue

  transitions.push_back(&decdoder);
  transitions.push_back(&issuer1);
  transitions.push_back(&issuer2);
  transitions.push_back(&addr);
  transitions.push_back(&alu);
  transitions.push_back(&load);
  transitions.push_back(&write);
}

// #endregion

// #beginregion --- Simulator Function ---

// return true if the simulation is
bool isFinished() {

  // itterate over all transitions
  for (auto transition : transitions) {

    // simulation can only be finished if,
    // after running every transition for a cycle
    // no transition has ran, therefore,
    // if a transition has moved, it can no longer move
    // so the simulation then must not be finished
    if (transition->canMove() == false) return false;
    else continue;
  }
  return true;
}

// reset all transitions
void resetTransitions() {
  for (auto transition : transitions) {
    transition->reset();
  }
}

void Simulator() {

  int step      = 1;
  bool finished = false;
  do {
    // --- PHASE 1 ---
    // run all the transitions
    auto transIt       = transitions.begin();
    bool transHasMoved = false;

    // itterate over transitions until no transitions have acted
    do {
      // reset state
      transHasMoved = false;
      transIt       = transitions.begin();

      while (transIt != transitions.end()) {
        // trigger the transition's compute function check the result
        transHasMoved = (*transIt)->compute();
        transIt++;
      }

      // keep itterating if any function has moved
    } while (transHasMoved == true);

    // --- PHASE 2 ---
    // Commit the node changes
    auto nodeIt = outputNodes.begin();
    while (nodeIt != outputNodes.end()) {
      // will take the entire queue from an output node
      // and place it into the queue for an input node
      (*nodeIt)->commit();
      nodeIt++;
    }

    // --- PHASE 3 ---
    // Run the print cycle
    cout << printCycle(step) << endl;
    step++;
    finished = isFinished();
    resetTransitions();
  } while (finished == false);
  return;
}

// #endregion