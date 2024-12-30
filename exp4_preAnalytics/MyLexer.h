#include <string>
#include <fstream>
#include <iostream>
#include <map>

using namespace std;

class MyLexer {
public:
    // 文件数据流
    string filePath;
    ifstream readStream;
    // 定义表示单词类型的枚举类型
    enum Type {
        IDENTIFIER = 0, DECIMAL_INT = 1, OCTAL_INT = 2, HEX_INT = 3, OPERATOR = 4, Error = -1
    };

    // 用于返回单词类型的结构体
    struct Token {
        Type type;
        string value;

        Token(Type t, string v) : type(t), value(v) {}
    };

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

    // 扫描一个单词并返回Token
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
                str.clear();
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
                    str.clear();
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
        return {StateToType[state], str};
    }

    // 打印Token
    void printToken(Token token) {
        switch (token.type) {
            case IDENTIFIER:
            case OPERATOR:
                cout << "<" << token.value << ",->" << endl;
                break;
            case DECIMAL_INT:
            case OCTAL_INT:
            case HEX_INT:
                cout << "<" << token.type << "," << token.value << ">" << endl;
                break;
            case Error:
                cout << "To an error state! The value of the words is \"" << token.value << "\"" << endl;
                break;
            default:
                cout << "The type of token is unknown!" << endl;
                break;
        }
    }

    // 测试Lexer
    void testLexer() {
        while (!readStream.eof()) {
            Token token = scan();
            if (token.type != Error)
                printToken(token);
        }
    }

public:
    // 构造函数
    MyLexer(const string &path) : filePath(path) {
        readStream.open(filePath);
        if (!readStream.is_open()) {
            cerr << "File not found!" << path << endl;
            exit(EXIT_FAILURE);
        }
    }

    // 析构函数
    ~MyLexer() {
        if (readStream.is_open()) {
            readStream.close();
        }
    }
};

