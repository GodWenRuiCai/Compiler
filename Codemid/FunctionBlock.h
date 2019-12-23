//
// Created by 84443 on 2019/11/8.
//

#ifndef COMPILER_FUNCTIONBLOCK_H
#define COMPILER_FUNCTIONBLOCK_H

#include "ExpressionMidCode.h"
#include "FunctionCallMid.h"
#include "IfelseBlock.h"
#include "WhileBlock.h"
#include "DoWhileBlock.h"
#include "ForLoopBlock.h"
#include "AssignCentence.h"
#include "PrintfCentence.h"
#include "ReturnCentence.h"
#include "ScanfCentece.h"
#include "../MIPSCode/Variable.h"

extern vector<Variable> nowFunctionVariables;
extern map<string, Variable> nowFunction_GetVar_byName_Map;
extern vector<TableItem> nowFunctionConsts;
extern vector<TableItem> globalConst;
extern string getNowFunctionType(string resultID);
extern bool functionHasNoCall_Or_MidReturn(string midCode);

void update_GetVar_byName_Map() {
    map<string, Variable> update_Map;
    for (Variable variable1 : nowFunctionVariables) {
        update_Map.insert(make_pair(variable1.VariableName, variable1));
    }
    nowFunction_GetVar_byName_Map = update_Map;
}

string getVarAddr(string var, int* flag) {
    if (nowFunction_GetVar_byName_Map.count(var) == 0) {
        for (Variable variable : globalVariable) {
            if (variable.VariableName == var) {
                (*flag) = (variable.var_type == INT_VAR || variable.var_type == INT_ARRAY) ? 1 : 0;
                return to_string(variable.var_addr);
            }
        }
        (*flag) = 1;
        /*
         * 注意char型函数的返回值
         */
        try {
            if (getNowFunctionType(var) == "CHARTK") {
                (*flag) = 0;
            }
        } catch (exception e) {
            (*flag) = 1;
        }
        return "$" + var;
    }
    long int addr = nowFunction_GetVar_byName_Map.at(var).var_addr;
    string reg = nowFunction_GetVar_byName_Map.at(var).thisRegister;
    if (nowFunction_GetVar_byName_Map.at(var).var_type == INT_PARA ||
        nowFunction_GetVar_byName_Map.at(var).var_type == CHAR_PARA) {
        if (nowFunction_GetVar_byName_Map.at(var).var_type == INT_PARA) {
            (*flag) = 1;
        } else (*flag) = 0;
        return reg;
    } else if (nowFunction_GetVar_byName_Map.at(var).var_type == INT_VAR ||
               nowFunction_GetVar_byName_Map.at(var).var_type == CHAR_VAR ||
               nowFunction_GetVar_byName_Map.at(var).var_type == CHAR_ARRAY ||
               nowFunction_GetVar_byName_Map.at(var).var_type == INT_ARRAY) {
        if (nowFunction_GetVar_byName_Map.at(var).var_type == INT_VAR ||
            nowFunction_GetVar_byName_Map.at(var).var_type == INT_ARRAY) {
            (*flag) = 1;
        } else (*flag) = 0;
        return to_string(addr);
    }
    return "";
}

string pushStack() {
    string result = "#------------------压栈-------------------------#\n";
    result += "#------压临时寄存器------#\n";
    result += "sw $t3, 0($sp)\nsub $sp, $sp, 4\n";result += "sw $t4, 0($sp)\nsub $sp, $sp, 4\n";
    result += "sw $t5, 0($sp)\nsub $sp, $sp, 4\n";result += "sw $s0, 0($sp)\nsub $sp, $sp, 4\n";
    result += "sw $s1, 0($sp)\nsub $sp, $sp, 4\n";result += "sw $s2, 0($sp)\nsub $sp, $sp, 4\n";
    result += "sw $s3, 0($sp)\nsub $sp, $sp, 4\n";result += "sw $s4, 0($sp)\nsub $sp, $sp, 4\n";
    result += "sw $s5, 0($sp)\nsub $sp, $sp, 4\n";result += "sw $s6, 0($sp)\nsub $sp, $sp, 4\n";
    result += "#------压函数变量------#\n";
    for (int i = nowFunctionVariables.size() - 1; i >= 0; i--) {
        int type = 0;
        string varAddr = getVarAddr(nowFunctionVariables[i].VariableName, &type);
        if (varAddr.at(0) != '$') {
            if (type == 1) result += "lw $t9, " + varAddr + "($0)\n";
            else result += "lb $t9, " + varAddr + "($0)\n";
            result += "sw $t9, 0($sp)\n";
        } else {
            result += "sw " + varAddr + ", 0($sp)\n";
        }
        result += "sub $sp, $sp, 4\n";
    }
    result += "sw $ra, 0($sp)\n";
    result += "sub $sp, $sp, 4\n";
    result += "#------------------压栈完成----------------------#\n";
    return result;
}

string popStack() {
    string result = "#--------------------出栈-------------------#\n";
    result += "#------恢复函数变量------#\n";
    result += "add $sp, $sp, 4\n";
    result += "lw $ra, 0($sp)\n";
    for (int i = 0; i < nowFunctionVariables.size(); i++) {
        result += "add $sp, $sp, 4\n";
        int type = 0;
        string varAddr = getVarAddr(nowFunctionVariables[i].VariableName, &type);
        if (varAddr[0] != '$') {
            result += "lw $t9, 0($sp)\n";
            if (type == 1) result += "sw $t9, " + varAddr + "($0)\n";
            else result += "sb $t9, " + varAddr + "($0)\n";
        } else {
            result += "lw " + varAddr + ", 0($sp)\n";
        }
    }
    result += "#-----恢复临时寄存器-----#\n";
    result += "add $sp, $sp, 4\nlw $s6, 0($sp)\n";result += "add $sp, $sp, 4\nlw $s5, 0($sp)\n";
    result += "add $sp, $sp, 4\nlw $s4, 0($sp)\n";result += "add $sp, $sp, 4\nlw $s3, 0($sp)\n";
    result += "add $sp, $sp, 4\nlw $s2, 0($sp)\n";result += "add $sp, $sp, 4\nlw $s1, 0($sp)\n";
    result += "add $sp, $sp, 4\nlw $s0, 0($sp)\n";result += "add $sp, $sp, 4\nlw $t5, 0($sp)\n";
    result += "add $sp, $sp, 4\nlw $t4, 0($sp)\n";result += "add $sp, $sp, 4\nlw $t3, 0($sp)\n";
    result += "#--------------------出栈完成-------------------#\n";
    return result;
}

bool isConst(string name, int* num, int* type) {
    if (nowFunction_GetVar_byName_Map.find(name) != nowFunction_GetVar_byName_Map.end())
        return false;
    for (TableItem item : nowFunctionConsts) {
        if (name == item.name) {
            (*num) = (item.type == "INTTK") ? item.const_int_value : item.const_char_value;
            if (type != nullptr) (*type) = (item.type == "INTTK") ? 1 : 0;
            return true;
        }
    }
    for (TableItem item1 : globalConst) {
        if (name == item1.name) {
            (*num) = (item1.type == "INTTK") ? item1.const_int_value : item1.const_char_value;
            if (type != nullptr) (*type) = (item1.type == "INTTK") ? 1 : 0;
            return true;
        }
    }
    return false;
}

string nowTranslateFunctionName;

class FunctionBlock {
    string kind;
    string functionName;
    vector<TableItem> parameters;
    vector<ExpressionMidCode> expressionMidCodes;
    vector<CentenceMid*> centences;
    int nowExpression;
    int nowCentence;
    vector<Variable> functionVariables;
    map<string, Variable> function_GetVar_byName_Map;
    vector<TableItem> functionConsts;

    string midCode;
    Function_Flow_Blocks* flowBlocks;

    void addCentence(vector<CentenceMid*>* centenceBlock, CentenceMid* centence) {
        centenceBlock->push_back(centence);
    }

public:
    /**
     * @brief: canBeInlined、changedGlobalVar用于内联优化
     */
    bool canBeInlined = true;
    vector<string> changedGlobalVar;

    string getMidCode_No_Optim() {
        return this->midCode;
    }

    int getParaNum() {
        return this->parameters.size();
    }

    string getName() { return this->functionName; }

    vector<TableItem>& getFunctionConsts() { return this->functionConsts;}

    vector<Variable>& getFunctionVariables() {
        return this->functionVariables;
    }

    map<string, Variable>& getVarMap() {
        return this->function_GetVar_byName_Map;
    }

    void functionBlock_addParameter(TableItem para) {
        this->parameters.push_back(para);
    }

    void set_Function_Kind(string kind) {
        this->kind = kind;
    }

    void set_Function_Name(string name) {
        this->functionName = name;
    }

    string Optim() {
        nowTranslateFunctionName = this->functionName;
        nowFunction_GetVar_byName_Map = this->function_GetVar_byName_Map;
        nowFunctionConsts = this->functionConsts;
        nowFunctionVariables = this->functionVariables;

        this->flowBlocks = new Function_Flow_Blocks(this->midCode);
        this->flowBlocks->registerGlobalVarReg(nowFunctionVariables);
        update_GetVar_byName_Map();
        /**
         * @brief: 开始进入对基本块进行翻译
         */
        return this->flowBlocks->translateToMips();
    }

    string toString() {
        nowTranslateFunctionName = this->functionName;
        nowFunction_GetVar_byName_Map = this->function_GetVar_byName_Map;
        nowFunctionConsts = this->functionConsts;
        nowFunctionVariables = this->functionVariables;

        string result = "";
        result += (this->kind == "INTTK") ? "int " :
                  ((this->kind == "CHARTK") ? "char " : "void ");
        result += this->functionName + "()\n";
        for (TableItem item : this->parameters) {
            result += "para ";
            result += (item.type == "INTTK") ? "int " : "char ";
            result += item.name + "\n";
        }
        result += get_centences_component_string(this->centences);

        string lastLine = "";
        for (int i = result.length() - 2; i >= 0; i--) {
            if (result.at(i) == '\n') break;
            lastLine = result.at(i) + lastLine;
        }
        if (lastLine.substr(0, 3) != "ret") result += "ret\n";
        this->midCode = result;
        /**
         * @brief： 用于判别函数是否可以用于内联，这里的条件较弱，函数内部不可以定义任何东西，不能调用函数.
         *          其实是可以定义常量之类的，为了方便这里没有这么写.
         *          即函数内不能定义任何东西
         * @attention: 不过，函数显然是可以修改parameter和全局变量的
         *          修改parameter没关系，因为内联会用temp_var替换
         *          但是修改全局变量时候要特别注意，如果func1修改了全局变量且func1可内联
         *          那么func2要内联func1的话，func2里面的定义不能和func1修改的全局变量重名
         */
        for (Variable variable : nowFunctionVariables) {
            if (variable.var_type != INT_PARA && variable.var_type != CHAR_PARA) {
                canBeInlined = false;
            }
        }
        if (!functionHasNoCall_Or_MidReturn(result) || nowFunctionConsts.size() > 0) {
            canBeInlined = false;
        }
        /**
         * @brief: 记录函数修改全局变量的值
         */
        changedGlobalVar = global_Variable_Changed(result);
        return result;
    }

    string toMips() {
        nowTranslateFunctionName = this->functionName;
        nowFunction_GetVar_byName_Map = this->function_GetVar_byName_Map;
        nowFunctionConsts = this->functionConsts;
        nowFunctionVariables = this->functionVariables;

        string result = "";
        result += this->functionName + ":\n";

        result += get_centences_component_mips(this->centences);
        if (result.find("jr $ra") == string::npos) {
            result += "jr $ra\n";
            return result;
        }
        string lastLine = "";
        for (int i = result.length() - 2; ; i--) {
            if (result.at(i) == '\n') break;
            lastLine = result.at(i) + lastLine;
        }
        if (lastLine != "jr $ra") result += "jr $ra\n";
        return result;
    }

    vector<CentenceMid*>* getCentenceBlock() {
        return &this->centences;
    }

    FunctionCallMidCode* addCentence_FunctionCall(vector<CentenceMid*>* centenceBlock, string s) {
        FunctionCallMidCode* functionCall = new FunctionCallMidCode(s);
        addCentence(centenceBlock, functionCall);
        this->nowCentence = this->centences.size() - 1;
        return functionCall;
    }

    IfelseBlock* addCentence_IF(vector<CentenceMid*>* centenceBlock) {
        IfelseBlock* ifelseBlock = new IfelseBlock();
        addCentence(centenceBlock, ifelseBlock);
        this->nowCentence = this->centences.size() - 1;
        return ifelseBlock;
    }

    WhileBlock* addCentence_WHILE(vector<CentenceMid*>* centenceBlock) {
        WhileBlock* whileBlock = new WhileBlock();
        addCentence(centenceBlock, whileBlock);
        this->nowCentence = this->centences.size() - 1;
        return whileBlock;
    }

    DoWhileBlock* addCentence_DOWHILE(vector<CentenceMid*>* centenceBlock) {
        DoWhileBlock* doWhileBlock = new DoWhileBlock();
        addCentence(centenceBlock, doWhileBlock);
        this->nowCentence = this->centences.size() - 1;
        return doWhileBlock;
    }

    ForLoopBlock* addCentence_FOR(vector<CentenceMid*>* centenceBlock) {
        ForLoopBlock* forLoopBlock = new ForLoopBlock();
        addCentence(centenceBlock, forLoopBlock);
        this->nowCentence = this->centences.size() - 1;
        return forLoopBlock;
    }

    AssignCentence* addCentence_ASSIGN(vector<CentenceMid*>* centenceBlock) {
        AssignCentence* assignCentence = new AssignCentence();
        addCentence(centenceBlock, assignCentence);
        this->nowCentence = this->centences.size() - 1;
        return assignCentence;
    }

    ReturnCentence* addCentence_RETURN(vector<CentenceMid*>* centenceBlock) {
        ReturnCentence* returnCentence = new ReturnCentence();
        addCentence(centenceBlock, returnCentence);
        this->nowCentence = this->centences.size() - 1;
        return returnCentence;
    }

    ScanfCentece* addCentence_SCANF(vector<CentenceMid*>* centenceBlock) {
        ScanfCentece* scanfCentece = new ScanfCentece();
        addCentence(centenceBlock, scanfCentece);
        this->nowCentence = this->centences.size() - 1;
        return scanfCentece;
    }

    PrintfCentence* addCentence_printf(vector<CentenceMid*>* centenceBlock) {
        PrintfCentence* printfCentence = new PrintfCentence();
        addCentence(centenceBlock, printfCentence);
        this->nowCentence = this->centences.size() - 1;
        return printfCentence;
    }

    CentenceMid* getNowCentence() {
        return this->centences[nowCentence];
    }

    ExpressionMidCode& getNowExp() {
        return this->expressionMidCodes[nowExpression];
    }

    void addExpressionMidCode() {
        ExpressionMidCode expMidCode = ExpressionMidCode();
        expMidCode.init();
        this->expressionMidCodes.push_back(expMidCode);
        this->nowExpression = expressionMidCodes.size() - 1;
    }

};

string get_centences_component_string(vector<CentenceMid*>& centencesBlock);

string get_centences_component_mips(vector<CentenceMid*>& centencesBlock);

string get_centences_component_mips(vector<CentenceMid*>& centencesBlock) {
    string result = "";
    for (CentenceMid* centence : centencesBlock) {
        switch (centence->kind) {
            case IFELSE:
                result += static_cast<IfelseBlock*>(centence)->toMips();
                break;
            case WHILE:
                result += static_cast<WhileBlock*>(centence)->toMips();
                break;
            case DOWHILE:
                result += static_cast<DoWhileBlock*>(centence)->toMips();
                break;
            case FOR:
                result += static_cast<ForLoopBlock*>(centence)->toMips();
                break;
            case PRINTF:
                result += static_cast<PrintfCentence*>(centence)->toMips();
                break;
            case SCANF:
                result += static_cast<ScanfCentece*>(centence)->toMips();
                break;
            case FUNCTIONCALL:
                result += static_cast<FunctionCallMidCode*>(centence)->toMips();
                break;
            case ASSIGN:
                result += static_cast<AssignCentence*>(centence)->toMips();
                break;
            case RETURN:
                result += static_cast<ReturnCentence*>(centence)->toMips();
                break;
        }
    }
    return result;
}

string get_centences_component_string(vector<CentenceMid*>& centencesBlock) {
    string result = "";
    for (CentenceMid* centence : centencesBlock) {
        switch (centence->kind) {
            case IFELSE:
                result += static_cast<IfelseBlock*>(centence)->toString();
                break;
            case WHILE:
                result += static_cast<WhileBlock*>(centence)->toString();
                break;
            case DOWHILE:
                result += static_cast<DoWhileBlock*>(centence)->toString();
                break;
            case FOR:
                result += static_cast<ForLoopBlock*>(centence)->toString();
                break;
            case PRINTF:
                result += static_cast<PrintfCentence*>(centence)->toString();
                break;
            case SCANF:
                result += static_cast<ScanfCentece*>(centence)->toString();
                break;
            case FUNCTIONCALL:
                result += static_cast<FunctionCallMidCode*>(centence)->toString();
                break;
            case ASSIGN:
                result += static_cast<AssignCentence*>(centence)->toString();
                break;
            case RETURN:
                result += static_cast<ReturnCentence*>(centence)->toString();
                break;
        }
    }
    return result;
}

#endif //COMPILER_FUNCTIONBLOCK_H
