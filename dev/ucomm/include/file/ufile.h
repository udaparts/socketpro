
#ifndef _U_STREAMING_FILE_SOCKETPRO_H_
#define _U_STREAMING_FILE_SOCKETPRO_H_

#include "../ucomm.h"

namespace SPA {
    namespace SFile {
        static const unsigned int sidFile = (unsigned int) SPA::tagServiceID::sidFile; //asynchronous file streaming

        static const unsigned int STREAM_CHUNK_SIZE = 10240;

        //request ids
        static const unsigned short idDownload = 0x7F70;
        static const unsigned short idStartDownloading = 0x7F71;
        static const unsigned short idDownloading = 0x7F72;
        static const unsigned short idUpload = 0x7F73;
        static const unsigned short idUploading = 0x7F74;
        static const unsigned short idUploadCompleted = 0x7F75;
        static const unsigned short idUploadBackup = 0x7F76;

        //file open flags
        static const unsigned int FILE_OPEN_TRUNCACTED = 1;
        static const unsigned int FILE_OPEN_APPENDED = 2;
        static const unsigned int FILE_OPEN_SHARE_READ = 4;
        static const unsigned int FILE_OPEN_SHARE_WRITE = 8;

        //error code
        static const int CANNOT_OPEN_LOCAL_FILE_FOR_WRITING = -1;
        static const int CANNOT_OPEN_LOCAL_FILE_FOR_READING = -2;
        static const int FILE_BAD_OPERATION = -3;
        static const int FILE_DOWNLOADING_INTERRUPTED = -4;

    } //namespace File
} //namespace SPA
#endif

