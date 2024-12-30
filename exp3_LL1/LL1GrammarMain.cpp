#include "LL1Grammar.h"

int main() {
    LL1Grammar grammar;
    grammar.readGrammar("P:/Clion/Compiler/exp3_LL1/g.txt");
    cout << "Original Grammar:" << endl;
    grammar.printP();
    grammar.transformToLL1();
    cout << "LL(1) Grammar:" << endl;
    grammar.printP();
    grammar.writeP("P:/Clion/Compiler/exp3_LL1/result.txt");
    cout << "Processing is complete!";
    return 0;
}