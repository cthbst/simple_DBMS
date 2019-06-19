#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "Table.h"

Table_t::Table_t(){
    users.reserve(INIT_TABLE_SIZE);
}

size_t Table_t::size(){
    return users.size();
}

bool Table_t::add_User(const User_t &user){
    if (primary_keys.count(user.id)){
        return 0;
    }
    users.push_back(user);
    primary_keys.insert(user.id);
    return 1;
}
