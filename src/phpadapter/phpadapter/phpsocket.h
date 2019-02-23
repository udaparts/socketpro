
#ifndef SPA_PHP_SOCKET_H
#define SPA_PHP_SOCKET_H

namespace PA {

    class CPhpSocket : public Php::Base {
    public:
        CPhpSocket(CClientSocket *cs);

    public:
        static void RegisterInto(Php::Namespace &cs);
        Php::Value __get(const Php::Value &name);
        void __set(const Php::Value &name, const Php::Value &value);
        int __compare(const CPhpSocket &socket) const;
        void __destruct();

    private:
        void __construct(Php::Parameters &params);
        void AbortDequeuedMessage();
        Php::Value Cancel(Php::Parameters &params);
        Php::Value DoEcho();
        Php::Value TurnOnZipAtSvr(Php::Parameters &params);
        Php::Value SetZipLevelAtSvr(Php::Parameters &params);

    private:
        CClientSocket *m_cs;
    };

} //namespace PA

#endif