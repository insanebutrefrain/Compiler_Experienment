#include "Grammar.h"

int main() {
    Grammar grammar;
    // 从文件中读入文法
    string filename = "P:\\Clion\\Compiler\\exp2_grammar\\g.txt"; // 请确保文件路径正确
    grammar.readFromFile(filename);
    // 输出文法
    grammar.printGrammar();

    return 0;
}
