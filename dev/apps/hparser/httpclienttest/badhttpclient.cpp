
#include "stdafx.h"
#include <iostream>
#include "badhttpclient.h"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
using namespace std;

namespace UHTTP {
    namespace Client {
        std::string BadHttpClient::m_host("localhost");
        CIpResolver::query BadHttpClient::m_Query(m_host, "20901");
        CIpResolver::query BadHttpClient::m_QuerySSL(m_host, "20901");
        CSslContext BadHttpClient::m_Ctx(nsSSL::context::tlsv1_client);

        BadHttpClient::BadHttpClient(CIoService &io, bool ssl)
        : m_io(io),
        m_Dns(io),
        m_Socket(io, m_Ctx),
        m_StatusCode(0),
        m_dHttpVersion(0.0),
        m_SSL(ssl) {

        }

        BadHttpClient::~BadHttpClient(void) {

        }

        const CErrorCode& BadHttpClient::GetErrorCode() const {
            return m_ec;
        }

        bool BadHttpClient::IsOpen() const {
            return m_Socket.next_layer().is_open();
        }

        unsigned int BadHttpClient::GetStatusCode() const {
            return m_StatusCode;
        }

        const std::string& BadHttpClient::GetContent() const {
            return m_Content;
        }

        const StringMap& BadHttpClient::GetHeaders() const {
            return m_Header;
        }

        double BadHttpClient::GetHttpVersion() const {
            return m_dHttpVersion;
        }

        bool BadHttpClient::Connect() {
            do {
                if (m_SSL)
                    m_remotepoint = m_Dns.resolve(m_QuerySSL, m_ec);
                else
                    m_remotepoint = m_Dns.resolve(m_Query, m_ec);
                if (m_ec)
                    break;

                m_mypoint = boost::asio::connect(m_Socket.lowest_layer(), m_remotepoint, m_ec);
                if (m_ec)
                    break;
                if (m_SSL && m_Socket.handshake(nsSSL::stream_base::client, m_ec))
                    break;
            } while (false);
            return !m_ec;
        }

        void BadHttpClient::CleanBuffer(boost::asio::streambuf &sb) {
            size_t size = (size_t) sb.in_avail();
            if (size) {
                std::string s(size, 0);
                std::istream is(&sb);
                char *buffer = &(s.at(0));
                size = (size_t) is.readsome(buffer, (unsigned int) size);
            }
        }

        bool BadHttpClient::DoHttpRequest(const char* req, size_t len, bool auto_shutdown, unsigned int multiple) {
            size_t size;
            size_t content_len = 0;
            if (!req)
                req = "";
            m_dHttpVersion = 0.0;
            m_Header.clear();
            m_StatusCode = NETWORK_PROBLEM;
            m_Content.clear();
            if (!IsOpen() && !Connect())
                return false;
            std::ostream request_stream(&m_Request);
            if (!len)
                len = ::strlen(req);
            request_stream.write(req, len);
            do {
                if (m_SSL)
                    size = boost::asio::write(m_Socket, m_Request, m_ec);
                else
                    size = boost::asio::write(m_Socket.next_layer(), m_Request, m_ec);
                if (m_ec)
                    break;
                CleanBuffer(m_Response);
                std::istream response_stream(&m_Response);
                do {
                    if (m_SSL)
                        size = boost::asio::read_until(m_Socket, m_Response, "\r\n", m_ec);
                    else
                        size = boost::asio::read_until(m_Socket.next_layer(), m_Response, "\r\n", m_ec);
                    if (m_ec)
                        break;
                    if (size == 0) {
                        m_StatusCode = CONNECTION_SHUTDOWN_GRACEFULLY;
                        break;
                    }
                    std::string http_version;
                    response_stream >> http_version;
                    response_stream >> m_StatusCode;
                    std::string status_message;
                    std::getline(response_stream, status_message);
                    if (!response_stream || http_version.substr(0, 5) != "HTTP/")
                        m_StatusCode = BAD_HTTP_RESPONE;
                    else
                        m_dHttpVersion = boost::lexical_cast<double>(http_version.data() + 5);

                    if (m_SSL)
                        size = boost::asio::read_until(m_Socket, m_Response, "\r\n\r\n", m_ec);
                    else
                        size = boost::asio::read_until(m_Socket.next_layer(), m_Response, "\r\n\r\n", m_ec);
                    if (m_ec)
                        break;
                    std::string header;
                    while (std::getline(response_stream, header) && header != "\r" && header.length() > 0) {
                        size_t pos = header.find_first_of(':');
                        if (pos == std::string::npos) {
                            m_StatusCode = BAD_HTTP_RESPONE;
                            continue;
                        }
                        std::string str0 = header.substr(0, pos);
                        boost::trim(str0);
                        if (!str0.size()) {
                            m_StatusCode = BAD_HTTP_RESPONE;
                            continue;
                        }
                        std::string str1 = header.substr(pos + 1);
                        boost::trim(str1);
                        m_Header[str0] = str1;
                        //cout << str0 << ": " << str1 << endl;
                    }
                    //cout << endl;

                    StringMap::iterator connection = m_Header.find("connection");
                    if (connection != m_Header.end() && connection->second == "close")
                        auto_shutdown = true;

                    StringMap::iterator content_lenth = m_Header.find("content-length");
                    if (content_lenth != m_Header.end())
                        content_len = boost::lexical_cast<size_t > (content_lenth->second.data());
                    else
                        content_len = 0;
                    if (m_Response.size() >= content_len) {
                        size = content_len;
                    } else {
                        size = content_len - m_Response.size();
                        if (m_SSL)
                            size = boost::asio::read(m_Socket, m_Response, boost::asio::transfer_at_least(size), m_ec);
                        else
                            size = boost::asio::read(m_Socket.next_layer(), m_Response, boost::asio::transfer_at_least(size), m_ec);
                    }

                    if (m_ec == boost::asio::error::eof)
                        m_ec.clear();

                    size = content_len;
                    if (size) {
                        char *buffer;
                        m_Content.resize(size);
                        buffer = &(m_Content.at(0));
                        len = (size_t) response_stream.readsome(buffer, (unsigned int) size);
                        //cout << m_Content << endl;
                    }

                    //cout << "---------------------" << endl;

                    --multiple;
                } while (multiple);
            } while (false);

            if (m_StatusCode >= BAD_RESPONE_HEADER || (m_ec && m_ec != boost::asio::error::eof))
                auto_shutdown = true;

            if (auto_shutdown) {
                CErrorCode ec;
                if (m_SSL)
                    m_Socket.shutdown(ec);
                m_Socket.lowest_layer().shutdown(nsIP::tcp::socket::shutdown_both, ec);
                m_Socket.lowest_layer().close(ec);
            }
            return !m_ec;
        }
    }
}
