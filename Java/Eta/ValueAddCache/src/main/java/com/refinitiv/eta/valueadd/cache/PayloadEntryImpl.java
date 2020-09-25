package com.refinitiv.eta.valueadd.cache;

import java.io.PrintWriter;
import java.nio.ByteBuffer;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.ElementList;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.FieldList;
import com.refinitiv.eta.codec.Map;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.Series;
import com.refinitiv.eta.codec.Vector;
import com.refinitiv.eta.codec.FilterList;
import com.refinitiv.eta.valueadd.cache.CacheJNIBuffer;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.valueadd.cache.CacheError;
import com.refinitiv.eta.valueadd.cache.CacheFactory;
import com.refinitiv.eta.valueadd.cache.PayloadCache;
import com.refinitiv.eta.valueadd.cache.PayloadEntry;
import com.refinitiv.eta.valueadd.cache.PayloadCursor;
import com.refinitiv.eta.valueadd.common.VaNode;

class PayloadEntryImpl extends VaNode implements PayloadEntry
{
    class TraceFormat
    {
        static final int TRACE_CONTAINER = -1;
        static final int TRACE_XML = 1;
    }

    private boolean _isEntryDestroyed = true;
    private PayloadCacheImpl _cacheInstance = null;
    private long _etaCacheEntryRef = 0;
    private short _dataType = DataTypes.UNKNOWN;
    private int _majorVer = 0;
    private int _minorVer = 0;

    // Variables for trace function only
    private static int TRACE_BUF_SIZE = 6144;
    private FieldList _fieldList = null;
    private ElementList _elementList = null;
    private Map _map = null;
    private Series _series = null;
    private Vector _vector = null;
    private FilterList _filterList = null;
    private DecodeIterator _traceDIter = null;
    private Buffer _traceBuffer = null;
    private PayloadCursorImpl _traceCursor = null;

    public PayloadEntryImpl(PayloadCache cacheInstance, long etaCacheEntryRef)
    {
        _cacheInstance = (PayloadCacheImpl)cacheInstance;
        _etaCacheEntryRef = etaCacheEntryRef;

        _isEntryDestroyed = false;
    }

    public static PayloadEntry create(PayloadCache cacheInstance, CacheError error)
    {
        long etaCacheEntryRef = ((PayloadCacheImpl)cacheInstance).createCacheEntry(error);
        if (etaCacheEntryRef != 0)
        {
            PayloadEntryImpl entry = new PayloadEntryImpl(cacheInstance, etaCacheEntryRef);

            ((PayloadCacheImpl)cacheInstance).addCacheEntry(entry);
            return entry;
        }
        else
            return null;
    }

    @Override
    public void destroy()
    {
        if (_isEntryDestroyed)
            return;

        etaDestroyEntry(_etaCacheEntryRef);

        _cacheInstance.removeCacheEntry(this);

        _isEntryDestroyed = true;
    }

    @Override
    public void clear()
    {
        if (_isEntryDestroyed)
            return;

        _dataType = DataTypes.UNKNOWN;

        etaClearEntry(_etaCacheEntryRef);
    }

    @Override
    public short dataType()
    {
        if (_isEntryDestroyed)
            return DataTypes.UNKNOWN;

        return _dataType;

    }

    @Override
    public int apply(DecodeIterator dIter, Msg msg, CacheError error)
    {
        if (error == null)
            throw new UnsupportedOperationException("PayloadEntryImpl.apply: error cannot be null, data not apply.");

        if (_isEntryDestroyed)
            return PayloadCacheImpl.populateErrorInfo((CacheErrorImpl)error, CodecReturnCodes.INVALID_DATA,
                                                      "PayloadCacheEntryImpl.apply error: the cache instance has been destroyed.");

        if (dIter == null || msg == null)
            return PayloadCacheImpl.populateErrorInfo((CacheErrorImpl)error, CodecReturnCodes.INVALID_ARGUMENT,
                                                      "PayloadCacheEntryImpl.apply error: dIter or msg cannot be null, data not apply.");

        int dataLen = msg.encodedMsgBuffer().length();
        CacheJNIBuffer applyBuffer = _cacheInstance.acquireCacheApplyJNIBuffer(dataLen, error);
        if (applyBuffer == null)
            return PayloadCacheImpl.populateErrorInfo((CacheErrorImpl)error, CodecReturnCodes.FAILURE,
                                                      "PayloadCacheEntryImpl.apply error: unable to create jni buffer, data not apply.");
        else
        {
            Buffer encodedBuf = msg.encodedMsgBuffer();
            encodedBuf.copy(applyBuffer._data);
        }

        _majorVer = dIter.majorVersion();
        _minorVer = dIter.minorVersion();
        int ret = etaApply(_etaCacheEntryRef, applyBuffer._upaBufferCPtr, dataLen, _majorVer, _minorVer, error);

        if (ret < CodecReturnCodes.SUCCESS && ret == error.errorId()) // is error, not warning
        {
            _cacheInstance.releaseCacheApplyJNIBuffer(applyBuffer);
            return ret;
        }

        _dataType = (short)msg.containerType();

        _cacheInstance.releaseCacheApplyJNIBuffer(applyBuffer);

        return ret;
    }

    @Override
    public int retrieve(EncodeIterator eIter, PayloadCursor cursor, CacheError error)
    {
        if (error == null)
            throw new UnsupportedOperationException("PayloadEntryImpl.retrieve: error cannot be null, unable receive data.");

        if (_isEntryDestroyed)
            return PayloadCacheImpl.populateErrorInfo((CacheErrorImpl)error, CodecReturnCodes.INVALID_DATA,
                                                      "PayloadCacheEntryImpl.retrieve error: the cache instance has been destroyed.");

        if (eIter == null)
            return PayloadCacheImpl.populateErrorInfo((CacheErrorImpl)error, CodecReturnCodes.INVALID_ARGUMENT,
                                                      "PayloadCacheEntryImpl.retrieve error: eIter cannot be null, unable receive data.");

        int fragmentSize = 0;
        ByteBuffer readByteBuffer = null;
        Buffer readBuffer = null;
        TransportBuffer readTransportBuffer = null;
        if ((readTransportBuffer = eIter.transportBuffer()) != null)
        {
            readByteBuffer = readTransportBuffer.data();
            fragmentSize = readByteBuffer.remaining();
        }
        else if ((readBuffer = eIter.buffer()) != null)
        {
            readByteBuffer = readBuffer.data();
            fragmentSize = readByteBuffer.remaining();
        }
        if (fragmentSize == 0)
            return PayloadCacheImpl.populateErrorInfo((CacheErrorImpl)error, CodecReturnCodes.INVALID_DATA,
                                                      "PayloadCacheEntryImpl.retrieve error: invalid buffer tied with eIter, unable receive data.");

        CacheJNIBuffer retrieveBuffer = _cacheInstance.acquireCacheRetrieveJNIBuffer(fragmentSize, error);
        if (retrieveBuffer == null)
            return PayloadCacheImpl.populateErrorInfo((CacheErrorImpl)error, CodecReturnCodes.FAILURE,
                                                      "PayloadCacheEntryImpl.retrieve error: unable to create jni buffer, unable receive data.");

        PayloadCursorImpl cursorUsed = (PayloadCursorImpl)cursor;
        if (cursorUsed != null && cursorUsed.isDestroyed())
            return PayloadCacheImpl.populateErrorInfo((CacheErrorImpl)error, CodecReturnCodes.INVALID_ARGUMENT,
                                                      "PayloadCacheEntryImpl.retrieve error: invalid cursor, unable receive data.");

        // may need to consider how to handle rwf version difference between eIter and eIter from apply function

        int ret = etaRetrieve(_etaCacheEntryRef, retrieveBuffer._upaBufferCPtr, fragmentSize,
                              eIter.majorVersion(), eIter.minorVersion(),
                              cursorUsed, (cursorUsed != null ? cursorUsed.getETACursorRef() : 0), error);

        if (ret < CodecReturnCodes.SUCCESS)
        {
            _cacheInstance.releaseCacheRetrieveJNIBuffer(retrieveBuffer);
            return ret;
        }

        retrieveBuffer._data.position(0);
        retrieveBuffer._data.limit(ret);

        readByteBuffer.put(retrieveBuffer._data);

        _cacheInstance.releaseCacheRetrieveJNIBuffer(retrieveBuffer);
        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int trace(int traceFormat, PrintWriter fileWriter, DataDictionary dictionary)
    {
        if (_isEntryDestroyed || fileWriter == null || dictionary == null || _dataType == DataTypes.NO_DATA)
            return CodecReturnCodes.FAILURE;

        if (_traceCursor == null)
        {
            _traceCursor = (PayloadCursorImpl)CacheFactory.createPayloadCursor();
            if (_traceCursor == null)
                return CodecReturnCodes.FAILURE;
        }
        else
            _traceCursor.clear();

        CacheJNIBuffer retrieveBuffer = _cacheInstance.acquireCacheRetrieveJNIBuffer(TRACE_BUF_SIZE, null);
        if (retrieveBuffer == null)
            return CodecReturnCodes.FAILURE;

        int ret = CodecReturnCodes.FAILURE;

        if (_traceDIter == null)
        {
            _traceDIter = CodecFactory.createDecodeIterator();
            _traceBuffer = CodecFactory.createBuffer();
        }

        if (traceFormat == TraceFormat.TRACE_XML)
        {
            while (!_traceCursor.isComplete())
            {
                retrieveBuffer.clear();
                ret = etaRetrieve(_etaCacheEntryRef, retrieveBuffer._upaBufferCPtr, TRACE_BUF_SIZE,
                                  _majorVer, _minorVer,
                                  _traceCursor, _traceCursor.getETACursorRef(), null);

                if (ret < CodecReturnCodes.SUCCESS)
                {
                    _cacheInstance.releaseCacheRetrieveJNIBuffer(retrieveBuffer);
                    return ret;
                }
                else
                {
                    retrieveBuffer._data.position(0);
                    retrieveBuffer._data.limit(ret);

                    _traceBuffer.data(retrieveBuffer._data);
                    _traceDIter.clear();
                    _traceDIter.setBufferAndRWFVersion(_traceBuffer, _majorVer, _minorVer);

                    switch (_dataType)
                    {
                        case DataTypes.FIELD_LIST:
                        {
                            if (_fieldList == null)
                                _fieldList = CodecFactory.createFieldList();
                            else
                                _fieldList.clear();

                            fileWriter.printf(_fieldList.decodeToXml(_traceDIter, dictionary));
                            break;
                        }
                        case DataTypes.ELEMENT_LIST:
                        {
                            if (_elementList == null)
                                _elementList = CodecFactory.createElementList();
                            else
                                _elementList.clear();

                            fileWriter.printf(_elementList.decodeToXml(_traceDIter, dictionary));
                            break;
                        }
                        case DataTypes.MAP:
                        {
                            if (_map == null)
                                _map = CodecFactory.createMap();
                            else
                                _map.clear();

                            fileWriter.printf(_map.decodeToXml(_traceDIter, dictionary));
                            break;
                        }
                        case DataTypes.VECTOR:
                        {
                            if (_vector == null)
                                _vector = CodecFactory.createVector();
                            else
                                _vector.clear();

                            fileWriter.printf(_vector.decodeToXml(_traceDIter, dictionary));
                            break;
                        }
                        case DataTypes.SERIES:
                        {
                            if (_series == null)
                                _series = CodecFactory.createSeries();
                            else
                                _series.clear();

                            fileWriter.printf(_series.decodeToXml(_traceDIter, dictionary));
                            break;
                        }
                        case DataTypes.FILTER_LIST:
                        {
                            if (_filterList == null)
                                _filterList = CodecFactory.createFilterList();
                            else
                                _filterList.clear();

                            fileWriter.printf(_filterList.decodeToXml(_traceDIter, dictionary));
                            break;
                        }
                        default:
                        {
                            System.out.println("PayloadEntryImpl.trace error: unsupported data type.");
                            ret = CodecReturnCodes.FAILURE;
                            break;
                        }
                    }

                    fileWriter.flush();
                }
            }
        }
        else
        {
            System.out.println("PayloadEntryImpl.trace error: unsupported trace format.");
            ret = CodecReturnCodes.FAILURE;
        }

        _cacheInstance.releaseCacheRetrieveJNIBuffer(retrieveBuffer);

        return ret >= 0 ? CodecReturnCodes.SUCCESS : CodecReturnCodes.FAILURE;
    }
	
    public void applyDestroy()
    {
        _isEntryDestroyed = true;
    }	
	
    /* **** native methods ************************************************************/

    public native void etaClearEntry(long entryRef);

    public native void etaDestroyEntry(long entryRef);

    public native int etaApply(long etaEntryRef, long etaApplyBufRef, int dataLen,
                               int majorVersion, int minorVersion, CacheError error);
 
    public native int etaRetrieve(long etaEntryRef, long etaRetrieveBufRef, int etaRetrieveBufLen,
                                  int majorVersion, int minorVersion,
                                  PayloadCursorImpl cursor, long etaCursorRef, CacheError error);
}