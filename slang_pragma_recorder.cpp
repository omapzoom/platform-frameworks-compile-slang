#include "slang_pragma_recorder.hpp"

#include "clang/Lex/Token.h"            /* for class clang::Token */

#include "clang/Basic/TokenKinds.h"     /* for enum clang::tok::* */

#include "clang/Lex/Preprocessor.h"     /* for class clang::Preprocessor */

namespace slang {

bool PragmaRecorder::GetPragmaNameFromToken(const Token& Token, std::string& PragmaName) {
    if(Token.isLiteral())
        PragmaName.assign(Token.getLiteralData(), Token.getLength());
    else if(Token.is(clang::tok::identifier))
        PragmaName.assign(Token.getIdentifierInfo()->getNameStart(), Token.getIdentifierInfo()->getLength());
    else
        return false;
    return true;
}

bool PragmaRecorder::GetPragmaValueFromToken(const Token& Token, std::string& PragmaValue) {
    /* Same as the GetPragmaName() */
    if(Token.is(clang::tok::r_paren))
        PragmaValue.clear();
    else
        return GetPragmaNameFromToken(Token, PragmaValue);
    return true;
}

PragmaRecorder::PragmaRecorder(PragmaList& Pragmas) : PragmaHandler(NULL), mPragmas(Pragmas) { return; }

void PragmaRecorder::HandlePragma(Preprocessor &PP, Token &FirstToken) {
    Token& CurrentToken = FirstToken;
    std::string PragmaName, PragmaValue = "";
    /* Pragma in ACC should be a name/value pair */

    if(GetPragmaNameFromToken(FirstToken, PragmaName)) {
        /* start parsing the value '(' PragmaValue ')' */
        const Token* NextToken = &PP.LookAhead(0);

        if(NextToken->is(clang::tok::l_paren))
            PP.LexUnexpandedToken(CurrentToken);
        else
            goto end_parsing_pragma_value;

        NextToken = &PP.LookAhead(0);
        if(GetPragmaValueFromToken(*NextToken, PragmaValue))
            PP.Lex(CurrentToken);
        else
            goto end_parsing_pragma_value;

        if(!NextToken->is(clang::tok::r_paren)) {
            NextToken = &PP.LookAhead(0);
            if(NextToken->is(clang::tok::r_paren))
                PP.LexUnexpandedToken(CurrentToken);
            else
                goto end_parsing_pragma_value;
        }

        /* Until now, we ensure that we have a pragma name/value pair */
        mPragmas.push_back( make_pair(PragmaName, PragmaValue) );
    }

end_parsing_pragma_value:
    /* Infor lex to eat the token */
    PP.LexUnexpandedToken(CurrentToken);

    return;
}

}   /* namespace slang */