#include "Command.h"
#include "SelectState.h"
#include <string.h>
#include <stdlib.h>

void field_state_handler(Command_t &cmd, Iter &it) {
    while (it != cmd.args.end()) {
        if (*it == "*"
                || *it == "id" || *it == "name" || *it == "email" || *it == "age"
                || (*it).substr(0,5) == "count" || (*it).substr(0,3) == "sum" || (*it).substr(0,3) == "avg"
            ) {
            add_select_field(cmd, *it);
            it++;
        } else if (*it == "from") {
            table_state_handler(cmd, ++it);
        } else {
            return;
        }
    }
}

void table_state_handler(Command_t &cmd, Iter &it) {
    if (it == cmd.args.end()) return;

    if (*(it++) != "user") return;
    if (it == cmd.args.end()) return;

    if (*it == "where") {
        where_state_handler(cmd, ++it);
    } else if (*it == "offset") {
        offset_state_handler(cmd, ++it);
    } else if (*it == "limit") {
        limit_state_handler(cmd, ++it);
    }
}

void where_state_handler(Command_t &cmd, Iter &it) {
    if (it == cmd.args.end()) return;
    
    cmd.condition.cnt_statment++;
    parse_compare_statment(cmd.condition.s[0], it);
    if (it == cmd.args.end()) return;
    
    if (*it == "and" || *it == "or") {
        cmd.condition.logic = (*(it++)=="and"?AND:OR);
        cmd.condition.cnt_statment++;
        parse_compare_statment(cmd.condition.s[1], it);
        if (it == cmd.args.end()) return;
    }
    
    if (*it == "offset") {
        offset_state_handler(cmd, ++it);
    } else if (*it == "limit") {
        limit_state_handler(cmd, ++it);
    }
}

void parse_compare_statment(CompareStatment_t &stat, Iter &it) {
    stat.lhs = *(it++);
    stat.op = *(it++);
    stat.rhs = *(it++);
}

void offset_state_handler(Command_t &cmd, Iter &it) {
    if (it == cmd.args.end()) return;

    cmd.sel_args.offset = stoi(*(it++));
    if (it == cmd.args.end()) return;
    
    if (*it == "limit") {
        limit_state_handler(cmd, ++it);
    }
}

void limit_state_handler(Command_t &cmd, Iter &it) {
    if (it == cmd.args.end()) return;
    cmd.sel_args.limit = stoi(*(it++));
}
