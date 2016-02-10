#include "encoder.h"

// Use only the neeeded aspects of each namespace
using std::endl;
using std::cerr;
using std::string;


int main(int argc, char* argv[]){
    Encoder *encoder = new Encoder();
    
    try{
        
        encoder->encoding();
        
    } catch(const string& msg){
        // If an exception occurs print the message and end the program
        cerr << msg << endl;
    }
    // Delete the Tokens that have been made
    delete encoder;
}


