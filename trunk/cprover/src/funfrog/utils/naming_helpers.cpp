//
// Created by Martin Blicha on 30.10.17.
//

#include "naming_helpers.h"
#include <cassert>
#include <algorithm>

namespace{
    bool is_number(const std::string& s)
    {
        return !s.empty() && std::find_if(s.begin(),
                                          s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
    }

    void unquote(std::string& name){
        assert(is_quoted(name));
        name.erase(name.size() - 1);
        name.erase(0,1);
    }
}

void unquote_if_necessary(std::string& name){
    if(is_quoted(name)){
        unquote(name);
    }
}


std::string quote(const std::string& name){
    return HifrogStringConstants::SMTLIB_QUOTE + name + HifrogStringConstants::SMTLIB_QUOTE;
}

std::string removeCounter(const std::string& name){
    auto pos = name.rfind(HifrogStringConstants::COUNTER_SEP);
    assert(pos != std::string::npos);
    assert(is_number(name.substr(pos + 1)));
    return name.substr(0, pos);
}

bool contains_counter(const std::string& name){
    auto pos = name.rfind(HifrogStringConstants::COUNTER_SEP);
    if(pos != std::string::npos){
        return is_number(name.substr(pos + 1));
    }
    return false;
}

void clean_name(std::string& name){
    unquote_if_necessary(name);
    if(contains_counter(name)){
        name = removeCounter(name);
    }
}

const std::string HifrogStringConstants::GLOBAL_OUT_SUFFIX { "#out" };
const std::string HifrogStringConstants::GLOBAL_INPUT_SUFFIX { "#in" };
const char HifrogStringConstants::SMTLIB_QUOTE = '|';
const char HifrogStringConstants::COUNTER_SEP = '#';