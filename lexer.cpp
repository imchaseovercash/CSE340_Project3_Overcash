//
// Created by Chase Overcash on 3/20/2019.
//

#include <iostream>
#include <istream>
#include <vector>
#include <string>
#include <cctype>

#include "lexer.h"
#include "inputbuf.h"

using namespace std;

string reserved[] = { "END_OF_FILE",
                      "REAL", "INT", "BOOLEAN", "STRING",
                      "WHILE", "TRUE", "FALSE", "COMMA", "COLON", "SEMICOLON",
                      "LBRACE", "RBRACE", "LPAREN", "RPAREN",
                      "EQUAL", "PLUS", "MINUS", "MULT", "DIV","AND", "OR", "XOR", "NOT",
                      "GREATER", "GTEQ", "LESS", "LTEQ", "NOTEQUAL",
                      "ID", "NUM", "REALNUM", "STRING_CONSTANT", "ERROR"
};

#define KEYWORDS_COUNT 7
string keyword[] = {
        "REAL", "INT", "BOOLEAN", "STRING",
        "WHILE", "TRUE", "FALSE"
};

void Token::Print()
{
    cout << "{" << this->lexeme << " , "
         << reserved[(int) this->token_type] << " , "
         << this->line_no << "}\n";

}

LexicalAnalyzer::LexicalAnalyzer()
{
    this->line_no = 1;
    tmp.lexeme = "";
    tmp.line_no = 1;
    tmp.token_type = ERROR;
}

int LexicalAnalyzer::get_line_no() {
    return line_no;
}

bool LexicalAnalyzer::SkipSpace()
{
    char c;
    bool space_encountered = false;

    if (input.EndOfInput())
        return false;

    input.GetChar(c);
    line_no += (c == '\n');

    while (!input.EndOfInput() && isspace(c)) {
        space_encountered = true;
        if (input.GetChar(c))
            line_no += (c == '\n');
    }

    if (!input.EndOfInput()) {
        input.UngetChar(c);
    }
    return space_encountered;
}

bool LexicalAnalyzer::IsKeyword(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return true;
        }
    }
    return false;
}

TokenType LexicalAnalyzer::FindKeywordIndex(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return (TokenType) (i + 1);
        }
    }
    return ERROR;
}


Token LexicalAnalyzer::ScanNumber()
{
    char c;
    tmp.lexeme = "";
    tmp.token_type = ERROR;
    tmp.line_no = line_no;

    if (input.EndOfInput())
        return tmp;

    input.GetChar(c);
    if (isdigit(c)) {
        tmp.token_type = NUM;
        if (c == '0') {
            tmp.lexeme = "0";
        } else {
            tmp.lexeme = "";
            while (!input.EndOfInput() && isdigit(c)) {
                tmp.lexeme += c;
                input.GetChar(c);
            }
            if (!input.EndOfInput()) {
                input.UngetChar(c);
            }
        }
        if (input.EndOfInput())
            return tmp;
        input.GetChar(c);

        if (c == '.') {           // possibly REALNUM
            input.GetChar(c);
            if (isdigit(c)) {     // definitely REALNUM
                tmp.lexeme += '.';
                while (isdigit(c)) {
                    tmp.lexeme += c;
                    input.GetChar(c);
                }
                if (!input.EndOfInput()) {
                    input.UngetChar(c);
                }
                tmp.token_type = REALNUM;
            } else {
                if (!input.EndOfInput()) {
                    input.UngetChar(c);
                }
                input.UngetChar('.');
                tmp.token_type = NUM;
            }
        } else {
            if (!input.EndOfInput())
                input.UngetChar(c);
            tmp.token_type = NUM;
        }
        return tmp;
    } else {
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.token_type = ERROR;
        return tmp;
    }
}

Token LexicalAnalyzer::ScanIdOrKeyword()
{
    char c;
    tmp.lexeme = "";
    tmp.token_type = ERROR;
    tmp.line_no = line_no;

    if (input.EndOfInput())
        return tmp;


    input.GetChar(c);
    if (isalpha(c)) {
        tmp.lexeme = "";
        while (!input.EndOfInput() && isalnum(c)) {
            tmp.lexeme += c;
            input.GetChar(c);
        }
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.token_type = ID;
        tmp.line_no = line_no;
        if (IsKeyword(tmp.lexeme))
            tmp.token_type = FindKeywordIndex(tmp.lexeme);

    } else {
        input.UngetChar(c);
        tmp.lexeme = "";
        tmp.token_type = ERROR;
    }
    return tmp;
}

Token LexicalAnalyzer::ScanStringCons()
{
    char c;
    tmp.lexeme = "";
    tmp.token_type = ERROR;
    tmp.line_no = line_no;

    if (input.EndOfInput())
        return tmp;

    input.GetChar(c);
    string lexeme = "";

    if (c == '"') {
        tmp.lexeme = "";
        input.GetChar(c);
        while (!input.EndOfInput() && isalnum(c)) {
            lexeme += c;
            input.GetChar(c);
        }
        if (!input.EndOfInput()) {
            if (c == '"') {
                tmp.lexeme += lexeme;
                tmp.token_type = STRING_CONSTANT;
            }
            else{
                tmp.lexeme = "";
                tmp.token_type = ERROR;
            }

        }
        else{
            tmp.lexeme = "";
            tmp.token_type = ERROR;
        }

        tmp.line_no = line_no;

    } else {
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
    }
    return tmp;
}
// you should unget tokens in the reverse order in which they
// are obtained. If you execute
//
//    t1 = lexer.GetToken();
//    t2 = lexer.GetToken();
//    t3 = lexer.GetToken();
//
// in this order, you should execute
//
//    lexer.UngetToken(t3);
//    lexer.UngetToken(t2);
//    lexer.UngetToken(t1);
//
// if you want to unget all three tokens. Note that it does not
// make sense to unget t1 without first ungetting t2 and t3
//
TokenType LexicalAnalyzer::UngetToken(Token tok)
{
    tokens.push_back(tok);;
    return tok.token_type;
}

Token LexicalAnalyzer::GetToken()
{
    char c;

    // if there are tokens that were previously
    // stored due to UngetToken(), pop a token and
    // return it without reading from input
    if (!tokens.empty()) {
        tmp = tokens.back();
        tokens.pop_back();
        return tmp;
    }

    SkipSpace();
    tmp.lexeme = "";
    tmp.token_type = END_OF_FILE;
    tmp.line_no = line_no;

    if (!input.EndOfInput())
        input.GetChar(c);
    else
        return tmp;

    switch (c) {
        case ',': tmp.token_type = COMMA;       return tmp;
        case ':': tmp.token_type = COLON;       return tmp;
        case ';': tmp.token_type = SEMICOLON;   return tmp;
        case '{': tmp.token_type = LBRACE;      return tmp;
        case '}': tmp.token_type = RBRACE;      return tmp;
        case '(': tmp.token_type = LPAREN;      return tmp;
        case ')': tmp.token_type = RPAREN;      return tmp;
        case '=': tmp.token_type = EQUAL;       return tmp;
        case '+': tmp.token_type = PLUS;        return tmp;
        case '-': tmp.token_type = MINUS;       return tmp;
        case '*': tmp.token_type = MULT;        return tmp;
        case '/': tmp.token_type = DIV;        return tmp;
        case '|': tmp.token_type = OR;          return tmp;
        case '^': tmp.token_type = AND;         return tmp;
        case '&': tmp.token_type = XOR;         return tmp;
        case '~': tmp.token_type = NOT;         return tmp;
        case '>':
            input.GetChar(c);
            if (c == '=') {
                tmp.token_type = GTEQ;
            } else {
                if (!input.EndOfInput()) {
                    input.UngetChar(c);
                }
                tmp.token_type = GREATER;
            }
            return tmp;
        case '<':
            input.GetChar(c);
            if (c == '=') {
                tmp.token_type = LTEQ;
            } else if (c == '>') {
                tmp.token_type = NOTEQUAL;
            } else {
                if (!input.EndOfInput()) {
                    input.UngetChar(c);
                }
                tmp.token_type = LESS;
            }
            return tmp;

            //STRING_CONSTANT
        case '"':
            input.UngetChar(c);
            return ScanStringCons();

        default:
            if (isdigit(c)) {
                input.UngetChar(c);
                return ScanNumber();
            } else if (isalpha(c)) {
                input.UngetChar(c);
                return ScanIdOrKeyword();
            } else if (input.EndOfInput()) {
                tmp.token_type = END_OF_FILE;
            } else {
                tmp.lexeme += c;
                tmp.token_type = ERROR;
            }
            return tmp;
    }
}

void syntaxError() {
    cout << "Syntax_Error ";
}

bool is_Primary(Token token) {
    return token.token_type == ID || token.token_type == NUM || token.token_type == REALNUM ||
           token.token_type == STRING_CONSTANT || token.token_type == TRUE || token.token_type == FALSE;
}

bool is_Arithmetic(Token t2) {
    return t2.token_type == PLUS || t2.token_type == MINUS || t2.token_type == MULT || t2.token_type == DIV;
}

bool is_BinaryBool(Token t2) {
    return t2.token_type == AND || t2.token_type == OR || t2.token_type == XOR;
}

bool is_Relational(Token t2) {
    return t2.token_type == GREATER || t2.token_type == GTEQ || t2.token_type == LESS || t2.token_type == NOTEQUAL ||
           t2.token_type == LTEQ;
}

bool is_Expr(Token token) {
    return is_Arithmetic(token) || is_BinaryBool(token) || is_Relational(token) || token.token_type == ID ||
           token.token_type == NOT;
}

void parse_varDecl()
{

}
void parse_Expr() {
    LexicalAnalyzer lexer;
    Token t1 = lexer.GetToken();
    if (is_Relational(t1)) {
        t1.Print();
        Token t2 = lexer.GetToken();
        if (is_Expr(t2)) {
            lexer.UngetToken(t2);
            parse_Expr();
            Token t3 = lexer.GetToken();
            if (is_Expr(t3)) {
                lexer.UngetToken(t3);
                parse_Expr();
            } else {
                syntaxError();
            }
        } else {
            syntaxError();
        }
    } else if (is_BinaryBool(t1)) {
        t1.Print();
        Token t2 = lexer.GetToken();
        if (is_Expr(t2)) {
            lexer.UngetToken(t2);
            parse_Expr();
            Token t3 = lexer.GetToken();
            if (is_Expr(t3)) {
                lexer.UngetToken(t3);
                parse_Expr();
            } else {
                syntaxError();
            }
        } else {
            syntaxError();
        }
    } else if (is_Arithmetic(t1)) {
        t1.Print();
        Token t2 = lexer.GetToken();
        if (is_Expr(t2)) {
            lexer.UngetToken(t2);
            parse_Expr();
            Token t3 = lexer.GetToken();
            if (is_Expr(t3)) {
                lexer.UngetToken(t3);
                parse_Expr();
            } else {
                syntaxError();
            }
        } else {
            syntaxError();
        }
    } else if (t1.token_type == NOT) {
        t1.Print();
        Token t2 = lexer.GetToken();
        if (is_Expr(t2)) {
            lexer.UngetToken(t2);
            parse_Expr();
        } else {
            syntaxError();
        }
    } else if (is_Primary(t1)) {
        t1.Print();
    } else {
        syntaxError();
    }

}
void parse_whileStmt();

void parse_assignStmt() {
    LexicalAnalyzer lexer;
    Token t1 = lexer.GetToken();
    if (is_Arithmetic(t1) || is_BinaryBool(t1) || is_Relational(t1) || t1.token_type == NOT || is_Primary(t1)) {
        lexer.UngetToken(t1);
        parse_Expr();
    } else {
        syntaxError();
    }
}

void parse_Stmt() {
    LexicalAnalyzer lexer;
    Token t1 = lexer.GetToken();
    if (t1.token_type == ID) {
        t1.Print();
        Token t2 = lexer.GetToken();
        if (t2.token_type == EQUAL) {
            t2.Print();
            parse_assignStmt();
        } else {
            syntaxError();
        }
    } else if (t1.token_type == WHILE) {
        t1.Print();
        parse_whileStmt();
    } else {
        syntaxError();
    }
}

void parse_stmtList() {
    LexicalAnalyzer lexer;
    Token t1 = lexer.GetToken();
    if (t1.token_type == ID) {
        t1.Print();
        Token t2 = lexer.GetToken();
        if (t2.token_type == EQUAL) {
            t2.Print();
            parse_assignStmt();
            Token t3 = lexer.GetToken();
            if(t3.token_type == ID || t3.token_type == WHILE)
            {
                lexer.UngetToken(t3);
                parse_stmtList();
            }
        } else {
            syntaxError();
        }
    } else if (t1.token_type == WHILE) {
        t1.Print();
        parse_whileStmt();
        Token t3 = lexer.GetToken();
        if(t3.token_type == ID || t3.token_type == WHILE)
        {
            lexer.UngetToken(t3);
            parse_stmtList();
        }
    } else {
        syntaxError();
    }
}

void parse_whileStmt() {
    LexicalAnalyzer lexer;
    Token t1 = lexer.GetToken();
    if (t1.token_type == LPAREN) {
        t1.Print();
        Token t2 = lexer.GetToken();
        if (is_Expr(t2))
        {
            lexer.UngetToken(t2);
            parse_Expr();
            t2 = lexer.GetToken();
            if (t2.token_type == RPAREN )
            {
               t2.Print();
               Token t3 = lexer.GetToken();
               if (t3.token_type == ID || t3.token_type == WHILE)
               {
                   lexer.UngetToken(t3);
                   parse_Stmt();
               } else{
                   syntaxError();
               }
            } else if (t2.token_type == LBRACE){
                t2.Print();
                parse_stmtList();
                Token t3 = lexer.GetToken();
                if (t3.token_type == RBRACE)
                {
                    t3.Print();
                }
            } else{
                syntaxError();
            }
        } else
        {
            syntaxError();
        }
    }
}



void parse_scope() {
    LexicalAnalyzer lexer;
    Token t1 = lexer.GetToken();
    if (t1.token_type == LBRACE) {
        t1.Print();
        Token t2 = lexer.GetToken();
        if (t2.token_type == ID) {
            Token t3 = lexer.GetToken();
            if (t3.token_type == COLON || t3.token_type == COMMA) {
                lexer.UngetToken(t3);
                parse_varDecl();
            } else if (t3.token_type == EQUAL) {
                parse_assignStmt();
            } else
                syntaxError();
        } else if (t2.token_type == WHILE) {
            t2.Print();
            parse_whileStmt();
        } else {
            syntaxError();
        }
    }

}

int main() {
    LexicalAnalyzer lexer;
    Token token;

    token = lexer.GetToken();
    token.Print();
    if (token.token_type == LBRACE) {
        lexer.UngetToken(token);
        parse_scope();
    } else {
        syntaxError();
    }


}
