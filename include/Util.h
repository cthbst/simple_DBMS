#ifndef DB_UTIL_H
#define DB_UTIL_H
#include "Command.h"
#include "Table.h"
#include <vector>
#include <string>

void print_user(const User_t &user, SelectArgs_t &sel_args);
void print_users(Table_t &table, const std::vector<size_t>& idxList, Command_t &cmd);
void handle_builtin_cmd(Table_t &table, Command_t &cmd);
int handle_query_cmd(Table_t &table, Command_t &cmd);
int handle_insert_cmd(Table_t &table, Command_t &cmd);
int handle_select_cmd(Table_t &table, Command_t &cmd);
void print_help_msg();

bool check_condition(const User_t &user, Command_t &cmd);
bool check_compare_statment(const User_t &user,const CompareStatment_t &s);
std::string get_string_variable(const User_t &user, const std::string &s);
int get_numeric_variable(const User_t &user, const std::string &s);
std::vector<size_t> select_valid_user(Table_t &table, Command_t &cmd);
void print_aggregate(Table_t &table, const std::vector<size_t>& idxList, Command_t &cmd);
void handle_delete_cmd(Table_t &table, Command_t &cmd);
void handle_update_cmd(Table_t &table, Command_t &cmd);

void print_likes(Table_t &table, Command_t &cmd);
void print_like(const std::pair<int,int> &like, SelectArgs_t &args);
void print_likes_aggregate(Table_t &table, Command_t &cmd);
void print_join(Table_t &table, const std::vector<size_t>& idxList, Command_t &cmd);

#endif
