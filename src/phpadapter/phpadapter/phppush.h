#ifndef PHP_SPA_CLIENTSIDE_CLIENTSOCKET_PUSH_H
#define PHP_SPA_CLIENTSIDE_CLIENTSOCKET_PUSH_H

namespace PA {

    typedef SPA::ClientSide::CClientSocket::CPushImpl CPush;

    class CPhpPush : public Php::Base {
    public:
        CPhpPush(CPush &p);
        CPhpPush(const CPhpPush &p) = delete;

    public:
        CPhpPush& operator=(const CPhpPush &p) = delete;
        static void RegisterInto(Php::Namespace &cs);
        void __destruct();

    private:
        void __construct(Php::Parameters &params);
        Php::Value Subscribe(Php::Parameters &params);
        Php::Value Unsubscribe();
        Php::Value Publish(Php::Parameters &params);
        Php::Value SendUserMessage(Php::Parameters &params);
        Php::Value PublishEx(Php::Parameters &params);
        Php::Value SendUserMessageEx(Php::Parameters &params);
        static std::vector<unsigned int> ToGroupIds(const Php::Value &v);

    private:
        CPush &Push;
    };

} //namespace PA

#endif
