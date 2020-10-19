package com.refinitiv.eta.valueadd.cache;

import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Int;
import com.refinitiv.eta.codec.RefreshMsg;

class CacheDictLoadingHelper
{
    // variables to be used to set dictionary
    Error _dictError = null;
    EncodeIterator _dictEncodeIter = null;
    Int _dictMinFidInt = null;
    CacheJNIBuffer _dictEncodedBuf = null;
    Buffer _dictEncodedBufWrap = null;
    String _dictKey = null;
    boolean _dictKeyCleared = false;
    RefreshMsg _dictMsg = null;
    Buffer _dictName = null;

    public CacheDictLoadingHelper()
    {
        // variables to be used to bind dictionary
        _dictError = TransportFactory.createError();
        _dictEncodeIter = CodecFactory.createEncodeIterator();
        _dictMinFidInt = CodecFactory.createInt();
        _dictEncodedBuf = new CacheJNIBuffer();
        _dictEncodedBufWrap = CodecFactory.createBuffer();
        _dictMsg = (RefreshMsg)CodecFactory.createMsg();
        _dictName = CodecFactory.createBuffer();
        _dictName.data("RWFFld");
    }

}