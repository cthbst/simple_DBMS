#include <string.h>
#include <stdlib.h>
#include <iostream>
#include "User.h"
#include "Command.h"

User_t::User_t(){
}

User_t command_to_User(const Command_t &cmd) {
    User_t user;
    user.id = std::stoi(cmd.args[3]);
    user.name = std::string(cmd.args[4]);
    user.email = std::string(cmd.args[5]);
    user.age = std::stoi(cmd.args[6]);
    return user;
}

