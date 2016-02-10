#include "encoder.h"
#include <string>
#include <iostream>
#include <vector>
#include <utility>
#include <cstdio>


using std::string;
using std::vector;
using std::endl;
using std::cerr;
using std::pair;
using std::cin;
using ASM::Token;
using ASM::Lexer;


// =========================== private methods ===========================

void Encoder::scan() {
    // Create a MIPS recognizer to tokenize
    // the input lines
    Lexer lexer;
    // Tokenize each line of the input
    string line;
    while(getline(cin,line)){
        tokLines.push_back(lexer.scan(line));
    }
}

// ===========================

void Encoder::makeSymbolTable() {
    // Iterate over the lines of tokens and scan labels
    
    vector<vector<Token*> >::iterator it;
    
    // collecting labels into symbol table
    for(it = tokLines.begin(); it != tokLines.end(); ++it){
        vector<Token*>::iterator it2;
        for(it2 = it->begin(); it2 != it->end(); ++it2){
            if ((*it2)->getKind() == ASM::LABEL) {
                for (int i = 0; i < symbolTable.size(); i++) {
                    if ((*it2)->getLexeme() == (symbolTable[i].first + ":")) {
                        throw string("ERROR: duplicate label found");
                    }
                }
                symbolTable.push_back(pair<string, int>((*it2)->getLexeme().substr(0, ((*it2)->getLexeme()).length() - 1), pc + 12));
            }
            
            // list all possible non-null line
            if (isOprand(*it2)) {
                pc += 4;
            }
        }
    }
}

// ===========================

void Encoder::putWord(int word) {
    for (int i = 0; i < 4; i++) {
        putchar((word >> (24 - 8 * i)) & 0xff);
    }
}

// ===========================

bool Encoder::isOprand(Token *it) {
    // list all possible opearands
    return (it->getKind() == ASM::DOTWORD ||
            (it->getKind() == ASM::ID &&
             (it->getLexeme() == "add"      ||
              it->getLexeme() == "sub"      ||
              it->getLexeme() == "mult"     ||
              it->getLexeme() == "multu"    ||
              it->getLexeme() == "div"      ||
              it->getLexeme() == "divu"     ||
              it->getLexeme() == "mfhi"     ||
              it->getLexeme() == "mflo"     ||
              it->getLexeme() == "lis"      ||
              it->getLexeme() == "lw"       ||
              it->getLexeme() == "sw"       ||
              it->getLexeme() == "slt"      ||
              it->getLexeme() == "sltu"     ||
              it->getLexeme() == "beq"      ||
              it->getLexeme() == "bne"      ||
              it->getLexeme() == "jr"       ||
              it->getLexeme() == "jalr")));
    
}


int Encoder::searchLabel(string name) {
    int value = -1;
    
    // search if the label is in the symbol table
    for (int i = 0; i < symbolTable.size(); i++) {
        if (symbolTable[i].first == name) {
            value = symbolTable[i].second;
            break;
        }
    }
    
    return value;
}

// ===========================

void Encoder::setMERL() {
    address = 0xc;
    
    vector<vector<Token*> >::iterator it;

    for(it = tokLines.begin(); it != tokLines.end(); ++it){
        vector<Token*>::iterator it2;
        for(it2 = it->begin(); it2 != it->end(); ++it2){
            if ((*it2)->getKind() == ASM::DOTWORD &&
                it2 == it->end() - 2 &&
                (*(it2 + 1))->getKind() == ASM::ID) {  // .word <label>
                relocationTable.push_back(0x01);
                relocationTable.push_back(address);
                address += 4;
            }
            else if (isOprand(*it2) && it2 != it->end() - 1) { // a non-null line
                address += 4;
            }
        }
    }
    
    clength = address;
    length = clength + 4 * relocationTable.size();
}

// ===========================
void Encoder::printHeader() {
    putWord(0x10000002); // header
    putWord(length);;
    putWord(clength);
}



void Encoder::printCode() {
    for (int i = 0; i < printTable.size(); i++) {
        putWord(printTable[i]);
    }
}



// ===========================
void Encoder::printNote() {
    for (int i = 0; i < relocationTable.size(); i++) {
        putWord(relocationTable[i]);
    }
}




// ===========================

// methods convert opcode and registers into one 32-bit instruction
// add, sub, slt, sltu:
int Encoder::binaryThree(string op, int rd, int rs, int rt) {
    int value;
    
    if (op == "add") {
        value = (rs << 21) | (rt << 16) | (rd << 11) | (1 << 5);
    }
    else if (op == "sub") {
        value = (rs << 21) | (rt << 16) | (rd << 11) | (1 << 5) | (1 << 1);
    }
    else if (op == "slt") {
        value = (rs << 21) | (rt << 16) | (rd << 11) | (1 << 5) | (1 << 3) | (1 << 1);
    }
    else {  // if (op == "sltu")
        value = (rs << 21) | (rt << 16) | (rd << 11) | (1 << 5) | (1 << 3) | (1 << 1) + 1;
    }
    
    return value;
}

// ===========================

// mult, multu, div, divu:
int Encoder::binaryTwo(string op, int rs, int rt) {
    int value;
    
    if (op == "mult") {
        value = (rs << 21) | (rt << 16) | (1 << 4) | (1 << 3);
    }
    else if (op == "multu") {
        value = (rs << 21) | (rt << 16) | (1 << 4) | (1 << 3) | 1;
    }
    else if (op == "div") {
        value = (rs << 21) | (rt << 16) | (1 << 4) | (1 << 3) | (1 << 1);
    }
    else {  // if (op == "divu")
        value = (rs << 21) | (rt << 16) | (1 << 4) | (1 << 3) | (1 << 1) | 1;
    }
    
    return value;
}

// ===========================

// mfhi, mflo, lis:
int Encoder::binaryOne(string op, int rd) {
    int value;
    
    if (op == "mfhi") {
        value = (rd << 11) | (1 << 4);
    }
    else if (op == "mflo") {
        value = (rd << 11) | (1 << 4) | (1 << 1);
    }
    else {  // if (op == "lis")
        value = (rd << 11) | (1 << 4) | (1 << 2);
    }
    
    return value;
}

// ===========================

// I-format instructions, lw, sw:
int Encoder::binaryLwSw(string op, int rt, int rs, int i) {
    int value;
    
    if (op == "lw") {
        value = (1 << 31) | (1 << 27) | (1 << 26) | (rs << 21) | (rt << 16) | (i & 0xffff);
    }
    else {  // if (op == "sw")
        value = (1 << 31) | (1 << 29) | (1 << 27) | (1 << 26) | (rs << 21) | (rt << 16) | (i & 0xffff);
    }
    
    return value;
}

// ===========================

// branch, beq, bne:
int Encoder::binaryBranch(string op, int rs, int rt, int i) {
    int value;
    
    if (op == "beq") {
        value = (4 << 26) | (rs << 21) | (rt << 16) | (i & 0xffff);
    }
    else {  // if (op == "bne")
        value = (5 << 26) | (rs << 21) | (rt << 16) | (i & 0xffff);
        
    }
    
    return value;
}

// ===========================

// jump, jr, jalr:
int Encoder::binaryJump(string op, int rs) {
    int value;
    
    if (op == "jr") {
        value = (rs << 21) | (1 << 3);
    }
    else{ // if (op == "jalr")
        value = (rs << 21) | (1 << 3) | 1;
    }
    
    return value;
}



// =========================== public methods ===========================
Encoder::Encoder() : pc(0), length(0), clength(0), address(0x0) {}

// ===========================

Encoder::~Encoder() {
    // Delete the Tokens that have been made
    vector<vector<Token*> >::iterator it;
    for(it = tokLines.begin(); it != tokLines.end(); ++it){
        vector<Token*>::iterator it2;
        for(it2 = it->begin(); it2 != it->end(); ++it2){
            delete *it2;
        }
    }
}

// ===========================

void Encoder::encoding() {
    pc = 0;  // initialize the program counter
    
    // Create a MIPS recognizer to tokenize the input lines
    // Tokenize each line of the input
    scan();
    
    // printToken();  // for testing only
    
    makeSymbolTable();
    
    // printSymbolTable();  // for testing only
    
    setMERL();
    
    printHeader();
    
    pc = 0xc; // re-initialize the program counter
    address = 0xc;
    
    // encoding into assembly code
    vector<vector<Token*> >::iterator it;
    
    
    for(it = tokLines.begin(); it != tokLines.end(); ++it){
        vector<Token*>::iterator it2;
        for(it2 = it->begin(); it2 != it->end(); ++it2){
            // ==== for each token ==== //
            // list all possible non-null line, increase the program counter
            if (isOprand(*it2) &&
                searchLabel((*it2)->getLexeme()) == -1 &&
                it2 != it->end() - 1) {
                pc += 4;
            }
            
            // encoding each instructions:
            // for .word instruction
            if ((*it2)->getKind() == ASM::DOTWORD) { // .word XX
                if (it2 == it->end() - 1) { // nothing after .word
                    throw string("ERROR: No instant value after .word");
                }
                // integer after .word
                else if ((*(it2 + 1))->getKind() == ASM::INT ||
                         (*(it2 + 1))->getKind() == ASM::HEXINT) {
                    int word = (*(it2 + 1))->toInt();
                    
                    printTable.push_back(word);
                    address += 4;
                }
                // label without :, after .word
                else if ((*(it2 + 1))->getKind() == ASM::ID) {
                    string name = (*(it2 + 1))->getLexeme();
                    int value = searchLabel(name);
                    
                    if (value == -1) {
                        throw string("ERROR: invalid label found: ") + name;
                    }
                    else {
                        
                        printTable.push_back(value);
                        address += 4;
                    }
                }
                else {
                    throw string("ERROR: Unexpected value after .word: ") + (*it2)->toString();
                }
            } // end .word instruction
            // =============================================================
            // for the label which is "label:"
            else if ((*it2)->getKind() == ASM::LABEL) {
                vector<Token*>::iterator it3;
                for (it3 = it->begin(); it3 != it2; ++it3) {
                    if ((*it3)->getKind() != ASM::LABEL) {
                        throw string("ERROR: Invalid input before label: ") + (*it2)->toString();
                    }
                }
            }  // end  label:
            // =============================================================
            // for the oprand and label
            else if ((*it2)->getKind() == ASM::ID) { // oprand, or label without :
                if (searchLabel((*it2)->getLexeme()) != -1 &&
                    it2 == it->end() - 1) {  // it is label
                    if (it2 == it->begin()) {
                        throw string("ERROR: Invalid use of label(ID) at the start of the line: ") + (*it2)->getLexeme();
                    }
                    
                    continue;
                }
                
                // for now, it2 cannot be label, see above
                if (it2 == it->end() - 1) {// oprand is the last token in that line
                    throw string("ERROR: Nothing after oprand: ") + (*it2)->toString();
                }
                
                // check if there are something not labels before oprand
                vector<Token*>::iterator it3;
                for (it3 = it->begin(); it3 != it2; ++it3) {
                    if ((*it3)->getKind() != ASM::LABEL) {
                        throw string("ERROR: Invalid label before oprand: ") + (*it2)->getLexeme();
                    }
                }
                
                // jump instructions:
                if ((*it2)->getLexeme() == "jr" ||
                    (*it2)->getLexeme() == "jalr") {
                    if ((*(it2 + 1))->getKind() == ASM::REGISTER) {
                        int rs =(*(it2 + 1))->toInt();
                        if (rs < 0 || rs > 31) {
                            throw string("ERROR: register out of range");
                        }
                        
                        int word = binaryJump((*it2)->getLexeme(), rs);
                        
                        printTable.push_back(word);
                        address += 4;
                    }
                    else {
                        throw string("ERROR: Non-register after oprand: ") + (*it2)->getLexeme();
                    }
                }
                // 3-register instructions: add, sub, slt, sltu
                else if ((*it2)->getLexeme() == "add" ||
                         (*it2)->getLexeme() == "sub" ||
                         (*it2)->getLexeme() == "slt" ||
                         (*it2)->getLexeme() == "sltu") {
                    if (it2 == it->end() - 6) { // the sixth last token in that line
                        if ((*(it2 + 1))->getKind() == ASM::REGISTER &&
                            (*(it2 + 2))->getKind() == ASM::COMMA &&
                            (*(it2 + 3))->getKind() == ASM::REGISTER &&
                            (*(it2 + 4))->getKind() == ASM::COMMA &&
                            (*(it2 + 5))->getKind() == ASM::REGISTER) {
                            int rd = (*(it2 + 1))->toInt();
                            int rs = (*(it2 + 3))->toInt();
                            int rt = (*(it2 + 5))->toInt();
                            
                            // check if registers less than 0 or larger than 31
                            if (rd < 0 || rd > 31) {
                                throw string("ERROR: register $rd out of range, after oprand: ") + (*it2)->getLexeme();
                            }
                            
                            if (rs < 0 || rs > 31) {
                                throw string("ERROR: register $rs out of range, after oprand: ") + (*it2)->getLexeme();
                            }
                            
                            if (rt < 0 || rt > 31) {
                                throw string("ERROR: register $rt out of range, after oprand: ") + (*it2)->getLexeme();
                            }
                            
                            
                            int word = binaryThree((*it2)->getLexeme(), rd, rs, rt);
                            printTable.push_back(word);
                            address += 4;
                        }
                        else {
                            throw string("ERROR: 5 tokens but not rd,rs,rt format after oprand: ") + (*it2)->getLexeme();
                        }
                    }
                    else {
                        throw string("ERROR: less than 5 tokens after oprand: ") + (*it2)->getLexeme();
                    }
                }
                // branch instructions: beq, bne
                else if ((*it2)->getLexeme() == "beq" ||
                         (*it2)->getLexeme() == "bne") {
                    if (it2 == it->end() - 6) { // the sixth last token in that line
                        if ((*(it2 + 1))->getKind() == ASM::REGISTER &&
                            (*(it2 + 2))->getKind() == ASM::COMMA    &&
                            (*(it2 + 3))->getKind() == ASM::REGISTER &&
                            (*(it2 + 4))->getKind() == ASM::COMMA    &&
                            ((*(it2 + 5))->getKind() == ASM::INT     ||
                             (*(it2 + 5))->getKind() == ASM::HEXINT  ||
                             (*(it2 + 5))->getKind() == ASM::ID)) {
                                int rs = (*(it2 + 1))->toInt();
                                int rt = (*(it2 + 3))->toInt();
                                int i = 0;
                                
                                // check if registers less than 0 or larger than 31
                                if (rs < 0 || rs > 31) {
                                    throw string("ERROR: register $rs out of range, after oprand: ") + (*it2)->getLexeme();
                                }
                                
                                if (rt < 0 || rt > 31) {
                                    throw string("ERROR: register $rt out of range, after oprand: ") + (*it2)->getLexeme();
                                }
                                
                                // check the immediate value
                                if ((*(it2 + 5))->getKind() == ASM::INT     ||
                                    (*(it2 + 5))->getKind() == ASM::HEXINT) {
                                    i = (*(it2 + 5))->toInt();
                                }
                                else { // (*(it2 + 5))->getKind() == ASM::ID
                                    string name = (*(it2 + 5))->getLexeme();
                                    int value = searchLabel(name);
                                    
                                    if (value == -1) {
                                        throw string("ERROR: invalid label found: ") + name;
                                    }
                                    
                                    i = (value - pc)/4;
                                    
                                }
                                
                                // check if i is in a range [-32768, 32767](int) or [0x0, 0xffff](hex)
                                if (!(((*(it2 + 5))->getKind() == ASM::INT  && ((i >= -32768 && i <= 32767))) ||
                                    ((*(it2 + 5))->getKind() == ASM::ID     &&  (i >= -32768 && i <= 32767))                             ||
                                    ((*(it2 + 5))->getKind() == ASM::HEXINT &&  (i <= 0xffff)))) {
                                    throw string("ERROR: immediate value i out of range, after oprand: ") + (*it2)->getLexeme();
                                }
                                
                                int word = binaryBranch((*it2)->getLexeme(), rs, rt, i);
                                printTable.push_back(word);
                                address += 4;
                            }
                        else {
                            throw string("ERROR: 5 tokens but not rs,rt,i format after oprand: ") + (*it2)->getLexeme();
                        }
                    }
                    else {
                        throw string("ERROR: less than 5 tokens after oprand: ") + (*it2)->getLexeme();
                    }
                }
                // 1-register instruction: mfhi, mflo, lis
                else if ((*it2)->getLexeme() == "mfhi" ||
                         (*it2)->getLexeme() == "mflo" ||
                         (*it2)->getLexeme() == "lis") {
                    if (it2 == it->end() - 2) {
                        if ((*(it2 + 1))->getKind() == ASM::REGISTER) {
                            int rd = (*(it2 + 1))->toInt();
                            
                            if (rd < 0 || rd > 31) {
                                throw string("ERROR: register $rd out of range, after oprand: ") + (*it2)->getLexeme();
                            }
                            
                            int word = binaryOne((*it2)->getLexeme(), rd);
                            
                            printTable.push_back(word);
                            address += 4;
                            
                        }
                        else {
                            throw string("ERROR: Non-register after oprand") + (*it2)->getLexeme();
                        }
                    }
                    else {
                        throw string("ERROR: More or less than 1 token after oprand") + (*it2)->getLexeme();
                    }
                }
                // 2-register instructions: mult, multu, div, divu
                else if ((*it2)->getLexeme() == "mult" ||
                         (*it2)->getLexeme() == "multu" ||
                         (*it2)->getLexeme() == "div" ||
                         (*it2)->getLexeme() == "divu") {
                    if (it2 == it->end() - 4) {
                        if ((*(it2 + 1))->getKind() == ASM::REGISTER &&
                            (*(it2 + 2))->getKind() == ASM::COMMA    &&
                            (*(it2 + 3))->getKind() == ASM::REGISTER) {
                            int rs = (*(it2 + 1))->toInt();
                            int rt = (*(it2 + 3))->toInt();
                            
                            if (rs < 0 || rs > 31) {
                                throw string("ERROR: register $rs out of range, after oprand: ") + (*it2)->getLexeme();
                            }
                            
                            if (rt < 0 || rt > 31) {
                                throw string("ERROR: register $rt out of range, after oprand: ") + (*it2)->getLexeme();
                            }
                            
                            int word = binaryTwo((*it2)->getLexeme(), rs, rt);
                            printTable.push_back(word);
                            address += 4;
                        }
                        else {
                            throw string("ERROR: 3 tokens but not rs,rt format after oprand: ") + (*it2)->getLexeme();
                        }
                    }
                    else {
                        throw string("ERROR: More or less than 3 tokens after oprand") + (*it2)->getLexeme();
                    }
                    
                    
                }
                // I-format instructions: lw, sw
                else if ((*it2)->getLexeme() == "lw" ||
                         (*it2)->getLexeme() == "sw") {
                    if (it2 == it->end() - 7) {
                        if ((*(it2 + 1))->getKind() == ASM::REGISTER &&
                            (*(it2 + 2))->getKind() == ASM::COMMA    &&
                            ((*(it2 + 3))->getKind() == ASM::INT || (*(it2 + 3))->getKind() == ASM::HEXINT) &&
                            (*(it2 + 4))->getKind() == ASM::LPAREN &&
                            (*(it2 + 5))->getKind() == ASM::REGISTER &&
                            (*(it2 + 6))->getKind() == ASM::RPAREN) {
                            int rt = (*(it2 + 1))->toInt();
                            int rs = (*(it2 + 5))->toInt();
                            int i = (*(it2 + 3))->toInt();
                            
                            if (rs < 0 || rs > 31) {
                                throw string("ERROR: register $rs out of range, after oprand: ") + (*it2)->getLexeme();
                            }
                            
                            if (rt < 0 || rt > 31) {
                                throw string("ERROR: register $rt out of range, after oprand: ") + (*it2)->getLexeme();
                            }
                            
                            if (!(((*(it2 + 3))->getKind() == ASM::INT  && ((i >= -32768 && i <= 32767))) ||
                                  ((*(it2 + 3))->getKind() == ASM::HEXINT &&  (i <= 0xffff)))) {
                                throw string("ERROR: immediate value i out of range, after oprand: ") + (*it2)->getLexeme();
                            }
                            
                            int word = binaryLwSw((*it2)->getLexeme(), rt, rs, i);
                            printTable.push_back(word);
                            address += 4;
                        }
                        else {
                            throw string("ERROR: 6 tokens but not $rt, i($rs) format after oprand: ") + (*it2)->getLexeme();
                        }
                    }
                    else {
                        throw string("ERROR: More or less than 6 tokens after oprand") + (*it2)->getLexeme();
                    }
                }
                // all other oprands
                else {
                    throw string("ERROR: Invalid assembly language instruction: ") + (*it2)->getLexeme();
                }
            }// end oprand and label
            // =============================================================
            // for integers and hexeIntegers
            else if ((*it2)->getKind() == ASM::INT ||
                     (*it2)->getKind() == ASM::HEXINT) {
                if (it2 == it->begin()) {
                    throw string("ERROR: Invalid assembly language program, except label or oprand, found a number");
                }
                else if ((*(it2 - 1))->getKind() == ASM::DOTWORD) {
                    // .word number, do nothing here
                }
                else if ((*(it2 - 1))->getKind() == ASM::COMMA) {
                    // branch's immediate number, do nothing here
                }
                else {
                    throw string("ERROR: Invalid input");
                }
            }// end integers and hexeIntegers
            // =============================================================
            // for registers
            else if ((*it2)->getKind() == ASM::REGISTER) {
                if (it2 == it->begin()) {
                    throw string("ERROR: Invalid register at the start: ") + (*it2)->getLexeme();
                }
                else if (((*(it2 - 1))->getKind() == ASM::ID && isOprand(*(it2 - 1))) ||
                         (*(it2 - 1))->getKind() == ASM::COMMA) {
                    // a valid register, do nothing here
                }
                else if (it2 != it->end() - 1 && ((*(it2 - 1))->getKind() == ASM::LPAREN && (*(it2 + 1))->getKind() == ASM::RPAREN)) {
                    // a valid register in lw/sw instruction, do nothing here
                }
                else {
                    throw string("ERROR: Invalid pointer found: ") + (*it2)->getLexeme();
                }
            } // end registers
            // =============================================================
            // for comma: ,
            else if ((*it2)->getKind() == ASM::COMMA) {
                if (it2 == it->begin()) {  // comma at starter
                    throw string("ERROR: comma at the beginning of the token line");
                }
                else if (it2 == it->end() - 1) {  // comma at the ending
                    throw string("ERROR: comma at the end of the token line");
                }
                else if ((*(it2 - 1))->getKind() == ASM::REGISTER &&
                         ((*(it2 + 1))->getKind() == ASM::REGISTER ||
                          (*(it2 + 1))->getKind() == ASM::INT ||
                          (*(it2 + 1))->getKind() == ASM::HEXINT ||
                          ((*(it2 + 1))->getKind() == ASM::ID &&
                           !isOprand(*it2)))) {
                              // a valid comma, in the format: register, register
                }
                else {
                    throw string("ERROR: Invalid use of comma");
                }
            } // end comma
            // =============================================================
            // for left parenthesis: (
            else if ((*it2)->getKind() == ASM::LPAREN) {
                if (it2 == it->begin()) {
                    throw string("ERROR: left parenthesis at the beginning of the token line");
                }
                else if (it2 == it->end() - 1) {
                    throw string("ERROR: left parenthesis at the end of the token line");
                }
                else if (((*(it2 - 1))->getKind() == ASM::INT || (*(it2 - 1))->getKind() == ASM::HEXINT) &&
                         (*(it2 + 1))->getKind() == ASM::REGISTER) {
                    // a correct left parenthesis useage, do nothing here
                }
                else {
                    throw string("ERROR: Invalid use of left parenthesis");
                }
            } // end left parenthesis
            // =============================================================
            // for right parenthesis: )
            else if ((*it2)->getKind() == ASM::RPAREN) {
                if (it2 == it->begin()) {
                    throw string("ERROR: right parenthesis at the beginning of the token line");
                }
                else if (it2 == it->end() - 1 && (*(it2 - 1))->getKind() == ASM::REGISTER) {
                    // a correct left parenthesis useage, do nothing here
                }
                else {
                    throw string("ERROR: Invalid use of left parenthesis");
                }
            } // end right parenthesis
            // =============================================================
            // all other conditions
            else {
                throw string("ERROR: Invalid assembly language token found: ") + (*it2)->getLexeme();
            }
        }
    }
    
    printCode();
    
    printNote();
}

// ===========================

void Encoder::printToken() {
    vector<vector<Token*> >::iterator it;
    
    for(it = tokLines.begin(); it != tokLines.end(); ++it){
        vector<Token*>::iterator it2;
        for(it2 = it->begin(); it2 != it->end(); ++it2){
            cerr << "  Token: "  << *(*it2) << endl;
        }
        
        cerr << "=========  New Line  =========" << endl;
    }
}

// ===========================

void Encoder::printSymbolTable() {
    for (int i = 0; i < symbolTable.size(); i++) {
        cerr << symbolTable[i].first << " " << symbolTable[i].second << endl;
    }
}



