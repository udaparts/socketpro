
#include "base64.h"
#include <math.h>

namespace SPA {

    char CBase64::encoding_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    unsigned int CBase64::mod_table[] = {0, 2, 1};

    char CBase64::decoding_table[] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
        , -1, 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21
        , 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
        , -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1
    };

    unsigned int CBase64::decode(const char *srcData, unsigned int len_in, unsigned char *decoded_data) {
        if (len_in % 4 != 0 || srcData == (const char *)0 || len_in == 0 || decoded_data == (unsigned char *)0)
            return 0;
        unsigned int i, j, output_length = len_in / 4 * 3;
        if (srcData[len_in - 1] == '=')
            --output_length;
        if (srcData[len_in - 2] == '=')
            --output_length;

        for (i = 0, j = 0; i < len_in;) {

            unsigned int sextet_a = srcData[i] == '=' ? 0 & i++ : decoding_table[srcData[i++]];
            unsigned int sextet_b = srcData[i] == '=' ? 0 & i++ : decoding_table[srcData[i++]];
            unsigned int sextet_c = srcData[i] == '=' ? 0 & i++ : decoding_table[srcData[i++]];
            unsigned int sextet_d = srcData[i] == '=' ? 0 & i++ : decoding_table[srcData[i++]];

            unsigned int triple = (sextet_a << 3 * 6)
                    + (sextet_b << 2 * 6)
                    + (sextet_c << 1 * 6)
                    + (sextet_d << 0 * 6);

            if (j < output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
            if (j < output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
            if (j < output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
        }

        return output_length;
    }

    unsigned int CBase64::encode(const unsigned char* srcData, unsigned int len_in, char* encoded_data) {
        unsigned int i, j, len = (unsigned int) (4.0 * ceil((double) len_in / 3.0));

        if (encoded_data == (char*)0)
            return 0;

        for (i = 0, j = 0; i < len_in;) {

            unsigned int octet_a = i < len_in ? srcData[i++] : 0;
            unsigned int octet_b = i < len_in ? srcData[i++] : 0;
            unsigned int octet_c = i < len_in ? srcData[i++] : 0;

            unsigned int triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

            encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
            encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
            encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
            encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
        }

        for (i = 0; i < mod_table[len_in % 3]; i++)
            encoded_data[len - 1 - i] = '=';

        encoded_data[len] = 0;

        return len;
    }

} //namespace Utilities