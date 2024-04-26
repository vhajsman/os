#include "shell.h"
#include "memory/memory.h"
#include "string.h"
#include "debug.h"

#define TOKEN_SEPARATOR '\n'

struct shell_parseout parse(char* userInput) {
    struct shell_parseout output;

    size_t len = strlen(userInput);
    if(! len) {
        output.tok_count = 0;
        output.tok_str = "";

        return output;
    }

    u8 quotes = 0;

    char* tok_str;
    size_t tok_count = 0;

    size_t i, of;
    for(i, of = 0; i < len; i ++) {
        switch(userInput[i]) {
            case '\0':
                i = len;
                // of ++;
                break;

            case '\n':
                if(!quotes) {
                    i = len;    // ? Breaks the for loop
                    tok_count ++;
                    of++;
                }

                break;
            
            case '\t':
            case ' ':
                if(!quotes) {
                    tok_str[i] = TOKEN_SEPARATOR;
                    tok_count ++;
                    of ++;
                }

                break;
            
            case '"':
                quotes = !quotes;
                of ++;
                break;

            default:
                tok_str[i - of] = userInput[i];
        }
    }
}

int shell_handleUserInput(char* userInput) {
    // Ignore comment
    if(userInput[0] == '#')
        return 0;

    // --- Parse ---

    struct shell_parseout p;

    char* buf_c;    itoa(buf_c, 10, p.tok_count);

    debug_message("Parser: User input parse. ", "Shell", KERNEL_MESSAGE);
    debug_append("Token count: ");
    debug_append(buf_c);
    debug_append(" Token array[...] = {");

    for(size_t i = 0; i < MAX_TOKS; i ++) {
        debug_append("'");
        debug_append(p.tok_arr[i]);
        debug_append("', ");
    }

    return 0;
}