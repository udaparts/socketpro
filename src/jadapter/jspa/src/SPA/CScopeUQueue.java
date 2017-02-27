package SPA;

import java.util.concurrent.atomic.*;

public final class CScopeUQueue {

    private static class ConcurrentStack<T> {

        private final AtomicReference<Node<T>> top = new AtomicReference<>();

        public void push(T item) {
            Node<T> oldHead;
            Node<T> newHead = new Node<>(item);
            do {
                oldHead = top.get();
                newHead.next = oldHead;
            } while (!top.compareAndSet(oldHead, newHead));
        }

        public T pop() {
            Node<T> oldHead;
            Node<T> newHead;
            do {
                oldHead = top.get();
                if (oldHead == null) {
                    return null;
                }
                newHead = oldHead.next;
            } while (!top.compareAndSet(oldHead, newHead));
            return oldHead.item;
        }

        private static class Node<T> {

            public final T item;
            public Node<T> next;

            public Node(T item) {
                this.item = item;
            }
        }
    }

    @Override
    protected void finalize() throws Throwable {
        Clean();
        super.finalize();
    }

    public static CUQueue Lock(tagOperationSystem os) {
        CUQueue UQueue = m_sQueue.pop();
        if (UQueue == null) {
            UQueue = new CUQueue();
        }
        UQueue.setOS(os);
        return UQueue;
    }

    public static CUQueue Lock() {
        return Lock(CUQueue.DEFAULT_OS);
    }

    public static void Unlock(CUQueue UQueue) {
        if (UQueue != null) {
            UQueue.SetSize(0);
            m_sQueue.push(UQueue);
        }
    }

    public final CUQueue Detach() {
        CUQueue q = m_UQueue;
        m_UQueue = null;
        return q;
    }

    public void Clean() {
        if (m_UQueue != null) {
            Unlock(m_UQueue);
            m_UQueue = null;
        }
    }

    public final void Attach(CUQueue q) {
        Unlock(m_UQueue);
        m_UQueue = q;
    }

    public final CUQueue getUQueue() {
        return m_UQueue;
    }

    public final CScopeUQueue Save(Byte b) {
        m_UQueue.Save(b);
        return this;
    }

    public final CScopeUQueue Save(Short s) {
        m_UQueue.Save(s);
        return this;
    }

    public final CScopeUQueue Save(Integer i) {
        m_UQueue.Save(i);
        return this;
    }

    public final CScopeUQueue Save(Long i) {
        m_UQueue.Save(i);
        return this;
    }

    public final CScopeUQueue Save(Float f) {
        m_UQueue.Save(f);
        return this;
    }

    public final CScopeUQueue Save(Double d) {
        m_UQueue.Save(d);
        return this;
    }

    public final CScopeUQueue Save(Boolean b) {
        m_UQueue.Save(b);
        return this;
    }

    public final CScopeUQueue Save(IUSerializer s) {
        s.SaveTo(m_UQueue);
        return this;
    }

    public final CScopeUQueue Save(int n) {
        m_UQueue.Save(n);
        return this;
    }

    public final CScopeUQueue Save(short n) {
        m_UQueue.Save(n);
        return this;
    }

    public final CScopeUQueue Save(char c) {
        m_UQueue.Save(c);
        return this;
    }

    public final CScopeUQueue Save(byte n) {
        m_UQueue.Save(n);
        return this;
    }

    public final CScopeUQueue Save(boolean b) {
        m_UQueue.Save(b);
        return this;
    }

    public final CScopeUQueue Save(long n) {
        m_UQueue.Save(n);
        return this;
    }

    public final CScopeUQueue Save(float f) {
        m_UQueue.Save(f);
        return this;
    }

    public final CScopeUQueue Save(double d) {
        m_UQueue.Save(d);
        return this;
    }

    public final CScopeUQueue Save(String s) {
        m_UQueue.Save(s);
        return this;
    }

    public final CScopeUQueue Save(byte[] bytes) {
        m_UQueue.Save(bytes);
        return this;
    }

    public final CScopeUQueue Save(java.util.UUID id) throws IllegalArgumentException {
        m_UQueue.Save(id);
        return this;
    }

    public final CScopeUQueue Save(Object obj) throws UnsupportedOperationException {
        m_UQueue.Save(obj);
        return this;
    }

    public final CScopeUQueue Save(CUQueue q) {
        m_UQueue.Save(q);
        return this;
    }

    public final CScopeUQueue Save(java.math.BigDecimal dec) throws IllegalArgumentException {
        m_UQueue.Save(dec);
        return this;
    }

    public final CScopeUQueue Save(java.util.Date dt) throws IllegalArgumentException {
        m_UQueue.Save(dt);
        return this;
    }

    private CUQueue m_UQueue = Lock();
    private final static ConcurrentStack<CUQueue> m_sQueue = new ConcurrentStack<>();
}
