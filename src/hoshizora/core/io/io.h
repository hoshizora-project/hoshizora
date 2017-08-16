#ifndef HOSHIZORA_IO_H
#define HOSHIZORA_IO_H

#include <ios>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include "hoshizora/core/util/includes.h"
#include "hoshizora/core/model/graph.h"

#include <cstdlib>
#include <unistd.h>

namespace hoshizora {
    struct IO {

        static inline bool isSpace(char c) {
            switch (c) {
                case '\r':
                case '\t':
                case '\n':
                case 0:
                case ' ' :
                    return true;
                default :
                    return false;
            }
        }

//        template<class Graph>
        static vector<pair<u32, u32>> fromFile0(string file_name) {
            ifstream ifs(file_name, ios::in);
            istreambuf_iterator<char> it(ifs);
            istreambuf_iterator<char> last;
            string data(it, last);

            auto start = 0ul, len = 0ul;
            auto first = 0ul, second = 0ul;

            vector<pair<u32, u32>> edge_list;
            const auto l = data.length();

            for (u32 i = 0, end = data.length(); i < end; ++i) {
                if (isSpace(data[i]))data[i] = '\0';
            }

            bool isFirst = true;
            for (u32 i = 0, end = data.length(); i < end; ++i) {
                if ((data[i] == '\0') || i == end - 1) {
                    if(i>0&&data[i-1]=='\0'){
                        start++;
                        continue;
                    }

                    if (isFirst) {
                        first = static_cast<u32>(atoi(data.data() + start));
                    } else {
                        second = static_cast<u32>(atoi(data.data() + start));
                        edge_list.emplace_back(make_pair(first, second));

                        if (start % 10000 == 0){
//                            debug::print(to_string(first) + ", " + to_string(second));
                        }
                    }
                    isFirst = !isFirst;
                    start = i + 1;
                }
            }

            return edge_list;
        }


        static vector<pair<u32, u32>> fromFile(string file_name) {
            ifstream ifs(file_name, ios::in);
            istreambuf_iterator<char> it(ifs);
            istreambuf_iterator<char> last;
            string data(it, last);

            auto start = 0ul, len = 0ul;
            auto first = 0ul, second = 0ul;

            vector<pair<u32, u32>> edge_list;
            const auto l = data.length();

            auto cdata = data.c_str();
            char buf[10];

            for (u32 i = 0, end = data.length(); i < end; ++i) {
                if (data[i] == ' ' || data[i] == '\t') {
                    first = static_cast<u32>(atoi(data.substr(start, len).c_str()));

                    start = i + 1;
                } else if (data[i] == '\n' || i == end - 1) {
                    second = static_cast<u32>(atoi(data.substr(start, len).c_str()));

                    edge_list.emplace_back(make_pair(first, second));
                    start = i + 1;

                    if (start % 1000 == 0)debug::print(start / (double) l * 100);
                } else {
                    len++;
                }
            }

            /*
            for (const auto &edge:edge_list) {
                std::cout << edge.first << " -> " << edge.second << std::endl;
            }
             */
            debug::print(edge_list.size());

            return edge_list;

            /*
            auto end = static_cast<u64>(file.tellg());
            file.seekg(0, ios::beg);
            auto n = end - file.tellg();
            auto bytes = heap::array<char>(n + 1);
            file.read(bytes, n);
            file.close();

            for (u64 i = 0; i < n; ++i) {
                if (isSpace(bytes[i]))bytes[i] = 0;
            }
            std::vector<bool> FL(n);
            FL[0] = bytes[0];
            for (u64 i = 1; i < n; ++i) {
                FL[i] = bytes[i] && !bytes[i - 1];
            }
             */

//            return nullptr;
        }

    };
}

#endif //HOSHIZORA_IO_H
