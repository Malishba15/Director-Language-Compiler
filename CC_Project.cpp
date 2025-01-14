#include <iostream>
#include <fstream>
#include <cctype>  // For character handling (e.g. isdigit, isalpha...)
#include <string>
#include <iomanip> //controlling input and output formatting 
#include <vector>  //using dynamic arrays to store and manage data
#include <sstream>
#include <set> // For unique line tracking
#include <unordered_map>
#include <list>
#include <iterator> // working with iterators std::next
#include <algorithm> //common algorithms such as find sort etc
#include <cmath>

using namespace std;


const int MAX_SIZE = 1000;

// Define global arrays and variables
const int MAX_ENTRIES = 1000; // Maximum number of symbol table entries
string names[MAX_ENTRIES];
string types[MAX_ENTRIES];
string values[MAX_ENTRIES]; // To store values of identifiers
int entryCount = 0;
struct Symbol {
    string name;
    string type;
    int sizeInBytes;
    int lineOfDeclaration;
};

enum TokenType {
    KEYWORD, IDENTIFIER, OPERATOR, PUNCTUATOR, LITERAL, INTEGER, FLOAT, CONSTANT, INVALID, END
};

struct Token {
    string lexeme;
    TokenType type;
    int lineNumber; // Line number where the token was found
};
bool Semantic_Check = false;
//Lexical Analyser

string DFASimulator(const string& lexeme) {
    const string keywords[] = { "scene", "frame", "script", "repeat", "cut", "cutif", "show", "prompt", "fade", "times" };
    const string operators[] = { "join", "trim", "amplify", "split", "splice", "assign", "or", "and", "not", "is", "isnt", "is greater", "is less", "is greater or equal", "is less or equal" };
    const string punctuators[] = { "{", "}", "(", ")", ";" };

    for (const string& keyword : keywords) {
        if (lexeme == keyword) return "KEYWORD";
    }
    for (const string& op : operators) {
        if (lexeme == op) return "OPERATOR";
    }
    for (const string& punct : punctuators) {
        if (lexeme == punct) return "PUNCTUATOR";
    }
    if (lexeme.front() == '"' && lexeme.back() == '"') return "LITERAL";

    if (isalpha(lexeme[0]) || lexeme[0] == '_') {
        for (char ch : lexeme) {
            if (!isalnum(ch) && ch != '_') return "INVALID";
        }
        return "IDENTIFIER";
    }

    bool hasDecimal = false;
    if (isdigit(lexeme[0]) || lexeme[0] == '.') {
        for (char ch : lexeme) {
            if (ch == '.') {
                if (hasDecimal) return "INVALID";
                hasDecimal = true;
            }
            else if (!isdigit(ch)) return "INVALID";
        }
        return hasDecimal ? "FLOAT" : "INTEGER";
    }

    return "INVALID";
}

int getSizeInBytes(const string& type) {
    if (type == "scene" || type == "INTEGER") return 4;
    if (type == "frame" || type == "FLOAT") return 4;
    if (type == "script" || type == "LITERAL") return 100;
    return 0;
}

void writeSymbolTable(Symbol symbolTable[], int symbolCount) {
    ofstream symFile("SymbolTable.txt");
    symFile << left << setw(30) << "Name" << setw(30) << "Type" << setw(30) << "Size(Bytes)" << setw(30) << "Line of Declaration" << "\n";
    symFile << string(120, '-') << "\n";

    for (int i = 0; i < symbolCount; ++i) {
        symFile << left << setw(30) << symbolTable[i].name << setw(30) << symbolTable[i].type << setw(30) << symbolTable[i].sizeInBytes << setw(30) << symbolTable[i].lineOfDeclaration << "\n";
    }
    symFile.close();
}

void writeTokens(string tokens[], int tokenCount) {
    ofstream tokenFile("Output.txt");
    tokenFile << "Tokens:\n";
    for (int i = 0; i < tokenCount; ++i) {
        tokenFile << tokens[i] << "\n";
    }
    tokenFile.close();
}

void lexicalAnalyzer(const string& inputFile) {
    ifstream file(inputFile);
    if (!file.is_open()) {
        cerr << "Error: Unable to open file.\n";
        return;
    }

    Symbol symbolTable[MAX_SIZE];
    string tokens[MAX_SIZE];
    int symbolCount = 0;
    int tokenCount = 0;
    int lineNumber = 1;

    string lexeme = "";
    char currentChar;
    bool inLiteral = false;
    bool inMultilineComment = false;

    char* filePtr = new char[MAX_SIZE];
    file.read(filePtr, MAX_SIZE);

    for (char* ptr = filePtr; ptr < filePtr + file.gcount(); ++ptr) {
        currentChar = *ptr;

        if (inMultilineComment) {
            if (currentChar == '*' && *(ptr + 1) == '#') {
                inMultilineComment = false;
                ptr++;
            }
            if (*ptr == '\n') {
                lineNumber++;
            }
            continue;
        }

        if (currentChar == '#' && *(ptr + 1) == '*') {
            inMultilineComment = true;
            ptr++;
            continue;
        }

        if (currentChar == '#' && *(ptr + 1) == '#') {
            while (*ptr != '\n' && *ptr != '\0') {
                ptr++;
            }
            lineNumber++;
            tokens[tokenCount++] = "EOL";
            continue;
        }

        if (currentChar == '\n') {
            lineNumber++;
            if (!lexeme.empty()) {
                string type = DFASimulator(lexeme);
                if (type != "INVALID") {
                    tokens[tokenCount++] = lexeme;
                    symbolTable[symbolCount++] = { lexeme, type, getSizeInBytes(type), lineNumber };
                }
                lexeme = "";
            }
            tokens[tokenCount++] = "EOL";  // Ensure EOL token at end of line
            continue;
        }

        if (currentChar == '"') {
            if (inLiteral) {
                lexeme += currentChar;
                tokens[tokenCount++] = lexeme;
                symbolTable[symbolCount++] = { lexeme, "LITERAL", getSizeInBytes("LITERAL"), lineNumber };
                lexeme = "";
                inLiteral = false;
            }
            else {
                if (!lexeme.empty()) {
                    string type = DFASimulator(lexeme);
                    if (type != "INVALID") {
                        tokens[tokenCount++] = lexeme;
                        symbolTable[symbolCount++] = { lexeme, type, getSizeInBytes(type), lineNumber };
                    }
                    lexeme = "";
                }
                inLiteral = true;
                lexeme += currentChar;
            }
            continue;
        }

        if (inLiteral) {
            lexeme += currentChar;
            continue;
        }
        //......

        // Handle multi-word operators
        if (isspace(currentChar)) {
            if (!lexeme.empty()) {
                string combinedLexeme = lexeme;
                char* nextPtr = ptr + 1;
                string nextWord = "", thirdWord = "", fourthWord = "";

                // Skip spaces to check the next word
                while (isspace(*nextPtr)) nextPtr++;
                while (*nextPtr && !isspace(*nextPtr) && !ispunct(*nextPtr) && *nextPtr != '\n') {
                    nextWord += *nextPtr;
                    nextPtr++;
                }

                // Combine current lexeme with the next word
                combinedLexeme += " " + nextWord;

                // Check for a three-part operator
                char* thirdPtr = nextPtr;
                while (isspace(*thirdPtr)) thirdPtr++;
                while (*thirdPtr && !isspace(*thirdPtr) && !ispunct(*thirdPtr) && *thirdPtr != '\n') {
                    thirdWord += *thirdPtr;
                    thirdPtr++;
                }

                // Combine to form a potential three-word operator
                string threeWordLexeme = combinedLexeme + " " + thirdWord;

                // Check for a four-part operator
                char* fourthPtr = thirdPtr;
                while (isspace(*fourthPtr)) fourthPtr++;
                while (*fourthPtr && !isspace(*fourthPtr) && !ispunct(*fourthPtr) && *fourthPtr != '\n') {
                    fourthWord += *fourthPtr;
                    fourthPtr++;
                }

                string fourWordLexeme = threeWordLexeme + " " + fourthWord;

                // Prioritize matching the longest valid operator
                if (DFASimulator(fourWordLexeme) == "OPERATOR") {
                    lexeme = fourWordLexeme;
                    ptr = fourthPtr - 1; // Skip processed characters
                }
                else if (DFASimulator(threeWordLexeme) == "OPERATOR") {
                    lexeme = threeWordLexeme;
                    ptr = thirdPtr - 1; // Skip processed characters
                }
                else if (DFASimulator(combinedLexeme) == "OPERATOR") {
                    lexeme = combinedLexeme;
                    ptr = nextPtr - 1; // Skip processed characters
                }

                // Tokenize the lexeme
                string type = DFASimulator(lexeme);
                if (type != "INVALID") {
                    tokens[tokenCount++] = lexeme;
                    symbolTable[symbolCount++] = { lexeme, type, getSizeInBytes(type), lineNumber };
                }
                lexeme = "";
            }

            // Handle single-character punctuators (unchanged)
            if (ispunct(currentChar)) {
                lexeme += currentChar;
                string type = DFASimulator(lexeme);
                tokens[tokenCount++] = lexeme;
                symbolTable[symbolCount++] = { lexeme, type, getSizeInBytes(type), lineNumber };
                lexeme = "";
            }
            continue;
        }

        //....    

        if (isspace(currentChar)) {
            if (!lexeme.empty()) {
                string type = DFASimulator(lexeme);
                if (type != "INVALID") {
                    tokens[tokenCount++] = lexeme;
                    symbolTable[symbolCount++] = { lexeme, type, getSizeInBytes(type), lineNumber };
                }
                lexeme = "";
            }
            continue;
        }

        if (currentChar == '!' || currentChar == '<' || currentChar == '>' || currentChar == '{' || currentChar == '}' || currentChar == '(' || currentChar == ')' || currentChar == ';') {
            if (!lexeme.empty()) {
                string type = DFASimulator(lexeme);
                if (type != "INVALID") {
                    tokens[tokenCount++] = lexeme;
                    symbolTable[symbolCount++] = { lexeme, type, getSizeInBytes(type), lineNumber };
                }
                lexeme = "";
            }
            lexeme += currentChar;
            string type = DFASimulator(lexeme);
            tokens[tokenCount++] = lexeme;
            symbolTable[symbolCount++] = { lexeme, type, getSizeInBytes(type), lineNumber };
            lexeme = "";
            continue;
        }

        lexeme += currentChar;
    }

    if (!lexeme.empty()) {
        string type = DFASimulator(lexeme);
        if (type != "INVALID") {
            tokens[tokenCount++] = lexeme;
            symbolTable[symbolCount++] = { lexeme, type, getSizeInBytes(type), lineNumber };
        }
    }

    delete[] filePtr;
    file.close();

    writeSymbolTable(symbolTable, symbolCount);
    writeTokens(tokens, tokenCount);
}

//Syntax Analyser

// Function prototypes
bool parseStatement();
bool parseDeclaration();
bool parseConditional();
bool parseLoop();
bool parseIO();
void parseProgram();
Token getNextToken();
void resetTokenIndex();
bool loadTokensFromFile(const string& filename);
void logError(const string& errorMsg, int lineNumber);
string getLineOfDeclaration(int lineNumber);
// Global variables
vector<Token> tokens;
int currentTokenIndex = 0;

TokenType stringToTokenType(const string& typeStr) {
    if (typeStr == "KEYWORD") return KEYWORD;
    if (typeStr == "IDENTIFIER") return IDENTIFIER;
    if (typeStr == "OPERATOR") return OPERATOR;
    if (typeStr == "PUNCTUATOR") return PUNCTUATOR;
    if (typeStr == "LITERAL") return LITERAL;
    if (typeStr == "INTEGER") return INTEGER;
    if (typeStr == "FLOAT") return FLOAT;
    if (typeStr == "CONSTANT") return CONSTANT;
    return INVALID;
}

bool loadTokensFromFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open " << filename << endl;
        return false;
    }

    bool headerSkipped = false;
    string line;
    int lineNumber = 1; // Track current line number
    while (getline(file, line)) {
        if (!headerSkipped) {
            if (line.find("Line of Declaration") != string::npos) {
                headerSkipped = true; // Header detected
            }
            continue;
        }

        if (line.find("---") != string::npos) {
            continue; // Skip separator line
        }

        istringstream iss(line);
        string lexeme, typeStr;


        // Check if the line contains a string literal (detects quotes for multi-word literals)
        size_t quoteStart = line.find("\"");
        size_t quoteEnd = line.rfind("\"");

        if (quoteStart != string::npos && quoteEnd != string::npos && quoteEnd > quoteStart) {
            // Extract the full literal (including spaces)
            lexeme = line.substr(quoteStart, quoteEnd - quoteStart + 1);  // includes quotes

            // The token type for literals is LITERAL, so set it directly.
            typeStr = "LITERAL";  // Hardcoded as "LITERAL" because it's clear the token is a literal.

        }
        else {
            // Handle normal tokens (non-literals)
            if (!(iss >> lexeme >> typeStr)) {
                cerr << "Error: Invalid format in " << filename << " on line " << lineNumber << endl;
                return false;
            }
        }


        TokenType type = stringToTokenType(typeStr);
        tokens.push_back({ lexeme, type, lineNumber });
        lineNumber++; // Increment for each line
    }

    file.close();
    return true;
}

string getLineOfDeclaration(int lineNumber) {
    ifstream symbolTable("SymbolTable.txt");
    if (!symbolTable.is_open()) {
        cerr << "Error: Could not open SymbolTable.txt for reading." << endl;
        return "";
    }

    string line;
    int lineCount = 1;

    while (getline(symbolTable, line)) {
        if (line.find("---") != string::npos) {
            continue; // Skip separator line
        }

        istringstream iss(line);
        string name, type, size, lineOfDecl;
        if (!(iss >> name >> type >> size >> lineOfDecl)) {
            cerr << "Error: Invalid format in SymbolTable.txt" << endl;
            return "";
        }

        if (lineCount == lineNumber) {
            symbolTable.close();
            return lineOfDecl;
        }
        lineCount++;
    }

    symbolTable.close();
    return "Line of Declaration not found";
}
/////////////////////////////////////Updated Symbol Table///////////////////////////////////////////////
// Node structure for the linked list
struct Node {
    string line;
    Node* next;

    Node(const string& line) : line(line), next(nullptr) {}
};

// Linked list class to handle operations
class LinkedList {
private:
    Node* head;
    Node* tail;

public:
    LinkedList() : head(nullptr), tail(nullptr) {}

    // Add a new line to the list if it's not already present
    void add(const string& line) {
        if (!exists(line)) {
            Node* newNode = new Node(line);
            if (!head) {
                head = tail = newNode;
            }
            else {
                tail->next = newNode;
                tail = newNode;
            }
        }
    }

    // Check if a line already exists in the list
    bool exists(const string& line) {
        Node* current = head;
        while (current) {
            if (current->line == line) {
                return true;
            }
            current = current->next;
        }
        return false;
    }

    // Convert the linked list to a comma-separated string
    string toString() const {
        string result;
        Node* current = head;
        while (current) {
            if (!result.empty()) {
                result += ", ";
            }
            result += current->line;
            current = current->next;
        }
        return result.empty() ? "-----" : result;
    }

    // Destructor to clean up memory
    ~LinkedList() {
        Node* current = head;
        while (current) {
            Node* temp = current;
            current = current->next;
            delete temp;
        }
    }
};

// Function to read and count the entries in the symbol table
int countEntries(ifstream& file) {
    string line;
    int count = 0;
    while (getline(file, line)) {
        if (line.find("---") != string::npos) {
            continue; // Skip separator line
        }
        count++;
    }
    return count;
}

// Function to parse the symbol table file into arrays
void parseSymbolTable(ifstream& file, int entryCount, string* names, string* types, string* sizes, string* lineOfDecls) {
    string line;
    int index = 0;
    file.clear();
    file.seekg(0);
    bool headerSkipped = false;
    while (getline(file, line) && index < entryCount) {
        if (!headerSkipped) {
            if (line.find("Line of Declaration") != string::npos) {
                headerSkipped = true; // Header detected
            }
            continue;
        }

        if (line.find("---") != string::npos) {
            continue; // Skip separator line
        }

        istringstream iss(line);
        string name, type, size, lineOfDecl;

        if (line[0] == '"') { // Handle quoted strings
            size_t closingQuote = line.find('"', 1);
            if (closingQuote != string::npos) {
                name = line.substr(1, closingQuote - 1);
                line = line.substr(closingQuote + 1);
                iss.clear();
                iss.str(line);
            }
            else {
                cerr << "Error: Mismatched quotes in SymbolTable.txt" << endl;
                return;
            }
        }
        else {
            iss >> name;
            // Check for multi-word operators
            if (name == "is" || name == "isnt") {
                string nextWord;
                while (iss >> nextWord) {
                    // Only consider valid combinations of multi-word operators
                    if (nextWord == "less" || nextWord == "greater" || nextWord == "or" || nextWord == "equal") {
                        name += " " + nextWord;
                    }
                    else {
                        iss.seekg(-static_cast<int>(nextWord.size()), ios::cur);
                        break;
                    }
                }
            }
        }

        iss >> type >> size >> lineOfDecl;

        names[index] = name;
        types[index] = type;
        sizes[index] = size;
        lineOfDecls[index] = lineOfDecl;

        index++;
    }
}

// Function to build usage linked lists for IDENTIFIER tokens
void buildUsageLists(int entryCount, string* names, string* types, string* lineOfDecls, LinkedList* usageLists) {
    for (int i = 0; i < entryCount; i++) {
        if (types[i] == "IDENTIFIER") { // Only handle IDENTIFIER tokens
            for (int j = 0; j < entryCount; j++) {
                if (names[i] == names[j]) {
                    usageLists[i].add(lineOfDecls[j]); // Add line to linked list
                }
            }
        }
    }
    //array for values of the identifiers 
    string* values = new string[entryCount];
    for (int i = 0; i < entryCount; i++)
    {
        if (types[i] == "IDENTIFIER") {
            values[i] = "required";
        }


    }
}

// Function to write the updated symbol table to a file
void writeUpdatedSymbolTable(
    int entryCount,
    string* names,
    string* types,
    string* sizes,
    string* lineOfDecls,
    LinkedList* usageLists,
    const string& outputFileName) {

    ofstream updatedSymbolTable(outputFileName);
    if (!updatedSymbolTable.is_open()) {
        cerr << "Error: Could not open " << outputFileName << " for writing." << endl;
        return;
    }

    updatedSymbolTable << left;
    updatedSymbolTable << setw(20) << "Name"
        << setw(15) << "Type"
        << setw(10) << "Size"
        << setw(30) << "LineOfDecl"
        << setw(50) << "Line of Usage"
        << "\n";
    updatedSymbolTable << "--------------------------------------------------------------------------------------\n";
    for (int i = 0; i < entryCount; i++) {
        // Add quotes around literals
        string formattedName = names[i];
        if (types[i] == "LITERAL") {
            formattedName = "\"" + formattedName + "\"";
        }

        updatedSymbolTable << setw(20) << formattedName
            << setw(15) << types[i]
            << setw(10) << sizes[i]
            << setw(30) << lineOfDecls[i]
            << setw(50) << usageLists[i].toString() << endl;
    }

    updatedSymbolTable.close();
    cout << "Updated symbol table written to " << outputFileName << " successfully." << endl;
}

void logError(const string& errorMsg, int lineNumber) {
    string lineOfDeclaration = getLineOfDeclaration(lineNumber);
    ofstream errorFile("Error.txt", ios::app);
    if (errorFile.is_open()) {
        errorFile << "Line " << lineOfDeclaration << ": " << errorMsg << endl;
        errorFile.close();
    }
    else {
        cerr << "Error: Could not open Error.txt for writing." << endl;
    }
}

Token getNextToken() {
    if (currentTokenIndex < tokens.size()) {
        return tokens[currentTokenIndex++];
    }
    return { "", END };
}
void resetTokenIndex() {
    currentTokenIndex = 0;
}
bool parseDeclaration() {
    Token token = getNextToken();

    if (token.lexeme == "script") {
        token = getNextToken();
        if (token.type == IDENTIFIER) {
            // Expect 'assign' operator
            token = getNextToken();
            if (token.lexeme == "assign") {
                // Expect an integer or float
                token = getNextToken();
                if (token.type == LITERAL) {
                    token = getNextToken();
                }
                else {
                    logError("Expected a LITERAL ", token.lineNumber);
                    return false;
                }
                // Expect semicolon at the end
                if (token.lexeme == ";") {
                    return true;
                }
                else {
                    logError("Missing semicolon after declaration", token.lineNumber);
                }
            }
        }
    }
    if (token.lexeme == "scene" || token.lexeme == "frame") {
        // Expect an identifier
        token = getNextToken();
        if (token.type == IDENTIFIER) {
            // Expect 'assign' operator
            token = getNextToken();
            if (token.lexeme == "assign") {
                // Expect an integer or float
                token = getNextToken();
                if (token.type == INTEGER || token.type == FLOAT) {
                    // Check for optional arithmetic operation
                    token = getNextToken();
                    if (token.lexeme == "join" || token.lexeme == "trim" ||
                        token.lexeme == "splice" || token.lexeme == "amplify" ||
                        token.lexeme == "split") {

                        // Expect another integer or float
                        token = getNextToken();
                        if (token.type != INTEGER && token.type != FLOAT) {
                            logError("Expected a number after '" + token.lexeme + "'", token.lineNumber);
                            return false;
                        }
                        token = getNextToken();
                    }

                    // Expect semicolon at the end
                    if (token.lexeme == ";") {
                        return true;
                    }
                    else {
                        logError("Missing semicolon after declaration", token.lineNumber);
                    }
                }
                else {
                    logError("Expected a number after 'assign'", token.lineNumber);
                }
            }
            else {
                logError("Missing 'assign' in declaration", token.lineNumber);
            }
        }
        else {
            logError("Expected identifier after keyword", token.lineNumber);
        }
    }
    return false;
}
bool cutEncountered = false; // Global or contextual parsing state
bool parseConditional() {


    Token token = getNextToken();

    if (token.lexeme == "cut") {
        cutEncountered = true; // Mark that 'cut' has been encountered
        token = getNextToken();

        // Check for `cut` syntax
        if (token.lexeme == "(") {
            token = getNextToken();

            // Check for an identifier
            if (token.type == IDENTIFIER) {
                token = getNextToken();

                // Check for a valid conditional operator
                if (token.lexeme == "is" || token.lexeme == "isnt" || token.lexeme == "is less" || token.lexeme == "is greater" ||
                    token.lexeme == "is less or equal" || token.lexeme == "is greater or equal") {
                    token = getNextToken();

                    // Check for a number (INTEGER, FLOAT, or IDENTIFIER)
                    if (token.type == INTEGER || token.type == FLOAT || token.type == IDENTIFIER) {
                        token = getNextToken();

                        // Check for closing parenthesis
                        if (token.lexeme == ")") {
                            token = getNextToken();

                            // Check for opening curly brace
                            if (token.lexeme == "{") {
                                // Parse the block inside the curly braces
                                while (true) {
                                    token = tokens[currentTokenIndex];

                                    // Check for the closing curly brace
                                    if (token.lexeme == "}") {
                                        getNextToken(); // Consume the closing curly brace
                                        return true;
                                    }

                                    // Parse the statements inside the block
                                    if (!parseStatement()) {
                                        return false;
                                    }
                                }
                            }
                            else {
                                logError("Missing '{' after condition.", token.lineNumber);
                            }
                        }
                        else {
                            logError("Missing ')' in condition.", token.lineNumber);
                        }
                    }
                    else {
                        logError("Invalid number in condition.", token.lineNumber);
                    }
                }
                else {
                    logError("Invalid conditional operator in condition.", token.lineNumber);
                }
            }
            else {
                logError("Expected identifier in condition.", token.lineNumber);
            }
        }
        else {
            logError("Missing '(' after 'cut'.", token.lineNumber);
        }
    }
    else if (token.lexeme == "cutif") {
        // Enforce that 'cutif' can only be used after 'cut'
        if (!cutEncountered) {
            logError("'cutif' cannot be used without a preceding 'cut'.", token.lineNumber);
            return false;
        }

        token = getNextToken();

        // Check for a conditional block similar to `cut`
        if (token.lexeme == "(") {
            token = getNextToken();

            // Same checks as 'cut'
            if (token.type == IDENTIFIER) {
                token = getNextToken();

                if (token.lexeme == "is" || token.lexeme == "isnt" || token.lexeme == "is less" || token.lexeme == "is greater" ||
                    token.lexeme == "is less or equal" || token.lexeme == "is greater or equal") {
                    token = getNextToken();

                    if (token.type == INTEGER || token.type == FLOAT || token.type == IDENTIFIER) {
                        token = getNextToken();

                        if (token.lexeme == ")") {
                            token = getNextToken();

                            if (token.lexeme == "{") {
                                while (true) {
                                    token = tokens[currentTokenIndex];
                                    if (token.lexeme == "}") {
                                        getNextToken();
                                        return true;
                                    }

                                    if (!parseStatement()) {
                                        return false;
                                    }
                                }
                            }
                            else {
                                logError("Missing '{' after 'cutif' condition.", token.lineNumber);
                            }
                        }
                        else {
                            logError("Missing ')' in 'cutif' condition.", token.lineNumber);
                        }
                    }
                    else {
                        logError("Invalid number in 'cutif' condition.", token.lineNumber);
                    }
                }
                else {
                    logError("Invalid conditional operator in 'cutif' condition.", token.lineNumber);
                }
            }
            else {
                logError("Expected identifier in 'cutif' condition.", token.lineNumber);
            }
        }
        else {
            logError("Missing '(' after 'cutif'.", token.lineNumber);
        }
    }
    else if (token.lexeme == "fade") {
        // Enforce that 'fade' can only be used after 'cut'
        if (!cutEncountered) {
            logError("'fade' cannot be used without a preceding 'cut'.", token.lineNumber);
            return false;
        }

        token = getNextToken();

        // Check for opening curly brace
        if (token.lexeme == "{") {
            while (true) {
                token = tokens[currentTokenIndex];

                // Check for the closing curly brace
                if (token.lexeme == "}") {
                    getNextToken(); // Consume the closing curly brace
                    return true;
                }

                // Parse the statements inside the block
                if (!parseStatement()) {
                    return false;
                }
            }
        }
        else {
            logError("Missing '{' after 'fade'.", token.lineNumber);
        }
    }
    else {
        logError("Unexpected token in conditional statement.", token.lineNumber);
    }

    return false;
}
bool parseLoop() {
    Token token = getNextToken();

    if (token.lexeme == "repeat") {
        token = getNextToken();

        // Check for opening parenthesis
        if (token.lexeme == "(") {
            token = getNextToken();

            // Validate condition (identifier and conditional operator)
            if (token.type == INTEGER) {
                token = getNextToken();

                if (token.lexeme == "times") {
                    token = getNextToken();

                    // Check for closing parenthesis
                    if (token.lexeme == ")") {
                        token = getNextToken();

                        // Check for opening curly brace
                        if (token.lexeme == "{") {
                            // Parse the block inside the curly braces
                            while (true) {
                                token = tokens[currentTokenIndex];

                                // Check for the closing curly brace
                                if (token.lexeme == "}") {
                                    getNextToken(); // Consume the closing curly brace
                                    return true;
                                }

                                // Parse the statements inside the block
                                if (!parseStatement()) {
                                    return false;
                                }
                            }
                        }
                        else {
                            logError("Missing '{' after repeat condition.", token.lineNumber);
                        }
                    }
                    else {
                        logError("Missing ')' in repeat condition.", token.lineNumber);
                    }
                }
                else {
                    logError("Invalid condition in repeat condition.", token.lineNumber);
                }
            }
            else {
                logError("Invalid Integer in repeat condition.", token.lineNumber);
            }
        }

        else {
            logError("Missing '(' after 'repeat'.", token.lineNumber);
        }
    }

    return false;
}

bool parseIO() {
    Token token = getNextToken();
    if (token.lexeme == "show" || token.lexeme == "prompt") {
        token = getNextToken();
        if (token.type == LITERAL || token.type == IDENTIFIER) {
            token = getNextToken();
            if (token.lexeme == ";") {
                return true;
            }
        }
        else {
            logError("Error: Invalid argument for IO statement", token.lineNumber);
            //cout << "Error: Invalid argument for IO statement.\n";
        }
    }
    return false;
}

bool parseIdentifier() {
    Token token = getNextToken();
    if (token.type == IDENTIFIER) {
        token = getNextToken();
        if (token.lexeme == "assign") {
            token = getNextToken();
            if (token.type == INTEGER || token.type == FLOAT || token.type == IDENTIFIER) {
                // Check for optional arithmetic operation
                token = getNextToken();
                if (token.lexeme == "join" || token.lexeme == "trim" ||
                    token.lexeme == "splice" || token.lexeme == "amplify" ||
                    token.lexeme == "split") {

                    // Expect another integer or float
                    token = getNextToken();
                    if (token.type != INTEGER && token.type != FLOAT) {
                        logError("Expected a number after '" + token.lexeme + "'", token.lineNumber);
                        return false;
                    }
                    token = getNextToken();
                }


                if (token.lexeme == ";") {
                    return true;
                }
            }
            else {
                logError("Error: Invalid identifier statement", token.lineNumber);
            }
        }
    }
    return false;
}

bool parseStatement() {
    Token token = tokens[currentTokenIndex];
    if (token.lexeme == "scene" || token.lexeme == "frame" || token.lexeme == "script") {
        return parseDeclaration();
    }
    else if (token.lexeme == "cut" || token.lexeme == "cutif" || token.lexeme == "fade") {
        return parseConditional();
    }
    else if (token.lexeme == "repeat") {
        return parseLoop();
    }
    else if (token.lexeme == "show" || token.lexeme == "prompt") {
        return parseIO();
    }
    else if (token.type == IDENTIFIER) {
        return parseIdentifier();
    }
    else {
        logError("Error: Unknown statement.", token.lineNumber);
    }
    return false;
}


void parseProgram() {
    resetTokenIndex();
    bool errorEncountered = false; // Flag to track if any syntax error occurs.

    while (currentTokenIndex < tokens.size()) {
        if (!parseStatement()) {
            // cout << "Syntax error encountered at token index " << currentTokenIndex << ".\n";
            errorEncountered = true;

            currentTokenIndex++;
        }
    }
    if (!errorEncountered) {
        cout << "Syntax analysis completed successfully.\n";
        const string inputFileName = "SymbolTable.txt";
        const string outputFileName = "UpdatedSymbolTable.txt";

        ifstream symbolTable(inputFileName);
        if (!symbolTable.is_open()) {
            cerr << "Error: Could not open " << inputFileName << " for reading." << endl;
        }

        int entryCount = countEntries(symbolTable);

        string* names = new string[entryCount];
        string* types = new string[entryCount];
        string* sizes = new string[entryCount];
        string* lineOfDecls = new string[entryCount];
        LinkedList* usageLists = new LinkedList[entryCount];

        parseSymbolTable(symbolTable, entryCount, names, types, sizes, lineOfDecls);

        buildUsageLists(entryCount, names, types, lineOfDecls, usageLists);

        writeUpdatedSymbolTable(entryCount, names, types, sizes, lineOfDecls, usageLists, outputFileName);

        delete[] names;
        delete[] types;
        delete[] sizes;
        delete[] lineOfDecls;
        delete[] usageLists;

        //ifstream symbolTable(inputFileName);
        if (!symbolTable.is_open()) {
            cerr << "Error: Could not open " << inputFileName << " for reading." << endl;
        }
        Semantic_Check = true;
    }
    else {
        cout << "Syntax analysis completed with errors.\n";
    }
}

///////////////////////////////////////Semantic Analysis////////////////////////////////
// Symbol Table Entry structure
struct SymbolTableEntry {
    string type;       // Type of the variable
    string scope;      // Scope of the variable
    bool isConstant;   // Indicates if the variable is a constant
    int lineOfDecl;    // Line number of declaration
    int size;          // Size of the variable (if applicable)
    string value;      // Value of the variable
};
// Function Prototypes
string handleShowStatement(const string& operand, unordered_map<string, SymbolTableEntry>& symbolTable);
string formatResult(double result);
string processOperators(const string& operand1, const string& operator1, const string& operand2, unordered_map<string, SymbolTableEntry>& symbolTable);
bool handleCondition(const string& checkvariable, const string& checkvalue, const string& operator2, unordered_map<string, SymbolTableEntry>& symbolTable);
int handleCutStatement(int i, int entryCount, string* names, unordered_map<string, SymbolTableEntry>& symbolTable);
int handleCutBlock(int i, int entryCount, string* names, unordered_map<string, SymbolTableEntry>& symbolTable, bool condition);
int skipBlock(int i, int entryCount, std::string* names);
int handleRepeatLoop(int i, int entryCount, string* names, unordered_map<string, SymbolTableEntry>& symbolTable);
bool isInteger(const string& value);
bool isFloat(const string& value);
bool isValidValueForType1(const string& type, const string& value);
int handleFadeStatement(int i, int entryCount, string* names, unordered_map<string, SymbolTableEntry>& symbolTable);
int cutwasrun(int i, int entryCount, string* names, unordered_map<string, SymbolTableEntry>& symbolTable);
int cutifwasrun(int i, int entryCount, string* names, unordered_map<string, SymbolTableEntry>& symbolTable);

string handleShowStatement(const string& operand, unordered_map<string, SymbolTableEntry>& symbolTable) {


    if (symbolTable.find(operand) != symbolTable.end()) {
        // Retrieve the symbol table entry for the operand
        SymbolTableEntry entry = symbolTable[operand];

        // Check if the type is "scene"
        if (entry.type == "scene") {
            try {
                // Convert the value to an integer and return it as a string
                int intValue = stoi(entry.value);
                return to_string(intValue);
            }
            catch (const invalid_argument& e) {
                return "Error: Value cannot be converted to an integer.";
            }
            catch (const out_of_range& e) {
                return "Error: Value is out of range for an integer.";
            }
        }
        else {
            // If the type is not "scene", return the value as is
            return entry.value;
        }
    }
    else {
        return operand;
    }
}
// handle the result for the oprations 
string formatResult(double result) {
    // Check if the result is a whole number
    if (result == static_cast<int>(result)) {
        // If the result is an integer, return it without decimals
        return to_string(static_cast<int>(result));
    }
    else {
        // Otherwise, format it with precision
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(2) << result; // Adjust precision as needed
        return stream.str();
    }
}

string processOperators(const string& operand1, const string& operator1,
    const string& operand2, unordered_map<string, SymbolTableEntry>& symbolTable) {

    // Retrieve operand values if they are variables
    string value1 = operand1;
    string value2 = operand2;

    if (symbolTable.find(operand1) != symbolTable.end()) {
        value1 = symbolTable[operand1].value;
    }
    if (symbolTable.find(operand2) != symbolTable.end()) {
        value2 = symbolTable[operand2].value;
    }

    try {
        if (operator1 == "splice" || operator1 == "split") {
            if (!isInteger(value1) || !isInteger(value2)) {
                return "Error: Both operands must be integers for splice and split.";
            }

            int num1 = stoi(value1);
            int num2 = stoi(value2);

            if (operator1 == "split") {
                if (num2 == 0) return "Error: Division by zero.";
                return formatResult(static_cast<double>(num1) / num2);
            }
            else if (operator1 == "splice") {
                if (num2 == 0) return "Error: Modulo by zero.";
                return to_string(num1 % num2);
            }
        }
        else {
            double num1 = stod(value1);
            double num2 = stod(value2);
            double result = 0.0;

            if (operator1 == "join") {
                result = num1 + num2;
            }
            else if (operator1 == "amplify") {
                result = num1 * num2;
            }
            else if (operator1 == "trim") {
                result = num1 - num2;
            }
            else {
                return "Error: Invalid operator.";
            }

            return formatResult(result);
        }
    }
    catch (...) {
        return "Error: Invalid operand(s) for numeric operation.";
    }

    // Fallback return to prevent warning
    return "Error: Unknown issue in processOperators.";
}

// Function to handle the condition logic and return a boolean indicating whether the condition was met
bool handleCondition(const string& checkvariable, const string& checkvalue, const string& operator2,
    unordered_map<string, SymbolTableEntry>& symbolTable) {
    if (symbolTable.find(checkvariable) != symbolTable.end()) {
        float variableValue = stof(symbolTable[checkvariable].value);
        float valueToCheck = stof(checkvalue);

        if (operator2 == "is less") return variableValue < valueToCheck;
        else if (operator2 == "is greater") return variableValue > valueToCheck;
        else if (operator2 == "isnt") return variableValue != valueToCheck;
        else if (operator2 == "is less or equal") return variableValue <= valueToCheck;
        else if (operator2 == "is greater or equal") return variableValue >= valueToCheck;
        else if (operator2 == "is") return variableValue == valueToCheck;
        else {
            cerr << "Error: Invalid operator '" << operator2 << "' in cut statement.\n";
            return false;
        }
    }
    else {
        cerr << "Error: Variable '" << checkvariable << "' not found in symbol table.\n";
        return false;
    }
}

int handleCutStatement(int i, int entryCount, string* names, unordered_map<string, SymbolTableEntry>& symbolTable) {
    // Ensure valid syntax: cut (var operator value)
    if (names[i + 1] != "(") {
        cerr << "Error: Expected '(' after 'cut'.\n";
        return i;
    }

    string checkvariable = names[i + 2];
    string operator2 = names[i + 3];
    string checkvalue = names[i + 4];

    if (names[i + 5] != ")") {
        cerr << "Error: Missing ')' after condition in 'cut' statement.\n";
        return i;
    }

    // Evaluate the condition
    bool result = handleCondition(checkvariable, checkvalue, operator2, symbolTable);

    // Move to the next token after ')'
    i += 6;

    // Check for opening brace '{'
    if (names[i] == "{") {
        i = handleCutBlock(i, entryCount, names, symbolTable, result);
    }
    else {
        cerr << "Error: Expected '{' after condition in 'cut' statement.\n";
    }

    return i; // Return the updated index
}
// Recursive function to handle blocks inside the cut statement
int handleCutBlock(int i, int entryCount, string* names, unordered_map<string, SymbolTableEntry>& symbolTable, bool condition);
int skipBlock(int i, int entryCount, std::string* names);



// Function to skip a block of code
int skipBlock(int i, int entryCount, std::string* names) {
    int braceCount = 1; // Start with the first opening brace
    while (i < entryCount && braceCount > 0) {
        ++i;
        if (names[i] == "{") ++braceCount;
        else if (names[i] == "}") --braceCount;
    }
    return i; // Return the updated index after skipping the block
}





int handleRepeatLoop(int i, int entryCount, string* names, unordered_map<string, SymbolTableEntry>& symbolTable) {
    // Ensure the syntax starts with `repeat` and is followed by `(`
    if (names[i + 1] != "(") {
        cerr << "Error: Expected '(' after 'repeat' at line " << i + 1 << ".\n";
        return i;
    }

    string loopVariable = names[i + 2];   // Identifier or Integer
    string conditionOperator = names[i + 3]; // "times" keyword
    int iterationCount = 0; // Number of iterations for the loop

    // Ensure the closing parenthesis `)` exists
    if (names[i + 4] != ")") {
        cerr << "Error: Expected ')' after condition in 'repeat' loop at line " << i + 1 << ".\n";
        return i;
    }

    // Validate the condition
    if (conditionOperator == "times") {
        try {
            iterationCount = stoi(loopVariable); // Parse the number of iterations
        }
        catch (const invalid_argument&) {
            cerr << "Error: Invalid loop count '" << loopVariable << "' in 'repeat' loop at line " << i + 1 << ".\n";
            return i;
        }
    }
    else {
        cerr << "Error: Invalid operator '" << conditionOperator << "' in 'repeat' loop at line " << i + 1 << ".\n";
        return i;
    }

    // Move to the next token after `)`
    i += 5;

    // Ensure the block starts with `{`
    if (names[i] != "{") {
        cerr << "Error: Expected '{' after 'repeat' loop condition at line " << i + 1 << ".\n";
        return i;
    }

    int braceStart = i; // Save the starting index of the block
    int braceCount = 1;

    // Loop for the specified number of iterations
    for (int count = 0; count < iterationCount - 1; ++count) {
        int tempI = braceStart + 1; // Start processing the block after the opening brace
        braceCount = 1; // Reset brace count for nested processing

        while (tempI < entryCount && braceCount > 0) {
            // Handle nested statements or keywords within the loop
            if (names[tempI] == "cut") {
                tempI = handleCutStatement(tempI, entryCount, names, symbolTable);
            }
            else if (names[tempI] == "show") {
                // Handle the `show` statement
                if (types[tempI + 1] == "LITERAL")
                {
                    cout << names[tempI + 1] << endl;
                }

                else
                {
                    string result = handleShowStatement(names[tempI + 1], symbolTable);
                    cout << result << endl;
                }
                tempI += 2; // Skip the `show` token and its operand
            }
            else if (names[tempI] == "repeat") {
                tempI = handleRepeatLoop(tempI, entryCount, names, symbolTable); // Recursive call for nested loops
            }
            else if (names[tempI] == "{") {
                ++braceCount;
                ++tempI;
            }
            else if (names[tempI] == "}") {
                --braceCount;
                ++tempI;
            }
            else {
                ++tempI; // Skip unknown tokens
            }
        }
    }

    // Skip the block after processing
    while (i < entryCount && braceCount > 0) {
        ++i;
        if (names[i] == "{") ++braceCount;
        else if (names[i] == "}") --braceCount;
    }

    return i; // Return the updated index
}

bool isInteger(const string& value) {
    return !value.empty() && (isdigit(value[0]) || value[0] == '-') &&
        all_of(value.begin() + 1, value.end(), ::isdigit);
}

bool isFloat(const string& value) {
    // A valid FLOAT must contain a decimal point
    if (value.find('.') == string::npos) {
        return false;
    }

    // Check if the value can be successfully parsed as a float
    istringstream iss(value);
    float f;
    return iss >> f && iss.eof();//ensure no extra characters
}
//type checking
bool isValidValueForType1(const string& type, const string& value) {
    if (type == "INTEGER") {
        return isInteger(value); // Valid if the value is an integer
    }
    else if (type == "FLOAT") {
        return isFloat(value); // Valid if the value is a float
    }
    else if (type == "LITERAL") {
        return true; // LITERALS can be any string
    }
    return false; // Unrecognized type
}

/////////////////////////////handle cut/cutif/fade///////////////////////////
// Function to process a cut statement

// Recursive function to handle blocks inside the cut statement

int handleCutBlock(int i, int entryCount, string* names, unordered_map<string, SymbolTableEntry>& symbolTable, bool condition);


// Function to process a cut statement
int handleFadeStatement(int i, int entryCount, string* names, unordered_map<string, SymbolTableEntry>& symbolTable) {



    bool result = true;//not checking any condition

    // move to {
    ++i;

    // Check for opening brace '{'
    if (names[i] == "{") {
        i = handleCutBlock(i, entryCount, names, symbolTable, result);
    }
    else {
        cerr << "Error: Expected '{' after condition in 'cut' statement.\n";
    }

    return i; // Return the updated index
}
// Function to skip process
int cutwasrun(int i, int entryCount, string* names, unordered_map<string, SymbolTableEntry>& symbolTable) {
    // Ensure valid syntax: cut (var operator value)
    if (names[i + 1] != "(") {
        cerr << "Error: Expected '(' after 'cut'.\n";
        return i;
    }

    string checkvariable = names[i + 2];
    string operator2 = names[i + 3];
    string checkvalue = names[i + 4];

    if (names[i + 5] != ")") {
        cerr << "Error: Missing ')' after condition in 'cut' statement.\n";
        return i;
    }

    // Evaluate the condition
    bool result = false;

    // Move to the next token after ')'
    i += 6;

    // Check for opening brace '{'
    if (names[i] == "{") {
        i = handleCutBlock(i, entryCount, names, symbolTable, result);
    }
    else {
        cerr << "Error: Expected '{' after condition in 'cut' statement.\n";
    }

    return i; // Return the updated index
}
// Function to process a cutif ,if cut was run statement
int cutifwasrun(int i, int entryCount, string* names, unordered_map<string, SymbolTableEntry>& symbolTable) {

    // Evaluate the condition
    bool result = false;

    // Move to the next token after ')'
    i += 1;

    // Check for opening brace '{'
    if (names[i] == "{") {
        i = handleCutBlock(i, entryCount, names, symbolTable, result);
    }
    else {
        cerr << "Error: Expected '{' after condition in 'cut' statement.\n";
    }

    return i; // Return the updated index
}
// Recursive function to handle blocks inside the cut statement
int handleCutBlock(int i, int entryCount, string* names, unordered_map<string, SymbolTableEntry>& symbolTable, bool condition) {
    int braceCount = 1; // Start with the first opening brace

    if (!condition) {
        // Skip the block if the condition is false
        while (i < entryCount && braceCount > 0) {
            ++i;
            if (names[i] == "{") ++braceCount;
            else if (names[i] == "}") --braceCount;
        }
    }
    else {
        // Process the block if the condition is true
        while (i < entryCount && braceCount > 0) {
            ++i;

            if (names[i] == "cut") {
                i = handleCutStatement(i, entryCount, names, symbolTable);
            }
            else if (names[i] == "show") {
                if (types[i + 1] == "LITERAL") {
                    cout << names[i + 1] << endl;
                }
                else {
                    string result = handleShowStatement(names[i + 1], symbolTable);
                    cout << result << endl;
                }
                ++i; // Skip operand
            }

            if (names[i] == "{") ++braceCount;
            else if (names[i] == "}") --braceCount;
        }
    }

    return i; // Return the updated index
}

// main semantic analysis
bool semanticsAnalysis(int entryCount, string* names, string* types,
    string* sizes, string* lineOfDecls, unordered_map<string, SymbolTableEntry>& symbolTable) {
    bool hasErrors = false;

    for (int i = 0; i < entryCount; ++i) {
        std::string token = names[i];
        std::string type = types[i];

        if (token == "cut") {
            std::string checkvariable = names[i + 2];//variable
            std::string operator2 = names[i + 3];//operator
            std::string checkvalue = names[i + 4];//value
            bool cutrun = handleCondition(checkvariable, checkvalue, operator2, symbolTable);
            i = handleCutStatement(i, entryCount, names, symbolTable);
            bool cutifrun;
            // Handle cutif condition
            if (names[i + 1] == "cutif")
            {
                i++;
                std::string checkvariable = names[i + 2];
                std::string operator2 = names[i + 3];
                std::string checkvalue = names[i + 4];
                cutifrun = handleCondition(checkvariable, checkvalue, operator2, symbolTable);


                if (cutrun)
                {
                    i = cutwasrun(i, entryCount, names, symbolTable);//skips cutif block
                }
                if (!cutrun)
                {
                    i = handleCutStatement(i, entryCount, names, symbolTable);
                }
            }
            if (names[i + 1] == "fade")
            {
                i++;
                if (cutrun || cutifrun)
                {
                    i = cutifwasrun(i, entryCount, names, symbolTable);
                }
                if (!cutrun && !cutifrun)
                {
                    i = handleFadeStatement(i, entryCount, names, symbolTable);
                }
            }
        }
        if (token == "show") {
            if (i + 1 < entryCount) {
                string operand = names[i + 1];
                if (types[i + 1] == "LITERAL") {
                    cout << operand << endl;
                }
                else {
                    string printit = handleShowStatement(operand, symbolTable);
                    cout << printit << endl;
                }
            }
            else {
                cerr << "Error: Invalid 'show' statement at line " << i + 1 << ".\n";
                hasErrors = true;
            }
            i = i + 1; // Increment index after 'show' processing
        }
        if (token == "prompt") {
            if (i + 1 < entryCount) {
                string variableName = names[i + 1];

                // Check if the variable is declared
                if (symbolTable.find(variableName) != symbolTable.end()) {
                    SymbolTableEntry& entry = symbolTable[variableName];
                    string inputValue;

                    // Prompt user for input
                    cout << "Enter value for " << variableName << " (" << entry.type << "): ";
                    cin >> inputValue;

                    // Validate based on the type
                    if (isValidValueForType1(entry.type, inputValue)) {
                        entry.value = inputValue; // Assign the value
                        cout << "Value assigned to " << variableName << ": " << inputValue << endl;
                    }
                    else {
                        cerr << "Error: Invalid value '" << inputValue << "' for type '" << entry.type
                            << "' at line " << lineOfDecls[i + 1] << ".\n";
                        hasErrors = true;
                    }
                }
                else {
                    cerr << "Error: Variable '" << variableName << "' is not declared at line "
                        << lineOfDecls[i + 1] << ".\n";
                    hasErrors = true;
                }
                i += 1; // Move past 'prompt' and the variable name
            }
            else {
                cerr << "Error: Incomplete 'prompt' statement at line " << lineOfDecls[i] << ".\n";
                hasErrors = true;
            }
        }
        else if (token == "repeat") {
            i = handleRepeatLoop(i, entryCount, names, symbolTable);
        }

        if (token == "scene" || token == "frame" || token == "script") {
            string key;
            string value;

            if (i + 3 < entryCount && names[i + 2] == "assign") {
                key = names[i + 1]; // The variable name
                value = names[i + 3]; // The assigned value

                string assignedValueType = types[i + 3];

                // Determine the expected type based on the keyword
                string expectedType;
                if (token == "scene") {
                    expectedType = "INTEGER";
                }
                else if (token == "frame") {
                    expectedType = "FLOAT";
                }
                else if (token == "script") {
                    expectedType = "LITERAL";
                }

                // Check if the assignment involves an operation
                if (i + 5 < entryCount &&
                    (names[i + 4] == "join" || names[i + 4] == "trim" || names[i + 4] == "split" ||
                        names[i + 4] == "splice" || names[i + 4] == "amplify")) {

                    string operator1 = names[i + 4];
                    string operand2 = names[i + 5];

                    if (symbolTable.find(key) != symbolTable.end()) {
                        value = symbolTable[key].value;  // Retrieve current value of the variable
                    }

                    // Operations are not valid for 'script'
                    if (token == "script") {
                        cerr << "Error: Invalid operation '" << operator1
                            << "' for type 'script' at line " << lineOfDecls[i + 4] << ".\n";
                        hasErrors = true;
                    }
                    else {
                        // Perform calculation and get result
                        string result = processOperators(value, operator1, operand2, symbolTable);

                        // Skip type enforcement and store the result directly
                        value = result;
                    }

                    i += 5; // Skip operator and operand
                }

                // Validate the type of the value if no operation was involved
                if (!isValidValueForType1(expectedType, value)) {
                    cerr << "Error: Type mismatch for '" << key << "'. Expected type for '" << token
                        << "' is " << expectedType << " but got " << assignedValueType
                        << " at line " << lineOfDecls[i + 3] << ".\n";
                    hasErrors = true;
                }
                else {
                    // Store in symbol table
                    SymbolTableEntry entry;
                    entry.type = expectedType;
                    entry.value = value;
                    entry.lineOfDecl = i + 1;
                    entry.scope = "global";
                    entry.size = 0;
                    symbolTable[key] = entry; // Add to symbol table


                }
            }
            else {
                cerr << "Error: Invalid declaration at line " << lineOfDecls[i] << ".\n";
                hasErrors = true;
            }
        }

    }

    return !hasErrors;  // Return false if errors found
}

int main() {
    lexicalAnalyzer("input.txt");
    cout << "Lexical Analysis complete. Output written to files.\n";

    ofstream clearFile("Error.txt", ios::trunc);
    clearFile.close();

    // Load tokens and parse program
    if (loadTokensFromFile("SymbolTable.txt")) {
        parseProgram();
    }
    else {
        cout << "Failed to load tokens from Output.txt\n";
    }

    // File paths
    const string inputFileName = "SymbolTable.txt";
    const string outputFileName = "UpdatedSymbolTable.txt";

    // Open input file
    ifstream file(inputFileName);
    if (!file.is_open()) {
        cerr << "Error: Could not open " << inputFileName << " for reading.\n";
        return 1;
    }
    if (Semantic_Check == true)
    {
        // Parse symbol table file into arrays
        const int entryCount = 100;  // Set a suitable size for your symbol table
        string names[entryCount], types[entryCount], sizes[entryCount], lineOfDecls[entryCount];
        parseSymbolTable(file, entryCount, names, types, sizes, lineOfDecls);
        file.close();

        // Perform semantic analysis
        unordered_map<string, SymbolTableEntry> symbolTable;

        if (semanticsAnalysis(entryCount, names, types, sizes, lineOfDecls, symbolTable)) {
            cout << "Semantic analysis completed successfully.\n";

            // Display the symbol table
            for (const auto& pair : symbolTable) {
                const string& key = pair.first;
                const SymbolTableEntry& entry = pair.second;

                cout << "Key: " << key
                    << ", Type: " << entry.type
                    << ", Value: " << entry.value
                    << ", Scope: " << entry.scope
                    << ", Line of Declaration: " << entry.lineOfDecl
                    << ", Size: " << entry.size << "\n";
            }
        }
        else {
            cerr << "Semantic analysis failed.\n";
        }
    }

    return 0; // Properly return from main function
}