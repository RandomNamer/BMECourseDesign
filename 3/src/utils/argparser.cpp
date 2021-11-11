//
// Created by ZZY on 2021/10/14.
//

#include "argparser.h"

ArgParser::ArgParser(char **argv, int argc) {
    for(int i = 0; i < argc; i++){
        this->tokens.push_back(string(argv[i]));
    }
}

const string &ArgParser::getOptionContent(const string &option) {
    vector<string>::const_iterator it = find(this->tokens.begin(), this->tokens.end(), option);
    if(it != this->tokens.end() && ++it != this->tokens.end()) return *it;
    return "";
}


bool ArgParser::isOptionExist(const string &option) {
    return find(this->tokens.begin(), this->tokens.end(), option) != this->tokens.end();
}






















