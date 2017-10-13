
#include "stdafx.h"
#include "webasynchandler.h"
#include <condition_variable>
#include <chrono>

typedef SPA::CMasterPool<false, CWebAsyncHandler> CWebMasterPool;
typedef SPA::ClientSide::CSocketPool<CWebAsyncHandler> CMyPool;

int main(int argc, char* argv[]) {
	CConnectionContext cc;
	std::cout << "Remote host: " << std::endl;
	std::getline(std::cin, cc.Host);
	//cc.Host = "localhost";
	cc.Port = 20901;
	cc.UserId = L"SomeUserId";
	cc.Password = L"A_Password_For_SomeUserId";
	cc.EncrytionMethod = SPA::tagEncryptionMethod::TLSv1;

	std::mutex mutex;
	std::condition_variable cv;

	CWebMasterPool master(L"");

	//CA file is located at the directory ../socketpro/bin
	CClientSocket::SSL::SetVerifyLocation("ca.cert.pem");

	auto cb = [](CMyPool *sender, CClientSocket * cs)->bool {
		int errCode;
		SPA::IUcert *cert = cs->GetUCert();
		//std::cout << cert->SessionInfo << std::endl;

		const char* res = cert->Verify(&errCode);

		//do ssl server certificate authentication here

		return (errCode == 0); //true -- user id and password will be sent to server
	};
	master.DoSslServerAuthentication = cb;
	bool ok = master.StartSocketPool(cc, 2, 1);
	if (!ok) {
		std::cout << "Failed in connecting to remote middle tier server, and press any key to close the application ......" << std::endl;
		::getchar();
		return 1;
	}
	auto handler = master.Seek();
	SPA::CDataSet &cache = CWebMasterPool::Cache; //accessing real-time update cache
	do {
		ok = handler->GetMasterSlaveConnectedSessions([](unsigned int master_connection, unsigned int slave_connection) {
			std::cout << "master connection: " << master_connection << ", slave connection: " << slave_connection << std::endl;
		});
		if (!ok)
			break;
		ok = handler->QueryPaymentMaxMinAvgs(L"", [](const CMaxMinAvg &mma, int res, const std::wstring & errMsg) {
			if (res) {
				std::cout << "QueryPaymentMaxMinAvgs error code: " << res << ", error message: ";
				std::wcout << errMsg.c_str() << std::endl;
			}
			else {
				std::cout << "QueryPaymentMaxMinAvgs max: " << mma.Max << ", min: " << mma.Min << ", avg: " << mma.Avg << std::endl;
			}
		});
		if (!ok)
			break;
		SYSTEMTIME st;
		CDBVariantArray vData;

		vData.push_back(1); //Google company id
		vData.push_back(L"Ted Cruz");
#ifdef WIN32_64
		::GetLocalTime(&st);
#else
		::gettimeofday(&st, nullptr);
#endif
		vData.push_back(st);

		vData.push_back(1); //Google company id
		vData.push_back("Donald Trump");
#ifdef WIN32_64
		::GetLocalTime(&st);
#else
		::gettimeofday(&st, nullptr);
#endif
		vData.push_back(st);

		vData.push_back(2); //Microsoft company id
		vData.push_back("Hillary Clinton");
#ifdef WIN32_64
		::GetLocalTime(&st);
#else
		::gettimeofday(&st, nullptr);
#endif
		vData.push_back(st);
		std::unique_lock<std::mutex> al(mutex);
		ok = handler->UploadEmployees(vData, [&mutex, &cv](int res, const std::wstring &errMsg, CInt64Array & vId) {
			for (auto it = vId.cbegin(), end = vId.cend(); it != end; ++it) {
				std::cout << "Last id: " << *it << std::endl;
			}
			std::unique_lock<std::mutex> a(mutex);
			cv.notify_one();
		});
		if (!ok)
			break;
		//Use wait_for instead of handler->WaitAll() for better completion event as a session may be shared by multiple threads
		auto status = cv.wait_for(al, std::chrono::seconds(5));
		if (status == std::cv_status::timeout) {
			std::cout << "The above requests are not completed in 5 seconds" << std::endl;
		}
	} while (false);
	if (!ok) {
		std::cout << "Socket closed with error code: " << handler->GetAttachedClientSocket()->GetErrorCode() << ", error message: ";
		std::wcout << handler->GetAttachedClientSocket()->GetErrorMsg().c_str() << std::endl;
	}
	std::cout << "Press a key to shutdown the demo application ......" << std::endl;
	::getchar();
	return 0;
}
