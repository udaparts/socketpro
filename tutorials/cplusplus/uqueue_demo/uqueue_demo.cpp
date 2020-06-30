
#include "stdafx.h"
#include <iostream>
#include "mystruct.h"

int main(int argc, char* argv[]) {
    SPA::CScopeUQueue su;
    CMyStruct msOriginal;
    SetMyStruct(msOriginal);

    su << msOriginal;
    CMyStruct ms;
    su >> ms;

    assert(su->GetSize() == 0);
    bool equal = (ms == msOriginal);
    assert(equal);
    std::cout << "Bytes in buffer: " << su->GetSize() << std::endl;
    std::cout << (equal ? "Equal" : "Not equal") << std::endl;
    std::cout << "Press a key to complete dequeuing messages from server ......" << std::endl;
    ::getchar();
    return 0;
}

