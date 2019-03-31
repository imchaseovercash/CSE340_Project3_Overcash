//
// Created by Chase Overcash on 3/27/2019.
//

#include <iostream>
#include <istream>
#include <vector>
#include <string>
#include <cctype>
#include "deque"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "lexer.h"
#include "inputbuf.h"
#include "algorithm"
//#include "execinfo.h"

using namespace std;


string types[32] = {
        "FALSE", // 0
        "TRUE", // 1
        "LBRACE", // 2
        "ID", // 3
        "WHILE", // 4
        "EQUAL", // 5
        "COLON", // 6
        "COMMA", // 7
        "LPAREN", // 8
        "RPAREN", // 9
        "SEMICOLON", //10
        "REAL", // 11
        "INT", //12
        "BOOLEAN", //13
        "STRING", //14
        "PLUS", //15
        "MULT", //16
        "DIV", //17
        "AND", //18
        "OR", // 19
        "XOR", //20
        "NOT", //21
        "GREATER", // 22
        "GTEQ", // 23
        "LESS", // 24
        "LTEQ", // 25
        "NOTEQUAL", //26
        "STRING\\_CONSTANT",//27
        "NUM", //28
        "REALNUM", //29
        "RBRACE", //30
        "END_OF_FILE" //31
};
struct scopeTree;
struct var_declTree;
struct scope_listTree;
struct id_listTree;
struct stmtTree;
struct while_stmtTree;
struct conditionTree;
struct stmt_listTree;
struct assign_stmtTree;
struct exprTree;
struct primaryTree;
struct notTree;
struct operatorTree;
struct node;
struct indexDeques;
struct programTree;
struct indexNode;
/////
///// Actual tree declarations
/////


struct indexNode {
    int start;
    int end;
    string type;
    vector<node *> declarations;
    vector<node *> references;
    vector<node *> LHS_references;
    vector<node *> temp_type_RHS;
    vector<node *> temp_type_LHS;
    vector<node *> operatorTypeMatcher;


};
struct node {

    Token token;
    Token assignedType;
    TokenType tokenType;
    int idNumber;
};
struct indexDeques {
    deque<indexNode *> scope_indexes;
    deque<indexNode *> completed_indexes;
    vector<node *> allDeclarations;
    bool isDeclarationErrorFound = false;
    string declarationErrorFound;
    bool isTypeErrorFound = false;
    string typeErrorFound;
    bool isRHS_TypeErrorFound = false;
    string RHS_typeErrorFound;
} deques;
struct operatorTree {
    struct node *OP_TYPE;
    struct exprTree *EXPR1;
    struct exprTree *EXPR2;
    LexicalAnalyzer lexer;
};
struct notTree {
    struct node *NOT;
    struct exprTree *EXPR;
    LexicalAnalyzer lexer;
};
struct primaryTree {
    struct node *PRIM_TYPE;
    LexicalAnalyzer lexer;
};
struct exprTree {
    struct primaryTree *PRIM;
    struct operatorTree *OP;
    struct notTree *NT;
    LexicalAnalyzer lexer;
};
struct assign_stmtTree {
    int lineNo;
    struct node *ID;
    struct node *EQ;
    struct exprTree *EXPR;
    struct node *SC;
    LexicalAnalyzer lexer;
};
struct stmt_listTree {
    int startLine;
    int endLine;
    struct stmtTree *STMT;
    struct stmt_listTree *STMT_LIST;
    LexicalAnalyzer lexer;
};
struct conditionTree {
    struct node *LP;
    struct exprTree *EXPR;
    struct node *RP;
    LexicalAnalyzer lexer;
};
struct while_stmtTree {
    int lineNo;
    struct conditionTree *CT;
    struct stmtTree *STMT;
    struct stmt_listTree *STMT_LIST;
    struct node *LB;
    struct node *RB;
    LexicalAnalyzer lexer;
};
struct stmtTree {
    struct assign_stmtTree *AST;
    struct while_stmtTree *WST;
    LexicalAnalyzer lexer;
};
struct id_listTree {
    struct node *ID;
    struct node *COM;
    struct id_listTree *ID_LIST;
    LexicalAnalyzer lexer;
};
struct var_declTree {
    struct id_listTree *ID_LIST;
    struct node *COL;
    struct node *TN;
    struct node *SC;
    int lineNo;
    LexicalAnalyzer lexer;
};
struct scope_listTree {
    struct var_declTree *VD;
    struct scopeTree *S;
    struct scope_listTree *SL;
    struct stmtTree *STMT;
    deque<string> *used_parses;
    LexicalAnalyzer lexer;
};
struct rbTree {
    struct node *RB;
    struct node *END;
    int lineNo;
    LexicalAnalyzer lexer;
};
struct scopeTree {
    struct node *LB;
    struct indexNode *index;
    struct rbTree *RBT;
    struct scope_listTree *SL;
    LexicalAnalyzer lexer;
};
struct programTree {
    struct scopeTree *S;
} program;

//void finishState() {
//
//}


void syntaxError() {
    cout << "Syntax Error";
    exit(EXIT_FAILURE);
}

void isErrorOneOne(string lex) {
    for (node *node1:deques.scope_indexes.back()->declarations) {
        if (node1->token.lexeme == lex) {
            deques.isDeclarationErrorFound = true;
            deques.declarationErrorFound = "ERROR CODE 1.1 " + lex;
        }
    }
}

void isErrorOneTwo(string lex) {
    bool declared = false;
    for (indexNode *indexNode : deques.scope_indexes) {
        for (node *node1: indexNode->declarations) {
            if (node1->token.lexeme == lex) {
                declared = true;
            }
        }

    }
    if (!declared) {
        deques.isDeclarationErrorFound = true;
        deques.declarationErrorFound = "ERROR CODE 1.2 " + lex;
        return;
    }
}

bool is_Primary(Token token) {
    return token.token_type == ID || token.token_type == NUM || token.token_type == REALNUM ||
           token.token_type == STRING_CONSTANT || token.token_type == TRUE || token.token_type == FALSE;
}

bool is_Arithmetic(Token token) {
    return token.token_type == PLUS || token.token_type == MINUS || token.token_type == MULT || token.token_type == DIV;
}

bool is_BinaryBool(Token token) {
    return token.token_type == AND || token.token_type == OR || token.token_type == XOR;
}

bool is_Relational(Token token) {
    return token.token_type == GREATER || token.token_type == GTEQ || token.token_type == LESS ||
           token.token_type == NOTEQUAL ||
           token.token_type == LTEQ;
}


struct scope_listTree *parse_scopeList(LexicalAnalyzer lexer);

struct scopeTree *parse_Scope(LexicalAnalyzer lexer);

struct exprTree *parse_Expr(LexicalAnalyzer lexer);

struct assign_stmtTree *parse_assignStmt(LexicalAnalyzer lexer);

struct while_stmtTree *parse_whileStmt(LexicalAnalyzer lexer);

struct var_declTree *parse_varDecl(LexicalAnalyzer lexer);

struct stmtTree *parse_Stmt(LexicalAnalyzer lexer);

struct stmt_listTree *parse_stmtList(LexicalAnalyzer lexer);

struct primaryTree *parse_Primary(LexicalAnalyzer lexer);

struct notTree *parse_notTree(LexicalAnalyzer lexer);

struct operatorTree *parse_Operator(LexicalAnalyzer lexer);

struct conditionTree *parse_Conditional(LexicalAnalyzer lexer);

struct id_listTree *parse_idList(LexicalAnalyzer lexer);

struct rbTree *parse_End(LexicalAnalyzer lexer);

struct primaryTree *parse_Primary(LexicalAnalyzer lexer) {
    struct primaryTree *primary = new(primaryTree);
    primary->PRIM_TYPE = new(node);
    Token t1 = lexer.GetToken();
    if (is_Primary(t1)) {
        //t1.Print();
        if (t1.token_type == ID) {
            if (!deques.isDeclarationErrorFound)
                isErrorOneTwo(t1.lexeme);
            node *node1 = new (node);
            int i = -1;
//            //// Type Error C1 Checking ////
            for (node *node : deques.allDeclarations) {
                if (node->token.lexeme == t1.lexeme) {
                    i = node->idNumber;
                }
            }
            node1->token = t1;
            node1->idNumber = i;
            deques.scope_indexes.back()->references.push_back(node1);
            deques.scope_indexes.back()->temp_type_LHS.push_back(node1);
            deques.scope_indexes.back()->temp_type_RHS.push_back(node1);
//
        }

        primary->PRIM_TYPE->
                tokenType = t1.token_type;
        primary->
                lexer = lexer;
        return
                primary;
    }

    else {

//printf("Syntax Error at line number %d \n", __LINE__);
        syntaxError();

    }
}

struct notTree *parse_notTree(LexicalAnalyzer lexer) {
    struct notTree *not_expr = new(notTree);
    not_expr->NOT = new (node);
    Token t1 = lexer.GetToken();
    if (t1.token_type == NOT) {
        //t1.Print();
        not_expr->NOT->tokenType = t1.token_type;
        not_expr->EXPR = parse_Expr(lexer);
        not_expr->lexer = not_expr->EXPR->lexer;
        return not_expr;
    }
    else {

        //printf("Syntax Error at line number %d \n", __LINE__);
        syntaxError();
    }
}

struct operatorTree *parse_Operator(LexicalAnalyzer lexer) {
    struct operatorTree *oper = new(operatorTree);
    oper->OP_TYPE = new(node);
    Token t1 = lexer.GetToken();
    //t1.Print();
    if (is_BinaryBool(t1)) {
        oper->OP_TYPE->tokenType = t1.token_type;
        oper->EXPR1 = parse_Expr(lexer);
        lexer = oper->EXPR1->lexer;
        oper->EXPR2 = parse_Expr(lexer);
        oper->lexer = oper->EXPR2->lexer;
        return oper;
    }
    else if (is_Relational(t1)) {
        oper->OP_TYPE->tokenType = t1.token_type;
        oper->EXPR1 = parse_Expr(lexer);
        lexer = oper->EXPR1->lexer;
        oper->EXPR2 = parse_Expr(lexer);
        oper->lexer = oper->EXPR2->lexer;
        return oper;
    }
    else if (is_Arithmetic(t1)) {
        oper->OP_TYPE->tokenType = t1.token_type;
        deques.scope_indexes.back()->temp_type_RHS.clear();
        oper->EXPR1 = parse_Expr(lexer);
        lexer = oper->EXPR1->lexer;
        oper->EXPR2 = parse_Expr(lexer);
        oper->lexer = oper->EXPR2->lexer;
        if (!deques.isRHS_TypeErrorFound) {
            for (node *n : deques.scope_indexes.back()->temp_type_RHS) {
                if (n->idNumber == 13 || n->idNumber == 14) {
                    deques.isRHS_TypeErrorFound = true;
                    deques.RHS_typeErrorFound = "TYPE MISMATCH " + to_string(t1.line_no) + " C3";
                }

            }
        }
        deques.scope_indexes.back()->temp_type_RHS.clear();


        return oper;
    }
    else {

        //printf("Syntax Error at line number %d \n", __LINE__);
        syntaxError();
    }
}

struct exprTree *parse_Expr(LexicalAnalyzer lexer) {
    struct exprTree *expr = new(exprTree);
    Token t1 = lexer.GetToken();
    if (is_Relational(t1) || is_Arithmetic(t1) || is_BinaryBool(t1)) // If still failing parsing, make all structs
    {
        lexer.UngetToken(t1);
        expr->OP = parse_Operator(lexer);
        expr->lexer = expr->OP->lexer;
        return expr;
    }
    else if (t1.token_type == NOT) {
        lexer.UngetToken(t1);
        expr->NT = parse_notTree(lexer);
        expr->lexer = expr->NT->lexer;
        return expr;
    }
    else if (is_Primary(t1)) {
        lexer.UngetToken(t1);
        expr->PRIM = parse_Primary(lexer);
        expr->lexer = expr->PRIM->lexer;
        return expr;
    }
    else {

        //printf("Syntax Error at line number %d \n", __LINE__);
        syntaxError();
    }

}

struct assign_stmtTree *parse_assignStmt(LexicalAnalyzer lexer) {
    //isSyntanxError();
    struct assign_stmtTree *assign_stmt = new(assign_stmtTree);
    assign_stmt->EQ = new(node);
    assign_stmt->ID = new(node);
    assign_stmt->SC = new(node);
    Token t1 = lexer.GetToken();
    if (t1.token_type == ID) {
        //t1.Print();
        //t2.Print();
        // "REAL", // 11
        // "INT", //12
        // "BOOLEAN", //13
        // "STRING", //14
        int typeNum = -1;
        for (node *n : deques.allDeclarations) {
            if (n->token.lexeme == t1.lexeme) {
                typeNum = n->idNumber;
            }
        }
        if (!deques.isDeclarationErrorFound)
            isErrorOneTwo(t1.lexeme);
        node *node1 = new (node);
        node1->token = t1;
        node1->tokenType = t1.token_type;
        node1->idNumber = typeNum;
        deques.scope_indexes.back()->references.push_back(node1);
        deques.scope_indexes.back()->LHS_references.push_back((node1));
        Token t2 = lexer.GetToken();
        if (t2.token_type == EQUAL) {
            //t2.Print();
            assign_stmt->EQ->tokenType = t2.token_type;
            assign_stmt->EXPR = parse_Expr(lexer);
            lexer = assign_stmt->EXPR->lexer;
            for (node *node2: deques.scope_indexes.back()->temp_type_LHS) {
                if (node1->idNumber == 11 && node1->idNumber != node2->idNumber &&
                    node1->idNumber != node2->idNumber + 1) {
                    deques.typeErrorFound = "TYPE MISMATCH " + to_string(t1.line_no) + " C2";
                    deques.isTypeErrorFound = true;
                }
                else if (node1->idNumber != node2->idNumber) {
                    deques.typeErrorFound = "TYPE MISMATCH " + to_string(t1.line_no) + " C1";
                    deques.isTypeErrorFound = true;
                }
            }
            deques.scope_indexes.back()->temp_type_LHS.clear();

//            if (deques.allDeclarations.back()->idNumber == 11 && !deques.isTypeErrorFound)
//            {
//                if (!(i == 11 || i == 12)) {
//                    deques.typeErrorFound = "TYPE MISMATCH " + to_string(t1.line_no) + " C2";
//                    deques.isTypeErrorFound = true;
//                }
//            }
//            else if (!deques.isTypeErrorFound && deques.allDeclarations.back()->idNumber != i) {
//                deques.typeErrorFound = "TYPE MISMATCH " + to_string(t1.line_no) + " C1";
//                deques.isTypeErrorFound = true;
//            }

            Token t3 = lexer.GetToken();
            if (t3.token_type == SEMICOLON) {
                //t3.Print();
                assign_stmt->SC->tokenType = t3.token_type;
                assign_stmt->lexer = lexer;
                return assign_stmt;
            }
            else {
                //printf("Syntax Error at line number %d \n", __LINE__);
                syntaxError();
            }
        }
        else {
            //printf("Syntax Error at line number %d \n", __LINE__);
            syntaxError();
        }
    }
    else {
        //printf("Syntax Error at line number %d \n", __LINE__);
        syntaxError();
    }

}

struct conditionTree *parse_Conditional(LexicalAnalyzer lexer) {
    //isSyntanxError();
    //// initialization
    struct conditionTree *conditional = new(conditionTree);
    conditional->LP = new (node);
    conditional->RP = new(node);
    Token t1 = lexer.GetToken();
    if (t1.token_type == LPAREN) {
        conditional->LP->tokenType = t1.token_type;
        //t1.Print();
        conditional->EXPR = parse_Expr(lexer);
        lexer = conditional->EXPR->lexer;
        Token t2 = lexer.GetToken();
        if (t2.token_type == RPAREN) {
            //t2.Print();
            conditional->RP->tokenType = t2.token_type;
            conditional->lexer = lexer;
            return conditional;
        }
        else {
            //printf("Syntax Error at line number %d \n", __LINE__);
            syntaxError();
        }

    }
    else {
        //printf("Syntax Error at line number %d \n", __LINE__);
        syntaxError();
    }
}

struct stmt_listTree *parse_stmtList(LexicalAnalyzer lexer) {
    //isSyntanxError();
    struct stmt_listTree *stmt_list = new (stmt_listTree);
    stmt_list->STMT = parse_Stmt(lexer);
    lexer = stmt_list->STMT->lexer;
    Token t1 = lexer.GetToken();
    if (t1.token_type == RBRACE && deques.scope_indexes.back()->type == "WHILE") {
        lexer.UngetToken(t1);
        stmt_list->lexer = lexer;
        return stmt_list;
    }
    else if (t1.token_type == WHILE || t1.token_type == ID) {
        lexer.UngetToken(t1);
        stmt_list->STMT_LIST = parse_stmtList(lexer);
        stmt_list->lexer = stmt_list->STMT_LIST->lexer;
        return stmt_list;
    }
    else {
        //printf("Syntax Error at line number %d \n", __LINE__);
        syntaxError();
    }
}

struct while_stmtTree *parse_whileStmt(LexicalAnalyzer lexer) {

    //isSyntanxError();

    struct while_stmtTree *while_stmt = new(while_stmtTree);
    while_stmt->LB = new(node);
    while_stmt->RB = new(node);

    Token t1 = lexer.GetToken();
    if (t1.token_type == WHILE) {
        //t1.Print();
        while_stmt->CT = parse_Conditional(lexer);
        lexer = while_stmt->CT->lexer;
        Token t2 = lexer.GetToken();

        //// parse stmt list
        if (t2.token_type == LBRACE) {
            //t2.Print();
            indexNode *index = new(indexNode);
            index->type = "WHILE";
            index->start = t2.line_no;
            deques.scope_indexes.push_back(index);
            while_stmt->LB->tokenType = t2.token_type;
            while_stmt->STMT_LIST = parse_stmtList(lexer);
            lexer = while_stmt->STMT_LIST->lexer;
            Token t3 = lexer.GetToken();
            if (t3.token_type == RBRACE) {
                //t3.Print();
                deques.scope_indexes.back()->end = t3.line_no;
                deques.completed_indexes.push_front(deques.scope_indexes.back());
                deques.scope_indexes.pop_back();
                while_stmt->RB->tokenType = t3.token_type;
                while_stmt->lexer = lexer;
                return while_stmt;
            }
            else {
                //printf("Syntax Error at line number %d \n", __LINE__);
                syntaxError();
            }
        }
        else if (!deques.scope_indexes.empty()) {
            lexer.UngetToken(t2);
            while_stmt->STMT = parse_Stmt(lexer);
            while_stmt->lexer = while_stmt->STMT->lexer;
            return while_stmt;
        }
        else {
            //printf("Syntax Error at line number %d \n", __LINE__);
            syntaxError();
        }
    }
    else {

        //printf("Syntax Error at line number %d \n", __LINE__);
        syntaxError();
    }
}

struct stmtTree *parse_Stmt(LexicalAnalyzer lexer) {
    //isSyntanxError();
    //// initialization
    struct stmtTree *stmt = new(stmtTree);
    Token t1 = lexer.GetToken();
    if (t1.token_type == WHILE) {
        lexer.UngetToken(t1);
        stmt->WST = parse_whileStmt(lexer);
        stmt->lexer = stmt->WST->lexer;
        return stmt;
    }
    else if (t1.token_type == ID) {
        lexer.UngetToken(t1);
        stmt->AST = parse_assignStmt(lexer);
        stmt->lexer = stmt->AST->lexer;
        return stmt;
    }
    else {

        //printf("Syntax Error at line number %d \n", __LINE__);
        syntaxError();
    }

}

struct id_listTree *parse_idList(LexicalAnalyzer lexer) {
    //isSyntanxError();
    //// initialization
    struct id_listTree *id_list = new(id_listTree);
    id_list->ID = new(node);

    Token t1 = lexer.GetToken();
    if (t1.token_type == ID) {
        //t1.Print();
        id_list->ID->tokenType = t1.token_type;
        if (!deques.isDeclarationErrorFound) {
            isErrorOneOne(t1.lexeme);
        }
        node *node1 = new (node);
        node1->token = t1;
        deques.scope_indexes.back()->temp_type_RHS.push_back(node1);
        Token t2 = lexer.GetToken();
        if (t2.token_type == COMMA) {
            //t2.Print();
            id_list->COM = new(node);
            id_list->COM->tokenType = t2.token_type;
            //// id_List
            id_list->ID_LIST = parse_idList(lexer);
            id_list->lexer = id_list->ID_LIST->lexer;
            return id_list;
        }
            //// base Case, var Decl
        else if (t2.token_type == COLON) {
            lexer.UngetToken(t2);
            id_list->lexer = lexer;
            return id_list;
        }
        else {

            //printf("Syntax Error at line number %d \n", __LINE__);
            syntaxError();
        }
    }
    else {

        //printf("Syntax Error at line number %d \n", __LINE__);
        syntaxError();
    }
}

struct var_declTree *parse_varDecl(LexicalAnalyzer lexer) {
    //isSyntanxError();
    //// initialization
    struct var_declTree *var_decl = new(var_declTree);
    //// id_list
    var_decl->ID_LIST = parse_idList(lexer);

    //// rest of var_decl
    lexer = var_decl->ID_LIST->lexer;
    var_decl->COL = new(node);
    var_decl->SC = new(node);
    var_decl->TN = new(node);

    Token t1 = lexer.GetToken();
    if (t1.token_type == COLON) {
        //t1.Print();
        var_decl->COL->tokenType = t1.token_type;

        Token t2 = lexer.GetToken();
        if (t2.token_type == REAL || t2.token_type == INT || t2.token_type == BOOLEAN || t2.token_type == STRING) {
            //t2.Print();
            // "REAL", // 11
            // "INT", //12
            // "BOOLEAN", //13
            // "STRING", //14
            int typeNum;
            if (t2.token_type == REAL) {
                typeNum = 11;
            }
            else if (t2.token_type == INT) {
                typeNum = 12;
            }
            else if (t2.token_type == BOOLEAN) {
                typeNum = 13;
            }
            else {
                typeNum = 14;
            }
            /// = ^^^^ might need to parse these^^^^
            var_decl->TN->tokenType = t2.token_type;

            for (node *node1 : deques.scope_indexes.back()->temp_type_RHS) {
                node1->idNumber = typeNum;
                deques.allDeclarations.push_back(node1);
                deques.scope_indexes.back()->declarations.push_back(node1);
            }
            deques.scope_indexes.back()->temp_type_RHS.clear();


            Token t3 = lexer.GetToken();
            //t3.Print();
            //// complete var_decl;
            if (t3.token_type == SEMICOLON) {
                var_decl->SC->tokenType = t3.token_type;
                var_decl->lexer = lexer;
                return var_decl;
            }
            else {

                //printf("Syntax Error at line number %d \n", __LINE__);
                syntaxError();
            }
        }
        else {

            //printf("Syntax Error at line number %d \n", __LINE__);
            syntaxError();
        }
    }
    else {

        //printf("Syntax Error at line number %d \n", __LINE__);
        syntaxError();
    }
}

struct scope_listTree *parse_scopeList(LexicalAnalyzer lexer) {
    //isSyntanxError();
    ////initialize
    struct scope_listTree *scope_list = new (scope_listTree);
//    scope_list->STMT = new(stmtTree);
//    scope_list->SL = new(scope_listTree);
//    scope_list->VD = new (var_declTree);
//    scope_list->S = new(scopeTree);
    scope_list->used_parses = new (deque<string>);

    Token t1 = lexer.GetToken();
    ////////////////////////////////////////
    ////    var_Decl or assign_Stmt
    ////////////////////////////////////////
    if (t1.token_type == ID) {
        Token t2 = lexer.GetToken();
        //t2.Print();
        ///////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////
        ////                  var_Decl
        //////////////////////////////////////////////////////
        //////////////////////////////////////////////////////
        if (t2.token_type == COLON || t2.token_type == COMMA) {
            lexer.UngetToken(t2);
            lexer.UngetToken(t1);
            scope_list->VD = parse_varDecl(lexer);
            lexer = scope_list->VD->lexer;

            scope_list->used_parses->push_back("VD");

            Token t3 = lexer.GetToken();
            ////////////////////////////////////////////////////////
            ////                var_decl
            ////////////////////////////////////////////////////////
            if (t3.token_type == RBRACE) {
                lexer.UngetToken(t3);
                scope_list->lexer = lexer;
                return scope_list;
            }
                ////////////////////////////////////////////////////
                ////            var_decl scope_list
                ///////////////////////////////////////////////////
            else if (t3.token_type == LBRACE || t3.token_type == ID || t3.token_type == WHILE) {
                lexer.UngetToken(t3);
                scope_list->SL = parse_scopeList(lexer);
                lexer = scope_list->SL->lexer;
                scope_list->used_parses->push_back("SL");
                scope_list->lexer = lexer;
                return scope_list;
            }
            else {
                //printf("Syntax Error at line number %d \n", __LINE__);
                syntaxError();
            }
        }
            ////////////////////////////////////////////////
            ////////////////////////////////////////////////
            ////             assign_Stmt
            ///////////////////////////////////////////////
            ///////////////////////////////////////////////
        else if (t2.token_type == EQUAL) {
            lexer.UngetToken(t2);
            lexer.UngetToken(t1);
            scope_list->STMT = parse_Stmt(lexer);
            lexer = scope_list->STMT->lexer;
            scope_list->used_parses->push_back("STMT");
            Token t3 = lexer.GetToken();
            ///////////////////////////////////////////
            //// stmt
            ///////////////////////////////////////////
            if (t3.token_type == RBRACE) {
                lexer.UngetToken(t3);
                scope_list->lexer = lexer;
                return scope_list;
            }
                ////////////////////////////////////////////
                //// stmt scope_list
                ////////////////////////////////////////////
            else if (t3.token_type == LBRACE || t3.token_type == ID || t3.token_type == WHILE) {
                lexer.UngetToken(t3);
                scope_list->SL = parse_scopeList(lexer);
                lexer = scope_list->SL->lexer;
                scope_list->used_parses->push_back("SL");
                scope_list->lexer = lexer;
                return scope_list;
            }
            else {
                //printf("Syntax Error at line number %d \n", __LINE__);
                syntaxError();
            }
        }
        else {

            //printf("Syntax Error at line number %d \n", __LINE__);
            syntaxError();
        }
    }
        ///////////////////////////////////////////////
        ///////////////////////////////////////////////
        ////             while_Stmt
        ///////////////////////////////////////////////
        ///////////////////////////////////////////////
    else if (t1.token_type == WHILE) {
        lexer.UngetToken(t1);

        //// initialization
        scope_list->STMT = parse_Stmt(lexer);
        lexer = scope_list->STMT->lexer;
        scope_list->used_parses->push_back("STMT");

        Token t2 = lexer.GetToken();
        ///////////////////////////////////////
        ////            stmt
        //////////////////////////////////////
        if (t2.token_type == RBRACE) {
            lexer.UngetToken(t2);

            scope_list->lexer = lexer;
            return scope_list;
        }
            //////////////////////////////////
            ////        stmt scope_list
            //////////////////////////////////
        else if (t2.token_type == LBRACE || t2.token_type == ID || t2.token_type == WHILE) {
            lexer.UngetToken(t2);
            scope_list->SL = parse_scopeList(lexer);
            lexer = scope_list->SL->lexer;
            scope_list->used_parses->push_back("SL");
            scope_list->lexer = lexer;
            return scope_list;
        }
        else {
            //printf("Syntax Error at line number %d \n", __LINE__);
            syntaxError();
        }
    }
        //////////////////////////////////
        //////////////////////////////////
        ////           scope
        //////////////////////////////////
        //////////////////////////////////
    else if (t1.token_type == LBRACE) {
        lexer.UngetToken(t1);

        //// initialization
        scope_list->S = parse_Scope(lexer);
        lexer = scope_list->S->lexer;
        scope_list->used_parses->push_back("S");

        Token t2 = lexer.GetToken();
        /////////////////////////////////////
        ////            scope
        /////////////////////////////////////
        if (t2.token_type == RBRACE) {
            lexer.UngetToken(t2);
            scope_list->lexer = lexer;
            return scope_list;
        }
            ////////////////////////////////////
            ////    scope scope_list
            ////////////////////////////////////
        else if (t2.token_type == LBRACE || t2.token_type == ID || t2.token_type == WHILE) {
            lexer.UngetToken(t2);

            scope_list->SL = parse_scopeList(lexer);
            lexer = scope_list->SL->lexer;
            scope_list->used_parses->push_back("SL");
            scope_list->lexer = lexer;
            return scope_list;
        }
        else {
            //printf("Syntax Error at line number %d \n", __LINE__);
            syntaxError();
        }
    }
    else {

        //printf("Syntax Error at line number %d \n", __LINE__);
        syntaxError();
    }
}

//// rb for Right Brace not Red-Black haha
struct rbTree *parse_End(LexicalAnalyzer lexer) {
    //isSyntanxError();
    struct rbTree *right_brace = new(rbTree);
    right_brace->RB = new (node);
    Token t1 = lexer.GetToken();
    //t1.Print();
    if (t1.token_type == RBRACE) {
        right_brace->RB->tokenType = t1.token_type;
        right_brace->lineNo = t1.line_no;
        Token t2 = lexer.GetToken();
        //t2.Print();
        if (t2.token_type == END_OF_FILE) {
            right_brace->END = new (node);
            right_brace->END->tokenType = t2.token_type;
            right_brace->lexer = lexer;
            return right_brace;
        }
        else {
            lexer.UngetToken(t2);
            right_brace->lexer = lexer;
            return right_brace;
        }
    }
    else {

        //printf("Syntax Error at line number %d \n", __LINE__);
        syntaxError();
    }
}

struct scopeTree *parse_Scope(LexicalAnalyzer lexer) {
    ////initialization
    struct scopeTree *scope = new(scopeTree);
    struct indexNode *index = new(indexNode);
    scope->index = new(indexNode);
    scope->LB = new(node);
    Token t1 = lexer.GetToken();
    //// make sure its scope
    if (t1.token_type == LBRACE) {
        //t1.Print();
        //// add left Brace index and scope type to deque and scopeTree struct
        index->start = t1.line_no;
        index->type = "SCOPE";
        scope->index = index;
        deques.scope_indexes.push_back(index);
        scope->LB->tokenType = t1.token_type;
        //// parse body////
        Token t2 = lexer.GetToken();
        lexer.UngetToken(t2);
        if (t2.token_type != LBRACE && t2.token_type != WHILE && t2.token_type != ID) {
            //printf("Syntax Error at line number %d \n", __LINE__);
            syntaxError();
        }
        scope->SL = parse_scopeList(lexer);
        lexer = scope->SL->lexer;
        ////parse right brace////
        Token t3 = lexer.GetToken();
        lexer.UngetToken(t3);
        if (t3.token_type != RBRACE) {
            //printf("Syntax Error at line number %d \n", __LINE__);
            syntaxError();
        }
        scope->RBT = parse_End(lexer);
        lexer = scope->RBT->lexer;
        //// add right Brace index and scope type to deque and scopeTree struct
        index->end = scope->RBT->lineNo;
        deques.scope_indexes.back()->end = index->end;
        if (deques.scope_indexes.back()->type == "WHILE") {
            //printf("Syntax Error at line number %d \n", __LINE__);
            syntaxError();
        }
        scope->index->end = index->end;
        deques.completed_indexes.push_front(deques.scope_indexes.back());
        //isSyntanxError();
        deques.scope_indexes.pop_back();
        //isSyntanxError();
        //// scope done
        scope->lexer = lexer;
        return scope;
    }
    else {

        //printf("Syntax Error at line number %d \n", __LINE__);
        syntaxError();
    }

}


int main() {
    //// initialize stuff
    struct programTree *program_tree = new(programTree);
    LexicalAnalyzer lexer;


    //// in case we need to check correct order
    //    Token token = lexicalAnalyzer->GetToken();
    //    token.Print();
    //    while (token.token_type != END_OF_FILE) {
    //        token = lexicalAnalyzer->GetToken();
    //        token.Print();
    //    }

    //// add scope tree to programs
    Token token = lexer.GetToken();
    if (token.token_type == LBRACE) {
        lexer.UngetToken(token);
        program_tree->S = parse_Scope(lexer);
    }
    else {
        //printf("Syntax Error at line number %d \n", __LINE__);
        syntaxError();
    }
//    for (indexNode *indexNode1 : deques.completed_indexes) {
//        for (node *node1: indexNode1->declarations) {
//            cout << node1->token.lexeme;
//        }
//        cout << "\n\n";
//    }
//    for (indexNode *indexNode1 : deques.completed_indexes) {
//        for (node *node1: indexNode1->references) {
//            cout << node1->token.lexeme;
//        }
//        cout << "\n\n";
//    }
    //// Error 1.1 & Error 1.2 ////
    if (deques.isDeclarationErrorFound) {
        cout << deques.declarationErrorFound;
        exit(EXIT_FAILURE);
    }
        //// Error 1.3 ////
    else {
        for (int i = 0; i < deques.completed_indexes.size(); ++i) {
            for (int j = 0; j < deques.completed_indexes.at(i)->declarations.size(); ++j) {
                bool used = false;
                for (int k = i; k < deques.completed_indexes.size(); ++k) {
                    for (int l = 0; l < deques.completed_indexes.at(k)->references.size(); ++l) {
                        if (deques.completed_indexes.at(i)->declarations.at(j)->token.lexeme ==
                            deques.completed_indexes.at(k)->references.at(l)->token.lexeme) {
                            used = true;
                        }
                    }
                }
                if (!used) {
                    cout << "ERROR CODE 1.3 " << deques.completed_indexes.at(i)->declarations.at(j)->token.lexeme;
                    exit(EXIT_FAILURE);
                }
            }

        }
    }
    if (deques.isTypeErrorFound) {
        cout << deques.typeErrorFound;
        exit(EXIT_FAILURE);
    }
}
//cout << "ERROR CODE 1.3 " << token.lexeme;
//                exit(EXIT_FAILURE);
//// print program tree

