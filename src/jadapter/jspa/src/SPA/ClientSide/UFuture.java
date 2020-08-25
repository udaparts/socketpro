package SPA.ClientSide;

import java.util.concurrent.*;
import java.util.concurrent.locks.*;

public class UFuture<V> implements Future<V>, IUFExtra, AutoCloseable {

    // States for Future.
    public final static int PENDING = 0;
    public final static int COMPLETED = 1;
    public final static int CANCELLED = 2;
    public final static int EXCEPTION = 3;

    private int m_state = PENDING;
    private V m_v = null;
    private final String m_mName;
    private final short m_reqId;
    private CAsyncServiceHandler m_handler = null;
    private SPA.CServerError m_se = null;
    private CSocketError m_ce = null;

    /**
     * Create an instance of UFuture
     *
     * @param mName A required request method name used for constructing error
     * message
     * @param reqId A required request id which cannot be zero
     * @param h An optional async service handler.
     */
    public UFuture(String mName, short reqId, CAsyncServiceHandler h) {
        if (mName == null || mName.length() == 0) {
            throw new IllegalArgumentException("Method name cannot be empty");
        }
        if (reqId == 0) {
            throw new IllegalArgumentException("Request id cannot be zero");
        }
        m_mName = mName;
        m_reqId = reqId;
        m_handler = h;
    }

    /**
     * Create an instance of UFuture
     *
     * @param mName A required request method name used for constructing error
     * message
     * @param reqId A required request id which cannot be zero
     */
    public UFuture(String mName, short reqId) {
        if (mName == null || mName.length() == 0) {
            throw new IllegalArgumentException("Method name cannot be empty");
        }
        if (reqId == 0) {
            throw new IllegalArgumentException("Request id cannot be zero");
        }
        m_mName = mName;
        m_reqId = reqId;
    }

    @Override
    public final String getMethodName() {
        return m_mName;
    }

    @Override
    public final short getReqId() {
        return m_reqId;
    }

    @Override
    public void close() {
        m_v = null;
        m_handler = null;
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

    @Override
    public void setException(SPA.CServerError ex) {
        if (ex == null) {
            throw new IllegalArgumentException("Parameter ex cannot be null");
        }
        try {
            m_lock.lock();
            if (m_state == PENDING) {
                m_se = ex;
                m_state = EXCEPTION;
            }
            m_cv.signalAll();
        } finally {
            m_lock.unlock();
        }
    }

    @Override
    public void setException(CSocketError ex) {
        if (ex == null) {
            throw new IllegalArgumentException("Parameter ex cannot be null");
        }
        try {
            m_lock.lock();
            if (m_state == PENDING) {
                m_ce = ex;
                m_state = EXCEPTION;
            }
            m_cv.signalAll();
        } finally {
            m_lock.unlock();
        }
    }

    @Override
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

    /**
     * Cancel the future if its state is pending
     *
     * @param mayInterruptIfRunning A boolean value. If it is true and its
     * handler is set, calling this method will automatically send an interrupt
     * request to remote server with the option value
     * CAsyncServiceHandler.DEFAULT_INTERRUPT_OPTION
     * @return True if successful. Otherwise, it returns false
     */
    @Override
    public boolean cancel(boolean mayInterruptIfRunning) {
        boolean cancelled = false;
        try {
            m_lock.lock();
            if (m_state == PENDING) {
                m_state = CANCELLED;
                cancelled = true;
                if (mayInterruptIfRunning && m_handler != null) {
                    cancelled = m_handler.Interrupt(CAsyncServiceHandler.DEFAULT_INTERRUPT_OPTION);
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
            res = (m_state == CANCELLED);
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
    public V get(long timeout, TimeUnit unit) throws SPA.CServerError, TimeoutException, CSocketError {
        TimeoutException te = null;
        SPA.CServerError se = null;
        CSocketError ce = null;
        V v = null;
        try {
            m_lock.lock();
            if (m_state == PENDING) {
                try {
                    if (m_cv.await(timeout, unit)) {
                        if (m_state == COMPLETED) {
                            v = m_v;
                        } else if (m_state == EXCEPTION) {
                            if (m_se != null) {
                                se = m_se;
                            } else {
                                ce = m_ce;
                            }
                        } else {
                            ce = new CSocketError(CAsyncServiceHandler.REQUEST_CANCELED, "Request " + m_mName + " canceled", m_reqId, false);
                        }
                    } else {
                        te = new TimeoutException("UFuture timeout");
                    }
                } catch (InterruptedException err) {
                    ce = new CSocketError(CAsyncServiceHandler.REQUEST_CANCELED, "Request " + m_mName + " interrupted", m_reqId, false);
                }
            } else if (m_state == COMPLETED) {
                v = m_v;
            } else if (m_state == EXCEPTION) {
                if (m_se != null) {
                    se = m_se;
                } else {
                    ce = m_ce;
                }
            } else {
                ce = new CSocketError(CAsyncServiceHandler.REQUEST_CANCELED, "Request " + m_mName + " canceled", m_reqId, false);
            }
        } finally {
            m_lock.unlock();
        }
        if (te != null) {
            throw te;
        } else if (ce != null) {
            throw ce;
        } else if (se != null) {
            throw se;
        }
        return v;
    }

    @Override
    public V get() throws SPA.CServerError, CSocketError {
        SPA.CServerError se = null;
        CSocketError ce = null;
        V v = null;
        try {
            m_lock.lock();
            if (m_state == PENDING) {
                try {
                    m_cv.await();
                    if (m_state == COMPLETED) {
                        v = m_v;
                    } else if (m_state == EXCEPTION) {
                        if (m_se != null) {
                            se = m_se;
                        } else {
                            ce = m_ce;
                        }
                    } else {
                        ce = new CSocketError(CAsyncServiceHandler.REQUEST_CANCELED, "Request " + m_mName + " canceled", m_reqId, false);
                    }
                } catch (InterruptedException err) {
                    ce = new CSocketError(CAsyncServiceHandler.REQUEST_CANCELED, "Request " + m_mName + " interrupted", m_reqId, false);
                }
            } else if (m_state == COMPLETED) {
                v = m_v;
            } else if (m_state == EXCEPTION) {
                if (m_se != null) {
                    se = m_se;
                } else {
                    ce = m_ce;
                }
            } else {
                ce = new CSocketError(CAsyncServiceHandler.REQUEST_CANCELED, "Request " + m_mName + " canceled", m_reqId, false);
            }
        } finally {
            m_lock.unlock();
        }
        if (ce != null) {
            throw ce;
        } else if (se != null) {
            throw se;
        }
        return v;
    }

    private final Lock m_lock = new ReentrantLock();
    private final Condition m_cv = m_lock.newCondition();
}
