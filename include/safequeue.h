#ifndef _SPA_THREAD_SAFE_DEQUEUE_H_
#define _SPA_THREAD_SAFE_DEQUEUE_H_

#include "commutil.h"

namespace SPA {

    static const size_t INITIAL_CAPACITY = 128;

    template<typename T>
    class CSafeDeque {
    public:

        CSafeDeque(size_t initailCapacity = INITIAL_CAPACITY) : m_capacity(initailCapacity), m_header(0), m_count(0) {
            if (m_capacity == 0) {
                m_capacity = INITIAL_CAPACITY;
            }
            m_p = (T*)::malloc(m_capacity * sizeof (T));
        }

        CSafeDeque(const T* p, size_t count, size_t initailCapacity = INITIAL_CAPACITY) : m_capacity(initailCapacity), m_header(0), m_count(0) {
            if (!p) {
                count = 0;
            }
            if (m_capacity == 0) {
                m_capacity = INITIAL_CAPACITY;
            }
            if (count > m_capacity) {
                m_capacity = count;
            }
            m_p = (T*)::malloc(m_capacity * sizeof (T));
            if (count) {
                ::memcpy(m_p, p, count * sizeof (T));
                m_count = count;
            }
        }

        CSafeDeque(CSafeDeque &&sd) : m_capacity(INITIAL_CAPACITY), m_header(0), m_count(0), m_p((T*)::malloc(INITIAL_CAPACITY * sizeof (T))) {
            CSpinAutoLock al(sd.m_sl);
            T *p = m_p;
            m_p = sd.m_p;
            sd.m_p = p;
            size_t size = m_count;
            m_count = sd.m_count;
            sd.m_count = size;
            size = m_header;
            m_header = sd.m_header;
            sd.m_header = size;
            size = m_capacity;
            m_capacity = sd.m_capacity;
            sd.m_capacity = size;
        }

        CSafeDeque(const CSafeDeque &sd) {
            CSpinAutoLock al(sd.m_sl);
            m_capacity = sd.m_capacity;
            m_header = sd.m_header;
            m_count = sd.m_count;
            m_p = (T*)::malloc(m_capacity * sizeof (T));
            ::memcpy(m_p + m_header, sd.m_p + m_header, m_count * sizeof (T));
        }

        ~CSafeDeque() {
            CSpinAutoLock al(m_sl);
            if (m_p) {
                ::free(m_p);
            }
        }

    public:
#if defined(MONITORING_SPIN_CONTENTION) && defined(ATOMIC_AVAILABLE)

        UINT64 contention() {
            return m_sl.Contention;
        }
#endif

        inline size_t size() const {
            m_sl.lock();
            size_t count = m_count;
            m_sl.unlock();
            return count;
        }

        inline size_t max_size() const {
            m_sl.lock();
            size_t max = m_capacity;
            m_sl.unlock();
            return max;
        }

        inline bool empty() const {
            return (!size());
        }

        void shrink_to_fit() {
            CSpinAutoLock al(m_sl);
            T *p = (T*)::malloc(m_count * sizeof (T));
            ::memcpy(p, m_p + m_header, m_count * sizeof (T));
            ::free(m_p);
            m_p = p;
            m_header = 0;
            m_capacity = m_count;
        }

        inline void push_back(const T* p, size_t count) {
            if (!p || !count) {
                return;
            }
            pushback(p, count);
        }

        inline void push_back(const T& value) {
            pushback(&value, 1);
        }

        inline void push_front(const T* p, size_t count) {
            if (!p || !count) {
                return;
            }
            pushfront(p, count);
        }

        inline void push_front(const T& value) {
            pushfront(&value, 1);
        }

        inline size_t pop_back(T* p, size_t count) {
            if (!p || !count) {
                return 0;
            }
            return popback(p, count);
        }

        inline size_t pop_back(T& value) {
            return popback(&value, 1);
        }

        inline size_t pop_front(T* p, size_t count) {
            if (!p || !count) {
                return 0;
            }
            return popfront(p, count);
        }

        inline size_t pop_front(T& value) {
            return popfront(&value, 1);
        }

        inline void clear() {
            m_sl.lock();
            m_count = 0;
            m_header = 0;
            m_sl.unlock();
        }

        void swap(CSafeDeque &sd) {
            if (this == &sd) {
                return;
            }
            CSpinAutoLock al0(sd.m_sl);
            {
                CSpinAutoLock al(m_sl);
                T *p = m_p;
                m_p = sd.m_p;
                sd.m_p = p;
                size_t size = m_count;
                m_count = sd.m_count;
                sd.m_count = size;
                size = m_header;
                m_header = sd.m_header;
                sd.m_header = size;
                size = m_capacity;
                m_capacity = sd.m_capacity;
                sd.m_capacity = size;
            }
        }

        inline CSafeDeque &operator=(CSafeDeque &&sd) {
            swap(sd);
            return *this;
        }

        CSafeDeque& operator=(const CSafeDeque &sd) {
            if (this == &sd) {
                return;
            }
            CSpinAutoLock al0(sd.m_sl);
            {
                CSpinAutoLock al(m_sl);
                if (m_p) {
                    ::free(m_p);
                }
                m_capacity = sd.m_capacity;
                m_header = sd.m_header;
                m_count = sd.m_count;
                m_p = (T*)::malloc(m_capacity * sizeof (T));
                ::memcpy(m_p + m_header, sd.m_p + m_header, m_count * sizeof (T));
            }
        }

        inline size_t push_back(const CSafeDeque &sd) {
            CSpinAutoLock al(sd.m_sl);
            if (!sd.m_p || !sd.m_count) {
                return 0;
            }
            return pushback(sd.m_p + sd.m_header, sd.m_count);
        }

        inline size_t push_front(const CSafeDeque &sd) {
            CSpinAutoLock al(sd.m_sl);
            if (!sd.m_p || !sd.m_count) {
                return 0;
            }
            return pushfront(sd.m_p + sd.m_header, sd.m_count);
        }

    private:

        size_t popfront(T* p, size_t count) {
            CSpinAutoLock al(m_sl);
            if (!m_count) {
                return 0;
            }
            if (count > m_count) {
                count = m_count;
            }
            ::memcpy(p, m_p + m_header, count * sizeof (T));
            m_count -= count;
            if (!m_count) {
                m_header = 0;
            } else {
                m_header += count;
            }
            return count;
        }

        size_t popback(T* p, size_t count) {
            CSpinAutoLock al(m_sl);
            if (!m_count) {
                return 0;
            }
            if (count > m_count) {
                count = m_count;
            }
            m_count -= count;
            ::memcpy(p, m_p + m_header + m_count, count * sizeof (T));
            if (!m_count) {
                m_header = 0;
            }
            return count;
        }

        void pushback(const T* p, size_t count) {
            CSpinAutoLock al(m_sl);
            size_t tail = m_capacity - (m_header + m_count);
            if (tail >= count) {
                ::memcpy(m_p + m_header + m_count, p, count * sizeof (T));
            } else if (tail + m_header > m_count + count) {
                ::memmove(m_p, m_p + m_header, m_count * sizeof (T));
                m_header = 0;
                ::memcpy(m_p + m_count, p, count * sizeof (T));
            } else {
                size_t newCap = count + m_capacity * 2;
                T *m = (T*)::malloc(newCap * sizeof (T));
                ::memcpy(m, m_p + m_header, m_count * sizeof (T));
                m_header = 0;
                ::free(m_p);
                m_p = m;
                ::memcpy(m_p + m_count, p, count * sizeof (T));
                m_capacity = newCap;
            }
            m_count += count;
        }

        void pushfront(const T* p, size_t count) {
            CSpinAutoLock al(m_sl);
            if (m_header >= count) {
                m_header -= count;
                ::memcpy(m_p + m_header, p, count * sizeof (T));
                m_count += count;
                return;
            }
            size_t space = m_capacity - m_count;
            if (space > count && m_count < m_capacity / 2) {
                ::memmove(m_p + m_capacity - m_count, m_p + m_header, m_count * sizeof (T));
                m_header = m_capacity - m_count - count;
                ::memcpy(m_p + m_header, p, count * sizeof (T));
                m_count += count;
            } else {
                size_t newCap = count + m_capacity * 2;
                T *m = (T*)::malloc(newCap * sizeof (T));
                size_t header = newCap - m_count - count;
                ::memcpy(m + header + count, m_p + m_header, m_count * sizeof (T));
                m_header = header;
                ::free(m_p);
                m_p = m;
                m_count += count;
                ::memcpy(m_p + header, p, count * sizeof (T));
                m_capacity = newCap;
            }
        }

    protected:
        CSpinLock m_sl;
        size_t m_capacity;
        size_t m_header;
        size_t m_count;
        T *m_p;
    };

}; //namespace SPA


#endif
