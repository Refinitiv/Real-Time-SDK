package com.thomsonreuters.upa.valueadd.reactor;

import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.MsgBase;

/* Interface for all watchlist handlers */
interface WlHandler
{
    /* Submit a request to a watchlist handler. */
    int submitRequest(WlRequest wlRequest, RequestMsg requestMsg, boolean isReissue, ReactorSubmitOptions submitOptions, ReactorErrorInfo errorInfo);
    /* Submit a Codec message to a watchlist handler. */
    int submitMsg(WlRequest wlRequest, Msg msg, ReactorSubmitOptions submitOptions, ReactorErrorInfo errorInfo);
    /* Watchlist handler read message method. */
    int readMsg(WlStream wlStream, DecodeIterator dIter, Msg msg, ReactorErrorInfo errorInfo);
    /* Used to notify the watchlist handler of a request timeout. */ 
    int requestTimeout(WlStream wlStream, ReactorErrorInfo errorInfo);
    /* Used to call back the user. */ 
    int callbackUser(String location, Msg msg, MsgBase rdmMsg, WlRequest wlRequest, ReactorErrorInfo errorInfo);    
    /* Used to add unsent msg back to request pending list. */ 
    void addPendingRequest(WlStream wlStream);
}
