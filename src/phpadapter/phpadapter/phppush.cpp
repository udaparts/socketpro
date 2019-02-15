#include "stdafx.h"
#include "phppush.h"

namespace PA {

	CPhpPush::CPhpPush(CPush &p) : Push(p) {
	}

	void CPhpPush::__construct(Php::Parameters &params) {
	}

	void CPhpPush::__destruct() {
	}

	Php::Value CPhpPush::Subscribe(Php::Parameters &params) {
		std::vector<unsigned int> vGroup = ToGroupIds(params[0]);
		return Push.Subscribe(vGroup.data(), (unsigned int)vGroup.size());
	}

	Php::Value CPhpPush::Unsubscribe() {
		Push.Unsubscribe();
		return true;
	}

	std::vector<unsigned int> CPhpPush::ToGroupIds(const Php::Value &v) {
		std::vector<unsigned int> vGroup;
		int count = v.length();
		for (int n = 0; n < count; ++n) {
			Php::Value pid = v.get(n);
			if (!pid.isNumeric()) {
				throw Php::Exception("An unsigned integer value expected for chat group id");
			}
			int64_t id = pid.numericValue();
			if (id <= 0 || id > SPA::UQUEUE_NULL_LENGTH) {
				throw Php::Exception("Bad topic group id value");
			}
			vGroup.push_back((unsigned int)id);
		}
		return vGroup;
	}

	Php::Value CPhpPush::Publish(Php::Parameters &params) {
		CComVariant vtMsg;
		ToVariant(params[0], vtMsg);
		std::vector<unsigned int> vGroup = ToGroupIds(params[1]);
		return Push.Publish(vtMsg, vGroup.data(), (unsigned int)vGroup.size());
	}

	Php::Value CPhpPush::SendUserMessage(Php::Parameters &params) {
		CComVariant vtMsg;
		ToVariant(params[0], vtMsg);
		std::wstring uid = SPA::Utilities::ToWide(params[1].rawValue());
		return Push.SendUserMessage(vtMsg, uid.c_str());
	}

	Php::Value CPhpPush::PublishEx(Php::Parameters &params) {
		const char *raw = params[0].rawValue();
		int len = params[0].length();
		std::vector<unsigned int> vGroup = ToGroupIds(params[1]);
		return Push.PublishEx((const unsigned char*)raw, (unsigned int)len, vGroup.data(), (unsigned int)vGroup.size());
	}

	Php::Value CPhpPush::SendUserMessageEx(Php::Parameters &params) {
		const char *raw = params[0].rawValue();
		int len = params[0].length();
		std::wstring uid = SPA::Utilities::ToWide(params[1].rawValue());
		return Push.SendUserMessageEx(uid.c_str(), (const unsigned char*)raw, (unsigned int)len);
	}

	void CPhpPush::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpPush> push(PHP_PUSH);
		push.method(PHP_CONSTRUCT, &CPhpPush::__construct, Php::Private);
		push.method("Unsubscribe", &CPhpPush::Unsubscribe);
		push.method("Subscribe", &CPhpPush::Subscribe, {
			Php::ByVal("groups", Php::Type::Array)
		});
		push.method("Publish", &CPhpPush::Publish, {
			Php::ByVal("msg", Php::Type::Null),
			Php::ByVal("groups", Php::Type::Array)
		});
		push.method("SendUserMessage", &CPhpPush::SendUserMessage, {
			Php::ByVal("msg", Php::Type::Null),
			Php::ByVal("receiver", Php::Type::String)
		});
		push.method("PublishEx", &CPhpPush::PublishEx, {
			Php::ByVal("msg", Php::Type::String),
			Php::ByVal("groups", Php::Type::Array)
		});
		push.method("SendUserMessageEx", &CPhpPush::SendUserMessageEx, {
			Php::ByVal("msg", Php::Type::String),
			Php::ByVal("receiver", Php::Type::String)
		});
		cs.add(push);
	}
}
