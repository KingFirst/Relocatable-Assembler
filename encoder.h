#ifndef ENCODER_H_
#define ENCODER_H_

#include "kind.h"
#include "lexer.h"

using ASM::Token;
using std::string;


class Encoder {
private:
    // private members
    std::vector< std::vector<Token*> > tokLines;
    std::vector<std::pair<string, int> >symbolTable;
    std::vector<int>relocationTable;
    std::vector<int>printTable;
    int pc; // program counter
    int address;
    int length;
    int clength;
    
    // private methods
    void scan();  // implement the tokLines
    
    void makeSymbolTable(); // implement the symbol table
    
    void putWord(int word); // convert integer into object code
    
    bool isOprand(Token *it); // true if it is one of MIPS opearand
    int searchLabel(string name);
    
    void setMERL();
    void printHeader();
    void printCode();
    void printNote();
    
    
    // methods convert opcode and registers into one 32-bit instruction
    // add, sub, slt, sltu:
    int binaryThree(string op, int rd, int rs, int rt);
    // mult, multu, div, divu:
    int binaryTwo(string op, int rs, int rt);
    // mfhi, mflo, lis:
    int binaryOne(string op, int rd);
    // I-format instructions, lw, sw:
    int binaryLwSw(string op, int rt, int rs, int i);
    // branch, beq, bne:
    int binaryBranch(string op, int rs, int rt, int i);
    // jump, jr, jalr:
    int binaryJump(string op, int rs);
    
public:
    Encoder();
    ~Encoder();
    void encoding();
    // these two are for testing purpose:
    void printToken();
    void printSymbolTable();
};





#endif  // ENCODER_H_


