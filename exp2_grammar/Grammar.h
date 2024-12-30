#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <map>
#include <vector>
#include <sstream>

using namespace std;

// Grammar类定义
class Grammar {
public:
    set<char> Vn;                   // 非终结符集合
    set<char> Vt;                   // 终结符集合
    char S;                         // 开始符
    vector<pair<char, vector<string>>> P;    // 产生式，左部映射到多个右部

    int findVnInP(char c) {
        for (int i = 0; i < P.size(); i++) {
            if (P[i].first == c) {
                return i;
            }
        }
        return -1;
    }

    // 从文件中读取文法
    bool readFromFile(const string &filename) {
        ifstream infile(filename);
        if (!infile.is_open()) {
            // 文件读取失败
            cerr << "File not found!" << filename << endl;
            return false;
        }

        string line; // 分别读取每一行
        bool isFirstLine = true; // 标记第一行（用于S的确定）

        while (getline(infile, line)) {
            // 跳过空行
            if (line.empty()) continue;

            // 分析产生式，格式假设为 "S->ab|c" 等
            stringstream ss(line);
            char left;
            ss >> left;             // 左部的非终结符
            ss.ignore(2);           // 忽略 "->"

            // 将左部字符加入非终结符集合
            Vn.insert(left);

            // 第一行的左部即为开始符
            if (isFirstLine) {
                S = left;
                isFirstLine = false;
            }

            // 读取右部候选式，使用 "|" 分隔
            string right;
            while (getline(ss, right, '|')) {
                if (right[right.size() - 1] == ';')
                    right.pop_back(); // 去除分号
                int index = findVnInP(left);
                if(index == -1)
                    P.push_back({left, {right}});
                else
                    P[index].second.push_back(right);

                // 将右部中的字符加入终结符或非终结符集合
                for (char c: right) {
                    if (isupper(c)) Vn.insert(c); // 大写字母为非终结符
                    else if (islower(c)) Vt.insert(c);            // 小写字母为终结符
                }
            }
        }
        infile.close();
        return true;
    }

    // 输出文法
    void printGrammar() const {
        for (const auto &prod: P) {
            cout << prod.first << " -> ";
            for (size_t i = 0; i < prod.second.size(); ++i) {
                cout << prod.second[i];
                if (i < prod.second.size() - 1) cout << " | ";
                else cout << " ;";
            }
            cout << endl;
        }
    }
};
