#ifndef __UBASE_64_ENCODE_DECODE_H__
#define __UBASE_64_ENCODE_DECODE_H__

namespace SPA {

    class CBase64 {
    public:
        static unsigned int encode(const unsigned char* code_in, unsigned int len_in, char* plaintext_out);
        static unsigned int decode(const char *data_in, unsigned int len_in, unsigned char *decoded_data);

    private:
        static char encoding_table[];
        static char decoding_table[];
        static unsigned int mod_table[];
    };
}

#endif
