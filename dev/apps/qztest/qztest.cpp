// qztest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../../pinc/uzip.h"
#include "../../include/membuffer.h"
#include <fstream>

int main(int argc, char* argv[]) {
    const unsigned int BUFFER_SIZE = 64 * 1024 * 1024;
    unsigned int len;
    std::wstring wsOut;
    typedef SPA::CScopeUQueueEx <BUFFER_SIZE, BUFFER_SIZE / 2 > CQuikZipBuffer;
    const wchar_t *input = L"This program and the accompanying materials are licensed and made available under the terms and conditions of the BSD License which accompanies this distribution. The full text of the license may be found at http://opensource.org/licenses/bsd-license.php If otherwise QLZ_STREAMING_BUFFER = 0 (streaming mode disabled).The filename prefix '32' or '64' tells if it's compiled for 32-bit x86 Windows or 64-bit x64 Windows. The 32-bit DLLs are using the stdcall calling convention. The 64-bit DLLs are using the new x64 convention which is the only convention existing.Usage is so simple it hardly needs description. Just call the functions byte[] compress(byte[] src) and byte[] decompress(byte[] src).This is the original C version. It has been extensively tested and bounds checked on on many 32- and 64-bit architectures such as x86, x64, UltraSPARC, MIPS, Itanium, PA-RISC, Alpha, Cell, POWER, 68k, ARM and SH4/5.";
    {
        CQuikZipBuffer qz, qzOut;
        qz << input;
        len = qzOut->GetMaxSize();
        bool b = SPA::Compress(SPA::zlBestSpeed, (void*) qz->GetBuffer(), qz->GetSize(),
                (void*) qzOut->GetBuffer(), len);
        qz->SetSize(0);
        qz->CleanTrack();
        qzOut->SetSize(len);
        len = qz->GetMaxSize();

        b = SPA::Decompress(SPA::zlBestSpeed, (void*) qzOut->GetBuffer(),
                qzOut->GetSize(), (void*) qz->GetBuffer(), len);
        qz->SetSize(len);
        qz >> wsOut;
        assert(wsOut == input);
    }

    {
        CQuikZipBuffer qz, qzOut;
        qz << input;
        len = qzOut->GetMaxSize();
        bool b = SPA::Compress(SPA::zlDefault, (void*) qz->GetBuffer(), qz->GetSize(),
                (void*) qzOut->GetBuffer(), len);
        qz->SetSize(0);
        qz->CleanTrack();
        qzOut->SetSize(len);
        len = qz->GetMaxSize();

        b = SPA::Decompress(SPA::zlDefault, (void*) qzOut->GetBuffer(),
                qzOut->GetSize(), (void*) qz->GetBuffer(), len);
        qz->SetSize(len);
        qz >> wsOut;
        assert(wsOut == input);
    }

    {
        len = BUFFER_SIZE;
        CQuikZipBuffer qz, qzOut;
        qz->CleanTrack();
        bool b = SPA::Compress(SPA::zlDefault, (void*) qz->GetBuffer(), BUFFER_SIZE * 14 / 16,
                (void*) qzOut->GetBuffer(), len);
        unsigned int ratio = BUFFER_SIZE * 14 / (16 * len) + 1;

        ratio = 0;
    }
    return 0;
}

