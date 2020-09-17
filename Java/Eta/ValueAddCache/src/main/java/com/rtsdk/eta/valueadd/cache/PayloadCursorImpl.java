package com.rtsdk.eta.valueadd.cache;

import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import com.rtsdk.eta.valueadd.cache.PayloadCursor;
import com.rtsdk.eta.valueadd.common.VaIteratableQueue;
import com.rtsdk.eta.valueadd.common.VaNode;

class PayloadCursorImpl extends VaNode implements PayloadCursor
{
    private boolean _isCursorDestroyed = true;
    boolean _isComplete = false;
    long _etaCursorRef = 0;

    private static VaIteratableQueue _activeCacheCursorPool = new VaIteratableQueue();
    private static VaIteratableQueue _freeCacheCursorPool = new VaIteratableQueue();
    private static Lock _globalCursorLock = new ReentrantLock();

    public PayloadCursorImpl()
    {
        _etaCursorRef = etaCreateCursor();
        if (_etaCursorRef == 0)
            throw new UnsupportedOperationException("PayloadCursorImpl constructor: cannot create eta cursor, PayloadCursor not created.");

        _isCursorDestroyed = false;
    }

    public static PayloadCursor create()
    {
        _globalCursorLock.lock();

        PayloadCursorImpl cursor = (PayloadCursorImpl)_freeCacheCursorPool.poll();
        if (cursor == null)
            cursor = new PayloadCursorImpl();
        else
        {
            cursor._isCursorDestroyed = false;
            cursor.clear();
        }

        _activeCacheCursorPool.add(cursor);

        _globalCursorLock.unlock();

        return cursor;
    }

    @Override
    public void destroy()
    {
        if (_isCursorDestroyed)
            return;

        _isCursorDestroyed = true;

        _globalCursorLock.lock();

        _activeCacheCursorPool.remove(this);
        _freeCacheCursorPool.add(this);

        _globalCursorLock.unlock();
    }

    @Override
    public void clear()
    {
        if (_isCursorDestroyed)
            return;

        _isComplete = false;
        etaClearCursor(_etaCursorRef);
    }

    @Override
    public boolean isComplete()
    {
        if (_isCursorDestroyed)
            return false;

        return _isComplete;
    }

    public boolean isDestroyed()
    {
        return _isCursorDestroyed;
    }

    public long getETACursorRef()
    {
        return _etaCursorRef;
    }

    public void setComplete(boolean isComplete)
    {
        _isComplete = isComplete;
    }

    public static void destroyAll()
    {
        PayloadCursorImpl cursor = null;
        _globalCursorLock.lock();

        while ((cursor = (PayloadCursorImpl)_freeCacheCursorPool.poll()) != null)
            cursor.etaDestroyCursor(cursor._etaCursorRef);

        while ((cursor = (PayloadCursorImpl)_activeCacheCursorPool.poll()) != null)
            cursor.etaDestroyCursor(cursor._etaCursorRef);

        _globalCursorLock.unlock();
    }

    /* **** native methods ************************************************************/

    public native long etaCreateCursor();

    public native void etaDestroyCursor(long cursorRef);

    public native void etaClearCursor(long cursorRef);
}