﻿//
// Created by 84443 on 2019/9/28.
//
#include "ExpressionDeal.h"
using namespace std;

#ifndef COMPILER_CENTENCESANALYSIS_H
#define COMPILER_CENTENCESANALYSIS_H

static bool hasReturnCentence = false;

bool Centence(vector<SINGLE_WORD>& Words, int& PointNum, ofstream& output);
bool ifCentence(vector<SINGLE_WORD>& Words, int& PointNum, ofstream& output);
bool whileCentence(vector<SINGLE_WORD>& Words, int& PointNum, ofstream& output);
bool forCentence(vector<SINGLE_WORD>& Words, int& PointNum, ofstream& output);
bool doWhileCentence(vector<SINGLE_WORD>& Words, int& PointNum, ofstream& output);
bool functionCall(vector<SINGLE_WORD>& Words, int& PointNum, ofstream& output, int isFactor);
bool assignCentence(vector<SINGLE_WORD>& Words, int& PointNum, ofstream& output);
bool printfCentence(vector<SINGLE_WORD>& Words, int& PointNum, ofstream& output);
bool scanfCentence(vector<SINGLE_WORD>& Words, int& PointNum, ofstream& output);
bool returnCentence(vector<SINGLE_WORD>& Words, int& PointNum, ofstream& output);
void condition(vector<SINGLE_WORD>& Words, int& PointNum, ofstream& output);
void parameterValueList(vector<SINGLE_WORD>& Words, int& PointNum, ofstream& output, vector<string>& types);

extern bool No_Symbol_Number(vector<SINGLE_WORD>& Words, int& PointNum, ofstream& output);

#endif //COMPILER_CENTENCESANALYSIS_H

/*
 * 处理语句列
 * 此时的PointNum已经指向了语句列的第一个词位置
 * 因此只需考虑后续动作即可
 */

void parameterValueList(vector<SINGLE_WORD>& Words, int& PointNum, ofstream& output, vector<string>& types) {
    if (WORD_TYPE == "RPARENT") {
        output << "<值参数表>" << endl;
        return;
    }

    ExpressionFlag flag;
    flag = expression(Words, PointNum, output);
    types.push_back((flag == INT_Express) ? "INTTK" : "CHARTK");

    while (WORD_TYPE == "COMMA") {
        PRINT_WORD_AND_ADDPOINT;
        flag = expression(Words, PointNum, output);
        types.push_back((flag == INT_Express) ? "INTTK" : "CHARTK");
    }
    output << "<值参数表>" << endl;
}

void condition(vector<SINGLE_WORD>& Words, int& PointNum, ofstream& output) {
    int flag = 0, line;

    if (expression(Words, PointNum, output) == CHAR_Express) flag = 1;

    if (WORD_TYPE == "LSS" || WORD_TYPE == "LEQ"
        || WORD_TYPE == "GRE" || WORD_TYPE == "GEQ"
        || WORD_TYPE == "EQL" || WORD_TYPE == "NEQ") {
        line = LINE;
        PRINT_WORD_AND_ADDPOINT;
        if (expression(Words, PointNum, output) == CHAR_Express) flag = 1;
    }

    if (flag == 1) symbolTable.addConditionTypeError(line);

    output << "<条件>" << endl;
}

bool ifCentence(vector<SINGLE_WORD>& Words, int& PointNum, ofstream& output) {
    if (WORD_TYPE != "IFTK")
        return false;
    PRINT_WORD_AND_ADDPOINT;
    if (WORD_TYPE == "LPARENT") {
        PRINT_WORD_AND_ADDPOINT;
        condition(Words, PointNum, output);
        if (WORD_TYPE == "RPARENT") {
            PRINT_WORD_AND_ADDPOINT;
        } else {
            symbolTable.loss_RPARENT_Error(LINE);
        }
        Centence(Words, PointNum, output);
        if (WORD_TYPE == "ELSETK") {
            PRINT_WORD_AND_ADDPOINT;
            Centence(Words, PointNum, output);
            output << "<条件语句>" << endl;
            return true;
        }
        else {
            output << "<条件语句>" << endl;
            return true;
        }
    }
    return false;
}

bool whileCentence(vector<SINGLE_WORD>& Words, int& PointNum, ofstream& output) {
    if (WORD_TYPE != "WHILETK")
        return false;
    PRINT_WORD_AND_ADDPOINT;
    if (WORD_TYPE == "LPARENT") {
        PRINT_WORD_AND_ADDPOINT;
        condition(Words, PointNum, output);
        if (WORD_TYPE == "RPARENT") {
            PRINT_WORD_AND_ADDPOINT;
        } else {
            symbolTable.loss_RPARENT_Error(LINE);
        }
        Centence(Words, PointNum, output);
        output << "<循环语句>" << endl;
        return true;

    }
    return false;
}

bool doWhileCentence(vector<SINGLE_WORD>& Words, int& PointNum, ofstream& output) {
    if (WORD_TYPE != "DOTK")
        return false;
    PRINT_WORD_AND_ADDPOINT;
    Centence(Words, PointNum, output);
    if (WORD_TYPE == "WHILETK") {
        PRINT_WORD_AND_ADDPOINT;
        if (WORD_TYPE == "LPARENT") {
            PRINT_WORD_AND_ADDPOINT;
            condition(Words, PointNum, output);
            if (WORD_TYPE == "RPARENT") {
                PRINT_WORD_AND_ADDPOINT;
            } else {
                symbolTable.loss_RPARENT_Error(LINE);
            }
            output << "<循环语句>" << endl;
            return true;
        }
    }
    return false;
}

bool forCentence(vector<SINGLE_WORD>& Words, int& PointNum, ofstream& output) {
    if (WORD_TYPE != "FORTK")
        return false;
    PRINT_WORD_AND_ADDPOINT;
    if (WORD_TYPE == "LPARENT") {
        PRINT_WORD_AND_ADDPOINT;
        if (WORD_TYPE == "IDENFR") {
            symbolTable.nameHasNotDefined(WORD_VALUE, LINE);
            symbolTable.hasAssignForConst(WORD_VALUE, LINE);
            PRINT_WORD_AND_ADDPOINT;
            if (WORD_TYPE == "ASSIGN") {
                PRINT_WORD_AND_ADDPOINT;
                expression(Words, PointNum, output);
                if (WORD_TYPE == "SEMICN") {
                    PRINT_WORD_AND_ADDPOINT;
                    condition(Words, PointNum, output);
                    if (WORD_TYPE == "SEMICN") {
                        PRINT_WORD_AND_ADDPOINT; //;
                        symbolTable.nameHasNotDefined(WORD_VALUE, LINE);
                        symbolTable.hasAssignForConst(WORD_VALUE, LINE);
                        PRINT_WORD_AND_ADDPOINT; //IDENFR
                        PRINT_WORD_AND_ADDPOINT; //=
                        symbolTable.nameHasNotDefined(WORD_VALUE, LINE);
                        PRINT_WORD_AND_ADDPOINT; //IDENFR
                        PRINT_WORD_AND_ADDPOINT; //+ | -
                        No_Symbol_Number(Words, PointNum, output); //STEP
                        output << "<步长>" << endl;
                        if (WORD_TYPE == "RPARENT") {
                            PRINT_WORD_AND_ADDPOINT;
                        } else {
                            symbolTable.loss_RPARENT_Error(LINE);
                        }
                        Centence(Words, PointNum, output);
                        output << "<循环语句>" << endl;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool assignCentence(vector<SINGLE_WORD>& Words, int& PointNum, ofstream& output) {
    if (WORD_TYPE != "IDENFR" || Words[PointNum + 1].WORD.first == "LPARENT")
        return false;
    symbolTable.nameHasNotDefined(WORD_VALUE, LINE);
    symbolTable.hasAssignForConst(WORD_VALUE, LINE);
    PRINT_WORD_AND_ADDPOINT;
    if (WORD_TYPE == "LBRACK") {
        int line = LINE;
        PRINT_WORD_AND_ADDPOINT;
        if (expression(Words, PointNum, output) != INT_Express) {
            symbolTable.addArrayIndexError(line);
        }
        if (WORD_TYPE == "RBRACK") {
            PRINT_WORD_AND_ADDPOINT;
        } else {
            symbolTable.loss_RBRACK_Error(LINE);
        }
    }
    if (WORD_TYPE == "ASSIGN") {
        PRINT_WORD_AND_ADDPOINT;
        expression(Words, PointNum, output);
    }
    output << "<赋值语句>" << endl;
    if (WORD_TYPE == "SEMICN") {
        PRINT_WORD_AND_ADDPOINT;
    }
    return true;
}

bool printfCentence(vector<SINGLE_WORD>& Words, int& PointNum, ofstream& output) {
    if (WORD_TYPE != "PRINTFTK")
        return false;
    PRINT_WORD_AND_ADDPOINT;
    if (WORD_TYPE == "LPARENT") {
        PRINT_WORD_AND_ADDPOINT;
        if (WORD_TYPE == "STRCON") {
            PRINT_WORD_AND_ADDPOINT;
            output << "<字符串>" << endl;
            if (WORD_TYPE == "COMMA") {
                PRINT_WORD_AND_ADDPOINT;
                expression(Words, PointNum, output);
            }
        }
        else {
            expression(Words, PointNum, output);
        }
        if (WORD_TYPE == "RPARENT") {
            PRINT_WORD_AND_ADDPOINT;
        } else {
            symbolTable.loss_RPARENT_Error(LINE);
        }
        if (WORD_TYPE == "SEMICN") {
            output << "<写语句>" << endl;
            PRINT_WORD_AND_ADDPOINT;
            return true;
        }

    }
    return false;
}

bool scanfCentence(vector<SINGLE_WORD>& Words, int& PointNum, ofstream& output) {
    if (WORD_TYPE != "SCANFTK")
        return false;
    PRINT_WORD_AND_ADDPOINT;
    if (WORD_TYPE == "LPARENT") {
        PRINT_WORD_AND_ADDPOINT;
        while (WORD_TYPE == "IDENFR") {
            symbolTable.nameHasNotDefined(WORD_VALUE, LINE);
            symbolTable.hasAssignForConst(WORD_VALUE, LINE);
            PRINT_WORD_AND_ADDPOINT;
            if (WORD_TYPE == "COMMA") {
                PRINT_WORD_AND_ADDPOINT;
            }
        }
        if (WORD_TYPE == "RPARENT") {
            PRINT_WORD_AND_ADDPOINT;
        } else {
            symbolTable.loss_RPARENT_Error(LINE);
        }
        if (WORD_TYPE == "SEMICN") {
            output << "<读语句>" << endl;
            PRINT_WORD_AND_ADDPOINT;
            return true;
        }
    }
    return false;
}

bool returnCentence(vector<SINGLE_WORD>& Words, int& PointNum, ofstream& output) {
    if (WORD_TYPE != "RETURNTK") {
        return false;
    }
    hasReturnCentence = true;
    ExpressionFlag flag = NONE_Express;
    int line = LINE;
    PRINT_WORD_AND_ADDPOINT;
    if (WORD_TYPE == "LPARENT") {
        PRINT_WORD_AND_ADDPOINT;
        flag = expression(Words, PointNum, output);
        if (WORD_TYPE == "RPARENT") {
            PRINT_WORD_AND_ADDPOINT;
        } else {
            symbolTable.loss_RPARENT_Error(LINE);
        }
    }
    if (WORD_TYPE == "SEMICN") {
        output << "<返回语句>" << endl;
        PRINT_WORD_AND_ADDPOINT;
        symbolTable.returnCentenceIsCorrect(flag, line);
        return true;
    }
    return false;
}

/*
 * 如何区分有返回值和无返回值？？？？？
 * 搜索标识符前的类型符
 */
bool functionCall(vector<SINGLE_WORD>& Words, int& PointNum, ofstream& output, int isFactor) {
    string name = WORD_VALUE;
    int line = LINE;
    if (WORD_TYPE != "IDENFR" || Words[PointNum + 1].WORD.first != "LPARENT")
        return false;
    symbolTable.functionHasNotDefined(name, line);
    PRINT_WORD_AND_ADDPOINT;
    if (WORD_TYPE == "LPARENT") {
        PRINT_WORD_AND_ADDPOINT;
        vector<string> types;
        parameterValueList(Words, PointNum, output, types);
        if (!symbolTable.parameterNumHasError(name, line, types.size())) {
            symbolTable.parameterTypeHasError(name, line, types);
        }
        if (WORD_TYPE == "RPARENT") {
            PRINT_WORD_AND_ADDPOINT;
        } else {
            symbolTable.loss_RPARENT_Error(LINE);
        }
        if (hasReturnValue(Words, name)) {
            output << "<有返回值函数调用语句>" << endl;
        }
        else {
            output << "<无返回值函数调用语句>" << endl;
        }
        if (WORD_TYPE == "SEMICN" && !isFactor) {
            PRINT_WORD_AND_ADDPOINT;
        }
    }
    return true;
}

bool Centence(vector<SINGLE_WORD>& Words, int& PointNum, ofstream& output) {
    hasReturnCentence = false;
    if (ifCentence(Words, PointNum, output)) { output << "<语句>" << endl; return true; }
    if (whileCentence(Words, PointNum, output)) { output << "<语句>" << endl; return true; }
    if (forCentence(Words, PointNum, output)) { output << "<语句>" << endl; return true; }
    if (doWhileCentence(Words, PointNum, output)) { output << "<语句>" << endl; return true; }
    if (functionCall(Words, PointNum, output, 0)) { output << "<语句>" << endl; return true; }
    if (assignCentence(Words, PointNum, output)) { output << "<语句>" << endl; return true; }
    if (printfCentence(Words, PointNum, output)) { output << "<语句>" << endl; return true; }
    if (scanfCentence(Words, PointNum, output)) { output << "<语句>" << endl; return true; }
    if (returnCentence(Words, PointNum, output)) { output << "<语句>" << endl; return true; }
    if (WORD_TYPE == "LBRACE") {
        PRINT_WORD_AND_ADDPOINT;
        while (Centence(Words, PointNum, output));
        output << "<语句列>" << endl;
        if (WORD_TYPE == "RBRACE") {
            PRINT_WORD_AND_ADDPOINT;
            output << "<语句>" << endl;
            return true;
        }
    }
    if (WORD_TYPE == "SEMICN") {
        PRINT_WORD_AND_ADDPOINT;
        output << "<语句>" << endl;
        return true;
    }
    symbolTable.noReturnCentenceError(hasReturnCentence, LINE);
    return false;
}