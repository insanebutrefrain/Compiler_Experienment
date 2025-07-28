#include "LR.h"

int main() {
    LR lr; // 初始化 LR 分析器
    lr.readGrammarFromFile(); // 从文法文件加载文法
    lr.computeFirst(); // 计算 FIRST 集
    lr.printFirstSet();
    lr.computeFollow(); // 计算 FOLLOW 集
    lr.printFollowSet();
    lr.createItemSets();
    lr.constructSLRTable();
    lr.writeSLRTable();
    cout << endl;
    lr.testAllExpressions();
    return 0;
}