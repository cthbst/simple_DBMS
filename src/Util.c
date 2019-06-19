#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "Util.h"
#include "Command.h"
#include "Table.h"
#include "SelectState.h"

///
/// Allocate State_t and initialize some attributes
/// Return: ptr of new State_t
///
State_t* new_State() {
    State_t *state = (State_t*)malloc(sizeof(State_t));
    state->saved_stdout = -1;
    return state;
}

///
/// Print shell prompt
///
void print_prompt(State_t *state) {
    if (state->saved_stdout == -1) {
        printf("db > ");
    }
}

///
/// Print the user in the specific format
///
void print_user(const User_t &user, SelectArgs_t *sel_args) {
    size_t idx;
    printf("(");
    for (idx = 0; idx < sel_args->fields_len; idx++) {
        if (!strncmp(sel_args->fields[idx], "*", 1)) {
            printf("%d, %s, %s, %d", user.id, user.name.c_str(), user.email.c_str(), user.age);
        } else {
            if (idx > 0) printf(", ");

            if (!strncmp(sel_args->fields[idx], "id", 2)) {
                printf("%d", user.id);
            } else if (!strncmp(sel_args->fields[idx], "name", 4)) {
                printf("%s", user.name.c_str());
            } else if (!strncmp(sel_args->fields[idx], "email", 5)) {
                printf("%s", user.email.c_str());
            } else if (!strncmp(sel_args->fields[idx], "age", 3)) {
                printf("%d", user.age);
            }
        }
    }
    printf(")\n");
}

///
/// Print the users for given offset and limit restriction
///
void print_users(Table_t &table, const std::vector<size_t>& idxList, Command_t *cmd) {
    int limit = cmd->cmd_args.sel_args.limit;
    int offset = cmd->cmd_args.sel_args.offset;

    if (offset == -1) {
        offset = 0;
    }

    for (int idx = offset; idx < (int)idxList.size(); idx++) {
        if (limit != -1 && (idx - offset) >= limit) {
            break;
        }
        print_user(table.users[ idxList[idx] ], &(cmd->cmd_args.sel_args));
    }
}

///
/// This function received an output argument
/// Return: category of the command
///
int parse_input(char *input, Command_t *cmd) {
    char *token;
    int idx;
    token = strtok(input, " ,\n");
    for (idx = 0; strlen(cmd_list[idx].name) != 0; idx++) {
        if (!strncmp(token, cmd_list[idx].name, cmd_list[idx].len)) {
            cmd->type = cmd_list[idx].type;
        }
    }
    while (token != NULL) {
        add_Arg(cmd, token);
        token = strtok(NULL, " ,\n");
    }
    return cmd->type;
}

///
/// Handle built-in commands
/// Return: command type
///
void handle_builtin_cmd(Table_t &table, Command_t *cmd, State_t *state) {
    if (!strncmp(cmd->args[0], ".exit", 5)) {
        exit(0);
    } else if (!strncmp(cmd->args[0], ".output", 7)) {
        if (cmd->args_len == 2) {
            if (!strncmp(cmd->args[1], "stdout", 6)) {
                close(1);
                dup2(state->saved_stdout, 1);
                state->saved_stdout = -1;
            } else if (state->saved_stdout == -1) {
                int fd = creat(cmd->args[1], 0644);
                state->saved_stdout = dup(1);
                if (dup2(fd, 1) == -1) {
                    state->saved_stdout = -1;
                }
                __fpurge(stdout); //This is used to clear the stdout buffer
            }
        }
    } else if (!strncmp(cmd->args[0], ".help", 5)) {
        print_help_msg();
    }
}

///
/// Handle query type commands
/// Return: command type
///
int handle_query_cmd(Table_t &table, Command_t *cmd) {
    if (!strncmp(cmd->args[0], "insert", 6)) {
        handle_insert_cmd(table, cmd);
        return INSERT_CMD;
    } else if (!strncmp(cmd->args[0], "select", 6)) {
        handle_select_cmd(table, cmd);
        return SELECT_CMD;
    } else if (!strncmp(cmd->args[0], "delete", 6)) {
        handle_delete_cmd(table, cmd);
        return DELETE_CMD;
    } else if (!strncmp(cmd->args[0], "update", 6)) {
        handle_update_cmd(table, cmd);
        return UPDATE_CMD;
    } else {
        return UNRECOG_CMD;
    }
}

void handle_update_cmd(Table_t &table, Command_t *cmd) {
    int arg_idx = 3;
    arg_idx = parse_compare_statment(cmd, 2, arg_idx);
    cmd->condition.cnt_statment--;
    if (arg_idx < (int)cmd->args_len) where_state_handler(cmd, arg_idx+1);

    char *field = cmd->condition.s[2].lhs;
    char *value = cmd->condition.s[2].rhs;

    std::vector<size_t> idxList = select_valid_user(table, cmd);
    
    if (!strncmp(field,"id",2)){
        if ( idxList.size() > 1 ) return;
        int targetId = atoi(value);
        if ( table.primary_keys.count(targetId) ) return;
    }
    
    for (size_t idx:idxList){
        User_t &user = table.users[idx];
        if (!strncmp(field,"id",2)){
            table.primary_keys.erase(user.id);
            user.id = atoi(value);
            table.primary_keys.insert(user.id);
        }
        else if (!strncmp(field,"name",4)) user.name = std::string(value);
        else if (!strncmp(field,"email",5)) user.email = std::string(value);
        else if (!strncmp(field,"age",3)) user.age = atoi(value);
    }
}


void handle_delete_cmd(Table_t &table, Command_t *cmd) {
    table_state_handler(cmd, 2);
    
    std::vector<size_t> idxList = select_valid_user(table, cmd);
    
    size_t len = 0 ;    
    for (size_t i=0, j=0; i<table.size(); i++){
        if (j<idxList.size() && i==idxList[j]){
            table.primary_keys.erase( table.users[i].id );
            j++;
            continue;
        }
        table.users[len] = table.users[i];
        len++;
    }
    table.users.resize(len);
}

///
/// The return value is the number of rows insert into table
/// If the insert operation success, then change the input arg
/// `cmd->type` to INSERT_CMD
///
int handle_insert_cmd(Table_t &table, Command_t *cmd) {
    User_t user = command_to_User(cmd);
    if ( table.add_User(user) ) {
        cmd->type = INSERT_CMD;
        return 1;
    }
    return 0;
}

///
/// The return value is the number of rows select from table
/// If the select operation success, then change the input arg
/// `cmd->type` to SELECT_CMD
///
int handle_select_cmd(Table_t &table, Command_t *cmd) {
    cmd->type = SELECT_CMD;
    field_state_handler(cmd, 1);
    
    std::vector<size_t> idxList = select_valid_user(table, cmd);
    
    int fields_len = cmd->cmd_args.sel_args.fields_len;
    char **fields = cmd->cmd_args.sel_args.fields;
    if (fields_len > 0 && 
            (     !strncmp(fields[0],"sum",3) || 
                !strncmp(fields[0],"avg",3) ||
                !strncmp(fields[0],"count",5) )){
        print_aggregate(table, idxList, cmd);
    } else {
        print_users(table, idxList, cmd);
    }
    return table.size();
}

void print_aggregate(Table_t &table, const std::vector<size_t>& idxList, Command_t *cmd) {
    if ( cmd->cmd_args.sel_args.limit == 0 ) return;
    if ( cmd->cmd_args.sel_args.offset > 1 ) return;

    int cnt = 0;
    int id_sum = 0;
    int age_sum = 0;
    
    for (int idx = 0; idx < (int)idxList.size(); idx++) {
        const User_t &user = table.users[ idxList[idx] ];
        cnt += 1;
        id_sum += user.id;
        age_sum += user.age;
    }
    
    int fields_len = cmd->cmd_args.sel_args.fields_len;
    char **fields = cmd->cmd_args.sel_args.fields;
    printf("(");
    int num_aggregate = 0;
    for (int i = 0; i < fields_len; i++) {    
        if ( !strncmp(fields[i], "count",5) ){
            if (num_aggregate++) printf(",");
            printf("%d", cnt);
        } else if ( !strncmp(fields[i], "sum(age)",8) ){
            if (num_aggregate++) printf(",");
            printf("%d", age_sum);
        } else if ( !strncmp(fields[i], "avg(age)",8) ){
            if (num_aggregate++) printf(",");
            printf("%.3f", (double)age_sum/cnt);
        } else if ( !strncmp(fields[i], "sum(id)",8) ){
            if (num_aggregate++) printf(",");
            printf("%d", id_sum);
        } else if ( !strncmp(fields[i], "avg(id)",8) ){
            if (num_aggregate++) printf(",");
            printf("%.3f", (double)id_sum/cnt);
        }
    }
    printf(")\n");
}


std::vector<size_t> select_valid_user(Table_t &table, Command_t *cmd){
    std::vector<size_t> ret;
    for (size_t i=0; i<table.size(); i++){
        const User_t &user = table.users[i];        
        if ( check_condition(user, cmd) ){
            ret.push_back(i);
        }
    }
    return ret;
}

int check_condition(const User_t &user, Command_t *cmd){
    int cnt_statment = cmd->condition.cnt_statment;
    
    if (cnt_statment == 0){
        return 1;
    } else if (cnt_statment == 1) {
        return check_compare_statment(user, &(cmd->condition.s[0]));
    } else if (cnt_statment == 2) {
        int lhs = check_compare_statment(user, &(cmd->condition.s[0]));
        int rhs = check_compare_statment(user, &(cmd->condition.s[1]));
        if (cmd->condition.logic == AND) return lhs && rhs;
        else if (cmd->condition.logic == OR) return lhs || rhs;
        else return 0;
    }
    return 0;
}

int check_compare_statment(const User_t &user, CompareStatment_t *s){
    if (!strncmp(s->lhs, "\"", 1) || !strncmp(s->lhs, "name", 4) || !strncmp(s->lhs, "email", 5)){
        // string compare
        std::string lhs = get_string_variable(user, s->lhs);
        std::string rhs = get_string_variable(user, s->rhs);
        int is_same = (lhs == rhs);
        if ( !strncmp(s->op,"=",1) ) return is_same;
        else return !is_same;        
    } else {
        // numeric comparision
        int lhs = get_numeric_variable(user, s->lhs);
        int rhs = get_numeric_variable(user, s->rhs);
        if ( !strncmp(s->op,"=",1) ) return lhs == rhs;
        else if ( !strncmp(s->op,"!=",2) ) return lhs != rhs;
        else if ( !strncmp(s->op,">=",2) ) return lhs >= rhs;
        else if ( !strncmp(s->op,"<=",2) ) return lhs <= rhs;
        else if ( !strncmp(s->op,">",1) ) return lhs > rhs;
        else if ( !strncmp(s->op,"<",1) ) return lhs < rhs;
        else return 0;
    }
}

std::string get_string_variable(const User_t &user, char* s){
    if (s[0]=='\"') return s;
    else if (s[0]=='n') return user.name;
    else if (s[0]=='e') return user.email;
    else return NULL;
}

int get_numeric_variable(const User_t &user, char* s){
    if (!strncmp(s,"age",3)) return (int)user.age;
    else if (!strncmp(s,"id",2)) return (int)user.id;
    else {
        return atoi(s);
    }
}


///
/// Show the help messages
///
void print_help_msg() {
    const char msg[] = "# Supported Commands\n"
    "\n"
    "## Built-in Commands\n"
    "\n"
    "  * .exit\n"
    "\tThis cmd archives the tarmble, if the db file is specified, then exit.\n"
    "\n"
    "  * .output\n"
    "\tThis cmd change the output strategy, default is stdout.\n"
    "\n"
    "\tUsage:\n"
    "\t    .output (<file>|stdout)\n\n"
    "\tThe results will be redirected to <file> if specified, otherwise they will display to stdout.\n"
    "\n"
    "  * .load\n"
    "\tThis command loads records stored in <DB file>.\n"
    "\n"
    "\t*** Warning: This command will overwrite the records already stored in current table. ***\n"
    "\n"
    "\tUsage:\n"
    "\t    .load <DB file>\n\n"
    "\n"
    "  * .help\n"
    "\tThis cmd displays the help messages.\n"
    "\n"
    "## Query Commands\n"
    "\n"
    "  * insert\n"
    "\tThis cmd inserts one user record into table.\n"
    "\n"
    "\tUsage:\n"
    "\t    insert <id> <name> <email> <age>\n"
    "\n"
    "\t** Notice: The <name> & <email> are string without any whitespace character, and maximum length of them is 255. **\n"
    "\n"
    "  * select\n"
    "\tThis cmd will display all user records in the table.\n"
    "\n";
    printf("%s", msg);
}

