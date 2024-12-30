#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include <set>

using namespace std;

// 用于读取文件的数据流
ifstream readStream("P:\\Clion\\Compiler\\exp1_myLexer\\program.txt");

// 定义表示单词类型的枚举类型
enum Type {
    IDENTIFIER = 0, DECIMAL_INT = 1, OCTAL_INT = 2, HEX_INT = 3, OPERATOR = 4, KeyWords = 5, Error = -1
};

// 用于返回单词类型的结构体
struct Token {
    Type type;
    string value;

    Token(Type t, string v) : type(t), value(v) {}
};

// 关键字集合
set<string> keyWords = {"if", "else", "then", "while", "do"};

// 对应状态转换图的状态
enum State {
    state0, state1, state2, state3,
    state4, state5, state6, state7,
    state8, state9, state10, state11
};
// 每一个终状对应的类型
map<State, Type> StateToType = {
        {state2,  IDENTIFIER},
        {state4,  DECIMAL_INT},
        {state7,  OCTAL_INT},
        {state10, HEX_INT},
        {state11, OPERATOR},
        {state1,  Error},
        {state3,  Error},
        {state5,  Error},
        {state6,  Error},
        {state8,  Error},
        {state9,  Error},
        {state0,  Error},
};

// 预处理程序，剔除掉空格符、缩进符、换行符、注释和注解
char preTreat(char ch) {
    while (readStream.get(ch)) {
        if (ch == ' ' || ch == '\n' || ch == '\t') {
            continue;
        }
        if (ch == '/') {
            if (readStream.peek() == '/') {
                while (readStream.get(ch)) {
                    if (ch == '\n') {
                        break;
                    }
                }
            } else if (readStream.peek() == '*') {
                readStream.get(ch);
                while (readStream.get(ch)) {
                    if (ch == '*' && readStream.peek() == '/') {
                        readStream.get(ch);
                        break;
                    }
                }
            }
            continue;
        }
        break;
    }
    return ch;
}

/*扫描一个单词
标识符返回0
十进制整数返回1
八进制整数返回2
十六进制整数返回3
运算符和界符返回4*/
Token scan() {
    char ch = ' ';
    string str;
    State state = state0;
    ch = preTreat(ch);
    str += ch;

    if ('a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z' || ch == '_') {
        state = state1;
        while (readStream.get(ch)) {
            if ('a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z' || '0' <= ch && ch <= '9' || ch == '_') {
                str += ch;
            } else {
                state = state2;
                readStream.putback(ch);
                break;
            }
        }
    } else if ('1' <= ch && ch <= '9') {
        state = state3;
        while (readStream.get(ch)) {
            if ('0' <= ch && ch <= '9') {
                str += ch;
            } else {
                state = state4;
                readStream.putback(ch);
                break;
            }
        }
    } else if (ch == '0') {
        state = state5;
        readStream.get(ch);
        if ('1' <= ch && ch <= '7') {
            state = state6;
            // 去除八进制数的0前缀
            str.clear();
            // 去除八进制数的0前缀
            str += ch;
            while (readStream.get(ch)) {
                if ('0' <= ch && ch <= '7') {
                    str += ch;
                } else {
                    state = state7;
                    readStream.putback(ch);
                    break;
                }
            }
        } else if (ch == 'x' || ch == 'X') {
            state = state8;
            str += ch;
            readStream.get(ch);
            if ('1' <= ch && ch <= '9' || 'a' <= ch && ch <= 'f' || 'A' <= ch && ch <= 'F') {
                state = state9;
                // 去除十六进制数的0x前缀
                str.clear();
                // 去除十六进制数的0x前缀
                str += ch;
                while (readStream.get(ch)) {
                    if ('0' <= ch && ch <= '9' || 'a' <= ch && ch <= 'f' || 'A' <= ch && ch <= 'F') {
                        str += ch;
                    } else {
                        state = state10;
                        readStream.putback(ch);
                        break;
                    }
                }
            }
        }
    } else if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '=' || ch == '<' || ch == '>' ||
               ch == '(' || ch == ')' || ch == ';') {
        state = state11;
    }
    if (keyWords.count(str) > 0) return {KeyWords, str};

    return {StateToType[state], str};
}

void printToken(Token token) {
    switch (token.type) {
        case KeyWords:
            cout << "<" << token.value << ",_>" << endl;
            break;
        case OPERATOR:
            cout << "<" << token.value << ",_>" << endl;
            break;
        case IDENTIFIER:
        case DECIMAL_INT:
        case OCTAL_INT:
        case HEX_INT:
            cout << "<" << token.type << "," << token.value << ">" << endl;
            break;
        case Error:
            // 到达错误状态，说明输入的单词在scan函数中状态转换有问题
            cout << "To an error state! The value of the words is \"" << token.value << "\"" << endl;
            break;
        default:
            // 出现错误的类型，可能是因为空指针或脏数据，概率比较小
            cout << "The type of token is unknown!" << endl;
            break;
    }
}

void testMyLexer() {
    // 文件读取失败
    if (!readStream.is_open()) {
        cout << "File not found!" << endl;
        return;
    }
    while (!readStream.eof()) {
        Token token = scan();
        if (token.type != Error)
            printToken(token);
    }
}



