#include <iostream>
#include <fstream>
#include <stack>
#include <algorithm>
#include <string>
#include <vector>
#include <cctype>
using namespace std;

// 优先级
int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

// 判断是否是操作符
bool isOperator(char c) {
    return (c == '+' || c == '-' || c == '*' || c == '/');
}

// 判断是否是数字
bool isDigit(char c) {
    return (c >= '0' && c <= '9');
}

// 转换中缀表达式到后缀表达式
vector<string> infixToPostfix(const string &expression) {
    stack<char> operators;
    vector<string> postfix;
    string number = "";

    for (int i = 0; i < expression.size(); ++i) {
        char c = expression[i];

        // 如果是数字，加入到输出
        if (isDigit(c)) {
            number += c;
            if (i == expression.size() - 1 || !isDigit(expression[i + 1])) {
                postfix.push_back(number);
                number = "";
            }
        } else if (c == '(') {
            // 左括号直接压栈
            operators.push(c);
        } else if (c == ')') {
            // 右括号弹出栈直到遇到左括号
            while (!operators.empty() && operators.top() != '(') {
                postfix.push_back(string(1, operators.top()));
                operators.pop();
            }
            operators.pop();  // 弹出左括号
        } else if (isOperator(c)) {
            // 处理操作符
            while (!operators.empty() && precedence(operators.top()) >= precedence(c)) {
                postfix.push_back(string(1, operators.top()));
                operators.pop();
            }
            operators.push(c);
        }
    }

    // 弹出剩余的操作符
    while (!operators.empty()) {
        postfix.push_back(string(1, operators.top()));
        operators.pop();
    }

    return postfix;
}

// 计算后缀表达式的值
int evaluatePostfix(const vector<string> &postfix) {
    stack<int> values;

    for (const string &token : postfix) {
        if (isdigit(token[0])) {
            // 如果是数字，压栈
            values.push(stoi(token));
        } else if (isOperator(token[0])) {
            // 如果是操作符，弹出两个操作数计算结果
            int b = values.top(); values.pop();
            int a = values.top(); values.pop();
            switch (token[0]) {
                case '+': values.push(a + b); break;
                case '-': values.push(a - b); break;
                case '*': values.push(a * b); break;
                case '/': values.push(a / b); break;
            }
        }
    }

    // 栈顶就是最终结果
    return values.top();
}

int main() {
    ifstream inputFile("P:\\Clion\\Compiler\\exp6_reversePolish\\file\\expression.txt");
    if (!inputFile) {
        cerr << "File not found: "<< endl;
        return 1;
    }

    string expression;
    while (getline(inputFile, expression)) {
        // 清除空格
        expression.erase(remove(expression.begin(), expression.end(), ' '), expression.end());

        // 转换为后缀表达式
        vector<string> postfix = infixToPostfix(expression);

        // 输出后缀表达式
        for (const string &token : postfix) {
            cout << token << ", ";
        }
        cout << endl;

        // 计算结果
        int result = evaluatePostfix(postfix);
        cout << "result: " << result << endl;
    }

    inputFile.close();
    return 0;
}
