#ifndef SPA_PHP_SOCKETPOOL_H
#define SPA_PHP_SOCKETPOOL_H

#include "poolstartcontext.h"

namespace PA {

    class CPhpSocketPool : public Php::Base {
    public:
        CPhpSocketPool(const CPoolStartContext &psc);
        CPhpSocketPool(const CPhpSocketPool &p) = delete;
        ~CPhpSocketPool();

    public:
        CPhpSocketPool& operator=(const CPhpSocketPool &p) = delete;
        static void RegisterInto(Php::Namespace &cs);
        Php::Value __get(const Php::Value &name);
        //void __set(const Php::Value &name, const Php::Value &value);
        void __destruct();
        int __compare(const CPhpSocketPool &pool) const;

    private:
        void __construct(Php::Parameters &params);
        Php::Value Seek();
        Php::Value Lock(Php::Parameters &params);

    private:
        unsigned int m_nSvsId;

        union {
            CPhpPool *Handler;
            CPhpDbPool *Db;
            CPhpFilePool *File;
            CPhpQueuePool *Queue;
        };
        std::string m_defaultDb;
        tagPoolType m_pt;
        std::string m_qName;
        unsigned int m_recvTimeout;
    };

} //namespace PA

#endif