// I have neither given nor received any unauthorized aid on this assignment
#include <fstream>
#include <iterator>
#include <regex>
#include <string>
#include <vector>

// can only use std on this project, so it's okay to use this
// would not use this under normal circumstances
using namespace std;

// #beginregion --- Global Variables ---

string compressionInput    = "original.txt";
string compressionOutput   = "cout.txt";
string decompressionInput  = "compressed.txt";
string decompressionOutput = "dout.txt";
vector<string> fileOutput; // contains the cycles for the file output
vector<string> fileInput;  // contains an itterable of the 32-bit lines of the file only
vector<string> dictImport; // stores the raw dictionary information
int mode = 0;

// #endregion

// #beginregion --- Main Function Declarations ---

void ParseFile();

// #endregion

// argc is # of arguments including program execution
// argv is the array of strings of every argument including execution
int main(int argc, char* argv[]) {

  // change path locations if arguments set
  //  1st argmment is  a  1 or 2 for de/compression mode
  string modeInput = argv[1];

  // switch defaults to new arguments if specified
  if (argc >= 3) {
    compressionInput  = argv[2];
    compressionOutput = argv[3];

    decompressionInput  = argv[2];
    decompressionOutput = argv[3];
  }

  // get the mode from the input
  mode = stoi(modeInput);
  
  // parse and load the file into memory
  ParseFile();

  return 0;
}

// generic parser to create an itterable of the input file
vector<string> Parser(string filePath, string delimiterRegex) {

  // open the file
  ifstream file(filePath);
  // set the regex
  regex del(delimiterRegex);

  // create the file itterator
  istream_iterator<string> fItterator(file);
  istream_iterator<string> fEnd;

  vector<string> output; // output variable

  bool dict = false;
  // itterate over the file
  while (fItterator != fEnd) {

    // split the line of the file by the regex delimiter
    sregex_token_iterator it((*fItterator).begin(), (*fItterator).end(), del, -1);
    sregex_token_iterator end;

    // make vector to contain token
    string parsedToken = *it;

    // after encountering "xxxx", insert to dictionary import instead
    if (dict) dictImport.push_back(parsedToken);

    // before encoutering an "xxxx"...
    if ((parsedToken != "xxxx") && (!dict)) {
      // add the token to the output
      output.push_back(parsedToken);
    } else {
      dict = true;
    }


    ++fItterator;
  } // END While

  // make sure to close the file when finished
  file.close();

  return output;
} // END parser()

// parse the file based on the mode
void ParseFile() {
  string reg = "\n";
  if (mode == 1) {
    fileInput = Parser(compressionInput, reg);
  } else if (mode == 2) {
    fileInput = Parser(decompressionInput, reg);
  }

} // END ParseFile