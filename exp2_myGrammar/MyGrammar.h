#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <sstream>
#include <algorithm>

using namespace std;

class Grammar {
public:
    set<char> Vn;                // 非终结符集合
    set<char> Vt;                // 终结符集合
    char S;                      // 开始符号
    vector<pair<char, vector<string>>> P; // 产生式，键为非终结符，值为产生式右部的集合


    // 清理字符串：去除空格和回车换行符
    string trim(const string &str) const {
        string result = str;
        result.erase(remove(result.begin(), result.end(), ' '), result.end());
        result.erase(remove(result.begin(), result.end(), '\n'), result.end());
        result.erase(remove(result.begin(), result.end(), '\r'), result.end());
        return result;
    }

    // 查找非终结符再数组中的位置
    int findVnInP(char c) {
        for (int i = 0; i < P.size(); i++)
            if (P[i].first == c)
                return i;
        return -1;
    }

    // 解析单个产生式
    void parseSingleProduction(const string &production, bool &isFirstProduction) {
        // 分割产生式，格式为 A->右部
        size_t pos = production.find("->");
        if (pos == string::npos) {
            cerr << "Grammatical formatting error: " << production << endl;
            exit(EXIT_FAILURE);
        }

        char left = production[0]; // 左部
        string right = production.substr(pos + 2); // 右部

        // 设置开始符号
        if (isFirstProduction) {
            S = left;
            isFirstProduction = false;
        }

        // 加入非终结符集合
        Vn.insert(left);

        // 解析右部并加入产生式
        stringstream ss(right);
        string part;
        while (getline(ss, part, '|')) {
            int index = findVnInP(left);
            if (index == -1)
                P.push_back({left, {part}});
            else
                P[index].second.push_back(part);
            // 解析右部的符号
            for (char c: part) {
                if (isupper(c)) {
                    Vn.insert(c);
                } else {
                    Vt.insert(c);
                }
            }
        }
    }

    // 读取文法文件并解析为Grammar结构
    void readFromFile(const string &filename) {
        ifstream infile(filename);
        if (!infile.is_open()) {
            cerr << "无法打开文件: " << filename << endl;
            exit(EXIT_FAILURE);
        }

        string line, all;
        bool isFirstProduction = true;

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
                parseSingleProduction(production, isFirstProduction);
            }
        }
        infile.close();
    }

    // 输出整理后的文法
    void print() const {
        for (const auto &[nonTerminal, productions]: P) {
            cout << nonTerminal << " -> ";
            for (size_t i = 0; i < productions.size(); ++i) {
                cout << productions[i];
                if (i < productions.size() - 1) cout << " | ";
            }
            cout << " ;" << endl; // 输出时重新加上分号
        }
    }
};

