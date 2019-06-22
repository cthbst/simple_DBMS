#include <iostream>
#include <string>
#include <iomanip>
#include "Util.h"
#include "Command.h"
#include "Table.h"
#include "SelectState.h"

///
/// Print the user in the specific format
///
void print_user(const User_t &user, SelectArgs_t &sel_args) {
    std::cout << "(";
    int cnt = 0;
    for (std::string &s : sel_args.fields){
        if (s == "*"){
            std::cout << user.id << ", " << user.name << ", " << user.email << ", " << user.age;
        } else {
            if (cnt++ != 0) std::cout << ", ";
            if (s == "id") std::cout << user.id;
            else if (s == "name") std::cout << user.name;
            else if (s == "email") std::cout << user.email;
            else if (s == "age") std::cout << user.age;
        }
    }
    std::cout << ")\n";
}

///
/// Print the users for given offset and limit restriction
///
void print_users(Table_t &table, const std::vector<size_t>& idxList, Command_t &cmd) {
    int limit = cmd.sel_args.limit;
    int offset = cmd.sel_args.offset;

    if (offset == -1) {
        offset = 0;
    }

    for (int idx = offset; idx < (int)idxList.size(); idx++) {
        if (limit != -1 && (idx - offset) >= limit) {
            break;
        }
        print_user(table.users[ idxList[idx] ], cmd.sel_args);
    }
}

void handle_builtin_cmd(Table_t &table, Command_t &cmd) {
    if (cmd.args[0] == ".exit") {
        exit(0);
    } else if (cmd.args[0] == ".help") {
        print_help_msg();
    } else if (cmd.args[0] == ".output") {
        if (!freopen(cmd.args[1].c_str(), "w", stdout)) exit(0);
    }
}

int handle_query_cmd(Table_t &table, Command_t &cmd) {
    if (cmd.args[0] == "insert") {
        handle_insert_cmd(table, cmd);
        return INSERT_CMD;
    } else if (cmd.args[0] == "select") {
        handle_select_cmd(table, cmd);
        return SELECT_CMD;
    } else if (cmd.args[0] == "delete") {
        handle_delete_cmd(table, cmd);
        return DELETE_CMD;
    } else if (cmd.args[0] == "update") {
        handle_update_cmd(table, cmd);
        return UPDATE_CMD;
    } else {
        return UNRECOG_CMD;
    }
}

void handle_update_cmd(Table_t &table, Command_t &cmd) {
    Iter it = cmd.args.begin() + 3;
    cmd.condition.cnt_statment++;
    parse_compare_statment(cmd.condition.s[2], it);
    cmd.condition.cnt_statment--;
    if (it != cmd.args.end()) where_state_handler(cmd, ++it);

    const std::string &field = cmd.condition.s[2].lhs;
    const std::string &value = cmd.condition.s[2].rhs;
    
    std::vector<size_t> idxList = select_valid_user(table, cmd);
    
    if (field == "id"){
        if ( idxList.size() > 1 ) return;
        int targetId = std::stoi(value);
        if ( table.primary_keys.count(targetId) ) return;
    }
    
    for (size_t idx:idxList){
        User_t &user = table.users[idx];
        if (field == "id"){
            table.primary_keys.erase(user.id);
            user.id = std::stoi(value);
            table.primary_keys.insert(user.id);
        }
        else if (field == "name") user.name = std::string(value);
        else if (field == "email") user.email = std::string(value);
        else if (field == "age") user.age = std::stoi(value);
    }
}


void handle_delete_cmd(Table_t &table, Command_t &cmd) {
    Iter it = cmd.args.begin() + 2;
    table_state_handler(cmd, it);
    
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
/// `cmd.type` to INSERT_CMD
///
int handle_insert_cmd(Table_t &table, Command_t &cmd) {
    if (cmd.args[2] == "user"){
        User_t user = command_to_User(cmd);
        if ( table.add_User(user) ) {
            cmd.type = INSERT_CMD;
            return 1;
        }
        return 0;
    } else {
        int u = stoi( cmd.args[3] );
        int v = stoi( cmd.args[4] );
        table.like[u] = v;
        table.like_pairs.push_back({u,v});
        return 1;
    }
}

///
/// The return value is the number of rows select from table
/// If the select operation success, then change the input arg
/// `cmd.type` to SELECT_CMD
///
int handle_select_cmd(Table_t &table, Command_t &cmd) {
    cmd.type = SELECT_CMD;
    Iter it = cmd.args.begin() + 1;
    field_state_handler(cmd, it);
    auto &fields = cmd.sel_args.fields;
    
    if (cmd.sel_args.join != ""){
        std::vector<size_t> idxList = select_valid_user(table, cmd);
        print_join(table, idxList, cmd);
    } else if ( cmd.sel_args.table_name == "user" ) {
        std::vector<size_t> idxList = select_valid_user(table, cmd);
    
        if (fields.size() && 
            (     fields[0].substr(0,3) == "sum" || 
                    fields[0].substr(0,3) == "avg" ||
                    fields[0].substr(0,5) == "count" )){
            print_aggregate(table, idxList, cmd);
        } else {
            print_users(table, idxList, cmd);
        }
    } else { // == "like"
        if (fields.size() && 
            (     fields[0].substr(0,3) == "sum" || 
                    fields[0].substr(0,3) == "avg" ||
                    fields[0].substr(0,5) == "count" )){
            print_likes_aggregate(table, cmd);
        } else {
            print_likes(table, cmd);
        }
    }
    return table.size();
}

void print_join(Table_t &table, const std::vector<size_t>& idxList, Command_t &cmd) {
    if (idxList.size() == 0) return;
    if ( cmd.sel_args.limit == 0 ) return;
    if ( cmd.sel_args.offset > 0 ) return;

    std::unordered_map<int,int> cnt_like[2];
    for (auto &x : table.like_pairs) {
        cnt_like[0][ x.first ]++;
        cnt_like[1][ x.second ]++; 
    }
    
    int idx = (cmd.sel_args.join == "id1"?0:1);
    int ans = 0;
    for (auto i : idxList) {
        ans += cnt_like[idx][ table.users[i].id ];
    }
    std::cout << "(" << ans << ")" << std::endl;
}

void print_likes(Table_t &table, Command_t &cmd) {
    int offset = cmd.sel_args.offset;
    int limit = cmd.sel_args.limit;
    if (offset == -1) offset = 0;
    for (int i = offset; i<(int)table.like_pairs.size(); i++) {
        if (limit != -1 && (i - offset) >= limit) {
            break;
        }
     
        print_like(table.like_pairs[i], cmd.sel_args);
    }
}

void print_like(const std::pair<int,int> &like, SelectArgs_t &args){
    int cnt = 0;
    std::cout << "(";
    for (auto &field : args.fields){
        if (field == "*") {
            std::cout << like.first << ", " << like.second;
        } else {
            if (cnt) std::cout << ", ";
            if (field == "id1") std::cout << like.first;
            else if (field == "id2") std::cout << like.second;
            cnt++;
        }
    }
    std::cout << ")\n";
}

void print_likes_aggregate(Table_t &table, Command_t &cmd){
    if ( cmd.sel_args.limit == 0 ) return;
    if ( cmd.sel_args.offset > 0 ) return;

    long long sum_id1 = 0;
    long long sum_id2 = 0;
    int sz = table.like_pairs.size();
    for (auto &x : table.like_pairs) {
        sum_id1 += x.first;
        sum_id2 += x.second;
    }
    
    int cnt = 0;
    std::cout << "(";
    for (auto &field : cmd.sel_args.fields){
        if (cnt++ > 0) std::cout << ", ";
        
        if (field.substr(0,5) == "count") {
            std::cout << sz;
        } else if (field == "sum(id1)") {
            std::cout << sum_id1;
        } else if (field == "sum(id2)") {
            std::cout << sum_id2;
        } else if (field == "avg(id1)") {
            std::cout << std::fixed << std::setprecision(3);
            std::cout << (double)sum_id1 / sz;
        } else if (field == "avg(id2)") {
            std::cout << std::fixed << std::setprecision(3);
            std::cout << (double)sum_id2 / sz;
        }
    }
    std::cout << ")\n";
}

void print_aggregate(Table_t &table, const std::vector<size_t>& idxList, Command_t &cmd) {
    if ( cmd.sel_args.limit == 0 ) return;
    if ( cmd.sel_args.offset > 0 ) return;

    int cnt = 0;
    int id_sum = 0;
    int age_sum = 0;
    
    for (int idx = 0; idx < (int)idxList.size(); idx++) {
        const User_t &user = table.users[ idxList[idx] ];
        cnt += 1;
        id_sum += user.id;
        age_sum += user.age;
    }
    
    auto &fields = cmd.sel_args.fields;
    std::cout << "(";
    int num_aggregate = 0;
    for (size_t i = 0; i < fields.size(); i++) {    
        if ( fields[i].substr(0,5) == "count" ){
            if (num_aggregate++) std::cout << ", ";
            std::cout << cnt;
        } else if ( fields[i] == "sum(age)" ){
            if (num_aggregate++) std::cout << ", ";
            std::cout << age_sum;
        } else if ( fields[i] == "avg(age)" ){
            if (num_aggregate++) std::cout << ", ";
            std::cout << std::fixed << std::setprecision(3);
            std::cout << (double)age_sum/cnt;
        } else if ( fields[i] == "sum(id)" ){
            if (num_aggregate++) std::cout << ", ";
            std::cout << id_sum;
        } else if ( fields[i] == "avg(id)" ){
            if (num_aggregate++) std::cout << ", ";
            std::cout << std::fixed << std::setprecision(3);
            std::cout << (double)id_sum/cnt;
        }
    }
    std::cout << ")\n";
}


std::vector<size_t> select_valid_user(Table_t &table, Command_t &cmd){
    std::vector<size_t> ret;
    for (size_t i=0; i<table.size(); i++){
        const User_t &user = table.users[i];        
        if ( check_condition(user, cmd) ){
            ret.push_back(i);
        }
    }
    return ret;
}

bool check_condition(const User_t &user, Command_t &cmd){
    int cnt_statment = cmd.condition.cnt_statment;

    if (cnt_statment == 0){
        return 1;
    } else if (cnt_statment == 1) {
        return check_compare_statment(user, cmd.condition.s[0]);
    } else if (cnt_statment == 2) {
        int lhs = check_compare_statment(user, cmd.condition.s[0]);
        int rhs = check_compare_statment(user, cmd.condition.s[1]);
        if (cmd.condition.logic == AND) return lhs && rhs;
        else if (cmd.condition.logic == OR) return lhs || rhs;
        else return 0;
    }
    return 0;
}

bool check_compare_statment(const User_t &user,const CompareStatment_t &s){
    if (s.lhs[0] == '\"' || s.lhs == "name" || s.lhs == "email"){
        // string compare
        std::string lhs = get_string_variable(user, s.lhs);
        std::string rhs = get_string_variable(user, s.rhs);
        int is_same = (lhs == rhs);
        if ( s.op[0] == '=' ) return is_same;
        else return !is_same;        
    } else {
        // numeric comparision
        int lhs = get_numeric_variable(user, s.lhs);
        int rhs = get_numeric_variable(user, s.rhs);
        if ( s.op == "=" ) return lhs == rhs;
        else if ( s.op == "!=" ) return lhs != rhs;
        else if ( s.op == ">=" ) return lhs >= rhs;
        else if ( s.op == "<=" ) return lhs <= rhs;
        else if ( s.op == ">" ) return lhs > rhs;
        else if ( s.op == "<" ) return lhs < rhs;
        else return 0;
    }
}

std::string get_string_variable(const User_t &user, const std::string &s){
    if (s[0]=='\"') return s;
    else if (s[0]=='n') return user.name;
    else if (s[0]=='e') return user.email;
    else return "";
}

int get_numeric_variable(const User_t &user, const std::string &s){
    if (s[0] == 'a') return user.age;
    if (s[0] == 'i') return user.id;
    return std::stoi(s);
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

