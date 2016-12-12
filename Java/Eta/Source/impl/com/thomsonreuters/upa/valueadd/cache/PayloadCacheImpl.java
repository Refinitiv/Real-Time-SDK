package com.thomsonreuters.upa.valueadd.cache;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import com.thomsonreuters.upa.rdm.Dictionary;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.valueadd.cache.CacheDictLoadingHelper;
import com.thomsonreuters.upa.valueadd.cache.CacheError;
import com.thomsonreuters.upa.valueadd.cache.PayloadCache;
import com.thomsonreuters.upa.valueadd.cache.PayloadCacheConfigOptions;
import com.thomsonreuters.upa.valueadd.cache.PayloadEntry;
import com.thomsonreuters.upa.valueadd.common.VaConcurrentQueue;
import com.thomsonreuters.upa.valueadd.common.VaIteratableQueue;
import com.thomsonreuters.upa.valueadd.common.VaNode;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StreamStates;

class PayloadCacheImpl extends VaNode implements PayloadCache
{
    class KeyUPADictRefMap extends VaNode
    {
        String _dictKey = null;
        long _upaDictRef = 0;

        KeyUPADictRefMap(String dictKey, long upaDictRef)
        {
            _dictKey = new String(dictKey);
            _upaDictRef = upaDictRef;
        }
    }

    private final static int DICT_INCREASE_SIZE = 2000;
    private final static int DEFAULT_BUFFER_SIZE = 500;
    private final static int MAX_BUFFER_ARRAY_SIZE = 21;
    private final static int MAX_BUFFER_ARRAY_POS = 20;
    private final static int MAX_FIXED_BUFFER_SIZE = DEFAULT_BUFFER_SIZE * MAX_BUFFER_ARRAY_POS;

    // flag to indicate is UPA JNI cache is initialized and UPA JNI library is loaded
    private static boolean _cacheInitialized = false;
    private static VaIteratableQueue _globalDictDbList = null;
    private static Lock _globalDictLock = new ReentrantLock();
    private static Lock _globalCacheLock = new ReentrantLock();
    private static VaConcurrentQueue _globalCacheList = new VaConcurrentQueue();

    private boolean _isCacheDestroyed = true;
    private VaIteratableQueue _cacheEntryList = null;
    private List<PayloadEntry> _appCacheEntryList = null;
    private String _dictKey = null;
    private boolean _dictKeyCleared = false;
    private long _upaCacheRef = 0;
    private CacheDictLoadingHelper _dictLoadingHelper = null;
    private VaIteratableQueue[] _retrieveJNIBufferArray = new VaIteratableQueue[MAX_BUFFER_ARRAY_SIZE];
    private VaIteratableQueue[] _applyJNIBufferArray = new VaIteratableQueue[MAX_BUFFER_ARRAY_SIZE];
    private int _maxRetrieveJNIBufferArrayPos = 3;
    private int _maxApplyJNIBufferArrayPos = 3;

    static
    {
        System.loadLibrary("rsslVACacheJNI");
    }

    public PayloadCacheImpl(long upaCacheInstance, PayloadCacheConfigOptions configOptions)
    {
        _upaCacheRef = upaCacheInstance;
        _cacheEntryList = new VaIteratableQueue();

        _globalCacheLock.lock();
        _globalCacheList.add(this);
        _globalCacheLock.unlock();

        _isCacheDestroyed = false;

        initializePooling();
    }

    public static PayloadCache create(PayloadCacheConfigOptions configOptions, CacheError error)
    {
        _globalCacheLock.lock();

        // initialize UPA JNI
        if (!_cacheInitialized)
        {
            if (upaInitializeJNICache() == CodecReturnCodes.FAILURE)
                throw new UnsupportedOperationException("PayloadCacheImpl: JNI upaInitializeJNICache() failed.");

            // set flag to indicate initialization is completed
            _cacheInitialized = true;
        }
        _globalCacheLock.unlock();

        long upaCacheInstance = upaCreateCache(configOptions.maxItems(), error);
        if (upaCacheInstance != 0)
            return new PayloadCacheImpl(upaCacheInstance, configOptions);

        return null;
    }

    @Override
    public void destroy()
    {
        if (_isCacheDestroyed)
            return;

        if (_upaCacheRef != 0)
        {
            destroyPayloadEntries();
            upaDestroyCache(_upaCacheRef);

            // release all memory from upac
            CacheJNIBuffer buffer = null;
            VaIteratableQueue flexLenBufList = null;
            for (int pos = 0; pos <= _maxRetrieveJNIBufferArrayPos; ++pos)
            {
                flexLenBufList = _retrieveJNIBufferArray[pos];
                flexLenBufList.rewind();
                while (flexLenBufList.hasNext())
                {
                    buffer = (CacheJNIBuffer)flexLenBufList.next();
                    upaFreeRsslBuffer(buffer._upaBufferCPtr);
                }
            }

            for (int pos = 0; pos <= _maxApplyJNIBufferArrayPos; ++pos)
            {
                flexLenBufList = _applyJNIBufferArray[pos];
                flexLenBufList.rewind();
                while (flexLenBufList.hasNext())
                {
                    buffer = (CacheJNIBuffer)flexLenBufList.next();
                    upaFreeRsslBuffer(buffer._upaBufferCPtr);
                }
            }
        }

        _globalCacheLock.lock();
        _globalCacheList.remove(this);
        if (_cacheInitialized && _globalCacheList.size() == 0)
        {
            uninitialize();

            upaUninitializeJNICache();
            _cacheInitialized = false;

        }
        _globalCacheLock.unlock();

        _isCacheDestroyed = true;
    }

    @Override
    public void destroyAll()
    {
        if (_isCacheDestroyed)
            return;

        _globalCacheLock.lock();

        int cacheCount = _globalCacheList.size();

        if (cacheCount == 0)
        {
            _globalCacheLock.unlock();
            return;
        }

        while (cacheCount-- > 0)
        {
            PayloadCacheImpl cache = (PayloadCacheImpl)_globalCacheList.poll();
            _globalCacheLock.unlock();
            cache.destroy();
            _globalCacheLock.lock();
        }

        _globalCacheLock.unlock();
    }

    @Override
    public int setDictionary(DataDictionary fidDictionary, String dictionaryKey, CacheError error)
    {
        if (error == null)
            throw new UnsupportedOperationException("PayloadCacheImpl.bindDictionary: error cannot be null, dictionary not bind.");

        if (_isCacheDestroyed)
            return populateErrorInfo((CacheErrorImpl)error, CodecReturnCodes.INVALID_DATA,
                                     "PayloadCacheImpl.bindDictionary error: the cache instance has been destroyed.");

        if (dictionaryKey == null || fidDictionary == null)
            return populateErrorInfo((CacheErrorImpl)error, CodecReturnCodes.INVALID_ARGUMENT,
                                     "PayloadCacheImpl.bindDictionary error: dictionaryKey or fidDictionary cannot be null, dictionary not bind.");

        if (_dictLoadingHelper == null)
        {
            _dictLoadingHelper = new CacheDictLoadingHelper();
            if (_globalDictDbList == null)
                _globalDictDbList = new VaIteratableQueue();
        }

        // dict bind again
        if (!_dictKeyCleared && _dictKey != null && !_dictKey.equals(dictionaryKey))
            return populateErrorInfo((CacheErrorImpl)error, CodecReturnCodes.INVALID_ARGUMENT,
                                     "PayloadCacheImpl.bindDictionary error: not allow to reload dictonary with new key.");

        if (!encodedFidDictBuf(fidDictionary))
            return populateErrorInfo((CacheErrorImpl)error, CodecReturnCodes.FAILURE,
                                     "PayloadCacheImpl.bindDictionary error: unable to get encoded dictionary buffer.");

        int bindRet = CodecReturnCodes.FAILURE;
        long upaDictDbRef = 0;
        KeyUPADictRefMap globalDictDb = getDictDb(dictionaryKey);
        if (globalDictDb == null) // need create one
        {
            // find none, create one
            upaDictDbRef = upaCreateFIDDictionary(dictionaryKey);

            if (upaDictDbRef == 0)
                return populateErrorInfo((CacheErrorImpl)error, CodecReturnCodes.FAILURE,
                                         "PayloadCacheImpl.bindDictionary error: unable to create upa dictonary with dictionaryKey.");

            // first time bind
            bindRet = upaLoadDictionaryFromBuffer(upaDictDbRef, _dictLoadingHelper._dictEncodedBuf._upaBufferCPtr,
                                                  _dictLoadingHelper._dictEncodedBufWrap.length(), error);

            upaFreeRsslBuffer(_dictLoadingHelper._dictEncodedBuf._upaBufferCPtr);

            if (bindRet < CodecReturnCodes.SUCCESS)
                return bindRet;

            globalDictDb = new KeyUPADictRefMap(dictionaryKey, upaDictDbRef);
            _globalDictLock.lock();
            _globalDictDbList.add(globalDictDb);
            _globalDictLock.unlock();
        }
        else
        {
            // could be first time load (loaded by other cache first) or reload
            bindRet = upaLoadDictionaryFromBuffer(globalDictDb._upaDictRef, _dictLoadingHelper._dictEncodedBuf._upaBufferCPtr,
                                                  _dictLoadingHelper._dictEncodedBufWrap.length(), error);

            upaFreeRsslBuffer(_dictLoadingHelper._dictEncodedBuf._upaBufferCPtr);

            if (bindRet < CodecReturnCodes.SUCCESS)
                return bindRet;

            upaDictDbRef = globalDictDb._upaDictRef;
        }

        bindRet = upaSetFidDbWithCache(_upaCacheRef, upaDictDbRef, dictionaryKey, error);
        if (bindRet < CodecReturnCodes.SUCCESS)
            return bindRet;

        if (_dictKey == null || _dictKeyCleared) // first time bind or reuse cache
        {
            _dictKey = globalDictDb._dictKey;
            _dictKeyCleared = false;
        }

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int setSharedDictionaryKey(String dictionaryKey, CacheError error)
    {
        if (error == null)
            throw new UnsupportedOperationException("PayloadCacheImpl.bindDictionary: error cannot be null, dictionary not bind.");

        if (dictionaryKey == null)
            return populateErrorInfo((CacheErrorImpl)error, CodecReturnCodes.INVALID_ARGUMENT,
                                     "PayloadCacheImpl.bindSharedDictionaryKey error: dictionaryKey cannot be null, dictionary not bind.");

        if (_isCacheDestroyed)
            return populateErrorInfo((CacheErrorImpl)error, CodecReturnCodes.INVALID_DATA,
                                     "PayloadCacheImpl.bindSharedDictionaryKey error: the cache instance has been destroyed.");

        if (_dictLoadingHelper == null)
        {
            _dictLoadingHelper = new CacheDictLoadingHelper();
            if (_globalDictDbList == null)
                _globalDictDbList = new VaIteratableQueue();
        }

        // check if it is reload. Key must match if reload dictionary
        if (!_dictKeyCleared && _dictKey != null)
        {
            if (!_dictKey.equals(dictionaryKey))
                return populateErrorInfo((CacheErrorImpl)error, CodecReturnCodes.INVALID_ARGUMENT,
                                         "PayloadCacheImpl.bindSharedDictionaryKey error: not allow to reload dictonary with new key.");
            else
                // already load, do nothing
                return CodecReturnCodes.SUCCESS;
        }

        KeyUPADictRefMap globalDictDb = getDictDb(dictionaryKey);
        // the shared dictionary is not available
        if (globalDictDb == null)
            return populateErrorInfo((CacheErrorImpl)error, CodecReturnCodes.INVALID_ARGUMENT,
                                     "PayloadCacheImpl.bindSharedDictionaryKey error: the shared dictionary is not available, dictionary not bind.");

        int bindRet = upaSetFidDbWithCache(_upaCacheRef, globalDictDb._upaDictRef, dictionaryKey, error);

        if (bindRet < CodecReturnCodes.SUCCESS)
            return bindRet;

        _dictKey = globalDictDb._dictKey;
        _dictKeyCleared = false;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int entryCount()
    {
        if (_isCacheDestroyed)
            return 0;

        return _cacheEntryList.size();
    }

    @Override
    public List<PayloadEntry> entryList()
    {
        if (_isCacheDestroyed)
            return null;

        int entryNum = _cacheEntryList.size();
        if (entryNum == 0)
            return null;

        if (_appCacheEntryList == null)
            _appCacheEntryList = new ArrayList<PayloadEntry>(entryNum);
        else
            _appCacheEntryList.clear();

        _cacheEntryList.rewind();
        while (_cacheEntryList.hasNext())
            _appCacheEntryList.add((PayloadEntry)_cacheEntryList.next());

        return _appCacheEntryList;
    }

    @Override
    public void clear()
    {
        if (_isCacheDestroyed)
            return;

        // destroy all entries
        destroyPayloadEntries();

        // so no need to set _dictKey to null
        _dictKeyCleared = true;
    }

    public static int populateErrorInfo(CacheErrorImpl errorInfo, int returnCode, String text)
    {
        errorInfo.errorId(returnCode);
        errorInfo.text(text);

        return returnCode;
    }

    private KeyUPADictRefMap getDictDb(String dictionaryKey)
    {
        KeyUPADictRefMap dictKeyUpaDict = null;

        _globalDictLock.lock();

        _globalDictDbList.rewind();
        while (_globalDictDbList.hasNext())
        {
            dictKeyUpaDict = (KeyUPADictRefMap)_globalDictDbList.next();
            if (dictKeyUpaDict._dictKey.equals(dictionaryKey))
            {
                _globalDictLock.unlock();
                return dictKeyUpaDict;
            }
        }

        _globalDictLock.unlock();
        return null;
    }

    private boolean encodedFidDictBuf(DataDictionary dict)
    {
        int dictSize = dict.numberOfEntries() * 50;
        _dictLoadingHelper._dictMinFidInt.value(dict.minFid());

        // get memory for holding dict bytes from upac, then release the memory after binding.
        long upaBufferRef = upaCreateRsslBuffer(_dictLoadingHelper._dictEncodedBuf, dictSize, null);
        if (upaBufferRef == 0)
            return false;
        else
            _dictLoadingHelper._dictEncodedBufWrap.data(_dictLoadingHelper._dictEncodedBuf._data);

        _dictLoadingHelper._dictMsg.clear();
        _dictLoadingHelper._dictMsg.msgClass(MsgClasses.REFRESH);
        _dictLoadingHelper._dictMsg.applyRefreshComplete();
        _dictLoadingHelper._dictMsg.domainType(DomainTypes.DICTIONARY);
        _dictLoadingHelper._dictMsg.applySolicited();
        _dictLoadingHelper._dictMsg.containerType(DataTypes.SERIES);
        _dictLoadingHelper._dictMsg.state().streamState(StreamStates.OPEN);
        _dictLoadingHelper._dictMsg.state().dataState(DataStates.OK);
        _dictLoadingHelper._dictMsg.state().code(StateCodes.NONE);
        _dictLoadingHelper._dictMsg.state().text().data("Field Dictionary Refresh complete");

        _dictLoadingHelper._dictMsg.applyHasMsgKey();
        _dictLoadingHelper._dictMsg.msgKey().applyHasName();
        _dictLoadingHelper._dictMsg.msgKey().name(_dictLoadingHelper._dictName);
        _dictLoadingHelper._dictMsg.msgKey().applyHasFilter();
        _dictLoadingHelper._dictMsg.msgKey().filter(Dictionary.VerbosityValues.NORMAL);

        _dictLoadingHelper._dictMsg.msgKey().applyHasServiceId();
        _dictLoadingHelper._dictMsg.msgKey().serviceId(0);

        _dictLoadingHelper._dictMsg.streamId(3);

        while (true)
        {
            _dictLoadingHelper._dictEncodeIter.setBufferAndRWFVersion(_dictLoadingHelper._dictEncodedBufWrap,
                                                                      Codec.majorVersion(), Codec.minorVersion());
            if (_dictLoadingHelper._dictMsg.encodeInit(_dictLoadingHelper._dictEncodeIter, 0) == CodecReturnCodes.FAILURE)
            {
                upaFreeRsslBuffer(_dictLoadingHelper._dictEncodedBuf._upaBufferCPtr);
                return false;
            }
            int ret = dict.encodeFieldDictionary(_dictLoadingHelper._dictEncodeIter, _dictLoadingHelper._dictMinFidInt,
                                                 Dictionary.VerbosityValues.NORMAL, _dictLoadingHelper._dictError);

            if (ret == CodecReturnCodes.DICT_PART_ENCODED)
            {
                upaFreeRsslBuffer(_dictLoadingHelper._dictEncodedBuf._upaBufferCPtr);
                dictSize = dictSize + DICT_INCREASE_SIZE;
                upaBufferRef = upaCreateRsslBuffer(_dictLoadingHelper._dictEncodedBuf, dictSize, null);
                if (upaBufferRef == 0)
                    return false;
                else
                    _dictLoadingHelper._dictEncodedBufWrap.data(_dictLoadingHelper._dictEncodedBuf._data);
            }
            else if (ret == CodecReturnCodes.FAILURE)
            {
                upaFreeRsslBuffer(_dictLoadingHelper._dictEncodedBuf._upaBufferCPtr);
                return false;
            }
            else
            {
                if (_dictLoadingHelper._dictMsg.encodeComplete(_dictLoadingHelper._dictEncodeIter, true) == CodecReturnCodes.FAILURE)
                {
                    upaFreeRsslBuffer(_dictLoadingHelper._dictEncodedBuf._upaBufferCPtr);
                    return false;
                }
                return true;
            }
        }
    }

    private void uninitialize()
    {
        // need to let all global dictionary db go
        _globalDictLock.lock();

        KeyUPADictRefMap one = null;
        while ((one = (KeyUPADictRefMap)_globalDictDbList.poll()) != null)
            one._dictKey = null;

        _globalDictLock.unlock();

        PayloadCursorImpl.destroyAll();
    }

    private void destroyPayloadEntries()
    {
        int cacheCount = _cacheEntryList.size();
        if (cacheCount == 0)
            return;

        PayloadEntryImpl next = null;
        while (cacheCount-- > 0)
        {
            next = (PayloadEntryImpl)_cacheEntryList.poll();
            next.applyDestroy();
        }

        upaDestroyAllEntries(_upaCacheRef);
    }

    private void initializePooling()
    {
        CacheJNIBuffer one = null;
        long upaBufferRef = 0;

        for (int pos = 0; pos < MAX_BUFFER_ARRAY_SIZE; ++pos)
        {
            _retrieveJNIBufferArray[pos] = new VaIteratableQueue();
            _applyJNIBufferArray[pos] = new VaIteratableQueue();
        }

        for (int pos = 0; pos <= _maxRetrieveJNIBufferArrayPos; ++pos)
        {
            one = new CacheJNIBuffer();
            upaBufferRef = upaCreateRsslBuffer(one, DEFAULT_BUFFER_SIZE * (pos + 1), null);
            if (upaBufferRef != 0)
                _retrieveJNIBufferArray[pos].add(one);
        }

        for (int pos = 0; pos <= _maxApplyJNIBufferArrayPos; ++pos)
        {
            one = new CacheJNIBuffer();
            upaBufferRef = upaCreateRsslBuffer(one, DEFAULT_BUFFER_SIZE * (pos + 1), null);
            if (upaBufferRef != 0)
                _applyJNIBufferArray[pos].add(one);
        }
    }

    public long createCacheEntry(CacheError error)
    {
        if (_isCacheDestroyed || _upaCacheRef == 0)
        {
            populateErrorInfo((CacheErrorImpl)error, CodecReturnCodes.FAILURE,
                              "PayloadCacheImpl.createCacheEntry error: use invalid cache instance to create cache entry.");
            return 0;
        }

        long upaCacheEntryInstance = upaCreateCacheEntry(_upaCacheRef, error);
        return upaCacheEntryInstance;
    }

    public void addCacheEntry(PayloadEntryImpl entry)
    {
        if (_isCacheDestroyed || _upaCacheRef == 0)
            return;

        _cacheEntryList.add(entry);
    }

    public void removeCacheEntry(PayloadEntryImpl entry)
    {
        if (_isCacheDestroyed || _upaCacheRef == 0)
            return;

        _cacheEntryList.remove(entry);
    }

    public CacheJNIBuffer acquireCacheRetrieveJNIBuffer(int length, CacheError error)
    {
        CacheJNIBuffer one = null;
        long upaBufferRef = 0;

        int pos = length / DEFAULT_BUFFER_SIZE;
        if (pos < MAX_BUFFER_ARRAY_POS)
        {
            if ((one = (CacheJNIBuffer)_retrieveJNIBufferArray[pos].poll()) == null)
            {
                one = new CacheJNIBuffer();
                upaBufferRef = upaCreateRsslBuffer(one, DEFAULT_BUFFER_SIZE * (pos + 1), error);
                if (upaBufferRef != 0)
                {
                    if (_maxRetrieveJNIBufferArrayPos < pos)
                        _maxRetrieveJNIBufferArrayPos = pos;
                    return one;
                }
                else
                    return null;
            }
            else
            {
                one.clear();
                return one;
            }
        }
        else
        // _retrieveJNIBufferArray[MAX_BUFFER_ARRAY_POS] will hold a list of bufs of flex length
        {
            _maxRetrieveJNIBufferArrayPos = MAX_BUFFER_ARRAY_POS;
            VaIteratableQueue flexLenBufList = _retrieveJNIBufferArray[MAX_BUFFER_ARRAY_POS];
            flexLenBufList.rewind();
            while (flexLenBufList.hasNext())
            {
                one = (CacheJNIBuffer)flexLenBufList.next();

                if (one._capability > length)
                {
                    flexLenBufList.remove();
                    one.clear();
                    return one;
                }
            }

            one = new CacheJNIBuffer();
            upaBufferRef = upaCreateRsslBuffer(one, length, error);
            if (upaBufferRef == 0)
                return null;
            else
                return one;
        }
    }

    public void releaseCacheRetrieveJNIBuffer(CacheJNIBuffer buffer)
    {
        if (buffer._capability > MAX_FIXED_BUFFER_SIZE)
            _retrieveJNIBufferArray[MAX_BUFFER_ARRAY_POS].add(buffer);
        else
        {
            int pos = buffer._capability / DEFAULT_BUFFER_SIZE - 1;
            _retrieveJNIBufferArray[pos].add(buffer);
        }
    }

    public CacheJNIBuffer acquireCacheApplyJNIBuffer(int length, CacheError error)
    {
        CacheJNIBuffer one = null;
        long upaBufferRef = 0;

        int pos = length / DEFAULT_BUFFER_SIZE;
        if (pos < MAX_BUFFER_ARRAY_POS)
        {
            if ((one = (CacheJNIBuffer)_applyJNIBufferArray[pos].poll()) == null)
            {
                one = new CacheJNIBuffer();
                upaBufferRef = upaCreateRsslBuffer(one, DEFAULT_BUFFER_SIZE * (pos + 1), error);
                if (upaBufferRef != 0)
                {
                    if (_maxApplyJNIBufferArrayPos < pos)
                        _maxApplyJNIBufferArrayPos = pos;
                    return one;
                }
                else
                    return null;
            }
            else
            {
                one.clear();
                return one;
            }
        }
        else
        // _applyJNIBufferArray[MAX_BUFFER_ARRAY_POS] will hold a list of bufs of flex length
        {
            _maxApplyJNIBufferArrayPos = MAX_BUFFER_ARRAY_POS;
            VaIteratableQueue flexLenBufList = _applyJNIBufferArray[MAX_BUFFER_ARRAY_POS];
            flexLenBufList.rewind();
            while (flexLenBufList.hasNext())
            {
                one = (CacheJNIBuffer)flexLenBufList.next();

                if (one._capability > length)
                {
                    flexLenBufList.remove();
                    one.clear();
                    return one;
                }
            }

            one = new CacheJNIBuffer();
            upaBufferRef = upaCreateRsslBuffer(one, length, error);
            if (upaBufferRef == 0)
                return null;
            else
                return one;
        }
    }

    public void releaseCacheApplyJNIBuffer(CacheJNIBuffer buffer)
    {
        if (buffer._capability > MAX_FIXED_BUFFER_SIZE)
            _applyJNIBufferArray[MAX_BUFFER_ARRAY_POS].add(buffer);
        else
        {
            int pos = buffer._capability / DEFAULT_BUFFER_SIZE - 1;
            _applyJNIBufferArray[pos].add(buffer);
        }
    }

    /* **** native methods ************************************************************/

    public native static int upaInitializeJNICache();

    public native static int upaUninitializeJNICache();

    public native static long upaCreateCache(int cacheConfigOption, CacheError error);

    public native void upaDestroyCache(long upaCacheRef);

    public native long upaCreateFIDDictionary(String dictKey);

    public native int upaLoadDictionaryFromBuffer(long upaDictRef, long upaDictWriteBufRef, int dictDataLength, CacheError error);

    public native int upaSetFidDbWithCache(long upaCacheRef, long upaDictRef, String dictKey, CacheError error);

    public native long upaCreateCacheEntry(long upaCacheRef, CacheError error);

    public native void upaDestroyAllEntries(long upaCacheInstance);

    public native long upaCreateRsslBuffer(CacheJNIBuffer retrieveBuffer, long length, CacheError error);

    public native int upaFreeRsslBuffer(long upaRsslbufferRef);
}