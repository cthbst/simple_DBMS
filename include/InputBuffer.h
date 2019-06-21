#ifndef INPUT_BUF_H
#define INPUT_BUF_H
#include <vector>
#include <string>

using InputBuffer_t = std::vector<std::string>;
using Inputs_t = std::vector<InputBuffer_t>;

Inputs_t read_inputs();
InputBuffer_t to_InputBuffer(std::string &str);

#endif
