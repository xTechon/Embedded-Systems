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
vector<string> fileOutput; // contains the lines for the file output
vector<string> fileInput;  // contains an itterable of the 32-bit lines of the file only
vector<string> dictImport; // stores the raw dictionary information
const int DICTIONARY_SIZE = 16;
const int BINARY_SIZE     = 32; // 32-bit binaries
const int BITMASK_LENGTH  = 4;
int mode                  = 0;

// #endregion

// #beginregion --- Main Function Declarations ---

void ParseFile();                 // parse the file regarless of mode
void CompressBinary();            // compression function containing relevant logic
void DecompressBinary();          // decompression function containing relevant logic
void CompileTokens();             // Turn the tokens into 32-bit strings based on mode
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

  // create tokens of the file based on mode
  if (mode == 1) {
    // compression function
    CompressBinary();
  } else if (mode == 2) {
    // decompression function
    DecompressBinary();
  }

  // compile the tokens to a useable format for the output function
  CompileTokens();

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

enum PATTERNS { ORIGNIAL, RLE, BITMASK, ONEBIT, TWOBITC, FOURBIT, TWOBITA, DIRECT };

// token used for both compression and decompression
// errors place -1 in the length field
struct token
{
  PATTERNS method;  // Debugabble compression method
  string full;      // the entire compressed binary string
  int length;       // store the length of the compression
  int rank;         // rank of the command
  int dictIn;       // dictionary index as integer
  int rle;          // used for RLE
  string command;   // 3-bit command string
  string original;  // the original binary string
  string SL;        // 5-bit Starting location or first mismatch location
  string ML2;       // 5-bit 2nd Mismatch location
  string bitmask;   // 4-bit bitmask
  string dictIndex; // 4-bit index of the related dictionary entry
};

// tracks the mismatches of a binary and a specific dictionary entry
struct dictMatch
{
  int index;
  int mismatch;
};

vector<token> preProcessedOutput; // a vector of tokens to either compress or decompress

// for debugging, tracks the current line before it's added to preprocessor
int line = 1;

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
int StringBinaryToInt(string input) { return stoi(input, nullptr, 2); }

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
} // END PatternToRank

// user enumerables to for better control string management
string PatternToStringBinary(PATTERNS input) {
  switch (input) {
  case ORIGNIAL:
    return "000";
  case RLE:
    return "001";
  case BITMASK:
    return "010";
  case ONEBIT:
    return "011";
  case TWOBITC:
    return "100";
  case FOURBIT:
    return "101";
  case TWOBITA:
    return "110";
  case DIRECT:
    return "111";
  }
} // END PatternToStringBinary

// Turn Binary string into the correct command
PATTERNS StringBinaryToPattern(string command) {
  if (command == "000") return ORIGNIAL;
  if (command == "001") return RLE;
  if (command == "010") return BITMASK;
  if (command == "011") return ONEBIT;
  if (command == "100") return TWOBITC;
  if (command == "101") return FOURBIT;
  if (command == "110") return TWOBITA;
  if (command == "111") return DIRECT;
  return ORIGNIAL;
} // END StringBinaryToPattern

int PatternToNumMismatch(PATTERNS input) {
  switch (input) {
  case ORIGNIAL:
    return 0;
  case RLE:
    return 0;
  case BITMASK:
    return 4;
  case ONEBIT:
    return 1;
  case TWOBITC:
    return 2;
  case FOURBIT:
    return 4;
  case TWOBITA:
    return 2;
  case DIRECT:
    return 0;
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
  for (int i = 0; i < BINARY_SIZE; i++) {
    // maximum lenght of bitmask, exit
    if (remainingDist == 0) break;

    bool match = binary[i] == entry[i];

    // first mismatch
    if (!match && !maskStart) {
      location  = i;
      maskStart = true;
      bitmask.append("1");
      remainingMismatches--;
      remainingDist--;
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

  output.first = location;

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

  output.method = BITMASK;

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
  output.dictIn    = index;

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

// function to run for all mismatch functions
token TokenPreamble(string binary, int index, PATTERNS command, bool skipCheck = false) {
  token output;
  string entry = dictionary[index];
  int location = MismatchLocation(binary, entry, PatternToNumMismatch(command));

  output.method = command;

  // return error if any, unless skipCheck is flagged
  if (location == -1 && !skipCheck) {
    output.length = -1;
    return output;
  }

  // fields that are in every token
  output.command  = PatternToStringBinary(command);
  output.rank     = PatternToRank(command);
  output.original = binary;

  // add mismatch location, there's always at least one
  bitset<5> loc = location;
  output.SL     = loc.to_string();

  // there's always a dictionary index
  // store the dictionary index as both integer and string
  bitset<4> ind    = index;
  output.dictIndex = ind.to_string();
  output.dictIn    = index;

  // generate the full pattern
  output.full = output.command + output.SL + output.dictIndex;

  // get the length of the full command
  output.length = output.full.length();

  return output;
}

// #endregion

// #beginregion --- Bit Mismatch Compression Functions ---
// All compression functions should return a token

// 1-bit Mismatch
token OneBitMismatch(string binary, int index) {
  token output;

  // errors will get carried out of the function
  output = TokenPreamble(binary, index, ONEBIT);

  return output;
}

// 2-bit consecutive mismatch
token TwoBitMismatch(string binary, int index) {
  token output;

  output = TokenPreamble(binary, index, TWOBITC);

  return output;
}

// 4-bit consecutive mismatch
token FourBitMismatch(string binary, int index) {
  token output;

  output = TokenPreamble(binary, index, FOURBIT);

  return output;
}

// 2-bit anywhere mismatch
token Arbitrary2Mismatch(string binary, int index) {
  token output;

  output = TokenPreamble(binary, index, TWOBITA, true);

  // get the locations for the mismatches
  pair<int, int> locations = TwoBitLocations(binary, dictionary[index]);

  // override what's provided by the token
  // add mismatch location 1
  bitset<5> loc = locations.first;
  output.SL     = loc.to_string();

  // add mismatch location 2
  bitset<5> loc2 = locations.second;
  output.ML2     = loc2.to_string();

  // generate the full pattern
  output.full = output.command + output.SL + output.ML2 + output.dictIndex;

  // get the length of the full command
  output.length = output.full.length();

  return output;
}

// Direct Match
token DirectMatch(string binary, int index) {
  token output;

  output = TokenPreamble(binary, index, DIRECT, true);

  // generate the full command
  output.full = output.command + output.dictIndex;

  // get the length of the command
  output.length = output.full.length();

  return output;
}

// Mismatch function decider
// Given a vector input of a binary and dictionary entries
// decide which mismatch functions to run and calculate the compression length for each
// also get the possible bitmask of each length
vector<token> DecideMismatches(string input, vector<dictMatch> reference) {

  // init a list of possible compressions
  vector<token> possibleCompressions;

  // temporary token usage
  token temp;
  token temp2;
  token bitmasking;

  // itterate over the dictionary list
  for (auto entry : reference) {

    // send the entry to the relevant function
    switch (entry.mismatch) {

    case 4:
      // 4-bit consecutive only
      temp = FourBitMismatch(input, entry.index);
      break;

    case 3:
      // Use bitmasking, skip temp
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

    // only use bitmasking if there is at least 1 mismatch
    if (entry.mismatch != 0) bitmasking = BitmaskingCompression(input, entry.index, entry.mismatch);

    // only append the bitmask token if there were no errors
    if (bitmasking.length != -1) possibleCompressions.push_back(bitmasking);

    // error, skip token
    if (temp.length == -1) continue;

    // add the token to the vector
    possibleCompressions.push_back(temp);

  } // END For Loop

  return possibleCompressions;
} // END DecideMismatches()

// #endregion

// #beginregion --- RLE Compression Functions ---

// given the current posion in the file and the previous token
// check if RLE encoding is possible and return valid tokens if possible
// returns a pair where the first value is the number of occurences of the
// current token and the second value is a vector of tokens to insert.
// otherwise, return {1, an empty vector}
pair<int, vector<token>> RunLengthEncoding(vector<string>::iterator cursor, token input) {
  vector<token> output;

  // used to count the number of times the current token appears consecutively
  int counter = 0;

  // keep counting the repetions of the token until there are no more repetions
  while (*cursor == input.original) {
    counter++;
    cursor++;
  }

  // exit with nothing if RLE was not possible
  if (counter <= 1) return {counter, output};

  // get the number of RLE tokens needed
  int chuncks = counter / 9;

  // create a maximum length rle token to be reused
  token rleToken   = input;
  rleToken.command = PatternToStringBinary(RLE);
  rleToken.rank    = PatternToRank(RLE);
  rleToken.method  = RLE;
  rleToken.length  = 6; // RLE tokens are always 6 length
  rleToken.rle     = 7; // count the number of occurances as index format

  bitset<3> rleBitwise = rleToken.rle;
  rleToken.full        = rleToken.command + rleBitwise.to_string();

  // create tokens for each segment needed
  for (int i = 0; i < chuncks; i++) {
    output.push_back(input);
    output.push_back(rleToken);
  }

  // add remaining amount if needed
  int remainder = counter % 9;

  if (remainder != 0) {
    // always start with the original token
    output.push_back(input);

    // add another RLE token if needed
    if (remainder > 1) {

      // modify the token for the remainder
      // -2, one for original token, one to account for indexing
      rleToken.rle  = remainder - 2;
      rleBitwise    = rleToken.rle;
      rleToken.full = rleToken.command + rleBitwise.to_string();

      output.push_back(rleToken);
    }
  }

  return {counter, output};
} // END RunLengthEncoding

// #endregion

// #beginregion --- Compression Function ---

// provide the case where no compression is performed
token NoCompression(string binary) {
  token output;

  // get the command and rank of the token
  output.method  = ORIGNIAL;
  output.command = PatternToStringBinary(ORIGNIAL);
  output.rank    = PatternToRank(ORIGNIAL);

  // get the original binary string
  output.original = binary;

  // create the full "compressed" string and its length
  output.full   = output.command + output.original;
  output.length = output.full.length();

  return output;
}

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

    // Always have a case where no compression is performed
    candidates.push_back(NoCompression(binary));

    // sort the candidates vector by smallest length
    sort(candidates.begin(), candidates.end(), LeastCompression);

    // for RLE: copy itterator to function to perform look ahead
    // always run a check for RLE on every command
    pair<int, vector<token>> rle = RunLengthEncoding(it, candidates[0]);

    // if the rle is not empty,
    if (rle.first > 1) {
      // insert the vector to the end of the preProcessor
      preProcessedOutput.insert(preProcessedOutput.end(), rle.second.begin(), rle.second.end());
      // make sure the line counter still matches
      line += rle.first;
      // jump the itterator
      it += rle.first - 1;
      // skip the rest of the loop
      continue;
    }

    line++; // Debuging

    // add the top candidate to the pre processed output
    preProcessedOutput.push_back(candidates[0]);
  }

  // Make sure file has the correct name
  fileOutputName = compressionOutput;
}

// #endregion

// #beginregion --- Decompression Functions ---

// flips the bit of the binary
// given the entire binary string and a location
string FlipBit(string binary, int location) {

  // get the digit from the string
  char digit = binary.at(location);

  // flip the bit
  if (digit == '0') digit = '1';
  else if (digit == '1') digit = '0';

  // replace the digit in the binary string
  binary.replace(location, 1, 1, digit);

  return binary;
}

// given a binary string, location, and a bitmask, apply the bitmask
string ApplyBitmask(string binary, string bitmask, int location) {

  int maskLength = bitmask.length();

  // get the portion of the binary string to apply a bitmask
  string target(binary.substr(location, maskLength));

  int tar  = StringBinaryToInt(target);
  int mask = StringBinaryToInt(bitmask);

  bitset<BITMASK_LENGTH> output = tar;
  bitset<BITMASK_LENGTH> bmask  = mask;

  // apply the bitmask
  output = output ^ bmask;

  // replace the portion of the binary with the new bitmasked portion
  binary.replace(location, BITMASK_LENGTH, output.to_string());

  return binary;
}

token DOriginal(string::iterator cursor) {
  token output;

  string binary = "";
  string original = "";

  // add the rank of the token
  output.rank = PatternToRank(ORIGNIAL);
  output.method = ORIGNIAL;

  // get the full command
  output.full = binary.append(cursor, cursor + (3 + BINARY_SIZE));

  // list the length of the token
  output.length = output.full.length();


  output.original = original.append(cursor + 3, cursor + (3 + BINARY_SIZE));

  return output;
} // END DOriginal

token DRLE(string::iterator cursor) {
  token output;

  int length = 3 + 3;

  output.rank   = PatternToRank(RLE);
  output.method = RLE;

  // add the command of the binary
  string com(cursor, cursor + 3);
  output.command = com;

  // add the length of the token
  output.length = length;

  string binary(cursor, cursor + length);

  // get the rle index
  string rle(cursor + 3, cursor + length);
  int count = StringBinaryToInt(rle);

  output.rle = count;

  // get the full compressed binary string
  output.full = binary;

  return output;
}

token DBitmask(string::iterator cursor) {
  token output;

  int length = 3 + 5 + 4 + 4;

  // token metadata
  output.rank    = PatternToRank(BITMASK);
  output.command = PatternToStringBinary(BITMASK);
  output.length  = length;
  output.method  = BITMASK;

  string binary(cursor, cursor + length);
  output.full = binary;

  // get the location
  string loc(cursor + 3, cursor + 3 + 5);
  int location = StringBinaryToInt(loc);

  // get the bitmask
  string bitmask(cursor + 3 + 5, cursor + 3 + 5 + 4);
  output.bitmask = bitmask;

  // get the dictionary Index
  string dictIndex(cursor + 3 + 5 + 4, cursor + length);
  int dictIn = StringBinaryToInt(dictIndex);

  output.dictIn    = dictIn;
  output.dictIndex = dictIndex;

  // get the original string
  string original = ApplyBitmask(dictionary[dictIn], bitmask, location);
  output.original = original;

  return output;
  // TODO BUG: line 89 on compression is choosing index 2 instead of index 4 (all 0000s)
} // END DBitmask

token DecompPreamble(string::iterator cursor, PATTERNS command) {
  token output;

  // command + location + dictionary index
  int length = 3 + 5 + 4;

  // add the rank of the token
  output.rank   = PatternToRank(command);
  output.method = command;

  // add the command of the binary
  string com(cursor, cursor + 3);
  output.command = com;

  // add the length of the token
  output.length = length;

  string binary(cursor, cursor + length);

  // get the full compressed binary string
  output.full = binary;

  // get the location
  string loc(cursor + 3, cursor + 3 + 5);
  output.SL = loc;
  //int location = StringBinaryToInt(loc);

  // get the dictionary index
  string dictIndex(cursor + 3 + 5, cursor + length);
  int dictIn = StringBinaryToInt(dictIndex);

  output.dictIn    = dictIn;
  output.dictIndex = dictIndex;

  return output;
}

token D1BitMis(string::iterator cursor) {

  token output = DecompPreamble(cursor, ONEBIT);
  string original;

  // derive the original binary string
  int loc  = StringBinaryToInt(output.SL);
  original = FlipBit(dictionary[output.dictIn], loc);

  output.original = original;

  return output;
} // END D1BitMis

token D2BitCMis(string::iterator cursor) {

  token output = DecompPreamble(cursor, TWOBITC);

  string original;

  // derive the original binary string
  int loc = StringBinaryToInt(output.SL);

  original = FlipBit(dictionary[output.dictIn], loc);
  original = FlipBit(original, loc + 1);

  output.original = original;

  return output;
} // END D2BitCMis

token D4BitMis(string::iterator cursor) {

  token output = DecompPreamble(cursor, FOURBIT);
  string original;

  // derive the original binary string
  int loc = StringBinaryToInt(output.SL);

  original = FlipBit(dictionary[output.dictIn], loc);
  original = FlipBit(original, loc + 1);
  original = FlipBit(original, loc + 2);
  original = FlipBit(original, loc + 3);

  output.original = original;

  return output;
} // END D4BitMis

token D2BitAMis(string::iterator cursor) {

  token output = DecompPreamble(cursor, TWOBITA);
  string original = "";

  // command + location + dictionary index
  int length = 3 + 5 + 5 + 4;

  // add the length of the token
  output.length = length;

  string binary(cursor, cursor + length);

  // get the full compressed binary string
  output.full = binary;

  // get the second location
  string loc2(cursor + 3 + 5, cursor + 3 + 5 + 5);
  int location2 = StringBinaryToInt(loc2);

  // get the dictionary index
  string dictIndex(cursor + 3 + 5 + 5, cursor + length);
  int dictIn = StringBinaryToInt(dictIndex);

  output.dictIn    = dictIn;
  output.dictIndex = dictIndex;

  int loc = StringBinaryToInt(output.SL);

  // derive the original binary string
  original = FlipBit(dictionary[dictIn], loc);
  original = FlipBit(original, location2);

  output.original = original;

  return output;
} // END D2BitAMis

token DDirect(string::iterator cursor) {

  token output;

  int length = 3 + 4;

  output.rank    = PatternToRank(DIRECT);
  output.command = PatternToStringBinary(DIRECT);
  output.length  = length;
  output.method  = DIRECT;

  string binary(cursor, cursor + length);
  output.full = binary;

  // get the dictionary Index
  string dictIndex(cursor + 3, cursor + length);
  int dictIn = StringBinaryToInt(dictIndex);

  output.dictIn    = dictIn;
  output.dictIndex = dictIndex;

  // derive the original binary string

  output.original = dictionary[dictIn];

  return output;
} // END DDirect

// #endregion

// #beginregion --- Decompression Tokenizer ---

// turn the file into one large string
string CombineFile(vector<string> input) {
  string output;
  for (auto line : input) {
    output.append(line);
  }
  return output;
} // END CombineFile

// given a length, check if it will cause an overflow
// returns true if there is an overflow
bool CheckForOverflow(string::iterator cursor, string::iterator end, int length) {

  // check to make sure a buffer overflow won't happen
  // Buffer overflow at the end of the
  auto test = cursor;
  int counter = 0;
  for (counter = 0; counter < length; counter++){
    if (test != end) test++;
    else break;
  }
  if (counter < length - 1) return true;
  return false;
}

// call the correct decompression function based on the command
token Decider(PATTERNS command, string::iterator cursor, string::iterator end) {
  token output;
  output.length = -1;


  switch (command) {
  case ORIGNIAL:
    if (CheckForOverflow(cursor, end, 3 + BINARY_SIZE)) return output;
    return DOriginal(cursor);
  case RLE:
    return DRLE(cursor);
  case BITMASK:
    //if (CheckForOverflow(cursor, end, 16)) return output;
    return DBitmask(cursor);
  case ONEBIT:
    return D1BitMis(cursor);
  case TWOBITC:
    if (CheckForOverflow(cursor, end, 12)) return output;
    return D2BitCMis(cursor);
  case FOURBIT:
    return D4BitMis(cursor);
  case TWOBITA:
    if (CheckForOverflow(cursor, end, 17)) return output;
    return D2BitAMis(cursor);
  case DIRECT:
    return DDirect(cursor);
  }
  return output;
} // END Decider

void Tokenizer(string program) {
  string command;
  PATTERNS curCommand;
  token curToken;
  token prevToken;
  // go over the program character by character
  for (auto it = program.begin(); it != program.end(); it++) {

    // get the next command as a string
    command.append(it, it + 3);

    // convert to an enumerable
    curCommand = StringBinaryToPattern(command);

    curToken = Decider(curCommand, it, program.end());
    
    // finished, padding will cause an error, leave before buffer overflow
    if (curToken.length == -1) break;

    // jump the itterator by the length field
    it = it + curToken.length;
    // negate the incomming increment
    it--;

    // clear the command string before next itteration
    command.clear();

    if (curToken.method != RLE) {

      // put the token on the preprocessor
      preProcessedOutput.push_back(curToken);
      prevToken = curToken;
      line++;

      // case for an RLE token
    } else {

      // add the last token RLE times
      for (int i = 0; i < curToken.rle + 1; i++) {
        preProcessedOutput.push_back(prevToken);
        line++;
      }
    }
  }
} // END Tokenizer

// #endregion

// #beginregion --- Decompression Function ---

// Decompresses the Binary using the global scoped fileInput variable
void DecompressBinary() {

  // import the dictionary
  ImportDictionary(dictImport);

  // turn the imported file into one large string
  string program = CombineFile(fileInput);

  // tokenize the string
  Tokenizer(program);

  // Make sure file has the correct name
  fileOutputName = decompressionOutput;
}

// #endregion

// #beginregion --- Token to File Functions ---

string CompileCompressedString(vector<token> input) {

  string output;

  // make one giant string
  for (auto binary : input) {
    output.append(binary.full);
  }

  return output;
} // END CompileCompressedString

string CompileDecompressedString(vector<token> input) {

  string output;

  // make one giant string
  for (auto binary : input) {
    output.append(binary.original);
  }

  return output;
} // END CompileDecompressedString

void AddDictionary() {
  fileOutput.push_back("xxxx");
  for (int i = 0; i < DICTIONARY_SIZE; i++) {
    fileOutput.push_back(dictionary[i]);
  }
} // END AddDictionary

// process for compiling the tokens to a file is the same
// only difference is the string field used
void CompileTokens() {
  // contain all tokens' full output as one string
  string rawOutput;

  if (mode == 1) {
    rawOutput = CompileCompressedString(preProcessedOutput);
  } else if (mode == 2) {
    rawOutput = CompileDecompressedString(preProcessedOutput);
  }

  // split giant string into 32-length chuncks

  // get the number of chuncks needed
  int substrings = rawOutput.length() / BINARY_SIZE;

  // split and feed the string into the file output vector
  for (int i = 0; i < substrings; i++) {
    fileOutput.push_back(rawOutput.substr(i * BINARY_SIZE, BINARY_SIZE));
  }

  // pad the last line with zeroes if needed
  if (rawOutput.length() % BINARY_SIZE != 0) {

    // get the remaining bits
    string last = rawOutput.substr(substrings * BINARY_SIZE);

    // pad with zeroes until it matches the length
    while (last.length() != BINARY_SIZE)
      last.append("0");

    // add the last line to the file output
    fileOutput.push_back(last);
  }

  // add the dictionary to the file output if in compressed mode
  if (mode == 1) AddDictionary();

} // END TokenToFile

// #endregion
