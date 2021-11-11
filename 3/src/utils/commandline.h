//
// Created by ZZY on 2021/10/28.
//

#include<map>
#include<string>
using namespace std;

#include "argparser.h"

class CommandLine{
    map<string, string> desc;
    map<string, string> args;

public:
    CommandLine(const map<string, string> cmdDescription ){
        desc = cmdDescription;
    }

    string operator[](string key){
        return args[key];
    }

    bool exists(string key){
        const map<string, string>::iterator it = args.find(key);
        return it != args.end();
    }


    void parse(int argc, char* argv[]){
        ArgParser argParser(argv, argc);
        auto cmdItr = desc.begin();
        while (cmdItr != desc.end()) {
            if (argParser.isOptionExist(cmdItr->first))
                args.insert(make_pair(cmdItr->first, argParser.getOptionContent(cmdItr->first)));
            ++cmdItr;
        }
    }

    void printOutAllArgs(){
        for(auto i = desc.begin(); i!=desc.end(); i++){
            cout<<i->first<<"     "<<i->second<<endl;
        }
    }
};
