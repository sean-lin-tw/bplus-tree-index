#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <string>
#include <string.h>

extern "C"
{
#include "include/relation.h"
}

using namespace std;

inline string trim(string& str);
int err_handler(int vec_size, int required_size);
void key_coppier(relation_t* cur_relation, index_t* key, string target);

int main()
{
    //-------------------- Initial a database --------------------
    relation_page_t db = {0};
    relation_t* cur_relation;
    index_t insert_key, backup_key;

    // Command type: R, I, D, S, q, c, d, p
    cout << endl;
    cout << "press R to Create relation" << endl;
    cout << "      I to Insert a record" << endl;
    cout << "      D to Delete a record" << endl;
    cout << "      Scan to Scan index file" << endl;
    cout << "      q to Search single value/ Range " << endl;
    cout << "      c to Data page statistics" << endl;
    cout << "      p to Display specific data page " << endl;
    cout << "      quit or exit to leave the program\n\n" << endl;

    string command, token_comma, token_else, str_buffer;
    vector<string> buffer_comma, buffer_else;
    stringstream ss_comma, ss_else;

    while(1)
    {
        getline(cin,command);

        if(cin.eof())
            break;
        else if(command == "")
            continue;

        ss_comma << command;

        /* parse the command by comma*/
        while(std::getline(ss_comma, token_comma, ','))
            buffer_comma.push_back(trim(token_comma));

        if(buffer_comma.at(0)=="R") {
            if(err_handler(buffer_comma.size(), 4)) {
                if(buffer_comma.at(2) == "integer")
                    relation__create(&db, buffer_comma.at(1).c_str(), TYPE_INT, stoi(buffer_comma.at(3)));
                else if(buffer_comma.at(2) == "String")
                    relation__create(&db, buffer_comma.at(1).c_str(), TYPE_STRING, stoi(buffer_comma.at(3)));
            }
        }
        else if(buffer_comma.at(0)=="I") {
            // get relation-name
            buffer_else.push_back(buffer_comma.at(1));

            // get key-value1
            buffer_else.push_back(buffer_comma.at(2));

            // string with ";" to be parsed
            for (int i=3; i<buffer_comma.size(); ++i) {
                ss_else << buffer_comma.at(i);
                while(std::getline(ss_else, token_else, ';'))
                    buffer_else.push_back(trim(token_else));

                // clear the stringstream
                ss_else.str("");
                ss_else.clear();
            }

            cur_relation = get_relation(&db, buffer_else.at(0).c_str());
            /* !!!!!!Remember to do error handling!!!!!! */
            for (int i=1; i<buffer_else.size(); i+=2) {
                key_coppier(cur_relation, &insert_key, buffer_else.at(i));
                if(cur_relation->ktype == TYPE_INT) {
                    relation__insert(cur_relation, insert_key, buffer_else.at(i+1).c_str());
                } else {
                    str_buffer = string("") + trim(buffer_else.at(i)) + string("]") + trim(buffer_else.at(i+1));
                    str_buffer.erase(0, 10);
                    str_buffer = string("[..") + str_buffer;
                    relation__insert(cur_relation, insert_key, str_buffer.c_str());
                }

            }

        }
        else if(buffer_comma.at(0)=="D") {
            if(err_handler(buffer_comma.size(), 3))
                cout << buffer_comma.at(1) << buffer_comma.at(2) << endl;
        }
        else if(buffer_comma.at(0)=="quit" || buffer_comma.at(0)=="exit") {
            break;
        }
        // string seperated with space
        else {
            // string with spaces to be parsed
            ss_else << buffer_comma.at(0);
            while(std::getline(ss_else, token_else, ' '))
                buffer_else.push_back(trim(token_else));

            /* handle different operations */
            if(buffer_else.at(0)=="s" || buffer_else.at(0)=="Scan" ) {
                cur_relation = get_relation(&db, buffer_else.at(1).c_str());
                if(err_handler(buffer_else.size(), 2))
                    relation__index_scan(cur_relation);
            }
            else if(buffer_else.at(0)=="q") {
                cur_relation = get_relation(&db, buffer_else.at(1).c_str());

                if(buffer_else.size() < 3) {
                    cout<< "Not enough Input(s). "
                        << "Please enter again." << endl;
                } else if (buffer_else.size() > 4) {
                    cout<< "Too many Inputs. "
                        << "Please enter again." << endl;
                } else if(buffer_else.size() == 3) {
                    key_coppier(cur_relation, &insert_key, buffer_else.at(2));
                    relation__find(cur_relation, insert_key);
                } else if(buffer_else.size() == 4) {
                    key_coppier(cur_relation, &insert_key, buffer_else.at(2));
                    key_coppier(cur_relation, &backup_key, buffer_else.at(3));
                    relation__find_range(cur_relation, insert_key, backup_key);
                }
            }
            else if(buffer_else.at(0)=="c") {
                cur_relation = get_relation(&db, buffer_else.at(1).c_str());
                if(err_handler(buffer_else.size(), 2))
                    relation__statistic(cur_relation);
            }
            else if(buffer_else.at(0)=="p") {
                cur_relation = get_relation(&db, buffer_else.at(1).c_str());
                if(err_handler(buffer_else.size(), 3))
                    relation__page_display(cur_relation, stoi(buffer_else.at(2)));
            }
            else {
                cout << "Incorrect input type. "
                     << "Please enter a new command." << endl;
            }
        }

        // Clear the buffer vector
        buffer_else.clear();
        buffer_comma.clear();

        // Clear the stringstream
        ss_comma.str("");
        ss_comma.clear();

        ss_else.str("");
        ss_else.clear();
    }

    return 0;
}


inline string trim(string& str)
{
    str.erase(0, str.find_first_not_of(' '));       //prefixing spaces
    str.erase(str.find_last_not_of(' ')+1);         //surfixing spaces
    str.erase(0, str.find_first_not_of('"'));       //prefixing spaces
    str.erase(str.find_last_not_of('"')+1);         //surfixing spaces
    return str;
}

int err_handler(int vec_size, int required_size)
{
    if(vec_size < required_size) {
        cout<< "Not enough Input(s). "
            << "Please enter again" << endl;
        return 0;
    } else if (vec_size > required_size) {
        cout<< "Too many Inputs. "
            << "Please enter again" << endl;
        return 0;
    } else {
        return 1;
    }
}

void key_coppier(relation_t* cur_relation, index_t* key, string target) {
    if(cur_relation->ktype == TYPE_INT)
        (*key).i = stoi(target);
    else
        strncpy((*key).str, trim(target).c_str(), 10);
}
