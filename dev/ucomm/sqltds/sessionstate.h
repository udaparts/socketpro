#ifndef _U_TDS_SESSION_STATE_H_
#define _U_TDS_SESSION_STATE_H_

#include "tdsdef.h"

namespace tds {
    namespace SessionState {

        class SessionStateOption : public ISerialize, public IDeserialize {
        public:

            SessionStateOption(unsigned char stateId = 0) : StateID(stateId) {
            }

        public:

            bool SaveTo(SPA::CUQueue &buff) {
                unsigned int byte;
                if (Value.size() >= 0xff) {
                    byte = 0xff;
                    buff << byte;
                    buff << (unsigned int) Value.size();
                } else {
                    byte = (unsigned char) Value.size();
                    buff << byte;
                }
                buff.Push(Value.data(), (unsigned int) Value.size());
                return true;
            }

            bool LoadFrom(SPA::CUQueue &buff) {
                unsigned char len;
                buff >> len;
                unsigned int length = len;
                if (len == 0xff) {
                    buff >> length;
                    length = ChangeEndian(length);
                }
                const unsigned char *start = buff.GetBuffer();
                Value.assign(start, start + length);
                buff.Pop(length);
                return true;
            }

        public:
            unsigned char StateID;
            std::vector<unsigned char> Value;
        };

        class SessionRecoveryData : public ISerialize, public IDeserialize {
        public:

            bool SaveTo(SPA::CUQueue &buff) {
                SPA::CScopeUQueue sb;
                SPA::CUQueue &cache = *sb;
                unsigned char len = (unsigned char) Database.size();
                cache << len;
                if (len) {
                    cache.Push((const unsigned char*) Database.c_str(), len << 1);
                }

                len = (unsigned char) Collation.size();
                cache << len;
                if (len) {
                    cache.Push(Collation.data(), len);
                }

                len = (unsigned char) Language.size();
                cache << len;
                if (len) {
                    cache.Push((const unsigned char*) Language.c_str(), len << 1);
                }

                for (auto it = Options.begin(), end = Options.end(); it != end; ++it) {
                    it->SaveTo(cache);
                }
                buff << ChangeEndian(cache.GetSize());
                buff.Push(cache.GetBuffer(), cache.GetSize());
                return true;
            }

            bool LoadFrom(SPA::CUQueue &buff) {
                Database.clear();
                Collation.clear();
                Language.clear();
                Options.clear();

                unsigned int totalLength;
                buff >> totalLength;
                totalLength = ChangeEndian(totalLength);
                if (!totalLength) return true;

                unsigned char byteLength;
                buff >> byteLength;
                --totalLength;
                if (!totalLength) return true;
                Database.assign((const char16_t *) buff.GetBuffer(), (size_t) byteLength);
                unsigned int bytes = ((unsigned int) (byteLength) << 1);
                buff.Pop(bytes);
                totalLength -= bytes;
                if (!totalLength) return true;

                buff >> byteLength;
                --totalLength;
                if (!totalLength) return true;
                if (byteLength) {
                    Collation.assign(buff.GetBuffer(), buff.GetBuffer() + byteLength);
                    totalLength -= byteLength;
                    buff.Pop((unsigned int) byteLength);
                    if (!totalLength) return true;
                }

                buff >> byteLength;
                --totalLength;
                if (!totalLength) return true;
                Language.assign((const char16_t *) buff.GetBuffer(), (size_t) byteLength);
                bytes = ((unsigned int) (byteLength) << 1);
                buff.Pop(bytes);
                totalLength -= bytes;

                while (totalLength) {

                }

                return true;
            }

        public:
            CDBString Database;
            std::vector<unsigned char> Collation;
            CDBString Language;
            std::vector<SessionStateOption> Options;
        };
    } //namespace SessionState

} //namespace tds

#endif
