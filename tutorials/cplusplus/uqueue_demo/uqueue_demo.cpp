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
    std::cout << "Bytes in buffer: " << su->GetSize() << "\n";
    std::cout << (equal ? "Equal" : "Not equal") << "\n";
    std::cout << "Press a key to kill the demo ......\n";
    ::getchar();
    return 0;
}
