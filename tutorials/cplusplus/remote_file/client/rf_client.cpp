
#include "stdafx.h"
#include <fstream>
#include "RemFile.h"

int main(int argc, char* argv[]) {
	SPA::UINT64 len;
	CConnectionContext cc;
	cc.Host = "192.168.1.109";
	cc.Port = 20901;
	cc.UserId = L"MyUserId";
	cc.Password = L"MyPassword";

	typedef CSocketPool<RemotingFile, CClientSocket> CMyPool;
	CMyPool spRf;

	bool ok = spRf.StartSocketPool(cc, 1, 1);
	if (!ok) {
		std::cout << "Can not connect to remote server" << std::endl;
		return -1;
	}
	auto rf = spRf.Seek();

	std::cout << "Input a remote file to download ......" << std::endl;
	std::wstring RemoteFile;
	std::getline(std::wcin, RemoteFile);

	//downloading test
	rf->GetStreamHelper()->Progress = [](const SPA::ClientSide::CStreamHelper *sh, SPA::UINT64 pos) {
		std::cout << "downloading " << (pos * 100) / sh->GetDownloadingStreamSize() << "%" << std::endl;
	};
	std::string LocalFile("spfile.test");
	std::ofstream bw(LocalFile, std::ios::out | std::ios::binary | std::ios::app);
	std::wstring res = rf->GetStreamHelper()->DownloadFile(bw, RemoteFile);
	if (res.size() == 0 && rf->WaitAll())
		std::wcout << L"Successful to download file " << RemoteFile << std::endl;
	else
		std::wcout << L"Failed to download file " << RemoteFile << std::endl;
	bw.close();

	//uploading test
	RemoteFile = L"spfile.testr";
	std::ifstream br(LocalFile, std::ios::binary | std::ios::in);
	if (br.is_open()) {
		br.seekg(0, std::ios_base::end);
#ifdef WIN32_64
		len = br.tellg().seekpos();
#else
		len = br.tellg();
#endif
		br.seekg(0, std::ios_base::beg);
		if (len > 0) {
			rf->GetStreamHelper()->Progress = [len](const SPA::ClientSide::CStreamHelper *sh, SPA::UINT64 pos) {
				std::cout << "uploading " << (pos * 100) / len << "%" << std::endl;
			};
		}
		res = rf->GetStreamHelper()->UploadFile(br, RemoteFile);
		if (res.size() == 0 && rf->WaitAll())
			std::cout << "Successful to upload file " << LocalFile << std::endl;
		else
			std::cout << "Failed to upload file " << LocalFile << std::endl;
		br.close();
	} else
		std::cout << "Cannot open " << LocalFile << " to upload" << std::endl;

	std::cout << "Press a key to shutdown the demo application ......" << std::endl;
	::getchar();
	return 0;
}

