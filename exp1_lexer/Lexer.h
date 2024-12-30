#ifndef LEXER_H
#define LEXER_H

#include <cstring>
#include <iostream>
#include <fstream>
#include <cctype>
#include <string>
#include <unordered_set>

using namespace std;

// 定义Token类型
enum TokenType {
    IDENTIFIER, // 标识符
    DECIMAL_INTEGER, // 十进制整数
    OCTAL_INTEGER, // 八进制整数
    HEXADECIMAL_INTEGER, // 十六进制整数
    OPERATOR, // 操作符
    KEYWORD, // 关键字
    UNKNOWN // 未知
};

// 定义关键字集合
const unordered_set<string> keywords = {
        "if", "then", "else", "while", "do"
};

// 定义Token结构体
struct Token {
    TokenType type; // Token类型
    string value; // Token值

    Token() {}

    Token(TokenType t, const string &v) : type(t), value(v) {}
};

// 定义Lexer类
class Lexer {
public:
    Lexer(const string &filename); // 构造函数，传入文件名

    Token scan(); // 扫描函数，返回Token

private:
    ifstream ifs; // 文件输入流
};

// 运行Lexer测试函数
void testLexer();

#endif // LEXER_H
