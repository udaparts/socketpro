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
        virtual bool IsDone() const;
        const PacketHeader& GetResponseHeader() const;
		bool HasMore() const;
		UINT64 GetCount() const;
		virtual bool Wait(unsigned int ms);
		int Send(const unsigned char* buffer, unsigned int bytes, unsigned int milliseconds, bool sync = true);

    protected:
        virtual void Reset();
		virtual bool ParseDone();
		virtual bool ParseStream() = 0;

	private:
		void OnResponse(const unsigned char* data, unsigned int bytes);

    protected:
		CTdsChannel& m_channel;
		SPA::CUQueue &m_buffer;
		tagTokenType m_tt;
        PacketHeader ResponseHeader;
		TokenDone m_Done;

	private:
		typedef std::unique_lock<std::mutex> CAutoLock;
		std::mutex m_cs;
		std::condition_variable m_cv;
		bool m_bWaiting;
		friend class CTdsChannel;
    };

}

#endif