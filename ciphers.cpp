#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "include/caesar_dec.h"
#include "include/caesar_enc.h"
#include "include/subst_dec.h"
#include "include/subst_enc.h"
#include "utils.h"

using namespace std;

// Initialize random number generator in .cpp file for ODR reasons
std::mt19937 Random::rng;

const string ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

// Function declarations go at the top of the file so we can call them
// anywhere in our program, such as in main or in other functions.
// Most other function declarations are in the included header
// files.

// When you add a new helper function, make sure to declare it up here!
void decryptSubstFile(const QuadgramScorer& scorer);

/**
 * Print instructions for using the program.
 */
void printMenu();

int main() {
  Random::seed(time(NULL));
  string command;

  cout << "Welcome to Ciphers!" << endl;
  cout << "-------------------" << endl;
  cout << endl;

  // Load Dictionary
  vector<string> dictionary;
  ifstream dictfile("dictionary.txt");
  if (!dictfile) {
    cerr << "Failed to Open dictionary.txt" << endl;
  }
  string tempWord;
  while (dictfile >> tempWord) {
    dictionary.push_back(tempWord);
  }
  dictfile.close();

  // Parse English_quadgram.txt
  ifstream in("english_quadgrams.txt");
  if (!in) {
    cerr << "Failed to Open english_quadgrams" << endl;
  }
  // Setup for QuadgramScorer
  vector<string> quadgrams;
  vector<int> counts;
  // Row looks like this: TION,13168375
  string row;
  while (getline(in, row)) {
    if (row.empty()) {
      continue;
    }
    char comma = row.find(',');
    if (comma == string::npos) {
      continue;
    }
    string quad = row.substr(0, comma);
    int cnt = stoi(row.substr(comma + 1));
    quadgrams.push_back(quad);
    counts.push_back(cnt);
  }
  QuadgramScorer scorer(quadgrams, counts);
  do {
    printMenu();
    cout << endl << "Enter a command (case does not matter): ";

    // Use getline for all user input to avoid needing to handle
    // input buffer issues relating to using both >> and getline
    getline(cin, command);
    cout << endl;

    if (command == "R" || command == "r") {
      string seed_str;
      cout << "Enter a non-negative integer to seed the random number "
              "generator: ";
      getline(cin, seed_str);
      Random::seed(stoi(seed_str));
    } else if (command == "C" || command == "c") {
      caesarEncryptCommand();
    } else if (command == "D" || command == "d") {
      caesarDecryptCommand(dictionary);
    } else if (command == "A" || command == "a") {
      applyRandSubstCipherCommand();
    } else if (command == "E" || command == "e") {
      computeEnglishnessCommand(scorer);
    } else if (command == "S" || command == "s") {
      decryptSubstCipherCommand(scorer);
    } else if (command == "F" || command == "f") {
      decryptSubstFile(scorer);
    }
    cout << endl;
  } while (!(command == "x" || command == "X") && !cin.eof());

  return 0;
}

void printMenu() {
  cout << "Ciphers Menu" << endl;
  cout << "------------" << endl;
  cout << "C - Encrypt with Caesar Cipher" << endl;
  cout << "D - Decrypt Caesar Cipher" << endl;
  cout << "E - Compute English-ness Score" << endl;
  cout << "A - Apply Random Substitution Cipher" << endl;
  cout << "S - Decrypt Substitution Cipher from Console" << endl;
  cout << "F - Decrypt Substitution Cipher from File" << endl;
  cout << "R - Set Random Seed for Testing" << endl;
  cout << "X - Exit Program" << endl;
}

// "#pragma region" and "#pragma endregion" group related functions in this file
// to tell VSCode that these are "foldable". You might have noticed the little
// down arrow next to functions or loops, and that you can click it to collapse
// those bodies. This lets us do the same thing for arbitrary chunks!
#pragma region CaesarEnc

char rot(char c, int amount) {  // TODO:student
  int index = ALPHABET.find(c);
  int rotatedChar = (index + amount) % 26;
  return ALPHABET[rotatedChar];
}

string rot(const string& line, int amount) {
  // TODO: student
  string newLine;
  for (int i = 0; i < line.size(); i++) {
    if (isalpha(line[i])) {
      char upper = toupper(line[i]);
      newLine += rot(upper, amount);
    } else if (isspace(line[i])) {
      newLine += line[i];
    }
  }
  return newLine;
}

void caesarEncryptCommand() {
  // TODO: student
  string text;
  string amount_string;
  cout << "Enter the text to encrypt:" << endl;
  getline(cin, text);
  cout << "Enter the number of characters to rotate by:" << endl;
  getline(cin, amount_string);
  int amount = stoi(amount_string);
  string result = rot(text, amount);
  cout << result;
}

#pragma endregion CaesarEnc

#pragma region CaesarDec

void rot(vector<string>& strings, int amount) {  // TODO: student
  for (string& line : strings) {
    // Takes the string as vector and passes it to earlier function
    line = rot(line, amount);
  }
}

string clean(const string& s) {
  // TODO: student
  string result;
  for (char c : s) {
    if (isalpha(c)) {
      result += toupper(c);
    }
  }
  return result;
}

vector<string> splitBySpaces(const string& s) {
  // TODO: student

  vector<string> words;
  string tempWord;
  for (char c : s) {
    // check if character is a space, if it is, that means word is done
    if (isspace(c)) {
      /* check if the tempWord is not empty first, use pushback to words
      vector, then clear it for next word*/
      if (!tempWord.empty()) {
        words.push_back(tempWord);
        tempWord.clear();
      }
    } else {
      // if it is not a space, it must be a character for the word
      tempWord += c;
    }
  }
  // tempWord at end needs to be pushed incase of no spaces at end of string
  if (!tempWord.empty()) {
    words.push_back(tempWord);
  }
  return words;
}

string joinWithSpaces(const vector<string>& words) {
  // TODO: student
  string result;
  for (int i = 0; i < words.size(); i++) {
    if (i > 0) {
      // Want a space after 1st word
      result += " ";
    }
    result += words[i];
  }
  return result;
}

int numWordsIn(const vector<string>& words, const vector<string>& dict) {
  // TODO: student
  int count = 0;
  for (const string& w : words) {
    for (const string& d : dict) {
      if (w == d) {
        count++;
        break;
      }
    }
  }
  return count;
}

void caesarDecryptCommand(const vector<string>& dict) {
  // TODO: student

  string textInput;
  cout << "Enter the text to Caesar decrypt:" << endl;
  getline(cin, textInput);

  // Takes the textInput and passes it through the split by spaces THEN
  // clean
  vector<string> words = splitBySpaces(textInput);
  for (string& cleaned : words) {
    cleaned = clean(cleaned);
  }

  bool found = false;
  // i represents the 'amount' for shifting
  for (int i = 0; i < 26; i++) {
    vector<string> shiftedWords = words;
    /* rot passes it through to char rot which does the shifts, returning it
       back here */
    rot(shiftedWords, i);
    int numMatches = numWordsIn(shiftedWords, dict);
    // 14(numMatches) * 2 (total 28) > 30(size) || LESS THAN HALF
    // 15(numMatches) * 2 (total 30) > 30(size) || EQUAL STILL FAILS
    // 16(numMatches) * 2 (total 32) > 30(size) || MORE THAN HALF

    if (numMatches * 2 > (int)shiftedWords.size()) {
      cout << joinWithSpaces(shiftedWords) << endl;
      found = true;
    }
  }
  if (!found) {
    cout << "No good decryptions found" << endl;
  }
}
#pragma endregion CaesarDec

#pragma region SubstEnc

string applySubstCipher(const vector<char>& cipher, const string& s) {
  // TODO: student
  string result;
  for (char c : s) {
    if (isalpha(c)) {
      char up = toupper(c);
      result.push_back(cipher[up - 'A']);
    } else {
      result.push_back(c);
    }
  }
  return result;
}

void applyRandSubstCipherCommand() {
  // TODO: student
  string textInput;
  cout << "Enter the text to Random Substitution encrypt:" << endl;
  getline(cin, textInput);
  vector<char> cipher = genRandomSubstCipher();
  string encryptedInput = applySubstCipher(cipher, textInput);
  cout << encryptedInput << endl;
}

#pragma endregion SubstEnc

#pragma region SubstDec

double scoreString(const QuadgramScorer& scorer, const string& s) {
  // TODO: student
  string filteredString;
  /* This part might be unnecessary, leaving it since I had trouble with it
    last time */
  for (char c : s) {
    if (isalpha(c)) {
      filteredString.push_back(toupper(c));
    }
  }
  double score = 0.0;

  // Incase string is less than 4
  if (filteredString.size() < 4) {
    return score;
  }
  // Gets the score
  for (int i = 0; i + 4 <= filteredString.size(); i++) {
    string chunk = filteredString.substr(i, 4);
    score += scorer.getScore(chunk);
  }
  return score;
}

void computeEnglishnessCommand(const QuadgramScorer& scorer) {
  // TODO: student
  string line;
  cout << "Enter a string for englishness scoring:" << endl;
  getline(cin, line);
  double score = scoreString(scorer, line);
  cout << score << endl;
}

vector<char> hillClimb(const QuadgramScorer& scorer, const string& ciphertext) {
  // TODO: student
  // Plaintext to Cipher
  vector<char> plainTocipher = genRandomSubstCipher();
  vector<char> key(26);
  for (int i = 0; i < 26; i++) {
    key[plainTocipher[i] - 'A'] = 'A' + i;
  }
  string plainText = applySubstCipher(key, ciphertext);
  double bestScore = scoreString(scorer, plainText);

  int noChange = 0;
  while (noChange < 1000) {
    int A = Random::randInt(25);
    int B = Random::randInt(25);
    // Reroll if same, only reroll B
    while (B == A) {
      B = Random::randInt(25);
    }
    vector<char> bestChance = key;
    swap(bestChance[A], bestChance[B]);
    string bestPlain = applySubstCipher(bestChance, ciphertext);
    double finalScore = scoreString(scorer, bestPlain);

    if (finalScore > bestScore) {
      key = bestChance;
      bestScore = finalScore;
      // Reset Counter
      noChange = 0;
    } else {
      noChange++;
    }
  }
  return key;
}

vector<char> decryptSubstCipher(const QuadgramScorer& scorer,
                                const string& ciphertext) {
  // TODO: student
  vector<char> bestKey;
  double bestScore = 0;
  for (int i = 0; i < 25; i++) {
    vector<char> key = hillClimb(scorer, ciphertext);
    string plaintext = applySubstCipher(key, ciphertext);
    double score = scoreString(scorer, plaintext);

    if (i == 0 || score > bestScore) {
      bestScore = score;
      bestKey = key;
    }
  }
  return bestKey;
}

void decryptSubstCipherCommand(const QuadgramScorer& scorer) {
  // TODO: student
  string ciphertext;
  cout << "Enter cipher text for Decrypting Substituion Cipher: " << endl;
  getline(cin, ciphertext);
  vector<char> key = decryptSubstCipher(scorer, ciphertext);
  string plaintext = applySubstCipher(key, ciphertext);
  cout << plaintext << endl;
}

#pragma endregion SubstDec

// Function to implement Decrypt Substitution Cipher(File)
void decryptSubstFile(const QuadgramScorer& scorer) {
  string inputFile;
  string outputFile;
  cout << "Enter Input File Name:" << endl;
  getline(cin, inputFile);
  cout << "Enter Output File Name:" << endl;
  getline(cin, outputFile);
  cout << "Recieved...Please Wait..." << endl;

  // Read file (like dictionary)
  string ciphertext;
  string line;
  ifstream infile(inputFile);
  while (getline(infile, line)) {
    ciphertext += line;
    ciphertext += '\n';
    //'\n'is needed so that it doesn't become one big sentence
  }
  infile.close();

  // Decrypt's the content use previous functions
  vector<char> key = decryptSubstCipher(scorer, ciphertext);
  string plaintext = applySubstCipher(key, ciphertext);

  // Outputs the file
  ofstream outfile(outputFile);
  outfile << plaintext;
  outfile.close();
  cout << "...Done! Thanks For Waiting." << endl;
  cout << "Decrypted Text Written to: " << outputFile << endl;
}