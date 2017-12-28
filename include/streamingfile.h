
#ifndef _SOCKETPRO_STREAMING_FILE_H_
#define _SOCKETPRO_STREAMING_FILE_H_

#include <fstream>
#include <deque>
#include "file/ufile.h"

namespace SPA {namespace ClientSide {

class CStreamingFile : public CAsyncServiceHandler
{
public:
	typedef std::function<void(CStreamingFile *file, int res, const std::wstring &errMsg) > DDownload;
	typedef DDownload DUpload;
	typedef std::function<void(CStreamingFile *file, UINT64 downloaded) > DTransferring;

	CStreamingFile(CClientSocket * cs = nullptr) : CAsyncServiceHandler(SFile::sidFile, cs) {
	}

private:
	struct CContext {
		CContext(bool uplaod, unsigned int flags) 
		: Uploading(uplaod), FileSize(~0), Flags(flags), Sent(false), m_if(nullptr), m_of(nullptr) {
		}
		bool Uploading;
		UINT64 FileSize;
		unsigned int Flags;
		bool Sent;
		std::string LocalFile;
		std::wstring FilePath;
		DDownload Download;
		DTransferring Transferring;
		DCanceled Aborted;
		std::ifstream *m_if;
		std::ofstream *m_of;
		
	};

public:
	virtual unsigned int CleanCallbacks() {
		{
			CAutoLock al(m_csFile);
			m_vContext.clear();
		}
		return CAsyncServiceHandler::CleanCallbacks();
	}

	UINT64 GetFileSize() {
		UINT64 file_size = (~0);
		CAutoLock al(m_csFile);
		if (m_vContext.size())
			file_size = m_vContext.front().FileSize;
		return file_size;
	}

	bool Upload(const char *localFile, const wchar_t *remoteFile, DUpload up, DTransferring trans, DCanceled aborted = DCanceled(), unsigned int flags = 0) {
		if (!localFile || !::strlen(localFile))
			return false;
		if (!remoteFile || !::wcslen(remoteFile))
			return false;
		CContext context(true, flags);
		context.Download = up;
		context.Transferring = trans;
		context.Aborted = aborted;
		context.FilePath = remoteFile;
		context.LocalFile = localFile;
		m_csFile.lock();
		m_vContext.push_back(context);
		m_csFile.unlock();
		return Initilize();
	}

	bool Download(const char *localFile, const wchar_t *remoteFile, DDownload dl, DTransferring trans, DCanceled aborted = DCanceled(), unsigned int flags = 0) {
		if (!localFile || !::strlen(localFile))
			return false;
		if (!remoteFile || !::wcslen(remoteFile))
			return false;
		CContext context(false, flags);
		context.Download = dl;
		context.Transferring = trans;
		context.Aborted = aborted;
		context.FilePath = remoteFile;
		context.LocalFile = localFile;
		m_csFile.lock();
		m_vContext.push_back(context);
		m_csFile.unlock();
		return Initilize();
	}

protected:
    virtual void OnResultReturned(unsigned short reqId, CUQueue &mc) {
		switch(reqId) {
		case SFile::idDownload:
			{
				int res;
				std::wstring errMsg;
				mc >> res >> errMsg;
				DDownload dl;
				{
					CAutoLock al(m_csFile);
					if (m_vContext.size()) {
						CContext &context = m_vContext.front();
						assert(!context.Uploading);
						if (context.m_of) {
							if (context.m_of->is_open())
								context.m_of->close();
							else if (!res) {
								res = -2;
								errMsg = L"Cannot open a local file for writing data";
							}
							delete context.m_of;
						}
						else {
							assert(res != 0);
						}
						dl = context.Download;
						m_vContext.pop_front();
					}
					else {
						assert(false);
					}
				}
				if (dl)
					dl(this, res, errMsg);
			}
			Initilize();
			break;
		case SFile::idStartDownloading: 
			{
				CAutoLock al(m_csFile);
				if (m_vContext.size()) {
					CContext &context = m_vContext.front();
					assert(!context.Uploading);
					assert(!context.m_of);
					mc >> context.FileSize;
					context.m_of = new std::ofstream;
					context.m_of->open(context.LocalFile, std::ios::out | std::ios::binary | std::ios::app | std::ios::trunc);
				}
				else {
					mc.SetSize(0);
					assert(false);
				}
			}
			break;
		case SFile::idDownloading:
			{
				DTransferring trans;
				UINT64 downloaded = (~0);
				{
					CAutoLock al(m_csFile);
					if (m_vContext.size()) {
						CContext &context = m_vContext.front();
						trans = context.Transferring;
						if (context.m_of->is_open()) {
							context.m_of->write((const char*)mc.GetBuffer(), mc.GetSize());
							downloaded = context.m_of->tellp();
						}
					}
					else {
						assert(false);
					}	
				}
				if (trans)
					trans(this, downloaded);
				mc.SetSize(0);
			}
			break;
		case SFile::idUpload:
			{
				
			}
			Initilize();
			break;
		default:
			CAsyncServiceHandler::OnResultReturned(reqId, mc);
		}
	}
	
private:
	bool Initilize() {
		size_t index = 0;
		ResultHandler rh;
		DServerException se;
		CAutoLock al(m_csFile);
		CClientSocket *cs = GetAttachedClientSocket();
		if (!cs->Sendable())
			return false;
		unsigned int recv = cs->GetBytesInReceivingBuffer();
		while (recv < 2 * SFile::STREAM_CHUNK_SIZE && index < m_vContext.size()) {
			CContext &context = m_vContext[index];
			if (context.Sent) {
				++index;
				continue;
			}
			if (context.Uploading && context.m_if && !context.m_if->is_open()) {
				if (index == 0) {
					delete context.m_if;
					if (context.Download) {
						context.Download(this, -2, L"Cannot open a local file for reading data");
					}
					m_vContext.erase(m_vContext.begin() + index);
					if (!cs->Sendable())
						return false;
					}
				else {
					++index;
				}
				continue;
			}
			if (context.Uploading) {
				context.m_if = new std::ifstream;
				context.m_if->open(context.LocalFile, std::ios::binary);
				if (!context.m_if->is_open()) {
					if (index == 0) {
						delete context.m_if;
						if (context.Download) {
							context.Download(this, -2, L"Cannot open a local file for reading data");
						}
						m_vContext.erase(m_vContext.begin() + index);
					}
					else {
						++index;
					}
					continue;
				}
				else {
					context.m_if->seekg(0, std::ios_base::end);
#ifdef WIN32_64
					context.FileSize = context.m_if->tellg().seekpos();
#else
					context.FileSize = context.m_if->tellg();
#endif
					if (!SendRequest(SFile::idUpload, context.FilePath.c_str(), context.Flags, context.FileSize, rh, context.Aborted, se)) {
						return false;
					}
					context.Sent = true;
					context.m_if->seekg(0, std::ios_base::beg);
					CScopeUQueue sb(MY_OPERATION_SYSTEM, IsBigEndian(), SFile::STREAM_CHUNK_SIZE);
					context.m_if->read((char*) sb->GetBuffer(), SFile::STREAM_CHUNK_SIZE);
					unsigned int ret = (unsigned int) context.m_if->gcount();
					while (ret > 0) {
						if (!SendRequest(SFile::idUploading, sb->GetBuffer(), ret, rh, context.Aborted, se)) {
							return false;
						}
						if (ret < SFile::STREAM_CHUNK_SIZE)
							break;
						recv = cs->GetBytesInReceivingBuffer();
						if (recv >= 2 * SFile::STREAM_CHUNK_SIZE)
							break;
						context.m_if->read((char*) sb->GetBuffer(), SFile::STREAM_CHUNK_SIZE);
						ret = (unsigned int) context.m_if->gcount();
					}
				}
			}
			else {
				if (!SendRequest(SFile::idDownload, context.FilePath.c_str(), context.Flags, rh, context.Aborted, se)) {
					return false;
				}
				context.Sent = true;
			}
			++index;
			recv = cs->GetBytesInReceivingBuffer();
		}
		return true;
	}

private:
	CUCriticalSection m_csFile;
	std::deque<CContext> m_vContext; //protected by m_csFile;
	CUCriticalSection m_csFileOne;
};

}; //ClientSide
}; //SPA

#endif