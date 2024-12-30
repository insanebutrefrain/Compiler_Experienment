#include "predictiveAnalytics.h"

int main() {
    // 创建预测分析器对象
    // 指定文法读取地址和表达式读取地址
    PredictiveAnalytics analyzer;
    analyzer.grammar.printP();
    analyzer.printFirstSet();
    analyzer.printFollowSet();
    cout << endl;
    // 构建预测分析表
    analyzer.constructParseTable();
    // 将预测分析表写入文件
    analyzer.writeParseTableToFile();
    // 测试表达式
    analyzer.testExpressions();
    return 0;
}
