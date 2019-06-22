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
    std::string name;
    int len;
    unsigned char type;
};

extern CMD_t cmd_list[];

struct SelectArgs_t{
    std::string table_name;
    std::vector< std::string > fields;
    std::string join;
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

    Condition_t();
};

struct Command_t {
    unsigned char type;
    std::vector<std::string> args;
    SelectArgs_t sel_args;
    Condition_t condition;

    Command_t();
    Command_t(const std::vector<std::string>&);
};

void add_select_field(Command_t &cmd, const std::string &argument);

#endif
