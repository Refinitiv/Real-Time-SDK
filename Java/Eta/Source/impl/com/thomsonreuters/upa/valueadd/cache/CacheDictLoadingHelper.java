package com.thomsonreuters.upa.valueadd.cache;

import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.RefreshMsg;

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