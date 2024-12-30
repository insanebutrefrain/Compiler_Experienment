cmake_minimum_required(VERSION 3.27)
project(Compiler)

set(CMAKE_CXX_STANDARD 17)

add_executable(Compiler main.cpp)

file(GLOB_RECURSE exp1_lexer "exp1_lexer/*.cpp" "exp1_lexer/*.h" "exp1_lexer/*.txt")
add_executable(exp1_lexer ${exp1_lexer})

file(GLOB_RECURSE exp1_myLexer "exp1_myLexer/*.cpp" "exp1_myLexer/*.h" "exp1_myLexer/*.txt")
add_executable(exp1_myLexer ${exp1_myLexer})

file(GLOB_RECURSE exp2_grammar "exp2_grammar/*.cpp" "exp2_grammar/*.h" "exp2_grammar/*.txt")
add_executable(exp2_grammar ${exp2_grammar})

file(GLOB_RECURSE exp2_myGrammar "exp2_myGrammar/*.cpp" "exp2_myGrammar/*.h" "exp2_myGrammar/*.txt")
add_executable(exp2_myGrammar ${exp2_myGrammar})

file(GLOB_RECURSE exp3_LL1 "exp3_LL1/*.cpp" "exp3_LL1/*.h" "exp3_LL1/*.txt")
add_executable(exp3_LL1 ${exp3_LL1})


file(GLOB_RECURSE exp4_predictive "exp4_preAnalytics/*.cpp" "exp4_preAnalytics/*.h" "exp4_preAnalytics/*.txt")
add_executable(exp4_predictive ${exp4_predictive})

file(GLOB_RECURSE exp5_LR "exp5_LR/*.cpp" "exp5_LR/*.h" "exp5_LR/*.txt")
add_executable(exp5_LR ${exp5_LR})

file(GLOB_RECURSE exp6_reversePolish "exp6_reversePolish/*.cpp" "exp6_reversePolish/*.h" "exp6_reversePolish/*.txt")
add_executable(exp6_reversePolish ${exp6_reversePolish})

# 使用算符优先方法完成的逆波兰式的转换
add_executable(operatorPrecedence exp6_reversePolish/operatorsPrecedence/operatorsPrecedenceMain.cpp)