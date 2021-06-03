#ifndef _U_TDS_REQUEST_BASE_H_
#define _U_TDS_REQUEST_BASE_H_

#include "tdsdef.h"
#include "../include/channelpool.h"
#include <mutex>
#include <condition_variable>

namespace tds {
	class CTdsChannel;
    class CReqBase {
	private:
		SPA::CScopeUQueue m_sb;

    public:
        CReqBase(CTdsChannel &channel);
        virtual ~CReqBase();

    public:
        bool IsDone();
        const PacketHeader& GetResponseHeader() const;
		bool HasMore();
		UINT64 GetCount();
		virtual bool Wait(unsigned int ms);
		int Send(const unsigned char* buffer, unsigned int bytes, unsigned int milliseconds, bool sync = true);
		int GetSQLError(SPA::CDBString& errMsg);

    protected:
        virtual void Reset();
		virtual bool ParseError();
		virtual bool ParseDone();
		virtual bool ParseStream() = 0;

	private:
		void OnResponse(const unsigned char* data, unsigned int bytes);
		inline bool HasMoreInternal() const;
		inline UINT64 GetCountInternal() const;
		inline bool IsDoneInternal() const;

    protected:
		CTdsChannel& m_channel;
		SPA::CUQueue &m_buffer;
		tagTokenType m_tt;
		TokenDone m_Done; //protected by m_cs

	private:
		typedef std::unique_lock<std::mutex> CAutoLock;
		std::mutex m_csSend;
		std::mutex m_cs;
		std::condition_variable m_cv;
		bool m_bWaiting; //protected by m_cs
		PacketHeader ResponseHeader; //protected by m_cs
		TokenInfo m_errCode; //protected by m_cs
		friend class CTdsChannel;
    };

}

#endif