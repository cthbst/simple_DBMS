#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "InputBuffer.h"
#include "Util.h"
#include "Table.h"


int main() {
    Inputs_t Inputs = read_inputs();
    Table_t table;

    for (InputBuffer_t &buf : Inputs){
        Command_t cmd;
        cmd.args = buf;
        if (cmd.type == BUILT_IN_CMD){
            handle_builtin_cmd(table, cmd);
        } else if (cmd.type == QUERY_CMD){
            handle_query_cmd(table, cmd);
        }
    }
    return 0;
}
