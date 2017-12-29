
#ifndef _U_STREAMING_FILE_SOCKETPRO_H_
#define _U_STREAMING_FILE_SOCKETPRO_H_

#include "../ucomm.h"

namespace SPA {
    namespace SFile {
        static const unsigned int sidFile = (unsigned int) sidReserved + 0x6FFFFFF3; //asynchronous file streaming

        static const unsigned int STREAM_CHUNK_SIZE = 10240;

        static const unsigned short idDownload = 0x7F70;
        static const unsigned short idStartDownloading = 0x7F71;
        static const unsigned short idDownloading = 0x7F72;

        static const unsigned short idUpload = 0x7F73;
        static const unsigned short idUploading = 0x7F74;
        static const unsigned short idUploadCompleted = 0x7F75;

        //error code
        static const int UNKNOWN_ERROR = -1;
        static const int CANNOT_OPEN_LOCAL_FILE_FOR_WRITING = -2;
        static const int CANNOT_OPEN_LOCAL_FILE_FOR_READING = -3;

    } //namespace File
} //namespace SPA
#endif

