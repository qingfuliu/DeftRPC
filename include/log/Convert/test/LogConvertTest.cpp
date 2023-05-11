#include<iostream>
#include"LogConvert.h"
#include <vector>
#include <sstream>
#include<map>
#include<unordered_map>
int main() {
    std::ostringstream oss;
    CLSN::LogConvert<std::vector<int>>::convert(oss, {1, 2, 34});
    std::cout << oss.str() << std::endl;
    oss.clear();
    oss.str("");


    CLSN::LogConvert<std::list<int>>::convert(oss, {1, 2, 34});
    std::cout << oss.str() << std::endl;
    oss.clear();
    oss.str("");

    CLSN::LogConvert<std::deque<int>>::convert(oss, {1, 2, 34});
    std::cout << oss.str() << std::endl;
    oss.clear();
    oss.str("");

    CLSN::LogConvert<std::set<int>>::convert(oss, {1, 2, 34});
    std::cout << oss.str() << std::endl;
    oss.clear();
    oss.str("");

    CLSN::LogConvert<std::unordered_set<int>>::convert(oss, {1, 2, 34});
    std::cout << oss.str() << std::endl;
    oss.clear();
    oss.str("");


    CLSN::LogConvert<int>::convert(oss, 1234);
    std::cout << oss.str() << std::endl;
    oss.clear();
    oss.str("");

    CLSN::LogConvert<std::string>::convert(oss, "1234");
    std::cout << oss.str() << std::endl;
    oss.clear();
    oss.str("");

    std::unordered_map<int,std::string> unordered_mapTest;
    unordered_mapTest[1]="val1";
    unordered_mapTest[2]="val2";
    CLSN::LogConvert< std::unordered_map<int,std::string> >::convert(oss, unordered_mapTest);
    std::cout << oss.str() << std::endl;
    oss.clear();
    oss.str("");


    std::map<std::string,int> mapTest;
    mapTest["val1"]=1;
    mapTest["val2"]=2;
    CLSN::LogConvert< std::map<std::string,int> >::convert(oss, mapTest);
    std::cout << oss.str() << std::endl;
    oss.clear();
    oss.str("");
}