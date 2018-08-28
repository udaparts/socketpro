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
		enum tagFileEvent {
			feExchange = 0,
			feTrans,
			feDiscarded
		};

		struct FileCb {
			bool Download;
			tagFileEvent EventType;
			SPA::PUQueue Buffer;
			std::shared_ptr<CNJFunc> Func;
		};

		uv_async_t m_fileType;
		std::deque<FileCb> m_deqFileCb; //Protected by m_csFile;
	};

}