#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "InputBuffer.h"
#include "Util.h"
#include "Table.h"


int main() {
    std::cin.tie(0); std::cin.sync_with_stdio(0);
    std::cout.tie(0);
    Inputs_t Inputs = read_inputs();
    Table_t table;

    for (InputBuffer_t &buf : Inputs){
        Command_t cmd(buf);
        if (cmd.type == BUILT_IN_CMD){
            handle_builtin_cmd(table, cmd);
        } else if (cmd.type == QUERY_CMD){
            handle_query_cmd(table, cmd);
        }
    }
    return 0;
}
