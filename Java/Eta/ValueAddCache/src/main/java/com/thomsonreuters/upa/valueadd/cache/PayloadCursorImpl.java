package com.thomsonreuters.upa.valueadd.cache;

import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import com.thomsonreuters.upa.valueadd.cache.PayloadCursor;
import com.thomsonreuters.upa.valueadd.common.VaIteratableQueue;
import com.thomsonreuters.upa.valueadd.common.VaNode;

class PayloadCursorImpl extends VaNode implements PayloadCursor
{
    private boolean _isCursorDestroyed = true;
    boolean _isComplete = false;
    long _upaCursorRef = 0;

    private static VaIteratableQueue _activeCacheCursorPool = new VaIteratableQueue();
    private static VaIteratableQueue _freeCacheCursorPool = new VaIteratableQueue();
    private static Lock _globalCursorLock = new ReentrantLock();

    public PayloadCursorImpl()
    {
        _upaCursorRef = upaCreateCursor();
        if (_upaCursorRef == 0)
            throw new UnsupportedOperationException("PayloadCursorImpl constructor: cannot create upa cursor, PayloadCursor not created.");

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
        upaClearCursor(_upaCursorRef);
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

    public long getUPACursorRef()
    {
        return _upaCursorRef;
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
            cursor.upaDestroyCursor(cursor._upaCursorRef);

        while ((cursor = (PayloadCursorImpl)_activeCacheCursorPool.poll()) != null)
            cursor.upaDestroyCursor(cursor._upaCursorRef);

        _globalCursorLock.unlock();
    }

    /* **** native methods ************************************************************/

    public native long upaCreateCursor();

    public native void upaDestroyCursor(long cursorRef);

    public native void upaClearCursor(long cursorRef);
}