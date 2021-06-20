#ifndef _H_SQL_TDS_DEFINES_H_
#define _H_SQL_TDS_DEFINES_H_

#include "../include/udatabase.h"

namespace tds {

    static const unsigned int CLIENT_EXE_VERSION = 0x01000000;
    static const unsigned short BUILD_VERSION = 0x0001; //Little Endian

    static const unsigned int CLIENT_DLL_VERSION = 0x01000001; //1.0.0.1
    static const unsigned int TDS_VERSION = 0x74000004;

    typedef SPA::CDBString CDBString;
    static const CDBString ApplicationName(u"UDAParts Core MSSQL Data Provider");

    static std::vector<unsigned char> TDS_NIC_ADDRESS({0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc});

    enum class SqlAuthenticationMethod {
        /// <include file='../../../../../../../doc/snippets/Microsoft.Data.SqlClient/SqlAuthenticationMethod.xml' path='docs/members[@name="SqlAuthenticationMethod"]/NotSpecified/*'/>
        NotSpecified = 0,

        /// <include file='../../../../../../../doc/snippets/Microsoft.Data.SqlClient/SqlAuthenticationMethod.xml' path='docs/members[@name="SqlAuthenticationMethod"]/SqlPassword/*'/>
        SqlPassword,

        /// <include file='../../../../../../../doc/snippets/Microsoft.Data.SqlClient/SqlAuthenticationMethod.xml' path='docs/members[@name="SqlAuthenticationMethod"]/ActiveDirectoryPassword/*'/>
        ActiveDirectoryPassword,

        /// <include file='../../../../../../../doc/snippets/Microsoft.Data.SqlClient/SqlAuthenticationMethod.xml' path='docs/members[@name="SqlAuthenticationMethod"]/ActiveDirectoryIntegrated/*'/>
        ActiveDirectoryIntegrated,

        /// <include file='../../../../../../../doc/snippets/Microsoft.Data.SqlClient/SqlAuthenticationMethod.xml' path='docs/members[@name="SqlAuthenticationMethod"]/ActiveDirectoryInteractive/*'/>
        ActiveDirectoryInteractive,

        /// <include file='../../../../../../../doc/snippets/Microsoft.Data.SqlClient/SqlAuthenticationMethod.xml' path='docs/members[@name="SqlAuthenticationMethod"]/ActiveDirectoryServicePrincipal/*'/>
        ActiveDirectoryServicePrincipal,

        /// <include file='../../../../../../../doc/snippets/Microsoft.Data.SqlClient/SqlAuthenticationMethod.xml' path='docs/members[@name="SqlAuthenticationMethod"]/ActiveDirectoryDeviceCodeFlow/*'/>
        ActiveDirectoryDeviceCodeFlow,

        /// <include file='../../../../../../../doc/snippets/Microsoft.Data.SqlClient/SqlAuthenticationMethod.xml' path='docs/members[@name="SqlAuthenticationMethod"]/ActiveDirectoryManagedIdentity/*'/>
        ActiveDirectoryManagedIdentity,

        /// <include file='../../../../../../../doc/snippets/Microsoft.Data.SqlClient/SqlAuthenticationMethod.xml' path='docs/members[@name="SqlAuthenticationMethod"]/ActiveDirectoryMSI/*'/>
        ActiveDirectoryMSI
    };

    static const unsigned short DEFAULT_PACKET_SIZE = 5840;

    static inline unsigned int ChangeEndian(unsigned int s) {
        unsigned char* p = (unsigned char*) &s;
        unsigned char b = p[0];
        p[0] = p[3];
        p[3] = b;
        b = p[1];
        p[1] = p[2];
        p[2] = b;
        return s;
    }

    /*
    static inline tagTokenType GetTokenType(Token token) {
            token &= 12;
            switch (token)
            {
            case 0:
                    //COLMETADATA and ALTMETADATA both use a 2-byte count
                    return tagTokenType::ttVariableCount;
            case 4:
                    return tagTokenType::ttVariableLength;
            case 8:
                    //no length
                    return tagTokenType::ttZero;
            case 12:
                    // followed by 1, 2, 4, or 8 bytes of data
                    return tagTokenType::ttFixedLength;
            default:
                    break;
            }
            return tagTokenType::ttUnknown;
    }
     */

    struct ISerialize {
        virtual bool SaveTo(SPA::CUQueue &buff) = 0;
    };

    struct IDeserialize {
        virtual bool LoadFrom(SPA::CUQueue &buff) = 0;
    };
};


#endif
