#pragma once

#include "../../../pinc/hpdefines.h"
#include <boost/noncopyable.hpp>

namespace UHTTP {
    namespace Client {

        /**
         * A simple helper class for client HTTP with synchronous processing style, which is mainly designed to hack and detect a remote HTTP server quality
         */
        class BadHttpClient : boost::noncopyable {
        public:
            static const unsigned int BAD_RESPONE_HEADER = 700;
            static const unsigned int BAD_HTTP_RESPONE = 701;
            static const unsigned int NETWORK_PROBLEM = 702;
            static const unsigned int CONNECTION_SHUTDOWN_GRACEFULLY = 703;
            static const unsigned int HTTP_STATUS_OK = 200;

        public:
            /**
             * Construct an instance
             * @param io A reference to boost asio io service object
             * @param ssl True for SSL/Tls enable; and false for no SSL/TLS
             */
            BadHttpClient(CIoService &io, bool ssl);
            ~BadHttpClient(void);

            /**
             * Check error code for network communication
             * @return An error code for SSL or TCP layer
             */
            const CErrorCode& GetErrorCode() const;

            /**
            /* Execute a HTTP request with your own hack string
            /* @param req A pointer to an ASCII string
            /* @param len An option length for the ASCII string. Note that you can embed a NULL char inside the ASCII string if you use the option parameter
            /* @param auto_shutdown True or false. If the input is true, the underlying socket and SSL layer will be closed after the HTTP request is executed and done. 
            /* Note that the method may automatically set the parameter to true if the HTTP server does not set connection to keep-alive or 
            /* the code detects network communication problem during processing so that the next request can be processed correctly
            /* @return True or false. True means that there is no network communication problem even if there indeed is an HTTP execution problem
             */
            bool DoHttpRequest(const char* req, size_t len = 0, bool auto_shutdown = false, unsigned int multiple = 1);

            /**
             * Check HTTP transaction status code
             * @return An HTTP status code, or a few others defined in the above
             */
            unsigned int GetStatusCode() const;

            /**
             * Check HTTP response content
             * @return HTTP response content
             */
            const std::string& GetContent() const;

            /**
             * Get a reference to HTTP response headers
             * @return A reference to HTTP response headers
             */
            const StringMap& GetHeaders() const;

            /**
             * Check if network communication is alive
             * @return True or false
             */
            bool IsOpen() const;

            /**
             * Check HTTP version
             * @return HTTP version
             */
            double GetHttpVersion() const;

        private:
            bool Connect();
            void CleanBuffer(boost::asio::streambuf &sb);

        private:
            CIoService &m_io;
            CErrorCode m_ec;
            CIpResolver::iterator m_mypoint;
            CIpResolver::iterator m_remotepoint;
            CIpResolver m_Dns;
            CSslSocket m_Socket;
            StringMap m_Header;
            boost::asio::streambuf m_Request;
            boost::asio::streambuf m_Response;
            unsigned int m_StatusCode;
            std::string m_Content;
            double m_dHttpVersion;
            bool m_SSL;

        private:
            static CIpResolver::query m_Query;
            static CIpResolver::query m_QuerySSL;
            static std::string m_host;
            static CSslContext m_Ctx;
        };

    }
}

