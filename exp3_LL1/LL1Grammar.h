#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <set>
#include <vector>
#include <sstream>
#include <regex>

using namespace std;

class LL1Grammar {
public:
    set<char> Vn;//非终结符
    set<char> Vt;//终结符
    char S;//开始符号
    vector<pair<char, vector<string>>> P;//产生式
    string grammarPath = "LL1Grammar.txt";
    string resultPath = "../result.txt";

    static std::vector<std::string> split(const std::string &str, char delimiter) {
        std::vector<std::string> tokens;
        std::istringstream stream(str);
        std::string token;
        while (std::getline(stream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    // 分隔字符串的辅助方法
    static vector<string> split(const string &str, const string &delimiter) {
        vector<string> tokens;
        regex re(delimiter);
        sregex_token_iterator it(str.begin(), str.end(), re, -1);
        sregex_token_iterator end;
        while (it != end) {
            tokens.push_back(*it++);
        }
        return tokens;
    }

    // 移除产生式右边重复的待选项
    void removeRepeatedChoices() {
        for (pair<char, vector<string>> &p: P) {
            set<string> choices;
            for (string s: p.second) {
                if (choices.find(s) != choices.end())
                    p.second.erase(find(p.second.begin(), p.second.end(), s));
                choices.insert(s);
            }
        }
    }

    // 移除无用产生式,即移除从开始符号无法到达的产生式
    void removeUnusedProductions() {
        set<char> used({S});
        int len;
        do {
            len = used.size();
            for (char c: used) {
                for (const auto &p: P) {
                    if (p.first == c) {
                        for (auto &j: p.second) {
                            for (char k: j) {
                                if (isupper(k))used.insert(k);
                            }
                        }
                    }
                }
            }
        } while (used.size() > len);
        vector<pair<char, vector<string>>> newP;
        for (auto i = 0; i < P.size(); i++) {
            if (used.count(P[i].first)) {
                newP.push_back(P[i]);
            }
        }
        P = newP;
    }

    // 读取文法文件
    void readGrammar(const string &filename) {
        ifstream readStream(filename);
        if (!readStream.is_open()) {
            cerr << "Failed to open readStream: " << filename << endl;
        }
        string all;
        char c;
        // 读取文件内容，将所有有效内容都读入到一个字符串中
        while (readStream.get(c)) {
            // 异常处理
            if (c == '\n' || c == ' ' || c == '\t')continue;
            if (c == '/' && readStream.peek() == '/')
                while (readStream.get(c) && c != '\n');
            if (c == '/' && readStream.peek() == '*') {
                while (readStream.get(c) && (c != '*' || readStream.peek() != '/'));
                readStream.get(c);
            } else all += c;
        }
        readStream.close();
        // 将字符串分割，得到每个产生式
        vector<string> productions = split(all, ';');
        // 记录开始符号
        bool isFirst = true;
        // 分别处理每个产生式
        for (const auto &i: productions) {
            // 分解产生式
            vector<string> production = split(i, "->");
            const string &left = production[0];
            const string &right = production[1];
            // 提取非终结符
            char leftChar = left[0];
            if (isFirst) {
                S = leftChar;
                isFirst = false;
            }
            if (isupper(leftChar))Vn.insert(leftChar);
            // 获取产生式
            vector<string> choices = split(right, '|');
            for (const auto &k: choices) {
                int index = findIndexInP(leftChar);
                if (index == -1)P.push_back({leftChar, {k}});
                else P[index].second.push_back(k);
                //提取终结符
                for (const auto &j: k) {
                    if (islower(j))Vt.insert(j);
                }
            }
        }
        // 如果有非终结符号S,则默认S是开始符号
        if (Vn.count('S'))S = 'S';
        removeRepeatedChoices();
        removeUnusedProductions();
    }

    //根据字符查找产生式在OriginalGrammar中的下标
    int findIndexInP(char c) {
        for (int i = 0; i < P.size(); i++) {
            if (P[i].first == c)return i;
        }
        return -1;
    }

    //将文法转换为LL1文法
    void transformToLL1() {
        remove_left_recursion();
        cout << "after remove left recursion: " << endl;
        printP();
        remove_left_gene();
        removeRepeatedChoices();
        removeUnusedProductions();
    }

    //获取一个未使用的字符
    char getAnUnusedChar() {
        char c = 'A';
        while (Vn.find(c) != Vn.end())c++;
        return c;
    }

    // 消除左递归
    void remove_left_recursion() {
        // 按照非终结符顺序处理
        for (size_t i = 0; i < P.size(); i++) {
            // 处理间接左递归
            for (size_t j = 0; j < i; j++) {
                vector<string> newChoices;
                for (const string &choice: P[i].second) {
                    if (!choice.empty() && choice[0] == P[j].first) {
                        // 替换间接递归
                        for (const string &replacement: P[j].second) {
                            newChoices.push_back(replacement + choice.substr(1));
                        }
                    } else {
                        newChoices.push_back(choice);
                    }
                }
                P[i].second = newChoices;
            }

            // 处理直接左递归
            vector<string> alpha; // A -> Aα 部分
            vector<string> beta;  // A -> β 部分
            for (const string &choice: P[i].second) {
                if (!choice.empty() && choice[0] == P[i].first) {
                    alpha.push_back(choice.substr(1)); // 提取α部分
                } else {
                    beta.push_back(choice); // 提取β部分
                }
            }

            // 如果存在直接左递归
            if (!alpha.empty()) {
                char newSymbol = getAnUnusedChar();
                Vn.insert(newSymbol); // 新增非终结符
                vector<string> newBeta, newAlpha;

                for (const string &b: beta) {
                    newBeta.push_back(b + newSymbol);
                }
                for (const string &a: alpha) {
                    newAlpha.push_back(a + newSymbol);
                }
                newAlpha.push_back("@"); // 空串 ε
                P[i].second = newBeta;           // 替换原产生式
                P.insert(P.begin() + i + 1, {newSymbol, newAlpha});
            }
        }
    }

    //提取左公因子
    void remove_left_gene() {
        bool isRepeat = false;
        for (pair<char, vector<string>> &p: P) {
            map<char, vector<int>> firstCharIndex;
            for (int i = 0; i < p.second.size(); i++) {
                firstCharIndex[p.second[i][0]].push_back(i);
            }
            for (const pair<const char, vector<int>> &aPair: firstCharIndex) {
                if (aPair.second.size() > 1) {
                    // 观察i.second中的公共前缀能不能延长
                    int len = 1;
                    while (true) {
                        if (p.second[aPair.second[0]].size() < len + 1)break;
                        char tryChar = p.second[aPair.second[0]][len];
                        bool isSame = true;
                        for (int i: aPair.second) {
                            if (p.second[i].size() < len + 1 || p.second[i][len] != tryChar) {
                                isSame = false;
                                break;
                            }
                        }
                        if (isSame)len++;
                        else break;
                    }
                    // 更新产生式并产生新的产生式
                    char newSymbol = getAnUnusedChar();
                    Vn.insert(newSymbol); // 新增非终结符
                    vector<string> newChoices;
                    vector<string> updateChoices;
                    set<char> isAdded;
                    for (int i = 0; i < p.second.size(); i++) {
                        if (find(aPair.second.begin(), aPair.second.end(), i) != aPair.second.end()) {
                            newChoices.push_back(
                                    p.second[i].substr(len).size() == 0 ? "@" : p.second[i].substr(len));
                            if (!isAdded.count(p.second[i][0])) {
                                updateChoices.push_back(p.second[i].substr(0, len) + newSymbol);
                                isAdded.insert(p.second[i][0]);
                            }
                        } else {
                            updateChoices.push_back(p.second[i]);
                        }
                    }
                    p.second = updateChoices;
                    P.insert(find(P.begin(), P.end(), p) + 1, {newSymbol, newChoices});
                    isRepeat = true;
                    break;
                }
            }
            if (isRepeat)break;
        }
        // 再次检查是否需要提取左公因子
        if (isRepeat)remove_left_gene();
    }


    // 打印文法到文件中
    void printP() {
        for (const auto &i: P) {
            cout << i.first << " -> ";
            for (const auto &j: i.second) {
                cout << j;
                if (j != i.second.back())cout << " | ";
            }
            cout << ";" << endl;
        }
        cout << endl;
    }

    // 将文法写入文件中
    void writeP(const string &filename) {
        ofstream writeStream(filename);
        if (!writeStream.is_open()) {
            cerr << "Failed to open writeStream: " << filename << endl;
        }
        for (const auto &i: P) {
            writeStream << i.first << " -> ";
            for (const auto &j: i.second) {
                writeStream << j;
                if (j != i.second.back())writeStream << " | ";
            }
            writeStream << ";" << endl;
        }
    }
};

