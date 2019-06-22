#ifndef TABLE_H
#define TABLE_H
#include <stdlib.h>
#include <stdio.h>
#include "User.h"
#include <vector>
#include <unordered_set>
#include <unordered_map>

#define INIT_TABLE_SIZE 10000
#define EXT_LEN 500

struct Table_t {
    std::vector<User_t> users;
    std::unordered_set<int> primary_keys;
    std::unordered_map<int,int> like;
    std::vector<std::pair<int,int>> like_pairs;
    
    Table_t ();
    size_t size();
    bool add_User(const User_t&);  
};

#endif
