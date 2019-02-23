#ifndef SPA_PHP_CONNECTION_CONTEXT_H
#define SPA_PHP_CONNECTION_CONTEXT_H

namespace PA {

    class CPhpConnContext : public SPA::ClientSide::CConnectionContext, public Php::Base {
    public:
        CPhpConnContext();
        CPhpConnContext(const CPhpConnContext &cc) = delete;

    public:
        CPhpConnContext& operator=(const CPhpConnContext &cc) = delete;
        static void RegisterInto(Php::Namespace &cs);
        void __construct(Php::Parameters &params);
        Php::Value __get(const Php::Value &name);
        void __set(const Php::Value &name, const Php::Value &value);

    };

}

#endif;