#ifndef SPA_PHP_CONSTS_H
#define SPA_PHP_CONSTS_H

namespace PA {

    struct tagOperationSystem : public Php::Base {
        void __construct(Php::Parameters &params);
        static void RegisterInto(Php::Namespace &spa);
    };

    struct tagBaseRequestID : public Php::Base {
        void __construct(Php::Parameters &params);
        static void RegisterInto(Php::Namespace &spa);
    };

    struct BaseServiceID : public Php::Base {
        void __construct(Php::Parameters &params);
        static void RegisterInto(Php::Namespace &spa);
    };

    struct tagEncryptionMethod : public Php::Base {
        void __construct(Php::Parameters &params);
        static void RegisterInto(Php::Namespace &spa);
    };

    void RegisterSpaConstsInto(Php::Namespace &spa);

} //namespace PA {

#endif
