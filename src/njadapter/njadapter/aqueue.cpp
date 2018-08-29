
#include "stdafx.h"
#include "aqueue.h"

namespace NJA {

	CAQueue::CAQueue(SPA::ClientSide::CClientSocket *cs) 
		: CAsyncQueue(cs)
	{
		::memset(&m_qType, 0, sizeof(m_qType));
		m_qType.data = this;
		int fail = uv_async_init(uv_default_loop(), &m_qType, queue_cb);
		assert(!fail);
	}

	CAQueue::~CAQueue()
	{
		SPA::CAutoLock al(m_csQ);
		uv_close((uv_handle_t*)&m_qType, nullptr);
	}

	SPA::ClientSide::CAsyncServiceHandler::DDiscarded CAQueue::Get(Isolate* isolate, Local<Value> abort, bool &bad) {
		bad = false;
		DDiscarded dd;
		if (abort->IsFunction()) {
			std::shared_ptr<CNJFunc> func(new CNJFunc);
			func->Reset(isolate, Local<Function>::Cast(abort));
			dd = [this, func](CAsyncServiceHandler *aq, bool canceled) {
				QueueCb qcb;
				qcb.EventType = qeDiscarded;
				qcb.Func = func;
				auto cs = this->GetAttachedClientSocket();
				bool endian;
				tagOperationSystem os = cs->GetPeerOs(&endian);
				qcb.Buffer = CScopeUQueue::Lock(os, endian);
				PAQueue ash = (PAQueue)aq;
				*qcb.Buffer << ash << canceled;
				CAutoLock al(this->m_csQ);
				this->m_deqQCb.push_back(qcb);
				int fail = uv_async_send(&this->m_qType);
				assert(!fail);
			};
		}
		else if (abort->IsNullOrUndefined()) {
		}
		else {
			isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "A callback expected for tracking socket closed or canceled events")));
			bad = true;
		}
		return dd;
	}

	void CAQueue::queue_cb(uv_async_t* handle) {

	}

	SPA::UINT64 CAQueue::GetKeys(Isolate* isolate, int args, Local<Value> *argv) {
		SPA::UINT64 index = GetCallIndex();
		DGetKeys gk;
		DDiscarded dd;
		if (args > 0) {
			if (argv[0]->IsFunction()) {
				std::shared_ptr<CNJFunc> func(new CNJFunc);
				func->Reset(isolate, Local<Function>::Cast(argv[0]));
				gk = [this, func](std::vector<std::string>& v) {
					QueueCb qcb;
					qcb.EventType = qeGetKeys;
					qcb.Func = func;
					auto cs = this->GetAttachedClientSocket();
					bool endian;
					tagOperationSystem os = cs->GetPeerOs(&endian);
					qcb.Buffer = CScopeUQueue::Lock(os, endian);
					PAQueue ash = this;
					*qcb.Buffer << ash << (unsigned int)v.size();
					for (auto it = v.begin(), end = v.end(); it != end; ++it) {
						*qcb.Buffer << *it;
					}
					CAutoLock al(this->m_csQ);
					this->m_deqQCb.push_back(qcb);
					int fail = uv_async_send(&this->m_qType);
					assert(!fail);
				};
			}
			else if (argv[0]->IsNullOrUndefined()) {
			}
			else {
				isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "A callback expected for GetKeys end result")));
				return 0;
			}
		}
		if (args > 1) {
			bool bad;
			dd = Get(isolate, argv[1],bad);
			if (bad)
				return 0;
		}
		return CAsyncQueue::GetKeys(gk, dd) ? INVALID_NUMBER : index;
	}

	SPA::UINT64 CAQueue::StartQueueTrans(Isolate* isolate, int args, Local<Value> *argv, const char *key) {
		SPA::UINT64 index = GetCallIndex();
		DQueueTrans qt;
		DDiscarded dd;
		if (args > 0) {
			if (argv[0]->IsFunction()) {
				std::shared_ptr<CNJFunc> func(new CNJFunc);
				func->Reset(isolate, Local<Function>::Cast(argv[0]));
				qt = [this, func](int errCode) {
					QueueCb qcb;
					qcb.EventType = qeStartQueueTrans;
					qcb.Func = func;
					auto cs = this->GetAttachedClientSocket();
					bool endian;
					tagOperationSystem os = cs->GetPeerOs(&endian);
					qcb.Buffer = CScopeUQueue::Lock(os, endian);
					PAQueue ash = this;
					*qcb.Buffer << ash << errCode;
					CAutoLock al(this->m_csQ);
					this->m_deqQCb.push_back(qcb);
					int fail = uv_async_send(&this->m_qType);
					assert(!fail);
				};
			}
			else if (argv[0]->IsNullOrUndefined()) {
			}
			else {
				isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "A callback expected for StartTrans end result")));
				return 0;
			}
		}
		if (args > 1) {
			bool bad;
			dd = Get(isolate, argv[1], bad);
			if (bad)
				return 0;
		}
		return CAsyncQueue::StartQueueTrans(key, qt, dd) ? INVALID_NUMBER : index;
	}

	SPA::UINT64 CAQueue::EndQueueTrans(Isolate* isolate, int args, Local<Value> *argv, bool rollback) {
		SPA::UINT64 index = GetCallIndex();
		DQueueTrans qt;
		DDiscarded dd;
		if (args > 0) {
			if (argv[0]->IsFunction()) {
				std::shared_ptr<CNJFunc> func(new CNJFunc);
				func->Reset(isolate, Local<Function>::Cast(argv[0]));
				qt = [this, func](int errCode) {
					QueueCb qcb;
					qcb.EventType = qeEndQueueTrans;
					qcb.Func = func;
					auto cs = this->GetAttachedClientSocket();
					bool endian;
					tagOperationSystem os = cs->GetPeerOs(&endian);
					qcb.Buffer = CScopeUQueue::Lock(os, endian);
					PAQueue ash = this;
					*qcb.Buffer << ash << errCode;
					CAutoLock al(this->m_csQ);
					this->m_deqQCb.push_back(qcb);
					int fail = uv_async_send(&this->m_qType);
					assert(!fail);
				};
			}
			else if (argv[0]->IsNullOrUndefined()) {
			}
			else {
				isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "A callback expected for EndTrans end result")));
				return 0;
			}
		}
		if (args > 1) {
			bool bad;
			dd = Get(isolate, argv[1], bad);
			if (bad)
				return 0;
		}
		return CAsyncQueue::EndQueueTrans(rollback, qt, dd) ? INVALID_NUMBER : index;
	}

	SPA::UINT64 CAQueue::CloseQueue(Isolate* isolate, int args, Local<Value> *argv, const char *key, bool permanent) {
		SPA::UINT64 index = GetCallIndex();
		DClose c;
		DDiscarded dd;
		if (args > 0) {
			if (argv[0]->IsFunction()) {
				std::shared_ptr<CNJFunc> func(new CNJFunc);
				func->Reset(isolate, Local<Function>::Cast(argv[0]));
				c = [this, func](int errCode) {
					QueueCb qcb;
					qcb.EventType = qeCloseQueue;
					qcb.Func = func;
					auto cs = this->GetAttachedClientSocket();
					bool endian;
					tagOperationSystem os = cs->GetPeerOs(&endian);
					qcb.Buffer = CScopeUQueue::Lock(os, endian);
					PAQueue ash = this;
					*qcb.Buffer << ash << errCode;
					CAutoLock al(this->m_csQ);
					this->m_deqQCb.push_back(qcb);
					int fail = uv_async_send(&this->m_qType);
					assert(!fail);
				};
			}
			else if (argv[0]->IsNullOrUndefined()) {
			}
			else {
				isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "A callback expected for Close end result")));
				return 0;
			}
		}
		if (args > 1) {
			bool bad;
			dd = Get(isolate, argv[1], bad);
			if (bad)
				return 0;
		}
		return CAsyncQueue::CloseQueue(key, c, permanent, dd) ? INVALID_NUMBER : index;
	}

	SPA::UINT64 CAQueue::FlushQueue(Isolate* isolate, int args, Local<Value> *argv, const char *key, tagOptimistic option) {
		SPA::UINT64 index = GetCallIndex();
		DFlush f;
		DDiscarded dd;
		if (args > 0) {
			if (argv[0]->IsFunction()) {
				std::shared_ptr<CNJFunc> func(new CNJFunc);
				func->Reset(isolate, Local<Function>::Cast(argv[0]));
				f = [this, func](SPA::UINT64 messageCount, SPA::UINT64 fileSize) {
					QueueCb qcb;
					qcb.EventType = qeFlushQueue;
					qcb.Func = func;
					auto cs = this->GetAttachedClientSocket();
					bool endian;
					tagOperationSystem os = cs->GetPeerOs(&endian);
					qcb.Buffer = CScopeUQueue::Lock(os, endian);
					PAQueue ash = this;
					*qcb.Buffer << ash << messageCount << fileSize;
					CAutoLock al(this->m_csQ);
					this->m_deqQCb.push_back(qcb);
					int fail = uv_async_send(&this->m_qType);
					assert(!fail);
				};
			}
			else if (argv[0]->IsNullOrUndefined()) {
			}
			else {
				isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "A callback expected for Flush end result")));
				return 0;
			}
		}
		if (args > 1) {
			bool bad;
			dd = Get(isolate, argv[1], bad);
			if (bad)
				return 0;
		}
		return CAsyncQueue::FlushQueue(key, f, option, dd) ? INVALID_NUMBER : index;
	}

	SPA::UINT64 CAQueue::Dequeue(Isolate* isolate, int args, Local<Value> *argv, const char *key, unsigned int timeout) {
		SPA::UINT64 index = GetCallIndex();
		DDequeue d;
		DDiscarded dd;
		if (args > 0) {
			if (argv[0]->IsFunction()) {
				std::shared_ptr<CNJFunc> func(new CNJFunc);
				func->Reset(isolate, Local<Function>::Cast(argv[0]));
				d = [this, func](SPA::UINT64 messageCount, SPA::UINT64 fileSize, unsigned int messagesDequeuedInBatch, unsigned int bytesDequeuedInBatch) {
					QueueCb qcb;
					qcb.EventType = qeDequeue;
					qcb.Func = func;
					auto cs = this->GetAttachedClientSocket();
					bool endian;
					tagOperationSystem os = cs->GetPeerOs(&endian);
					qcb.Buffer = CScopeUQueue::Lock(os, endian);
					PAQueue ash = this;
					*qcb.Buffer << ash << messageCount << fileSize << messagesDequeuedInBatch << bytesDequeuedInBatch;
					CAutoLock al(this->m_csQ);
					this->m_deqQCb.push_back(qcb);
					int fail = uv_async_send(&this->m_qType);
					assert(!fail);
				};
			}
			else if (argv[0]->IsNullOrUndefined()) {
			}
			else {
				isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "A callback expected for Dequeue end result")));
				return 0;
			}
		}
		if (args > 1) {
			bool bad;
			dd = Get(isolate, argv[1], bad);
			if (bad)
				return 0;
		}
		return CAsyncQueue::Dequeue(key, d, timeout, dd) ? INVALID_NUMBER : index;
	}

	SPA::UINT64 CAQueue::Enqueue(Isolate* isolate, int args, Local<Value> *argv, const char *key, unsigned short idMessage, const unsigned char *pBuffer, unsigned int size) {
		SPA::UINT64 index = GetCallIndex();
		DEnqueue e;
		DDiscarded dd;
		if (args > 0) {
			if (argv[0]->IsFunction()) {
				std::shared_ptr<CNJFunc> func(new CNJFunc);
				func->Reset(isolate, Local<Function>::Cast(argv[0]));
				e = [this, func](SPA::UINT64 indexMessage) {
					QueueCb qcb;
					qcb.EventType = qeEnqueue;
					qcb.Func = func;
					auto cs = this->GetAttachedClientSocket();
					bool endian;
					tagOperationSystem os = cs->GetPeerOs(&endian);
					qcb.Buffer = CScopeUQueue::Lock(os, endian);
					PAQueue ash = this;
					*qcb.Buffer << ash << indexMessage;
					CAutoLock al(this->m_csQ);
					this->m_deqQCb.push_back(qcb);
					int fail = uv_async_send(&this->m_qType);
					assert(!fail);
				};
			}
			else if (argv[0]->IsNullOrUndefined()) {
			}
			else {
				isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "A callback expected for Enqueue end result")));
				return 0;
			}
		}
		if (args > 1) {
			bool bad;
			dd = Get(isolate, argv[1], bad);
			if (bad)
				return 0;
		}
		return CAsyncQueue::Enqueue(key, idMessage, pBuffer, size, e, dd) ? INVALID_NUMBER : index;
	}

	SPA::UINT64 CAQueue::EnqueueBatch(Isolate* isolate, int args, Local<Value> *argv, const char *key, const unsigned char *pBuffer, unsigned int size) {
		SPA::UINT64 index = GetCallIndex();
		DEnqueue e;
		DDiscarded dd;
		if (args > 0) {
			if (argv[0]->IsFunction()) {
				std::shared_ptr<CNJFunc> func(new CNJFunc);
				func->Reset(isolate, Local<Function>::Cast(argv[0]));
				e = [this, func](SPA::UINT64 indexMessage) {
					QueueCb qcb;
					qcb.EventType = qeEnqueueBatch;
					qcb.Func = func;
					auto cs = this->GetAttachedClientSocket();
					bool endian;
					tagOperationSystem os = cs->GetPeerOs(&endian);
					qcb.Buffer = CScopeUQueue::Lock(os, endian);
					PAQueue ash = this;
					*qcb.Buffer << ash << indexMessage;
					CAutoLock al(this->m_csQ);
					this->m_deqQCb.push_back(qcb);
					int fail = uv_async_send(&this->m_qType);
					assert(!fail);
				};
			}
			else if (argv[0]->IsNullOrUndefined()) {
			}
			else {
				isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "A callback expected for EnqueueBatch end result")));
				return 0;
			}
		}
		if (args > 1) {
			bool bad;
			dd = Get(isolate, argv[1], bad);
			if (bad)
				return 0;
		}
		return CAsyncQueue::EnqueueBatch(key, pBuffer, size, e, dd) ? INVALID_NUMBER : index;
	}
}
