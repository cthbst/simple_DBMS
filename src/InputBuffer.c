#include <stdio.h>
#include <stdlib.h>
#include "InputBuffer.h"
#include <iostream>
#include <sstream>
#include <algorithm>

Inputs_t read_inputs(){
    Inputs_t ret;
    std::string s;
    while ( std::getline(std::cin, s) ){
        ret.push_back( to_InputBuffer(s) );
    }
    return ret;
}

InputBuffer_t to_InputBuffer(std::string &str){
    std::replace(str.begin(), str.end(), ',', ' ');
    InputBuffer_t ret;
    std::stringstream ss(str);
    std::string s;
    while (ss >> s){
        ret.push_back(s);
    }
    return ret;
}
