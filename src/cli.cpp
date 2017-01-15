#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace std;

inline string trim(string& str);
int err_handler(int vec_size, int required_size);

int main()
{
    // Command type: R, I, D, S, q, c, d, p
    cout << endl;
    cout << "press R to Create relation" << endl;
    cout << "      I to Insert a record" << endl;
    cout << "      D to Delete a record" << endl;
    cout << "      Scan to Scan index file" << endl;
    cout << "      q to Search single value/ Range " << endl;
    cout << "      c to Data page statistics" << endl;
    cout << "      p to Display specific data page " << endl;
    cout << "      quit to leave the program\n\n" << endl;

    string command, token_comma, token_else;
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
                for(int i=1; i<=buffer_comma.size()-1; ++i)
                    cout << buffer_comma.at(i);
                cout << endl;
            }
        } else if(buffer_comma.at(0)=="I") {
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

            /* !!!!!!Remember to do error handling!!!!!! */
            for (int i=0; i<buffer_else.size(); ++i)
                cout << buffer_else.at(i);
            cout << endl;

        } else if(buffer_comma.at(0)=="D") {
            if(err_handler(buffer_comma.size(), 3))
                cout << buffer_comma.at(1) << buffer_comma.at(2) << endl;
        } else if(buffer_comma.at(0)=="quit") {
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
                if(err_handler(buffer_else.size(), 2))
                    cout << buffer_else.at(1) << endl;
            } else if(buffer_else.at(0)=="q") {
                if(buffer_else.size() < 3) {
                    cout<< "Not enough Input(s). "
                        << "Please enter again." << endl;
                } else if (buffer_else.size() > 4) {
                    cout<< "Too many Inputs. "
                        << "Please enter again." << endl;
                } else {
                    for (int i=1; i<buffer_else.size(); ++i)
                        cout << buffer_else.at(i);
                    cout << endl;
                }
            } else if(buffer_else.at(0)=="c") {
                if(err_handler(buffer_else.size(), 2))
                    cout << buffer_else.at(1) << endl;
            } else if(buffer_else.at(0)=="p") {
                if(err_handler(buffer_else.size(), 3))
                    cout << buffer_else.at(1) << buffer_else.at(2) << endl;
            } else {
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
