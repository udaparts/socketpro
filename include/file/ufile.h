
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

        //error codes for exchanging file between two machines
    } //namespace File
} //namespace SPA
#endif

