
#ifndef	__USOCKET__H__STREAM_HEADER__
#define __USOCKET__H__STREAM_HEADER__

#define WM_SOCKET_SVR_NOTIFY					(0x0480)
#define	WM_ASK_FOR_PROCESSING					(0x0500)
#define	WM_REQUEST_PROCESSED					(0x0501)
#define	WM_WORKER_THREAD_DYING					(0x0502)
#define	WM_SET_PROCESSING_FUNC					(0x0503)
#define WM_SET_PRETRANS_FUNC					(0x0504)
#define	WM_CANCEL_REQUEST						(0x0505)
#define	WM_CONTINUE_PROCESSING					(0x0506)

#define RETURN_RESULT_RANDOM	((unsigned short)0x4000)
#define IS_ROUTING_PARTNER		((unsigned short)0x2000)

#define ERROR_NO_ERROR				0
#define ERROR_EVALUATION			(0x01)

#include "../../include/ucomm.h"

enum tagConstance {
    IO_BUFFER_SIZE = 1460 * 2,
    IO_EXTRA = 256
};

namespace SPA {

#pragma pack(push,1)

    /**
     */
    struct CQueuedRequestInfo {
        unsigned short RequestId;
        unsigned short Reserved;
        unsigned int Size;
    };
#pragma pack(pop)

#pragma pack(push,1)

    struct CSwitchInfo {

        CSwitchInfo()
        : ServiceId(SPA::sidStartup),
        MajorVersion(6),
        MinorVersion(0),
        SockMajorVersion(1),
        SockMinorVersion(8),
        Param0(0),
        Param1(SPA::GetOS()),
        Param2(0),
        SwitchTime(0) {
        }
        unsigned int ServiceId;
        unsigned short MajorVersion;
        unsigned short MinorVersion;
        unsigned short SockMajorVersion;
        unsigned short SockMinorVersion;
        unsigned int Param0;
        unsigned int Param1;
        unsigned int Param2;
        UINT64 SwitchTime;
    };

    /*
     * Treat
     * Top 1 bit for queued request 
     * The top bits (2, 3, and 4) for operation systems from left to right
     * The 5th bit for big endian
     * The 6th bit for SPECIAL
     * The 7 -- 8-th bits for default (zlib) on-line compression
     * Treat will be not set due to FastZip because max zip ratio will not over 90.
     */
    struct CStreamHeader {
    public:
        static const unsigned char IS_BIG_ENDIAN = 0x8;
        static const unsigned char QUEUE_BIT = 0x80;
        static const unsigned char OS_BITS = 0x70;
        static const unsigned char odFastZip = 64;
        static const unsigned char odZipped = 128;
        static const unsigned char IS_SPECIAL = 0x4;
        static const unsigned char ALL_ZIPPED = 0x3;
        static const unsigned char FULL_BYTE = 0xFF;

    public:

        CStreamHeader()
        : RequestId(0),
        Treat(((unsigned char) SPA::GetOS() << 4) + ((unsigned char) SPA::IsBigEndian() << 3)),
        Ratio(0),
        Size(0) {

        }

        inline bool IsSpecial(bool old = false) const {
            if (old || IsFake())
                return false;
            return ((Treat & IS_SPECIAL) == IS_SPECIAL);
        }

        inline void SetSpcial(bool special) {
            if (special)
                Treat |= IS_SPECIAL;
            else
                Treat &= (~IS_SPECIAL);
        }

        inline bool GetQueued() const {
            if (IsFake())
                return false;
            return (QUEUE_BIT == (Treat & QUEUE_BIT));
        }

        inline void SetQueued(bool bQueued) {
            if (bQueued)
                Treat |= QUEUE_BIT;
            else
                Treat &= (~QUEUE_BIT);
        }

        inline bool IsDefaultZipped(bool old = false) const {
            if (IsFake())
                return false;
            if (old)
                return (0 < (Treat & odZipped));
            return (0 < (Treat & ALL_ZIPPED) || (Ratio & odZipped) == odZipped);
        }

        inline bool IsFastZipped(bool old = false) const {
            if (IsFake())
                return false;
            if (old)
                return (0 < (Treat & odFastZip));
            return (0 < Ratio && (0 == (Treat & ALL_ZIPPED)) && (Ratio & odZipped) == 0);
        }

        inline tagOperationSystem GetOS() const {
            if (IsFake())
                return SPA::GetOS();
            return (tagOperationSystem) ((Treat & OS_BITS) >> 4);
        }

        inline bool IsBigEndian() const {
            if (IsFake())
                return SPA::IsBigEndian();
            return (IS_BIG_ENDIAN == (Treat & IS_BIG_ENDIAN));
        }

        inline void SetOS(tagOperationSystem os) {
            assert(!IsFake());
            Treat = ((unsigned char) os << 4) + (Treat & 0xF) + (Treat & QUEUE_BIT);
        }

        inline void SetBigEndian(bool be = SPA::IsBigEndian()) {
            assert(!IsFake());
            if (be)
                Treat |= IS_BIG_ENDIAN;
            else
                Treat &= (~IS_BIG_ENDIAN);
        }

        inline bool IsFake() const {
            return (0xFF == Treat && 0xFF == Ratio);
        }

        inline void MakeFake() {
            Treat = FULL_BYTE;
            Ratio = FULL_BYTE;
        }

        /*
            FastZip ratio is always less than 128. 
            DefaultZip is always less than 1020
         */

        inline void OrTreat(unsigned char treat, bool old = false, bool fast = false) {
            assert(!IsFake());
            if (old) {
                if (fast)
                    Treat = odFastZip;
                else
                    Treat = odZipped;
            } else if (treat >= 3) {
                treat = ALL_ZIPPED;
                Ratio = 0xFF;
            } else if (treat == 0) {
                if (Ratio < odZipped)
                    treat = ALL_ZIPPED;
            }
            Treat |= treat;
        }

        inline unsigned short GetZipRatio(bool old = false) {
            unsigned short s = 0;
            if (IsFake())
                return 0;
            unsigned char high = (ALL_ZIPPED & Treat);
            if (!old && ALL_ZIPPED == high) {
                if (Ratio == FULL_BYTE) {
                    s = ALL_ZIPPED;
                }
                //s still is 0
            } else
                s = high;
            s <<= 8;
            s += Ratio;
            return s;
        }

        inline void SetRatio(unsigned char r) {
            assert(!IsFake());
            Ratio = r;
        }

        unsigned short RequestId;

    private:
        unsigned char Treat;
        unsigned char Ratio;

    public:
        unsigned int Size;
    };

#pragma pack(pop)

}; //namespace SPA


#endif