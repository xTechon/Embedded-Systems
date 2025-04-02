// I have neither given nor received any unauthorized aid on this assignment
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <ostream>
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
const int DICTIONARY_SIZE = 16;
const int BINARY_SIZE     = 32; // 32-bit binaries
int mode                  = 0;

// #endregion

// #beginregion --- Main Function Declarations ---

void ParseFile();
void GenerateDictionary(vector<string>);

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

  // generate the dictionary from the file
  GenerateDictionary(fileInput);

  return 0;
}

// #beginregion --- Global Function Variables ---

struct dictVal
{
  string binary;
  int frequency;
  int rank;
};

map<string, dictVal> dictMap;       // collect stats for each binary
vector<dictVal> dictVect;           // sort the binaries based on stats
string dictionary[DICTIONARY_SIZE]; // format the binaries for easier debugging

// #endregion

// #beginregion --- Global Function Declarations ---

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

// itterate over a string vector, and output the vector to a file
// each obj in the vector is placed on a newline
void OutputFile(string filePath) {

  ofstream outFile;
  outFile.open(filePath);

  for (auto item : fileOutput) {
    // print to file
    outFile << item << "\n";
  }

  outFile.close();
  return;
} // END OutputFile

// sort based on frequency first followed by rank
bool Ranker(dictVal i, dictVal j) {
  bool freq     = i.frequency > j.frequency;
  // sort by rank if frequency is the same
  bool conflict = i.frequency == j.frequency;
  if (conflict) return i.rank < j.rank;
  else return freq;
}

// Generate a Dictionary from Uncompressed Binary
void GenerateDictionary(vector<string> input) {

  // keep track of the dictionary's order
  int counter = 0;

  // itterate over the input string of 32-bit binaries
  for (auto binary : input) {

    auto frequency    = dictMap.find(binary);
    auto doesNotExist = dictMap.end();

    // initalize the binary if it doesn't exist in the dictionaryMap
    // increment otherwise
    if (frequency == doesNotExist) {
      dictVal temp;
      temp.binary     = binary;
      temp.frequency  = 1;
      temp.rank       = counter;
      dictMap[binary] = temp;
    } else {
      frequency->second.frequency++;
    }

    // increment the counter for the next itteration
    counter++;
  } // End for loop

  // convert to vector for sorting and indexing
  for (auto entry : dictMap) {
    dictVect.push_back(entry.second);
  }

  // sort the vector
  sort(dictVect.begin(), dictVect.end(), Ranker);

  // convert dict vector to string array
  for (int i = 0; i < DICTIONARY_SIZE; i++) {
    dictionary[i] = dictVect[i].binary;
  }

} // END Generate Dictionary

// Import a Dictionary from Compressed Binary
void ImportDictionary(vector<string> import) {
  for (int i = 0; i < DICTIONARY_SIZE; i++) {
    dictionary[i] = import[i];
  }
} // END ImportDictionary

// #endregion

// #beginregion --- Bit Mismatch Calculation Functions ---

// calculate the number of mismatched bits between two binary strings
int CalcMismatch(string lhs, string rhs, int length) {
  int counter = 0;
  for (int i = 0; i < length; i++) {
    if (lhs[i] == rhs[i]) counter++;    
  }
  return counter;
}

// create a list of mismatches of the input string with each entry in the dictionary
vector<int> CalculateDictionaryMismatch(string input) {
  vector<int> output;
  for (int i = 0; i < DICTIONARY_SIZE; i++) {
    output.push_back(CalcMismatch(input, dictionary[i], BINARY_SIZE));
  }
  return output;
}

struct dictMatch
{
  int index;
  int mismatch;
};

// given a list of dictionary mismatch entries
// filter out mismatch numbers greater than LIMIT
// while storing their corresponding index
vector<dictMatch> FilterDictionary(vector<int> list, int limit) {
  vector<dictMatch> output;
  for (int i = 0; i < DICTIONARY_SIZE; i++) {

    if (list[i] > limit) continue;

    dictMatch temp;
    temp.mismatch = list[i];
    temp.index    = i;

    output.push_back(temp);
  }
  return output;
}

// #endregion

// #beginregion --- Bit Mismatch Compression Functions ---

// Mismatch function decider

// 1-bit Mismatch

// 2-bit consecutive mismatch

// 4-bit consecutive mismatch

// 2-bit anywhere mismatch

// Direct Match

// #endregion
