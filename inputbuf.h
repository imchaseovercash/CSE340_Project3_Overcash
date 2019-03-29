//
// Created by Chase Overcash on 3/20/2019.
//

#ifndef CSE340_PROJECT3_OVERCASH_INPUTBUF_H
#define CSE340_PROJECT3_OVERCASH_INPUTBUF_H

#include <string>

class InputBuffer {
public:
    bool GetChar(char&);
    char UngetChar(char);
    std::string UngetString(std::string);
    bool EndOfInput();

private:
    std::vector<char> input_buffer;
};
#endif //CSE340_PROJECT3_OVERCASH_INPUTBUF_H
