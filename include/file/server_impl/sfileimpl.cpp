
#include "sfileimpl.h"
extern std::wstring g_pathRoot;

namespace SPA { namespace ServerSide {

CSFileImpl::CSFileImpl() : m_FileSize(0)
{

}

void CSFileImpl::OnReleaseSource(bool bClosing, unsigned int info) {
	if (m_of.is_open()) {
		m_of.close();
	}
}

void CSFileImpl::OnFastRequestArrive(unsigned short reqId, unsigned int len) {
	BEGIN_SWITCH(reqId)
		M_I3_R2(idUpload, Upload, std::wstring, unsigned int, UINT64, int, std::wstring)
		M_I0_R0(idUploading, Uploading)
    END_SWITCH
}

int CSFileImpl::OnSlowRequestArrive(unsigned short reqId, unsigned int len) {
	BEGIN_SWITCH(reqId)
		M_I2_R2(idDownload, Download, std::wstring, unsigned int, int, std::wstring)
    END_SWITCH
	return 0;
}

void CSFileImpl::Uploading() {
	if (m_of.is_open()) {

	}
}

void CSFileImpl::Upload(const std::wstring &filePath, unsigned int flags, UINT64 fileSize, int &res, std::wstring &errMsg) {
	res = 0;
	assert(!m_FileSize);
	assert(!m_of.is_open());
	m_FileSize = fileSize;
}

void CSFileImpl::Download(const std::wstring &filePath, unsigned int flags, int &res, std::wstring &errMsg) {
	std::wifstream reader;
	reader.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		bool absoulute;
#ifdef WIN32_64
		std::size_t pos = filePath.find(L":\\");
		absoulute = (pos > 0);
#else
		absoulute = (filePath.front() == L'/');
#endif
		std::wstring path;
		if (absoulute) {
			path = filePath;
		}
		else {
			path = g_pathRoot + filePath;
		}
		reader.open(path, std::ios::binary);
		res = 0;
		reader.seekg(0, std::ios::end);
#ifdef WIN32_64
		UINT64 StreamSize = reader.tellg().seekpos();
#else
		static_assert(sizeof (std::streampos) >= sizeof (INT64), "Large file not supported");
		UINT64 StreamSize = is.tellg();
#endif
		if (SendResult(idStartDownloading, StreamSize) != sizeof(StreamSize))
			return; //socket closed or canceled
		if (StreamSize) {
			reader.seekg(0, std::ios_base::beg);
			CScopeUQueue sb(MY_OPERATION_SYSTEM, IsBigEndian(), STREAM_CHUNK_SIZE);
			reader.read((wchar_t*) sb->GetBuffer(), STREAM_CHUNK_SIZE);
			unsigned int ret = (unsigned int) reader.gcount();
			while (ret > 0) {
				if (SendResult(idDownloading, sb->GetBuffer(), res) != ret)
					return; //socket closed or canceled
				reader.read((wchar_t*) sb->GetBuffer(), STREAM_CHUNK_SIZE);
				ret = (unsigned int) reader.gcount();
			}
		}
	}
	catch(std::system_error& e) {
		res = e.code().value();
		std::string msg = e.code().message();
		errMsg = SPA::Utilities::ToWide(msg.c_str(), msg.size());
		return;
	}
	catch(...) {
		res = -1;
		errMsg = L"Unknown error";
		return;
	}
}

} //namespace ServerSide
} //namespace SPA