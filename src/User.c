#include <string.h>
#include <stdlib.h>
#include "User.h"
#include "Command.h"

User_t::User_t(){
}

User_t command_to_User(Command_t *cmd) {
    User_t user;
    user.id = atoi(cmd->args[1]);
    user.name = std::string(cmd->args[2]);
    user.email = std::string(cmd->args[3]);
    user.age = atoi(cmd->args[4]);
    return user;
}

