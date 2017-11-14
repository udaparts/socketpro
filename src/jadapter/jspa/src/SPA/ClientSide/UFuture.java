package SPA.ClientSide;

import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;

public class UFuture<V> {

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

    }

    public void setCanceled() {

    }

    public boolean cancel(boolean mayInterruptIfRunning) {
        return false;
    }

    public boolean isCancelled() {
        return false;
    }

    public boolean isDone() {
        return false;
    }

    public V get(long timeout, TimeUnit unit) throws TimeoutException {
        return m_v;
    }

    public V get() {
        return m_v;
    }

    //private final Lock m_lock = new Lock();
    //private final Condition m_cv;
}
