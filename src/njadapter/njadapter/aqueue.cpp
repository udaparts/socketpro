
#include "stdafx.h"
#include "aqueue.h"
#include "njasyncqueue.h"

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
			dd = [func](CAsyncServiceHandler *aq, bool canceled) {
				QueueCb qcb;
				qcb.EventType = qeDiscarded;
				qcb.Func = func;
				auto cs = aq->GetAttachedClientSocket();
				bool endian;
				tagOperationSystem os = cs->GetPeerOs(&endian);
				qcb.Buffer = CScopeUQueue::Lock(os, endian);
				PAQueue ash = (PAQueue)aq;
				*qcb.Buffer << ash << canceled;
				CAutoLock al(ash->m_csQ);
				ash->m_deqQCb.push_back(qcb);
				int fail = uv_async_send(&ash->m_qType);
				assert(!fail);
			};
		}
		else if (!abort->IsNullOrUndefined()) {
			isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "A callback expected for tracking socket closed or canceled events")));
			bad = true;
		}
		return dd;
	}

	void CAQueue::queue_cb(uv_async_t* handle) {
		Isolate* isolate = Isolate::GetCurrent();
		v8::HandleScope handleScope(isolate); //required for Node 4.x
		CAQueue* obj = (CAQueue*)handle->data; //sender
		assert(obj);
		if (!obj)
			return;
		SPA::CAutoLock al(obj->m_csQ);
		while (obj->m_deqQCb.size()) {
			QueueCb &cb = obj->m_deqQCb.front();
			PAQueue processor;
			*cb.Buffer >> processor;
			Local<Function> func = Local<Function>::New(isolate, *cb.Func);
			Local<Object> njQ = NJAsyncQueue::New(isolate, processor, true);
			switch (cb.EventType) {
			case qeDiscarded:
			{
				bool canceled;
				*cb.Buffer >> canceled;
				assert(!cb.Buffer->GetSize());
				Local<Value> argv[] = { v8::Boolean::New(isolate, canceled), njQ };
				func->Call(Null(isolate), 2, argv);
			}
			break;
			case qeGetKeys:
			{
				unsigned int size;
				*cb.Buffer >> size;
				unsigned int index = 0;
				Local<Array> jsKeys = Array::New(isolate);
				while (cb.Buffer->GetSize())
				{
					std::string s;
					*cb.Buffer >> s;
					auto str = String::NewFromUtf8(isolate, s.c_str());
					bool ok = jsKeys->Set(index, str);
					assert(ok);
					++index;
				}
				assert(index == size);
				Local<Value> argv[] = { jsKeys, njQ };
				func->Call(Null(isolate), 2, argv);
			}
			break;
			case qeEnqueueBatch:
			case qeEnqueue:
			{
				SPA::UINT64 indexMessage;
				*cb.Buffer >> indexMessage;
				assert(!cb.Buffer->GetSize());
				Local<Value> im = Number::New(isolate, (double)indexMessage);
				Local<Value> argv[] = { im, njQ };
				func->Call(Null(isolate), 2, argv);
			}
			break;
			case qeCloseQueue:
			case qeEndQueueTrans:
			case qeStartQueueTrans:
			{
				int errCode;
				*cb.Buffer >> errCode;
				assert(!cb.Buffer->GetSize());
				Local<Value> jsCode = Int32::New(isolate, errCode);
				Local<Value> argv[] = { jsCode, njQ };
				func->Call(Null(isolate), 2, argv);
			}
			break;
			case qeFlushQueue:
			{
				SPA::UINT64 messageCount, fileSize;
				*cb.Buffer >> messageCount >> fileSize;
				assert(!cb.Buffer->GetSize());
				Local<Value> mc = Number::New(isolate, (double)messageCount);
				Local<Value> fs = Number::New(isolate, (double)fileSize);
				Local<Value> argv[] = { mc, fs, njQ };
				func->Call(Null(isolate), 3, argv);
			}
			break;
			case qeDequeue:
			{
				SPA::UINT64 messageCount, fileSize;
				unsigned int messagesDequeuedInBatch, bytesDequeuedInBatch;
				*cb.Buffer >> messageCount >> fileSize >> messagesDequeuedInBatch >> bytesDequeuedInBatch;
				assert(!cb.Buffer->GetSize());
				Local<Value> mc = Number::New(isolate, (double)messageCount);
				Local<Value> fs = Number::New(isolate, (double)fileSize);
				Local<Value> mdib = Uint32::New(isolate, messagesDequeuedInBatch);
				Local<Value> bdib = Uint32::New(isolate, bytesDequeuedInBatch);
				Local<Value> argv[] = { mc, fs, mdib, bdib, njQ };
				func->Call(Null(isolate), 5, argv);
			}
			break;
			default:
				assert(false); //shouldn't come here
				break;
			}
			CScopeUQueue::Unlock(cb.Buffer);
			obj->m_deqQCb.pop_front();
		}
	}

	SPA::UINT64 CAQueue::GetKeys(Isolate* isolate, int args, Local<Value> *argv) {
		SPA::UINT64 index = GetCallIndex();
		DGetKeys gk;
		DDiscarded dd;
		if (args > 0) {
			if (argv[0]->IsFunction()) {
				std::shared_ptr<CNJFunc> func(new CNJFunc);
				func->Reset(isolate, Local<Function>::Cast(argv[0]));
				gk = [func](CAsyncQueue *aq, std::vector<std::string>& v) {
					QueueCb qcb;
					qcb.EventType = qeGetKeys;
					qcb.Func = func;
					auto cs = aq->GetAttachedClientSocket();
					bool endian;
					tagOperationSystem os = cs->GetPeerOs(&endian);
					qcb.Buffer = CScopeUQueue::Lock(os, endian);
					PAQueue ash = (PAQueue)aq;
					*qcb.Buffer << ash << (unsigned int)v.size();
					for (auto it = v.begin(), end = v.end(); it != end; ++it) {
						*qcb.Buffer << *it;
					}
					CAutoLock al(ash->m_csQ);
					ash->m_deqQCb.push_back(qcb);
					int fail = uv_async_send(&ash->m_qType);
					assert(!fail);
				};
			}
			else if (!argv[0]->IsNullOrUndefined()) {
				isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "A callback expected for GetKeys end result")));
				return 0;
			}
		}
		if (args > 1) {
			bool bad;
			dd = Get(isolate, argv[1], bad);
			if (bad)
				return 0;
		}
		return CAsyncQueue::GetKeys(gk, dd) ? index : INVALID_NUMBER;
	}

	SPA::UINT64 CAQueue::StartQueueTrans(Isolate* isolate, int args, Local<Value> *argv, const char *key) {
		SPA::UINT64 index = GetCallIndex();
		DQueueTrans qt;
		DDiscarded dd;
		if (args > 0) {
			if (argv[0]->IsFunction()) {
				std::shared_ptr<CNJFunc> func(new CNJFunc);
				func->Reset(isolate, Local<Function>::Cast(argv[0]));
				qt = [func](CAsyncQueue *aq, int errCode) {
					QueueCb qcb;
					qcb.EventType = qeStartQueueTrans;
					qcb.Func = func;
					auto cs = aq->GetAttachedClientSocket();
					bool endian;
					tagOperationSystem os = cs->GetPeerOs(&endian);
					qcb.Buffer = CScopeUQueue::Lock(os, endian);
					PAQueue ash = (PAQueue)aq;
					*qcb.Buffer << ash << errCode;
					CAutoLock al(ash->m_csQ);
					ash->m_deqQCb.push_back(qcb);
					int fail = uv_async_send(&ash->m_qType);
					assert(!fail);
				};
			}
			else if (!argv[0]->IsNullOrUndefined()) {
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
		return CAsyncQueue::StartQueueTrans(key, qt, dd) ? index : INVALID_NUMBER;
	}

	SPA::UINT64 CAQueue::EndQueueTrans(Isolate* isolate, int args, Local<Value> *argv, bool rollback) {
		SPA::UINT64 index = GetCallIndex();
		DQueueTrans qt;
		DDiscarded dd;
		if (args > 0) {
			if (argv[0]->IsFunction()) {
				std::shared_ptr<CNJFunc> func(new CNJFunc);
				func->Reset(isolate, Local<Function>::Cast(argv[0]));
				qt = [func](CAsyncQueue *aq, int errCode) {
					QueueCb qcb;
					qcb.EventType = qeEndQueueTrans;
					qcb.Func = func;
					auto cs = aq->GetAttachedClientSocket();
					bool endian;
					tagOperationSystem os = cs->GetPeerOs(&endian);
					qcb.Buffer = CScopeUQueue::Lock(os, endian);
					PAQueue ash = (PAQueue)aq;
					*qcb.Buffer << ash << errCode;
					CAutoLock al(ash->m_csQ);
					ash->m_deqQCb.push_back(qcb);
					int fail = uv_async_send(&ash->m_qType);
					assert(!fail);
				};
			}
			else if (!argv[0]->IsNullOrUndefined()) {
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
		return CAsyncQueue::EndQueueTrans(rollback, qt, dd) ? index : INVALID_NUMBER;
	}

	SPA::UINT64 CAQueue::CloseQueue(Isolate* isolate, int args, Local<Value> *argv, const char *key, bool permanent) {
		SPA::UINT64 index = GetCallIndex();
		DClose c;
		DDiscarded dd;
		if (args > 0) {
			if (argv[0]->IsFunction()) {
				std::shared_ptr<CNJFunc> func(new CNJFunc);
				func->Reset(isolate, Local<Function>::Cast(argv[0]));
				c = [func](CAsyncQueue *aq, int errCode) {
					QueueCb qcb;
					qcb.EventType = qeCloseQueue;
					qcb.Func = func;
					auto cs = aq->GetAttachedClientSocket();
					bool endian;
					tagOperationSystem os = cs->GetPeerOs(&endian);
					qcb.Buffer = CScopeUQueue::Lock(os, endian);
					PAQueue ash = (PAQueue)aq;
					*qcb.Buffer << ash << errCode;
					CAutoLock al(ash->m_csQ);
					ash->m_deqQCb.push_back(qcb);
					int fail = uv_async_send(&ash->m_qType);
					assert(!fail);
				};
			}
			else if (!argv[0]->IsNullOrUndefined()) {
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
		return CAsyncQueue::CloseQueue(key, c, permanent, dd) ? index : INVALID_NUMBER;
	}

	SPA::UINT64 CAQueue::FlushQueue(Isolate* isolate, int args, Local<Value> *argv, const char *key, tagOptimistic option) {
		SPA::UINT64 index = GetCallIndex();
		DFlush f;
		DDiscarded dd;
		if (args > 0) {
			if (argv[0]->IsFunction()) {
				std::shared_ptr<CNJFunc> func(new CNJFunc);
				func->Reset(isolate, Local<Function>::Cast(argv[0]));
				f = [func](CAsyncQueue *aq, SPA::UINT64 messageCount, SPA::UINT64 fileSize) {
					QueueCb qcb;
					qcb.EventType = qeFlushQueue;
					qcb.Func = func;
					auto cs = aq->GetAttachedClientSocket();
					bool endian;
					tagOperationSystem os = cs->GetPeerOs(&endian);
					qcb.Buffer = CScopeUQueue::Lock(os, endian);
					PAQueue ash = (PAQueue)aq;
					*qcb.Buffer << ash << messageCount << fileSize;
					CAutoLock al(ash->m_csQ);
					ash->m_deqQCb.push_back(qcb);
					int fail = uv_async_send(&ash->m_qType);
					assert(!fail);
				};
			}
			else if (!argv[0]->IsNullOrUndefined()) {
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
		return CAsyncQueue::FlushQueue(key, f, option, dd) ? index : INVALID_NUMBER;
	}

	SPA::UINT64 CAQueue::Dequeue(Isolate* isolate, int args, Local<Value> *argv, const char *key, unsigned int timeout) {
		SPA::UINT64 index = GetCallIndex();
		DDequeue d;
		DDiscarded dd;
		if (args > 0) {
			if (argv[0]->IsFunction()) {
				std::shared_ptr<CNJFunc> func(new CNJFunc);
				func->Reset(isolate, Local<Function>::Cast(argv[0]));
				d = [func](CAsyncQueue *aq, SPA::UINT64 messageCount, SPA::UINT64 fileSize, unsigned int messagesDequeuedInBatch, unsigned int bytesDequeuedInBatch) {
					QueueCb qcb;
					qcb.EventType = qeDequeue;
					qcb.Func = func;
					auto cs = aq->GetAttachedClientSocket();
					bool endian;
					tagOperationSystem os = cs->GetPeerOs(&endian);
					qcb.Buffer = CScopeUQueue::Lock(os, endian);
					PAQueue ash = (PAQueue)aq;
					*qcb.Buffer << ash << messageCount << fileSize << messagesDequeuedInBatch << bytesDequeuedInBatch;
					CAutoLock al(ash->m_csQ);
					ash->m_deqQCb.push_back(qcb);
					int fail = uv_async_send(&ash->m_qType);
					assert(!fail);
				};
			}
			else if (!argv[0]->IsNullOrUndefined()) {
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
		return CAsyncQueue::Dequeue(key, d, timeout, dd) ? index : INVALID_NUMBER;
	}

	SPA::UINT64 CAQueue::Enqueue(Isolate* isolate, int args, Local<Value> *argv, const char *key, unsigned short idMessage, const unsigned char *pBuffer, unsigned int size) {
		SPA::UINT64 index = GetCallIndex();
		DEnqueue e;
		DDiscarded dd;
		if (args > 0) {
			if (argv[0]->IsFunction()) {
				std::shared_ptr<CNJFunc> func(new CNJFunc);
				func->Reset(isolate, Local<Function>::Cast(argv[0]));
				e = [func](CAsyncQueue *aq, SPA::UINT64 indexMessage) {
					QueueCb qcb;
					qcb.EventType = qeEnqueue;
					qcb.Func = func;
					auto cs = aq->GetAttachedClientSocket();
					bool endian;
					tagOperationSystem os = cs->GetPeerOs(&endian);
					qcb.Buffer = CScopeUQueue::Lock(os, endian);
					PAQueue ash = (PAQueue)aq;
					*qcb.Buffer << ash << indexMessage;
					CAutoLock al(ash->m_csQ);
					ash->m_deqQCb.push_back(qcb);
					int fail = uv_async_send(&ash->m_qType);
					assert(!fail);
				};
			}
			else if (!argv[0]->IsNullOrUndefined()) {
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
		return CAsyncQueue::Enqueue(key, idMessage, pBuffer, size, e, dd) ? index : INVALID_NUMBER;
	}

	SPA::UINT64 CAQueue::EnqueueBatch(Isolate* isolate, int args, Local<Value> *argv, const char *key, const unsigned char *pBuffer, unsigned int size) {
		SPA::UINT64 index = GetCallIndex();
		DEnqueue e;
		DDiscarded dd;
		if (args > 0) {
			if (argv[0]->IsFunction()) {
				std::shared_ptr<CNJFunc> func(new CNJFunc);
				func->Reset(isolate, Local<Function>::Cast(argv[0]));
				e = [func](CAsyncQueue *aq, SPA::UINT64 indexMessage) {
					QueueCb qcb;
					qcb.EventType = qeEnqueueBatch;
					qcb.Func = func;
					auto cs = aq->GetAttachedClientSocket();
					bool endian;
					tagOperationSystem os = cs->GetPeerOs(&endian);
					qcb.Buffer = CScopeUQueue::Lock(os, endian);
					PAQueue ash = (PAQueue)aq;
					*qcb.Buffer << ash << indexMessage;
					CAutoLock al(ash->m_csQ);
					ash->m_deqQCb.push_back(qcb);
					int fail = uv_async_send(&ash->m_qType);
					assert(!fail);
				};
			}
			else if (!argv[0]->IsNullOrUndefined()) {
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
		return CAsyncQueue::EnqueueBatch(key, pBuffer, size, e, dd) ? index : INVALID_NUMBER;
	}
}
