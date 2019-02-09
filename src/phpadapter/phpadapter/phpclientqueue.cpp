#include "stdafx.h"
#include "phpclientqueue.h"

namespace PA {

	CPhpClientQueue::CPhpClientQueue(CClientQueue &cq) : m_cq(cq) {
	}

	void CPhpClientQueue::__construct(Php::Parameters &params) {
	}

	void CPhpClientQueue::__set(const Php::Value &name, const Php::Value &value) {
		if (name == "Optimistic") {
			if (!value.isNumeric()) {
				throw Php::Exception("An integer value expected for message storing flush option");
			}
			auto data = value.numericValue();
			if (data < SPA::oMemoryCached || data > SPA::oDiskCommitted) {
				throw Php::Exception("Bad value expected for message storing flush option");
			}
			m_cq.SetOptimistic((SPA::tagOptimistic)data);
		}
		else if (name == "RoutingQueueIndex" || name == "RoutingQueueIndexEnabled") {
			m_cq.EnableRoutingQueueIndex(value.boolValue());
		}
		else {
			Php::Base::__set(name, value);
		}
	}

	Php::Value CPhpClientQueue::__get(const Php::Value &name) {
		if (name == "Available") {
			return m_cq.IsAvailable();
		}
		else if (name == "MessageCount") {
			return (int64_t)m_cq.GetMessageCount();
		}
		else if (name == "QueueSize") {
			return (int64_t)m_cq.GetQueueSize();
		}
		else if (name == "JobSize") {
			return (int64_t)m_cq.GetJobSize();
		}
		else if (name == "FileName" || name == "QueueFileName") {
			return m_cq.GetQueueFileName();
		}
		else if (name == "QueueName") {
			return m_cq.GetQueueName();
		}
		else if (name == "QueueStatus") {
			return m_cq.GetQueueOpenStatus();
		}
		else if (name == "Secure") {
			return m_cq.IsSecure();
		}
		else if (name == "TTL") {
			return (int64_t)m_cq.GetTTL();
		}
		else if (name == "MessagesInDequeuing") {
			return (int64_t)m_cq.GetMessagesInDequeuing();
		}
		else if (name == "Optimistic") {
			return m_cq.GetOptimistic();
		}
		else if (name == "LastMessageTime") {
			Php::Object datetime("DateTime", "2013-01-01");
			auto mytime = SPA::ClientSide::ClientCoreLoader.GetLastQueueMessageTime(m_cq.GetHandle());
			Php::Object span("DateInterval", "PT" + std::to_string(mytime) + "S");
			datetime.call("add", span);
			return datetime;
		}
		else if (name == "DequeueEnabled") {
			return m_cq.IsDequeueEnabled();
		}
		else if (name == "DequeueShared") {
			return m_cq.IsDequeueShared();
		}
		else if (name == "LastIndex") {
			return (int64_t)m_cq.GetLastIndex();
		}
		else if (name == "RoutingQueueIndex" || name == "RoutingQueueIndexEnabled") {
			return m_cq.IsRoutingQueueIndexEnabled();
		}
		return Php::Base::__get(name);
	}

	void CPhpClientQueue::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpClientQueue> cq(PHP_CLIENTQUEUE);
		cq.method(PHP_CONSTRUCT, &CPhpClientQueue::__construct, Php::Private);
		cq.method("AbortJob", &CPhpClientQueue::AbortJob);
		cq.method("EndJob", &CPhpClientQueue::EndJob);
		cq.method("StartJob", &CPhpClientQueue::StartJob);
		cq.method("RemoveByTTL", &CPhpClientQueue::RemoveByTTL);
		cq.method("Reset", &CPhpClientQueue::Reset);
		cq.method("CancelQueuedMessages", &CPhpClientQueue::CancelQueuedMessages, {
			Php::ByVal("startIndex", Php::Type::Numeric),
			Php::ByVal("endIndex", Php::Type::Numeric)
		});
		cs.add(cq);
	}

	Php::Value CPhpClientQueue::AbortJob() {
		return m_cq.AbortJob();
	}

	Php::Value CPhpClientQueue::EndJob() {
		return m_cq.EndJob();
	}

	Php::Value CPhpClientQueue::StartJob() {
		return m_cq.StartJob();
	}

	Php::Value CPhpClientQueue::RemoveByTTL() {
		return (int64_t)m_cq.RemoveByTTL();
	}
	void CPhpClientQueue::Reset() {
		m_cq.Reset();
	}

	Php::Value CPhpClientQueue::CancelQueuedMessages(Php::Parameters &params) {
		auto start = params[0].numericValue();
		auto end = params[1].numericValue();
		return (int64_t)m_cq.CancelQueuedRequests((SPA::UINT64)start, (SPA::UINT64)end);
	}
}
