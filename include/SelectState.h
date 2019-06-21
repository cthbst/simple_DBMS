#ifndef SELECT_STATE_H
#define SELECT_STATE_H

#include "Command.h"
using Iter = std::vector<std::string>::iterator;

void field_state_handler(Command_t &cmd, Iter &it);
void table_state_handler(Command_t &cmd, Iter &it);
void offset_state_handler(Command_t &cmd, Iter &it);
void limit_state_handler(Command_t &cmd, Iter &it);

void where_state_handler(Command_t &cmd, Iter &it);
void parse_compare_statment(CompareStatment_t &stat, Iter &it);

#endif
