//
// Created by ZZY on 2021/10/14.
//

#ifndef DCMTK_ARGPARSER_H
#define DCMTK_ARGPARSER_H

#include<string>
#include<vector>
using namespace std;


class ArgParser {
public:
    ArgParser(char *argv[], int argc);
    const string& getOptionContent(const string& option);
    bool isOptionExist(const string& option);
private:
    vector<string> tokens;
};


#endif //DCMTK_ARGPARSER_H
