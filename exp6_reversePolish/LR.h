#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <stack>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <queue>
#include <set>
#include "MyLexer.h"

using namespace std;

class LR {
public:
    // 构造函数
    LR(string expressionPath = "P:\\Clion\\Compiler\\exp6_reversePolish\\file\\expression.txt") : lexer(expressionPath) {
    }

    string expressionPath = "P:\\Clion\\Compiler\\exp6_reversePolish\\file\\expression.txt";
    string processPath = "P:\\Clion\\Compiler\\exp6_reversePolish\\file\\process.csv";
    string resultPath = "P:\\Clion\\Compiler\\exp6_reversePolish\\file\\result.txt";
    string grammarPath = "P:\\Clion\\Compiler\\exp6_reversePolish\\file\\g.txt";

    MyLexer lexer;
    // 动作类型
    enum type {
        ERROR,//错误
        SHIFT, //移入
        REDUCE, //规约
        GOTO,// 转移
        ACC,//接受
    };
    // 状态表
    struct Action {
        type act;
        int content;
    };

    // 记录LR分析表的数据结构
    struct Production {
        char LHS;      // 左部
        string RHS;    // 右部
    };

    vector<char> Vt; // 非终结符集合
    vector<char> Vn; // 终结符集合
    char S; // 开始符号
    vector<Production> productionTable; // 产生式表

    map<int, map<char, Action>> actionTable; // 动作表
    map<int, map<char, Action>> gotoTable; // 转移表
    map<char, set<char>> firstSet; // FIRST 集
    map<char, set<char>> followSet; // FOLLOW 集
    vector<vector<pair<char, string>>> itemSets; // 项目集族
    map<pair<int, char>, int> goTable;

/**
 * 读取文法文件
 * @param fileName
 */
    void readGrammarFromFile(string fileName = "P:\\Clion\\Compiler\\exp5_LR\\file\\g.txt") {
        ifstream infile(fileName);
        if (!infile.is_open()) {
            cerr << "无法打开文件: " << fileName << endl;
            exit(EXIT_FAILURE);
        }

        string line, all;

        while (getline(infile, line)) {
            // 忽略注释（以 // 开头的内容）
            size_t commentPos = line.find("//");
            if (commentPos != string::npos) {
                line = line.substr(0, commentPos);
            }
            // 去除空格和无效字符
            line = trim(line);
            // 如果行为空，跳过
            if (line.empty()) continue;
            all += line;
        }
        // 分割行中的多个产生式（以分号分隔）
        stringstream ss(all);
        string production;
        while (getline(ss, production, ';')) {
            production = trim(production); // 去除空格
            if (!production.empty()) {
                parseSingleProduction(production);
            }
        }
        infile.close();
        // 拓展文法
        extendGrammar();
    }

    // 检查文法是否符合拓展文法的要求
    void extendGrammar() {
        if (productionTable[0].LHS != productionTable[1].LHS) return;
        char newChar = getAnUnusedChar();
        Production newProduction = {newChar, string(1, productionTable[0].LHS)};
        S = newChar;
        Vn.insert(Vn.begin(), newChar);
        productionTable.insert(productionTable.begin(), newProduction);
    }

    // 获取一个未被使用的字符
    char getAnUnusedChar() {
        char c = 'A';
        bool isUsed = false;
        do {
            isUsed = false;
            for (auto p: productionTable) {
                if (p.LHS == c) {
                    isUsed = true;
                }
            }
            if (isUsed) c++;
        } while (isUsed);
        return c;
    }

    // 解析单个产生式
    void parseSingleProduction(string production) {
        // 分割产生式，格式为 A->右部
        size_t pos = production.find("->");
        if (pos == string::npos) {
            cerr << "Grammatical formatting error: " << production << endl;
            exit(EXIT_FAILURE);
        }

        char left = production[0]; // 左部
        // 加入非终结符
        if (find(Vn.begin(), Vn.end(), left) == Vn.end())
            Vn.push_back(left);

        string right = production.substr(pos + 2); // 右部

        // 解析右部并加入产生式
        stringstream ss(right);
        string part;
        while (getline(ss, part, '|')) {
            for (auto c: part) {
                // 加入非终结符
                if (!isupper(c) && find(Vt.begin(), Vt.end(), c) == Vt.end())
                    Vt.push_back(c);
            }
            productionTable.push_back({left, trim(part)});
        }
    }

    // 清理字符串：去除空格和回车换行符
    string trim(const string &str) const {
        string result = str;
        result.erase(remove(result.begin(), result.end(), ' '), result.end());
        result.erase(remove(result.begin(), result.end(), '\n'), result.end());
        result.erase(remove(result.begin(), result.end(), '\r'), result.end());
        return result;
    }

    /**
     * 构造项目集族
     */
    void createItemSets() {
        itemSets.clear();
        vector<pair<char, string>> temp(1, {productionTable[0].LHS, "." + productionTable[0].RHS});
        // 构造闭包
        createClosure(temp);
        itemSets.push_back(temp); // 初始项目集
        // 递归构造所有项目集族
        int len;
        do {
            len = itemSets.size();
            for (int i = 0; i < len; i++) {
                addSet(i);
            }
        } while (len < itemSets.size());

    }


    // 构造从itemSets[i]开始的项目集族
    void addSet(int order) {
        // 记录所有待加入项目集族的项目集
        map<char, vector<pair<char, string>>> tempAll;
        vector<char> charOrder;
        for (const auto &item: itemSets[order]) {
            auto index = item.second.find('.');
            if (index + 1 < item.second.size()) {
                char nextChar = item.second[index + 1];
                // 将"."后移一位
                auto newItem = item;
                newItem.second[index] = newItem.second[index + 1];
                newItem.second[index + 1] = '.';
                // 确定顺序
                if (!tempAll.count(nextChar))charOrder.push_back(nextChar);
                // 将新项目加入tempAll
                tempAll[nextChar].push_back(newItem);
            }
        }
        // 构造闭包并添加到DFA
        for (auto ch: charOrder) {
            createClosure(tempAll[ch]);
            // 判断是否已经在itemSets中
            int index = findInItemSets(tempAll[ch]);
            if (index == -1) {
                goTable[{order, ch}] = itemSets.size();
                itemSets.push_back(tempAll[ch]);
            } else {
                goTable[{order, ch}] = index;
            }
        }
    }

    // 判断项目集是否已经在itemSets中
    int findInItemSets(vector<pair<char, string>> &vector) {
        for (int i = 0; i < itemSets.size(); i++) {
            if (itemSets[i].size() != vector.size()) continue;
            int sameNumber = 0;
            for (auto j: vector) {
                bool isSame = false;
                for (auto k: itemSets[i]) {
                    if (j.first == k.first && j.second == k.second) {
                        isSame = true;
                        break;
                    }
                }
                if (isSame)sameNumber++;
            }
            if (sameNumber >= vector.size()) return i;
        }
        return -1;
    }


    // 构造闭包
    void createClosure(vector<pair<char, string>> &temp) {
        int len;
        map<int, bool> isAdded;
        do {
            len = temp.size();
            for (auto &item: temp) {
                auto index = item.second.find('.');
                if (index + 1 >= item.second.size()) continue;
                char nextChar = item.second[index + 1];
                if (isupper(nextChar)) {
                    for (int i = 0; i < productionTable.size(); i++) {
                        if (productionTable[i].LHS == nextChar && !isAdded[i]) {
                            temp.push_back({nextChar, "." + productionTable[i].RHS});
                            isAdded[i] = true;
                        }
                    }
                }
            }
        } while (len < temp.size());
    }

    /**
     * 构造SLR分析表
     */
    void constructSLRTable() {
        // 初始化
        for (int i = 0; i < itemSets.size(); i++) {
            for (char j: Vt)
                actionTable[i][j] = {ERROR, -1};

            for (char j: Vn)
                if (j != S)
                    gotoTable[i][j] = {ERROR, -1};
        }
        //构造Action表和Goto表
        for (int i = 0; i < itemSets.size(); i++) {
            for (char &j: Vt) {
                // 移进
                if (goTable.count({i, j})) {
                    actionTable[i][j] = {SHIFT, goTable[{i, j}]};
                }
            }
            for (char &j: Vn) {
                // 转移
                if (goTable.count({i, j})) {
                    gotoTable[i][j] = {GOTO, goTable[{i, j}]};
                }
            }
            for (const auto item: itemSets[i]) {
                // 规约
                if (item.second.back() == '.') {
                    if (item.first == S) {
                        // 接受
                        actionTable[i]['#'] = {ACC, -1};
                    } else {
                        string right = item.second.substr(0, item.second.size() - 1);
                        int index;
                        for (int j = 0; j < productionTable.size(); j++) {
                            if (productionTable[j].LHS == item.first && productionTable[j].RHS == right) {
                                index = j;
                                break;
                            }
                        }
                        for (char ch: followSet[item.first]) {
                            actionTable[i][ch] = {REDUCE, index};
                        }
                    }
                }
            }
        }
    }

    // 将SLR分析表写入文件
    void writeSLRTable(string path = "P:\\Clion\\Compiler\\exp5_LR\\file\\LR_Table.csv") {
        ofstream file(path);
        if (!file.is_open()) {
            cerr << "Failed to open file: " << path << endl;
            return;
        }
        file << "状态,";
        for (const auto &ch: Vt)file << ch << ",";
        file << "#,";
        for (const auto &ch: Vn) {
            if (ch != S) {
                file << ch;
                if (ch != Vn.back())file << ",";
            }
        }
        file << endl;
        for (int i = 0; i < itemSets.size(); i++) {
            file << i << ",";
            for (const auto &ch: Vt) {
                if (actionTable[i][ch].act == SHIFT) file << "s" << actionTable[i][ch].content << ",";
                else if (actionTable[i][ch].act == REDUCE) file << "r" << actionTable[i][ch].content << ",";
                else file << " ,";
            }
            auto type = actionTable[i]['#'].act;
            if (type == ACC) file << "acc";
            else if (type == SHIFT) file << "s" << actionTable[i]['#'].content;
            else if (type == REDUCE) file << "r" << actionTable[i]['#'].content;
            else file << " ";
            file << ",";
            for (const auto &ch: Vn) {
                if (ch == S)continue;
                else if (gotoTable[i][ch].act == GOTO) file << gotoTable[i][ch].content;
                else file << " ";
                if (ch != Vn.back())file << ",";
            }
            file << endl;
        }
    }
    /**
     * First 集合和 Follow 集合的构建
     */
    // 计算 FIRST 集
    void computeFirst() {
        for (auto &prod: productionTable) {
            if (firstSet.count(prod.LHS) == 0) {
                firstSet[prod.LHS] = {}; // 初始化
            }
        }

        bool changed;
        do {
            changed = false;
            for (auto &prod: productionTable) {
                char nonTerminal = prod.LHS;
                string rhs = prod.RHS;

                set<char> tempFirst = computeFirstForString(rhs);

                // Merge new FIRST symbols into the set
                size_t prevSize = firstSet[nonTerminal].size();
                firstSet[nonTerminal].insert(tempFirst.begin(), tempFirst.end());
                if (firstSet[nonTerminal].size() > prevSize) {
                    changed = true;
                }
            }
        } while (changed);
    }

    // 获取一个字符串的 FIRST 集
    set<char> computeFirstForString(const string &str) {
        set<char> result;
        bool nullable = true; // 是否可以为空

        for (char symbol: str) {
            if (isTerminal(symbol)) {
                result.insert(symbol);
                nullable = false;
                break;
            } else if (isNonTerminal(symbol)) {
                set<char> symbolFirst = firstSet[symbol];
                result.insert(symbolFirst.begin(), symbolFirst.end());
                if (symbolFirst.count('@') == 0) {
                    nullable = false;
                    break;
                } else {
                    result.erase('@'); // 空串不传播到后续符号
                }
            }
        }

        if (nullable) {
            result.insert('@'); // 整个字符串可以为空
        }
        return result;
    }

    // 计算 FOLLOW 集
    void computeFollow() {
        for (auto &prod: productionTable) {
            followSet[prod.LHS] = {}; // 初始化 FOLLOW 集
        }
        followSet[productionTable[0].LHS].insert('#'); // 起始符号的 FOLLOW 集包含终止符号 #

        bool changed;
        do {
            changed = false;
            for (auto &prod: productionTable) {
                char lhs = prod.LHS;
                string rhs = prod.RHS;

                for (size_t i = 0; i < rhs.size(); i++) {
                    char symbol = rhs[i];
                    if (isNonTerminal(symbol)) {
                        set<char> tempFollow;

                        // 获取后续符号的 FIRST 集
                        if (i + 1 < rhs.size()) {
                            string suffix = rhs.substr(i + 1);
                            set<char> suffixFirst = computeFirstForString(suffix);
                            tempFollow.insert(suffixFirst.begin(), suffixFirst.end());
                            tempFollow.erase('@'); // 空串不加入 FOLLOW 集
                        }

                        // 如果后续符号可空，或者是最后一个符号，加入左部的 FOLLOW 集
                        if (i + 1 == rhs.size() || computeFirstForString(rhs.substr(i + 1)).count('@')) {
                            tempFollow.insert(followSet[lhs].begin(), followSet[lhs].end());
                        }

                        // Merge new FOLLOW symbols into the set
                        size_t prevSize = followSet[symbol].size();
                        followSet[symbol].insert(tempFollow.begin(), tempFollow.end());
                        if (followSet[symbol].size() > prevSize) {
                            changed = true;
                        }
                    }
                }
            }
        } while (changed);
    }

    // 判断是否是终结符
    bool isTerminal(char c) {
        return !isNonTerminal(c) && c != '@';
    }

    // 判断是否是非终结符
    bool isNonTerminal(char c) {
        for (auto &prod: productionTable) {
            if (prod.LHS == c) return true;
        }
        return false;
    }

    // 打印 FIRST 集
    void printFirstSet() {
        for (auto &entry: firstSet) {
            cout << "FIRST(" << entry.first << ") = {";
            for (auto it = entry.second.begin(); it != entry.second.end(); ++it) {
                cout << *it;
                if (next(it) != entry.second.end()) cout << ", ";
            }
            cout << "}" << endl;
        }
    }

    // 打印 FOLLOW 集
    void printFollowSet() {
        for (auto &entry: followSet) {
            cout << "FOLLOW(" << entry.first << ") = {";
            for (auto it = entry.second.begin(); it != entry.second.end(); ++it) {
                cout << *it;
                if (next(it) != entry.second.end()) cout << ", ";
            }
            cout << "}" << endl;
        }
    }

    /**
     * LR分析
     */
    struct Node {
        int value;
        MyLexer::Token token;
        Node *left;
        Node *right;

        Node() : value(), token(), left(nullptr), right(nullptr) {}

    };


    void printLastTree(Node *node) {
        if (node != nullptr) {
            printLastTree(node->left);
            printLastTree(node->right);
            if (node->token.type != -1)cout << node->token.value << ",";
        }
    }

    void writeResult(string expression, Node *node, ofstream &resultStream) {
        resultStream << expression << endl;
        writeNode(node, resultStream);
        resultStream << "\t" << node->value << endl << endl;
    }

    void writeNode(Node *node, ofstream &resultStream) {
        if (node != nullptr) {
            writeNode(node->left, resultStream);
            writeNode(node->right, resultStream);
            if (node->token.type != -1)resultStream << node->token.value << ",";
        }
    }

    // 测试所有表达式
    void testAllExpressions() {
        ofstream processStream(processPath);
        ofstream resultStream(resultPath);
        while (!lexer.readStream.eof()) {
            string originalExpression;
            vector<pair<char, MyLexer::Token>> expressionVector;
            while (!lexer.readStream.eof() && lexer.readStream.peek() != ';') {
                MyLexer::Token token = lexer.scan();
                originalExpression += token.value;
                if (token.type == MyLexer::Type::IDENTIFIER || token.type == MyLexer::Type::DECIMAL_INT ||
                    token.type == MyLexer::Type::OCTAL_INT || token.type == MyLexer::Type::HEX_INT) {
                    expressionVector.push_back({'i', token});
                } else if (token.type == MyLexer::Type::OPERATOR) {
                    expressionVector.push_back({token.value[0], token});
                } else {
                    return;
                }
            }


            string expression = getExpression(expressionVector);
            cout << expression << endl;
            // 过程
            processStream << originalExpression << ",--->," << expression << ", , " << endl;
            processStream << "步骤,状态栈,符号栈,输入串,备注," << endl;
            // 获取语法分析树的同时记录过程
            Node *root = testSingleExpression(expressionVector, processStream);
            // 换行
            processStream << " , , , , " << endl;
            // 控制台输出结果
            cout << originalExpression << endl;
            printLastTree(root);
            cout << "\t" << root->value << endl;
            cout << endl << endl;
            // 文件输出结果
            writeResult(originalExpression, root, resultStream);
            // 忽略分号
            lexer.readStream.ignore();
        }
    }

    // 获取处理后的表达式
    string getExpression(vector<pair<char, MyLexer::Token>> expression) {
        string s;
        for (auto &i: expression) {
            s += i.first;
        }
        return s;
    }

    // 测试单个表达式
    Node *testSingleExpression(vector<pair<char, MyLexer::Token>> expressionVector, ofstream &processStream) {
        stack<Node *> nodeStack; // 节点栈 用于存放待规约的各子树
        int step = 0; //记录步骤
        stack<int> stateStack({0}); // 状态栈
        stack<char> symbolStack({'#'}); // 符号栈
        stack<pair<char, MyLexer::Token>> expressionStack({{'#', MyLexer::Token()}}); // 符号栈和表达式栈
        reverse(expressionVector.begin(), expressionVector.end());
        for (const auto &x: expressionVector)
            expressionStack.push(x);
        // 记录LR分析过程
        logPrecess(processStream, step, stateStack, symbolStack, expressionStack);
        while (!stateStack.empty() && !expressionStack.empty() && !symbolStack.empty()) {
            string comment = " ";
            Action action = actionTable[stateStack.top()][expressionStack.top().first];
            if (action.act == ERROR) return nullptr;
            else if (action.act == ACC) return nodeStack.top();
            else if (action.act == SHIFT) {
                //移进
                stateStack.push(action.content);
                symbolStack.push(expressionStack.top().first);
                //// 将规约的字符存入节点栈
                Node *node = new Node();
                node->token = expressionStack.top().second;
                MyLexer::Token token = expressionStack.top().second;
                if (token.type != MyLexer::Error && token.type != MyLexer::OPERATOR)
                    node->value = stoi(token.value);
                else node->value = 0;
                nodeStack.push(node);
                // 弹出输入串
                expressionStack.pop();
            } else if (action.act == REDUCE) {
                // 规约
                Production production = productionTable[action.content];
                comment = string(1, production.LHS) + "->" + production.RHS;
                for (int i = 0; i < production.RHS.size(); i++) {
                    stateStack.pop();
                    // 弹出符号栈
                    symbolStack.pop();
                }
                symbolStack.push(production.LHS);
                stateStack.push(gotoTable[stateStack.top()][production.LHS].content);
                // 将规约的字符存入节点栈
                dealWithNode(nodeStack, action.content);
            }
            logPrecess(processStream, ++step, stateStack, symbolStack, expressionStack, comment);
        }
        return nullptr;
    }

    void dealWithNode(stack<Node *> &nodeStack, int num) {
        Node *node = new Node();
        char charArray[] = {'L', 'E', 'E', 'T', 'T', 'F', 'F'};
        node->token.value = string(1, charArray[num]);
        switch (num) {
            case 1:
                node->value = nodeStack.top()->value;
                node->right = nodeStack.top();
                nodeStack.pop();
                node->token = nodeStack.top()->token;
                nodeStack.pop();
                node->value += nodeStack.top()->value;
                node->left = nodeStack.top();
                nodeStack.pop();
                break;
            case 2:
            case 4:
            case 6:
                node->value += nodeStack.top()->value;
                node->left = nodeStack.top();
                nodeStack.pop();
                break;
            case 3:
                node->value = nodeStack.top()->value;
                node->right = nodeStack.top();
                nodeStack.pop();
                node->token = nodeStack.top()->token;
                nodeStack.pop();
                node->value *= nodeStack.top()->value;
                node->left = nodeStack.top();
                nodeStack.pop();
                break;
            case 5:
                nodeStack.pop();
                node->value = nodeStack.top()->value;
                node->left = nodeStack.top();
                nodeStack.pop();
                nodeStack.pop();
                break;
            default:
                cerr << "Error: dealWithNode" << endl;
        }
        nodeStack.push(node);
    }

    // 记录符号栈的变化
    void logPrecess(ofstream &processStream, int step, stack<int> stateStack,
                    stack<char> symbolStack,
                    stack<pair<char, MyLexer::Token>> expressionStack,
                    const string &comment = " ") {
        string allState, allSymbol, allExpression;
        vector<int> allStateInt;
        while (!stateStack.empty()) {
            allStateInt.push_back(stateStack.top());
            stateStack.pop();
        }
        reverse(allStateInt.begin(), allStateInt.end());
        for (int i: allStateInt) allState += to_string(i) + " ";

        while (!symbolStack.empty()) {
            allSymbol += string(1, symbolStack.top()) + " ";
            symbolStack.pop();
        }
        while (!expressionStack.empty()) {
            allExpression += string(1, expressionStack.top().first) + " ";
            expressionStack.pop();
        }
        reverse(allSymbol.begin(), allSymbol.end());
        processStream << step << "," << allState << "," << allSymbol << "," << allExpression << "," << comment << endl;
    }
};