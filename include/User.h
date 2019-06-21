#ifndef USER_H
#define USER_H
#include "Command.h"
#include <string>

struct User_t {
    unsigned int id;
    std::string name;
    std::string email;
    unsigned int age;
    
    User_t();
};

User_t command_to_User(const Command_t &cmd);

#endif
