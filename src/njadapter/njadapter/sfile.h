#pragma once

#include "../../../include/streamingfile.h"

namespace NJA {

    class CSFile : public SPA::ClientSide::CStreamingFile {
    public:
        CSFile(SPA::ClientSide::CClientSocket * cs);
        CSFile(const CSFile &sf) = delete;
        ~CSFile();

        typedef CSFile* PSFile;

    public:
        CSFile& operator=(const CSFile &sf) = delete;
        SPA::UINT64 Exchange(Isolate* isolate, int args, Local<Value> *argv, bool download, const wchar_t *localFile, const wchar_t *remoteFile, unsigned int flags);

    private:
        static void file_cb(uv_async_t* handle);
        void SetLoop();

    private:

        enum class tagFileEvent {
            feExchange = 0,
            feTrans,
            feDiscarded,
            feException
        };

        struct FileCb {
            bool Download;
            tagFileEvent EventType;
            SPA::PUQueue Buffer;
            std::shared_ptr<CNJFunc> Func;

            FileCb(bool download, tagFileEvent et) : Download(download), EventType(et), Buffer(SPA::CScopeUQueue::Lock()) {
            }

            FileCb(FileCb&& fcb) noexcept : Download(fcb.Download), EventType(fcb.EventType), Buffer(fcb.Buffer), Func(std::move(fcb.Func)) {
                fcb.Buffer = nullptr;
            }

            FileCb(const FileCb& fcb) = delete;
            FileCb& operator=(const FileCb& fcb) = delete;
            FileCb& operator=(FileCb&& fcb) = delete;
        };

        uv_async_t m_fileType;
        std::deque<FileCb> m_deqFileCb; //Protected by m_csFile;
    };

}