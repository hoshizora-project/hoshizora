#ifndef HOSHIZORA_IO_H
#define HOSHIZORA_IO_H

#include <ios>
#include <fstream>
#include "hoshizora/core/util/includes.h"
#include "hoshizora/core/model/graph.h"

namespace hoshizora {
    struct IO {
        template<class Graph>
        static Graph fromFile(string file_name) {
            ifstream file(file_name, ios::in | ios::binary | ios::ate);
            i64 end = file.tellg();
            file.seekg(0, ios::beg);
            i64 n = end - file.tellg();
            auto bytes = heap::array<char>(n + 1);
            file.read(bytes, n);
            file.close();



            return nullptr;
        }
    };
}


#endif //HOSHIZORA_IO_H
