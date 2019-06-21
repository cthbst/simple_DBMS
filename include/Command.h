#ifndef COMMAND_H
#define COMMAND_H
#include <vector>
#include <string>

enum { 
    UNRECOG_CMD,
    BUILT_IN_CMD,
    QUERY_CMD,
};

enum {
    INSERT_CMD = 100,
    SELECT_CMD,
	DELETE_CMD,
	UPDATE_CMD,
};

struct CMD_t{
    char name[256];
    int len;
    unsigned char type;
};

extern CMD_t cmd_list[];

struct SelectArgs_t{
    std::vector< std::string > fields;
    int offset;
    int limit;

    SelectArgs_t();
};

typedef enum {
	NIL,
	AND,
	OR,
} Logic_t;

struct CompareStatment_t{
    std::string lhs;
    std::string op;
    std::string rhs;
};

struct Condition_t{
	CompareStatment_t s[3];
    Logic_t logic;
	int cnt_statment;
};

struct Command_t {
    unsigned char type;
    std::vector<std::string> args;
    SelectArgs_t sel_args;
    Condition_t condition;

    Command_t();
};

void add_Arg(Command_t &cmd, const std::string &argument);
void add_select_field(Command_t &cmd, const std::string &argument);

#endif
