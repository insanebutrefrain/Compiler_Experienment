#include "LL1Grammar.h"
#include "MyLexer.h"
#include <stack>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>

class PredictiveAnalytics {
public:
    LL1Grammar grammar;
    MyLexer lexer;
    map<pair<char, char>, string> parseTable;
    string resultFilePath = "P:\\Clion\\Compiler\\exp4_preAnalytics\\file\\result.txt";
    string processFilePath = "P:\\Clion\\Compiler\\exp4_preAnalytics\\file\\process.csv";

    // 构造函数
    PredictiveAnalytics(const string &grammarFilePath = "P:\\Clion\\Compiler\\exp4_preAnalytics\\file\\g.txt",
                        const string &expressionFilePath = "P:\\Clion\\Compiler\\exp4_preAnalytics\\file\\expression.txt")
            : grammar(), lexer(expressionFilePath) {
        grammar.readGrammar(grammarFilePath);
        grammar.transformToLL1();
        constructParseTable();
    }

    // 构造 LL(1) 分析表
    void constructParseTable() {
        parseTable.clear(); // 初始化解析表
        // 遍历每个非终结符
        for (auto &nt: grammar.Vn) {
            // 遍历与该非终结符相关的每个产生式
            for (auto &rhs: grammar.P[grammar.findIndexInP(nt)].second) {
                // 计算该产生式右部的 FIRST 集
                set<char> rhsFirstSet = getFirst(rhs);

                // 将产生式加入 parseTable，对于 FIRST 集中的所有符号
                for (char terminal: rhsFirstSet) {
                    if (terminal == '@') continue; // 空串不直接加入表
                    parseTable[{nt, terminal}] = rhs; // 存储产生式
                }

                // 如果 FIRST 集中包含空串，将 FOLLOW 集中的符号也加入表
                if (rhsFirstSet.count('@')) {
                    set<char> followSet = getFollow(nt);
                    for (char terminal: followSet) {
                        parseTable[{nt, terminal}] = rhs; // 存储完整产生式
                    }
                }
            }
        }
    }


    // 获取一个字符串的 FIRST 集
    set<char> getFirst(const string &str) {
        set<char> firstSet;
        bool isNullable = true;

        for (char symbol: str) {
            if (grammar.Vt.count(symbol)) {
                // 如果是终结符，直接加入 FIRST 集
                firstSet.insert(symbol);
                isNullable = false;
                break;
            } else if (grammar.Vn.count(symbol)) {
                // 如果是非终结符，加入其 FIRST 集
                set<char> symbolFirstSet = computeFirst(symbol);
                firstSet.insert(symbolFirstSet.begin(), symbolFirstSet.end());
                if (symbolFirstSet.count('@') == 0) {
                    isNullable = false;
                    break;
                } else {
                    // 如果包含空串，继续处理下一个符号
                    firstSet.erase('@');
                }
            }
        }
        if (isNullable) {
            // 如果整个字符串可空，加入空串
            firstSet.insert('@');
        }
        return firstSet;
    }

    // 递归计算非终结符的 FIRST 集
    set<char> computeFirst(char c) {
        static map<char, set<char>> firstCache; // 用于缓存 FIRST 集以避免重复计算
        if (firstCache.count(c)) {
            return firstCache[c];
        }

        set<char> firstSet;

        for (auto &production: grammar.P) {
            if (production.first == c) {
                for (const string &rhs: production.second) {
                    set<char> rhsFirstSet = getFirst(rhs);
                    firstSet.insert(rhsFirstSet.begin(), rhsFirstSet.end());
                }
            }
        }
        // 缓存计算结果
        firstCache[c] = firstSet;
        return firstSet;
    }

    //打印预测分析表到文件中
    void writeParseTableToFile(
            const string &filename = "P:\\Clion\\Compiler\\exp4_preAnalytics\\file\\predictive_table.csv") {
        ofstream outFile(filename);
        if (!outFile) {
            cerr << "Error: Unable to open file " << filename << " for writing." << endl;
            return;
        }
        outFile << " ,";
        for (auto i: grammar.Vt)
            outFile << i << ",";
        outFile << "#\n";
        for (auto i: grammar.P) {
            outFile << i.first << ",";
            for (auto j: grammar.Vt) {
                if (parseTable.count({i.first, j}))
                    outFile << i.first << "->" << parseTable[{i.first, j}];
                else outFile << " ";
                outFile << ",";
            }
            if (parseTable.count({i.first, '#'}))
                outFile << i.first << "->" << parseTable[{i.first, '#'}];
            else outFile << " ";
            outFile << "\n";
        }
    }

    // 获取 FOLLOW 集
    set<char> getFollow(char nonTerminal) {
        static map<char, set<char>> followCache; // 用于缓存 FOLLOW 集
        static bool isInitialized = false;

        if (!isInitialized) {
            // 初始化 FOLLOW 集
            for (auto &nt: grammar.Vn) {
                followCache[nt] = {};
            }
            // 起始符号 FOLLOW 集包含结束标志符
            followCache[grammar.S].insert('#');
            isInitialized = true;
        }

        bool changed = true; // 标志是否有变化
        while (changed) {
            changed = false;

            // 遍历所有产生式
            for (auto &production: grammar.P) {
                char lhs = production.first; // 左部非终结符

                for (const string &rhs: production.second) {
                    for (size_t i = 0; i < rhs.size(); i++) {
                        char symbol = rhs[i];

                        if (grammar.Vn.count(symbol)) { // 如果是非终结符
                            set<char> followSetTmp;

                            // 如果是最后一个符号，将左部 FOLLOW 集加入
                            if (i == rhs.size() - 1) {
                                followSetTmp = followCache[lhs];
                            } else {
                                // 获取后续符号的 FIRST 集
                                string suffix = rhs.substr(i + 1);
                                set<char> firstSet = getFirst(suffix);

                                // 将非空符号加入 FOLLOW 集
                                for (char firstSymbol: firstSet) {
                                    if (firstSymbol != '@') {
                                        followSetTmp.insert(firstSymbol);
                                    }
                                }

                                // 如果后续符号可空，将左部 FOLLOW 集加入
                                if (firstSet.count('@')) {
                                    followSetTmp.insert(followCache[lhs].begin(), followCache[lhs].end());
                                }
                            }

                            // 更新 FOLLOW 集
                            size_t prevSize = followCache[symbol].size();
                            followCache[symbol].insert(followSetTmp.begin(), followSetTmp.end());
                            if (followCache[symbol].size() > prevSize) {
                                changed = true;
                            }
                        }
                    }
                }
            }
        }

        return followCache[nonTerminal];
    }

    void printFirstSet() {
        for (auto i: grammar.Vn) {
            cout << "FIRST(" << i << ") = {";
            auto firstSet = computeFirst(i);
            for (auto j: firstSet) {
                cout << j;
                if (++firstSet.find(j) != firstSet.end())cout << ", ";
            }
            cout << "}" << endl;
        }
    }

    // 打印 FOLLOW 集
    void printFollowSet() {
        for (auto nonTerminal: grammar.Vn) {
            cout << "FOLLOW(" << nonTerminal << ") = {";
            set<char> followSet = getFollow(nonTerminal);
            for (char symbol: followSet) {
                cout << symbol;
                if (++followSet.find(symbol) != followSet.end())cout << ", ";
            }
            cout << "}" << endl;
        }
    }

    // 分隔字符串的工具
    vector<string> split(const string &str, char delimiter) {
        vector<string> tokens;
        istringstream stream(str);
        string token;

        while (getline(stream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    // 测试所有表达式
    void testExpressions(string filePath = "P:\\Clion\\Compiler\\exp4_preAnalytics\\file\\expression.txt") {
        // 用于写入结果和过程的文件
        ofstream resultStream(resultFilePath);
        ofstream processStream(processFilePath);
        if (!resultStream.is_open()) {
            cerr << "Failed to open file: " << resultFilePath << endl;
            return;
        }
        if (!processStream.is_open()) {
            cerr << "Failed to open file: " << processFilePath << endl;
            return;
        }
        while (!lexer.readStream.eof()) {
            string originalExpression, expression;
            while (!lexer.readStream.eof() && lexer.readStream.peek() != ';') {
                MyLexer::Token token = lexer.scan();
                originalExpression += token.value;
                if (token.type == MyLexer::Type::IDENTIFIER || token.type == MyLexer::Type::DECIMAL_INT ||
                    token.type == MyLexer::Type::OCTAL_INT || token.type == MyLexer::Type::HEX_INT) {
                    expression += 'i';
                } else if (token.type == MyLexer::Type::OPERATOR) { expression += token.value; }
                else {
                    return;
                }
            }
            processStream << originalExpression << ",--->," << expression << ", " << endl;
            bool isCorrect = testSingleExpression(expression, processStream);
            processStream << " , , , " << endl;
            cout << originalExpression << "\t" << (isCorrect ? "Correct" : "Wrong") << endl;
            resultStream << originalExpression << " : " << (isCorrect ? "正确" : "错误") << endl;
            lexer.readStream.ignore();
        }
    }

// 解析单个表达式
    bool testSingleExpression(const string &expression, ofstream &processStream) {
        stack<char> symbolStack({'#', grammar.S}), expressionStack({'#'});
        for (int i = expression.size() - 1; i >= 0; i--) {
            expressionStack.push(expression[i]);
        }
        processStream << "步骤, 符号栈,输入串,备注" << endl;
        int step = 0;
        logState(step, processStream, symbolStack, expressionStack);
        while (!symbolStack.empty() || !expressionStack.empty()) {
            if (parseTable.count({symbolStack.top(), expressionStack.top()})) {
                string choice = parseTable[{symbolStack.top(), expressionStack.top()}];
                string comment = string(1, symbolStack.top()) + " -> " + choice;
                symbolStack.pop();
                reverse(choice.begin(), choice.end());
                for (char symbol: choice) {
                    if (symbol != '@')
                        symbolStack.push(symbol);
                }
                logState(++step, processStream, symbolStack, expressionStack, comment);
            } else if (symbolStack.top() == expressionStack.top()) {
                while (!symbolStack.empty() && !expressionStack.empty() && symbolStack.top() == expressionStack.top()) {
                    symbolStack.pop();
                    expressionStack.pop();
                }
                if (!symbolStack.empty() && !expressionStack.empty())
                    logState(++step, processStream, symbolStack, expressionStack);
            } else {
                return false;
            }
        }
        return true;
    }

    // 记录状态
    void logState(int step, ofstream &processStream, stack<char> symbolStack, stack<char> expressionStack,
                  const string &comment = " ") {
        string symbol, expression;
        while (!symbolStack.empty()) {
            symbol += symbolStack.top();
            symbolStack.pop();
        }
        while (!expressionStack.empty()) {
            expression += expressionStack.top();
            expressionStack.pop();
        }
        reverse(symbol.begin(), symbol.end());
        processStream << step << "," << symbol << "," << expression << "," << comment << '\n';
    }
};
