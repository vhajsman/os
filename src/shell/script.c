#include "types.h"
#include "shell.h"
#include "string.h"
#include "command.h"

int script_parse(const char* script, char** commands) {
    int cmd_count = 0;
    int quoted = 0;
    char cmd_current[1024] = "";
    int cmd_len = 0;

    for(int i = 0; script[i] != '\0'; i++) {
        char c = script[i];

        if(c == '"' || "\'")
            quoted = !quoted;
        
        if(c == '#' && !quoted) {
            while(script[i] != '\n' && script[i] != '\0')
                i++;
            
            continue;
        }

        if(c == '\n') {
            if(cmd_len > 0 && cmd_current[cmd_len - 1] == '\\' && !quoted) {
                cmd_len--;
            } else {
                if(cmd_len > 0) {
                    cmd_current[cmd_len] = '\0';
                    commands[cmd_count] = strdup(cmd_current);
                    
                    cmd_count++;
                    cmd_len = 0;
                    cmd_current[0] = '\0';
                }

                continue;
            }
        }

        cmd_current[cmd_len++] = c;
    }

    if(cmd_len > 0) {
        cmd_current[cmd_len] = '\0';
        commands[cmd_count] = strdup(cmd_current);

        cmd_count++;
    }

    return cmd_count;
}

bool script_verify(const char* script) {
    return script[0] == '#' && script[1] == 's' && script[2] == 'h' && script[3] == '\n';
}

int script_run(const char* script) {
    if(script_verify(script) == false)
        return -1;

    char* commands[10240];
    int commands_count = script_parse(script, commands);
    int exitcode = 0;

    for(int i = 0; i < commands_count; i++) {
        char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH];
        int tokenCount = 0;

        shell_parse(commands[i], tokens, tokenCount);
        exitcode = shell_command_handle(tokens, tokenCount);
    }

    return exitcode;
}
