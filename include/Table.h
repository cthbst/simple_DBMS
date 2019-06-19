#ifndef TABLE_H
#define TABLE_H
#include <stdlib.h>
#include <stdio.h>
#include "User.h"
#include <vector>
#include <unordered_set>

#define INIT_TABLE_SIZE 10000
#define EXT_LEN 500

struct Table_t {
    std::vector<User_t> users;
    std::unordered_set<int> primary_keys;
    
    Table_t ();
    size_t size();
    bool add_User(const User_t&);  
};

#endif
