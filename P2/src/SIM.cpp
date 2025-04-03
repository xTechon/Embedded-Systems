// I have neither given nor received any unauthorized aid on this assignment
#include <bitset>
#include <fstream>
#include <iterator>
#include <ostream>
#include <regex>
#include <string>
#include <utility>
#include <vector>

// can only use std on this project, so it's okay to use this
// would not use this under normal circumstances
using namespace std;

// #beginregion --- Global Variables ---

string compressionInput    = "original.txt";
string compressionOutput   = "cout.txt";
string decompressionInput  = "compressed.txt";
string decompressionOutput = "dout.txt";
string fileOutputName      = compressionOutput;
vector<string> fileOutput; // contains the cycles for the file output
vector<string> fileInput;  // contains an itterable of the 32-bit lines of the file only
vector<string> dictImport; // stores the raw dictionary information
const int DICTIONARY_SIZE = 16;
const int BINARY_SIZE     = 32; // 32-bit binaries
int mode                  = 0;

// #endregion

// #beginregion --- Main Function Declarations ---

void ParseFile();                 // parse the file regarless of mode
void CompressBinary();            // compression function containing relevant logic
void DecompressBinary();          // decompression function containing relevant logic
void OutputFile(string filePath); // output file regarless of mode

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

  if (mode == 1) {
    // compression function
    CompressBinary();
  } else if (mode == 2) {
    // decompression function
    DecompressBinary();
  }

  // write to file regardless of mode
  OutputFile(fileOutputName);

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

// token used for both compression and decompression
// errors place -1 in the length field
struct token
{
  int length;       // store the length of the compression
  int rank;         // rank of the command
  string original;  // the original binary string
  string command;   // 3-bit command string
  string SL;        // 5-bit Starting location or first mismatch location
  string ML2;       // 5-bit 2nd Mismatch location
  string bitmask;   // 4-bit bitmask
  string dictIndex; // 4-bit index of the related dictionary entry
  string full;      // the entire compressed binary string
};

// tracks the mismatches of a binary and a specific dictionary entry
struct dictMatch
{
  int index;
  int mismatch;
};

enum PATTERNS { ORIGNIAL, RLE, BITMASK, ONEBIT, TWOBITC, FOURBIT, TWOBITA, DIRECT };

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

// turn a binary bit string into C++ integer
int StringToBinary(string input) { return stoi(input, nullptr, 2); }

// use Enumerables to track ranking easier
int PatternToRank(PATTERNS input) {
  switch (input) {
  case ORIGNIAL:
    return 0;
  case RLE:
    return 1;
  case BITMASK:
    return 2;
  case ONEBIT:
    return 3;
  case TWOBITC:
    return 4;
  case FOURBIT:
    return 5;
  case TWOBITA:
    return 6;
  case DIRECT:
    return 7;
  }
}

// #endregion

// #beginregion --- Bitmasking Functions ---

// given a binary and an entry
// and the number of mismatches
// and the length of the bitmask
// find the location of the first mismatch
// make sure they no more than bitmask length from each other
// and the first mismatch cannot be less than the bitmask length distance from end of binary
// returns a pair where first value is the int starting location and second is the bitmask
// returns -1 in the location field if a bitmask is not possible
pair<int, string> GenerateBitmask(string binary, string entry, int numMis, int bitLength) {

  pair<int, string> output;
  string bitmask;
  
  int location            = 0;
  int remainingMismatches = numMis;
  int remainingDist       = bitLength;
  bool maskStart          = false;

  // itterate over length of binary - bitmask length
  for (int i = 0; i < (BINARY_SIZE - bitLength); i++) {
    // maximum lenght of bitmask, exit
    if (remainingDist == 0) break;

    bool match = binary[i] == entry[i];

    // first mismatch
    if (!match && !maskStart) {
      location  = i;
      maskStart = true;
      bitmask.append("1");
      remainingMismatches--;
      continue;
    }

    // generate the bitmask
    if (maskStart) {
      if (match) bitmask.append("0");
      else {
        bitmask.append("1");
        remainingMismatches--;
      }
      remainingDist--;
    }
  } // END for loop
  
  output.first  = location;

  // a bitmask is not possible if there are still mismatches
  // not captured by the entire bitmask
  // or the bitmask was not able to complete
  if ((remainingDist != 0) || (remainingMismatches != 0)) output.first = -1;

  output.second = bitmask;
  return output;
} // END GenerateBitmask

token BitmaskingCompression(string binary, int index, int mismatches) {
  token output;
  string entry = dictionary[index];

  // get a bitmask if any are possible
  pair<int, string> bitmask = GenerateBitmask(binary, entry, mismatches, BITMASK_LENGTH);

  // return with error if bitmask is not possible
  if (bitmask.first == -1) {
    output.length = -1;
    return output;
  }

  // add command string
  output.command = PatternToStringBinary(BITMASK);

  // rank from pdf
  output.rank = PatternToRank(BITMASK);

  // add starting location
  bitset<5> loc = bitmask.first;
  output.SL     = loc.to_string();

  // add dictionary index
  bitset<4> ind    = index;
  output.dictIndex = ind.to_string();

  // add bitmask
  output.bitmask = bitmask.second;

  // add the original binary to the token
  output.original = binary;

  // generate the full pattern
  output.full = output.command + output.SL + output.bitmask + output.dictIndex;

  // get the length of the full command
  output.length = output.full.length();

  return output;
} // END Bitmasking

// #endregion

// #beginregion --- Bit Mismatch Calculation Functions ---

// calculate the number of mismatched bits between two binary strings
int CalcMismatch(string lhs, string rhs, int length) {
  int counter = 0;
  for (int i = 0; i < length; i++) {
    if (lhs[i] != rhs[i]) counter++;
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

// given a binary and an entry
// find the location of the mismatches
// verify if they are adjacent or not
// if they are not adjacent, return -1
int MismatchLocation(string binary, string entry, int numMis) {
  int location = 0;

  bool consecutive    = false;
  int mismatchCounter = numMis;

  for (int i = 0; i < BINARY_SIZE; i++) {
    // no more mismatches
    if (mismatchCounter == 0) break;

    bool match = binary[i] == entry[i];

    // first instance
    if (!match && !consecutive) {
      consecutive = true;
      location    = i;
      mismatchCounter--;
      continue;
    }

    // adjacent mismatch
    if (!match && consecutive) {
      mismatchCounter--;
      continue;
    }

    // mismatch not adjacent
    if (match && consecutive) {
      location = -1;
      break;
    }
  }

  return location;
} // END MismatchLocation()

// find the locations of two, nonconsecutive mismatches
pair<int, int> TwoBitLocations(string binary, string entry) {
  pair<int, int> locations;

  int counter = 0;

  for (int i = 0; i < BINARY_SIZE; i++) {

    if (counter == 2) break;

    bool match = binary[i] == entry[i];

    if (!match) {
      if (counter == 0) locations.first = i;
      if (counter == 1) locations.second = i;
      counter++;
    }
  } // END For loop

  return locations;
} // END TwoBitLocations

// #endregion

// #beginregion --- Bit Mismatch Compression Functions ---
// All compression functions should return a token

// 1-bit Mismatch
token OneBitMismatch(string binary, int index) {
  token output;
  string entry = dictionary[index];
  int location = MismatchLocation(binary, entry, 1);

  // return error if any
  if (location == -1) {
    output.length = -1;
    return output;
  }

  // add command string
  output.command = "011";

  // rank from pdf
  output.rank = PatternToRank(ONEBIT);

  // add mismatch location
  bitset<5> loc = location;
  output.SL     = loc.to_string();

  // add dictionary index
  bitset<4> ind    = index;
  output.dictIndex = ind.to_string();

  // add the original binary to the token
  output.original = binary;

  // generate the full pattern
  output.full = output.command + output.SL + output.dictIndex;

  // get the length of the full command
  output.length = output.full.length();

  return output;
}

// 2-bit consecutive mismatch
token TwoBitMismatch(string binary, int index) {
  token output;
  string entry = dictionary[index];
  int location = MismatchLocation(binary, entry, 2);

  // return error if any
  if (location == -1) {
    output.length = -1;
    return output;
  }

  // add command string
  output.command = "100";

  // rank from pdf
  output.rank = PatternToRank(TWOBITC);

  // add mismatch location
  bitset<5> loc = location;
  output.SL     = loc.to_string();

  // add dictionary index
  bitset<4> ind    = index;
  output.dictIndex = ind.to_string();

  // add the original binary to the token
  output.original = binary;

  // generate the full pattern
  output.full = output.command + output.SL + output.dictIndex;

  // get the length of the full command
  output.length = output.full.length();

  return output;
}

// 4-bit consecutive mismatch
token FourBitMismatch(string binary, int index) {
  token output;
  string entry = dictionary[index];
  int location = MismatchLocation(binary, entry, 4);

  // return error if any
  if (location == -1) {
    output.length = -1;
    return output;
  }

  // add command string
  output.command = "101";

  // rank from pdf
  output.rank = PatternToRank(FOURBIT);

  // add mismatch location
  bitset<5> loc = location;
  output.SL     = loc.to_string();

  // add dictionary index
  bitset<4> ind    = index;
  output.dictIndex = ind.to_string();

  // add the original binary to the token
  output.original = binary;

  // generate the full pattern
  output.full = output.command + output.SL + output.dictIndex;

  // get the length of the full command
  output.length = output.full.length();

  return output;
}

// 2-bit anywhere mismatch
token Arbitrary2Mismatch(string binary, int index) {
  token output;
  string entry = dictionary[index];

  // get the locations for the mismatches
  pair<int, int> locations = TwoBitLocations(binary, entry);

  // add command string
  output.command = "110";

  // rank from pdf
  output.rank = PatternToRank(TWOBITA);

  // add mismatch location 1
  bitset<5> loc = locations.first;
  output.SL     = loc.to_string();

  // add mismatch location 2
  bitset<5> loc2 = locations.second;
  output.ML2     = loc2.to_string();

  // add dictionary index
  bitset<4> ind    = index;
  output.dictIndex = ind.to_string();

  // add the original binary to the token
  output.original = binary;

  // generate the full pattern
  output.full = output.command + output.SL + output.ML2 + output.dictIndex;

  // get the length of the full command
  output.length = output.full.length();

  return output;
}

// Direct Match
token DirectMatch(string binary, int index) {
  token output;

  // set the rank of the command
  output.rank = PatternToRank(DIRECT);

  // the original binary for the token
  output.original = binary;

  // command string
  output.command = "111";

  // add dictionary index
  bitset<4> ind    = index;
  output.dictIndex = ind.to_string();

  // generate the full command
  output.full = output.command + output.dictIndex;

  // get the length of the command
  output.length = output.full.length();

  return output;
}

// Mismatch function decider
// Given a vector input of a binary and dictionary entries
// decide which mismatch functions to run and calculate the compression
// length for each
vector<token> DecideMismatches(string input, vector<dictMatch> reference) {

  // init a list of possible compressions
  vector<token> possibleCompressions;

  // temporary token usage
  token temp;
  token temp2;

  // itterate over the dictionary list
  for (auto entry : reference) {

    // send the entry to the relevant function
    switch (entry.mismatch) {

    case 4:
      // 4-bit consecutive only
      temp = FourBitMismatch(input, entry.index);
      break;

    case 3:
      // Use bitmasking, can't do 3
      // TODO: add bitmasking to mismatch
      temp.length = -1;
      break;

    case 2:
      // 2-bit Mismatch
      // can be consecutive or arbitrary
      temp  = TwoBitMismatch(input, entry.index);
      temp2 = Arbitrary2Mismatch(input, entry.index);

      // skip error from temp2
      if (temp2.length == -1) break;
      // make sure 2nd token is added
      possibleCompressions.push_back(temp2);
      break;

    case 1:
      // 1-bit Mismatch
      temp = OneBitMismatch(input, entry.index);
      break;

    case 0:
      // Direct match
      temp = DirectMatch(input, entry.index);
      break;
    }

    // error, skip token
    if (temp.length == -1) continue;

    // add the token to the vector
    possibleCompressions.push_back(temp);

  } // END For Loop

  return possibleCompressions;
} // END DecideMismatches()

// #endregion

// #beginregion --- Compression Function ---

// sort based on smallest compression followed by smallest rank
bool LeastCompression(token i, token j) {
  bool len      = i.length < j.length;
  bool conflict = i.length == j.length;

  if (conflict) return i.rank < j.rank;
  else return len;
}

// Compresses the Binary using the global scoped fileInput variable
void CompressBinary() {

  // generate the dictionary from the file
  GenerateDictionary(fileInput);

  string previousBinary;
  // itterate over the file
  for (auto it = fileInput.begin(); it < fileInput.end(); it++) {

    string binary = *it;

    // possible compression candidates
    vector<token> candidates;

    // add the mismatch compression candidates
    // calculate the mismatch numbers of each entry in the dictionary
    vector<int> dictMismatches = CalculateDictionaryMismatch(binary);

    // filter the mismatches down to what is possible to compress
    vector<dictMatch> dictFiltered = FilterDictionary(dictMismatches, 4);

    // get the tokens of mismatch possibilities
    vector<token> mismatches = DecideMismatches(binary, dictFiltered);

    // add the mismatch tokens to the candidates vector
    candidates.insert(candidates.begin(), mismatches.begin(), mismatches.end());

    // TODO: RLE and Bitmask HERE
    // for RLE: copy itterator to function to perform look ahead

    // sort the candidates vector by smallest length
    sort(candidates.begin(), candidates.end(), LeastCompression);
  }

  // Make sure file has the correct name
  fileOutputName = compressionOutput;
}

// #endregion

// #beginregion --- Decompression Function ---

// Decompresses the Binary using the global scoped fileInput variable
void DecompressBinary() {

  // Make sure file has the correct name
  fileOutputName = decompressionOutput;
}

// #endregion