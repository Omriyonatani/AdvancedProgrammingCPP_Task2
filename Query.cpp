#include "Query.h"
#include "TextQuery.h"
#include <memory>
#include <set>
#include <algorithm>
#include <iostream>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <regex>
using namespace std;


shared_ptr<QueryBase> QueryBase::factory(const string& s){
                             // breaking input into word using string stream 
    stringstream words(s);  // Used for breaking words 
    string word;             // to store individual words 
    int count = 0;           // to count number of words
    string s1;               // s1,s2 to store the words we want to find..
    string s2;
    vector<string> myWords;  // vector of strings that hold the words
    while (words >> word){   // insert and count word by word
        myWords.push_back(word);
        count++;
    }
    if(count!=1 && count !=3){
        throw std::invalid_argument("Unrecognized search\n");
    }else if(count==1){
        s1=myWords.at(0);
        return std::shared_ptr<QueryBase>(new WordQuery(s1));
    }else if(count==3){
        if(myWords.at(0)=="AND"){
            s1=myWords.at(1);
            s2=myWords.at(2);
            return std::shared_ptr<QueryBase>(new AndQuery(s1,s2));
        }else if(myWords.at(0)=="OR"){
            s1=myWords.at(1);
            s2=myWords.at(2);
            return std::shared_ptr<QueryBase>(new OrQuery(s1,s2));
        }else if(myWords.at(0)=="AD"){
            s1=myWords.at(1);
            s2=myWords.at(2);
            return std::shared_ptr<QueryBase>(new AdjacentQuery(s1,s2));
        }else{
            throw std::invalid_argument("Unrecognized search\n");
        }
    }else{
        throw std::invalid_argument("Unrecognized search");
    }
}

QueryResult AndQuery::eval (const TextQuery& text) const{
    QueryResult left_result = text.query(left_query);
    QueryResult right_result = text.query(right_query);
    auto ret_lines = make_shared<set<line_no>>();
    set_intersection(left_result.begin(), left_result.end(),
        right_result.begin(), right_result.end(), 
        inserter(*ret_lines, ret_lines->begin()));
    return QueryResult(rep(), ret_lines, left_result.get_file());
}


QueryResult OrQuery::eval(const TextQuery &text) const{
    QueryResult left_result = text.query(left_query);
    QueryResult right_result = text.query(right_query);
    auto ret_lines = make_shared<set<line_no>>(left_result.begin(), left_result.end());
    ret_lines->insert(right_result.begin(), right_result.end());
    return QueryResult(rep(), ret_lines, left_result.get_file());
}


QueryResult AdjacentQuery::eval (const TextQuery& text) const{
    QueryResult left_result = text.query(left_query);
    QueryResult right_result = text.query(right_query);
    auto ret_lines = make_shared<set<line_no>>();
    // go all over the words sets and check if there 2 same lines +\-1 (Adjacent).
    for(auto i=left_result.begin(); i!=left_result.end(); i++){
        for(auto j=right_result.begin(); j!=right_result.end(); j++){
            if(*i == *j+1){
                ret_lines->emplace(*j);
            }else if(*i == *j-1){
                ret_lines->emplace(*i);
            }
        }
    }
    return QueryResult(rep(), ret_lines, left_result.get_file());
}



std::ostream &print(std::ostream &os, const QueryResult &qr){
    if(qr.sought.substr(0,2) == "AD"){
        os << "\"" << qr.sought << "\"" << " occurs " << qr.lines->size() << " times:" <<std::endl;
        int count=qr.lines->size();
        for(auto num  : *qr.lines){
            os << "\t(line " << num + 1 << ") " << *(qr.file->begin() + num) << std::endl;
            os << "\t(line " << num + 2 << ") " << *(qr.file->begin() + num+1) << std::endl;
            if(count>1){
                count--;
                cout<<std::endl;
            }
        }
    }else{
        os << "\"" << qr.sought << "\"" << " occurs " << qr.lines->size() << " times:" <<std::endl;
        for (auto num : *qr.lines){
            os << "\t(line " << num + 1 << ") " << *(qr.file->begin() + num) << std::endl;
        }
    }
    return os;
}