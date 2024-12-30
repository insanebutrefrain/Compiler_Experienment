#include "Lexer.h"

// 构造函数，打开文件
Lexer::Lexer(const string &filename) : ifs(filename) {
    // 打开文件
    if (!ifs.is_open()) {
        // 如果文件打开失败，输出错误信息并退出程序
        cerr << "Error: Could not open file " << filename << endl;
        exit(1); // 退出程序，返回错误
    }
}


// 扫描函数，返回一个Token对象
Token Lexer::scan() {
    char ch;
    while (ifs.get(ch)) {
        if (isspace(ch)) continue; // 跳过空白字符

        // 处理标识符
        if (isalpha(ch) || ch == '_') {
            string identifier(1, ch);
            while (ifs.get(ch) && (isalnum(ch) || ch == '_')) {
                identifier += ch;
            }
            ifs.unget(); // 回退一个字符
            if (keywords.count(identifier)) {
                return Token(KEYWORD, identifier); // 关键字
            }
            return Token(IDENTIFIER, identifier); // 标识符
        }

        // 处理数字
        if (isdigit(ch)) {
            string number(1, ch);
            if (ch == '0') {
                ifs.get(ch);
                if (ch == 'x' || ch == 'X') { // 十六进制
                    string hexValue;
                    while (ifs.get(ch) && (isdigit(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'))) {
                        hexValue += ch;
                    }
                    ifs.unget();
                    return Token(HEXADECIMAL_INTEGER, hexValue); // 返回去掉前缀的十六进制
                } else if (ch >= '1' && ch <= '7') { // 八进制
                    number += ch;
                    while (ifs.get(ch) && (ch >= '0' && ch <= '7')) {
                        number += ch;
                    }
                    ifs.unget();
                    // 去掉八进制前导零
                    if (number.length() > 1 && number[0] == '0') {
                        return Token(OCTAL_INTEGER, string(1, number[1])); // 返回去掉前导零的数字
                    }
                    return Token(OCTAL_INTEGER, number);
                } else {
                    ifs.unget(); // 如果是0，直接返回
                    return Token(OCTAL_INTEGER, "0");
                }
            } else { // 十进制
                while (ifs.get(ch) && isdigit(ch)) {
                    number += ch;
                }
                ifs.unget();

                // 处理前导零
                if (number.length() > 1 && number[0] == '0') {
                    return Token(DECIMAL_INTEGER, string(1, number[1])); // 返回去掉前导零的数字
                }

                return Token(DECIMAL_INTEGER, number); // 返回正常的十进制数字
            }
        }

        // 处理运算符和界符
        if (strchr("+-*/><=();", ch)) {
            return Token(OPERATOR, string(1, ch));
        }

        return Token(UNKNOWN, string(1, ch)); // 未知字符
    }
    return Token(UNKNOWN, ""); // EOF
}

// 运行词法分析器测试函数
void testLexer() {
    const string filename = "P:\\Clion\\Compiler\\exp1_lexer\\program.txt";
    Lexer lexer(filename);
    Token token;

    while ((token = lexer.scan()).type != UNKNOWN) {
        switch (token.type) {
            case IDENTIFIER:
                cout << "<0 , " << token.value << ">\n"; // 标识符
                break;
            case DECIMAL_INTEGER:
                cout << "<1 , " << token.value << ">\n"; // 十进制整数
                break;
            case OCTAL_INTEGER:
                cout << "<2 , " << token.value << ">\n"; // 八进制整数
                break;
            case HEXADECIMAL_INTEGER:
                cout << "<3 , " << token.value << ">\n"; // 十六进制整数
                break;
            case OPERATOR:
                cout << "<" << token.value << " , ->\n"; // 运算符
                break;
            case KEYWORD:
                cout << "<" << token.value << " , ->\n"; // 关键字
                break;
            case UNKNOWN:
            default:
                break;
        }
    }
}
