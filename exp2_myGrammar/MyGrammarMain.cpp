#include "MyGrammar.h"

int main() {
    Grammar grammar;
    grammar.readFromFile("P:\\Clion\\Compiler\\exp2_myGrammar\\g.txt");
    grammar.print();
    return 0;
}
