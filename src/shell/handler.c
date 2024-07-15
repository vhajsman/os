#include "shell.h"
#include "memory/memory.h"
#include "string.h"
#include "debug.h"
#include "console.h"

#define TOKEN_SEPARATOR '\n'

char* replace_spaces(const char* str, char replace_with) {
    int inside_quotes = 0;
    size_t len = strlen(str);

    char* result = (char*) malloc((len + 1) * sizeof(char));

    if (!result) {
        puts("Handler: Malloc failed");
        debug_message("Malloc failed", "Shell->handler", KERNEL_ERROR);

        return NULL;
    }

    size_t i, j;
    for (i = 0, j = 0; i < len; i++, j++) {
        if (str[i] == '"' && !inside_quotes) {
            inside_quotes = 1;
            result[j] = str[i];
        } else if (str[i] == '"' && inside_quotes) {
            inside_quotes = 0;
            result[j] = str[i];
        } else if (str[i] == ' ' && !inside_quotes) {
            result[j] = replace_with;
        } else {
            result[j] = str[i];
        }
    }

    result[j] = '\0';

    return result;
}

char** split_string(const char* str, int* num_parts) {
    int len = 0;
    const char* ptr = str;

    // Count the number of parts
    while (*ptr != '\0') {
        if (*ptr == TOKEN_SEPARATOR) {
            while (*ptr == TOKEN_SEPARATOR) {
                ptr++;  // Skip consecutive spaces
            }
        } else {
            len++;
            while (*ptr != ' ' && *ptr != '\0') {
                ptr++;
            }
        }
    }

    if (len == 0) {
        *num_parts = 0;
        return NULL;
    }

    // Allocate memory for the array of strings
    char** parts = (char**)malloc(len * sizeof(char*));
    if (parts == NULL) {
        *num_parts = 0;
        return NULL;
    }

    // Split the string and store the parts in the array
    int part_index = 0;
    ptr = str;
    while (*ptr != '\0') {
        if (*ptr == TOKEN_SEPARATOR) {
            while (*ptr == TOKEN_SEPARATOR) {
                ptr++;  // Skip consecutive spaces
            }
        } else {
            const char* start = ptr;
            while (*ptr != ' ' && *ptr != '\0') {
                ptr++;
            }

            int part_len = ptr - start;
            parts[part_index] = (char*)malloc((part_len + 1) * sizeof(char));
            if (parts[part_index] == NULL) {
                // Free memory allocated so far

                for (int i = 0; i < part_index; i++) {
                    free(parts[i]);
                }

                free(parts);
                *num_parts = 0;

                return NULL;
            }

            strncpy(parts[part_index], start, part_len);

            parts[part_index][part_len] = '\0';  // Null-terminate the string
            part_index++;
        }
    }

    *num_parts = len;
    return parts;
}

void arrcpy(char** src, char* dest[], size_t dest_size) {
    for(size_t i = 0; i < dest_size; i ++) {
        if(src[i] != NULL) {
            strncpy(dest[i], src[i], MAX_TOKS - 1);
            dest[i][MAX_TOKS - 1] = '\0';
        } else {
            dest[i][0] = '\0';
        }
    }
}

struct shell_execcall shell_genExecutionCall(struct shell_parseout* parseData) {
    struct shell_execcall call;

    call.prompt = shell_getPrompt();

    call.call = parseData->tok_arr[0];
    call.argc = parseData->tok_count > 0 ? parseData->tok_count - 1 : 0;

    if(parseData->tok_count > 1) {
        for(size_t i = 0; i < sizeof(parseData->tok_arr) / sizeof(parseData->tok_arr[0]); i ++) 
            call.argv[i] = parseData->tok_arr[i + 1];
    } else {
        call.argv = NULL;
    }
}

int shell_handleUserInput(char* userInput) {
    // Ignore comment
    if(userInput[0] == '#')
        return 0;

    // --- Parse ---

    struct shell_parseout p;

    arrcpy(&userInput, p.tok_arr, MAX_TOKS);
    p.tok_str = replace_spaces(userInput, TOKEN_SEPARATOR);
    p.tok_count = sizeof(p.tok_arr) / sizeof(p.tok_arr[0]);

    char* buf_c;
    itoa(buf_c, 10, p.tok_count);

    debug_message("Parser: User input parse. ", "Shell", KERNEL_MESSAGE);
    debug_append("Token count: ");
    debug_append(buf_c);
    debug_append(" Token array[...] = {");

    for(size_t i = 0; i < MAX_TOKS; i ++) {
        char* ct = p.tok_arr[i];
        if(ct == NULL)
            continue;

        debug_append("'");
        debug_append(p.tok_arr[i]);
        debug_append("', ");
    }

    debug_append("}");

    return 0;
}