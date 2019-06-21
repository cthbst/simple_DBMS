#include <stdlib.h>
#include <string.h>
#include "Command.h"

CMD_t cmd_list[] = {
    { ".exit", 5, BUILT_IN_CMD },
    { ".output", 7, BUILT_IN_CMD },
    { ".load", 5, BUILT_IN_CMD },
    { ".help", 5, BUILT_IN_CMD },
    { "insert", 6, QUERY_CMD },
    { "select", 6, QUERY_CMD },
    { "delete", 6, QUERY_CMD },
    { "update", 6, QUERY_CMD },
    { "", 0, UNRECOG_CMD },
};

SelectArgs_t::SelectArgs_t(){
    offset = -1;
    limit = -1;
}

Condition_t::Condition_t(){
    cnt_statment = 0;
}

Command_t::Command_t(){
    type = UNRECOG_CMD;
}

Command_t::Command_t(const std::vector<std::string> &_args){
    type = UNRECOG_CMD;
    args = _args;
    for (size_t i = 0; i < 8; i++){
        if (args[0] == cmd_list[i].name){
            type = cmd_list[i].type;
        }
    }
}

void add_select_field(Command_t &cmd, const std::string &argument) {
    cmd.sel_args.fields.push_back(argument);
}
