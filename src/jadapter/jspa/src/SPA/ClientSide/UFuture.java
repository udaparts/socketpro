package SPA.ClientSide;

import java.util.concurrent.*;
import java.util.concurrent.locks.*;

public class UFuture<V> implements Future<V> {

    // States for Future.
    public final static int PENDING = 0;
    public final static int COMPLETED = 1;
    public final static int CANCELLED = 2;

    private int m_state = PENDING;
    private V m_v = null;
    private CAsyncServiceHandler m_handler = null;

    public UFuture(CAsyncServiceHandler h) {
        m_handler = h;
    }

    public UFuture() {
    }

    public void set(V v) {
        try {
            m_lock.lock();
            if (m_state == PENDING) {
                m_v = v;
                m_state = COMPLETED;
            }
            m_cv.signalAll();
        } finally {
            m_lock.unlock();
        }
    }

    public int getState() {
        int state = PENDING;
        try {
            m_lock.lock();
            state = m_state;
        } finally {
            m_lock.unlock();
        }
        return state;
    }

    public void setCanceled() {
        try {
            m_lock.lock();
            if (m_state == PENDING) {
                m_state = CANCELLED;
            }
            m_cv.signalAll();
        } finally {
            m_lock.unlock();
        }
    }

    @Override
    public boolean cancel(boolean mayInterruptIfRunning) {
        boolean cancelled = false;
        try {
            m_lock.lock();
            if (m_state == PENDING) {
                m_state = CANCELLED;
                cancelled = true;
                if (mayInterruptIfRunning && m_handler != null) {
                    cancelled = m_handler.getAttachedClientSocket().Cancel();
                }
            }
            m_cv.signalAll();
        } finally {
            m_lock.unlock();
        }
        return cancelled;
    }

    @Override
    public boolean isCancelled() {
        boolean res = false;
        try {
            m_lock.lock();
            res = (m_state >= CANCELLED);
        } finally {
            m_lock.unlock();
        }
        return res;
    }

    @Override
    public boolean isDone() {
        boolean res = false;
        try {
            m_lock.lock();
            res = (m_state != PENDING);
        } finally {
            m_lock.unlock();
        }
        return res;
    }

    @Override
    public V get(long timeout, TimeUnit unit) throws InterruptedException, TimeoutException, ExecutionException {
        TimeoutException te = null;
        InterruptedException ie = null;
        ExecutionException ee = null;
        V v = null;
        try {
            m_lock.lock();
            if (m_state == PENDING) {
                try {
                    if (m_cv.await(timeout, unit)) {
                        if (m_state == COMPLETED) {
                            v = m_v;
                        } else {
                            ee = new ExecutionException("Task canceled", new Throwable("cancelled"));
                        }
                    } else {
                        te = new TimeoutException("UFuture timeout");
                    }
                } catch (InterruptedException err) {
                    ie = err;
                }
            } else if (m_state == COMPLETED) {
                v = m_v;
            } else {
                ee = new ExecutionException("Task already canceled", new Throwable("cancelled"));
            }
        } finally {
            m_lock.unlock();
        }
        if (te != null) {
            throw te;
        } else if (ie != null) {
            throw ie;
        } else if (ee != null) {
            throw ee;
        }
        return v;
    }

    @Override
    public V get() throws InterruptedException, ExecutionException {
        InterruptedException ie = null;
        ExecutionException ee = null;
        V v = null;
        try {
            m_lock.lock();
            if (m_state == PENDING) {
                try {
                    m_cv.await();
                    if (m_state == COMPLETED) {
                        v = m_v;
                    } else {
                        ee = new ExecutionException("Task canceled", new Throwable("cancelled"));
                    }
                } catch (InterruptedException err) {
                    ie = err;
                }
            } else if (m_state == COMPLETED) {
                v = m_v;
            } else {
                ee = new ExecutionException("Task already canceled", new Throwable("cancelled"));
            }
        } finally {
            m_lock.unlock();
        }
        if (ie != null) {
            throw ie;
        } else if (ee != null) {
            throw ee;
        }
        return v;
    }

    private final Lock m_lock = new ReentrantLock();
    private final Condition m_cv = m_lock.newCondition();
}
