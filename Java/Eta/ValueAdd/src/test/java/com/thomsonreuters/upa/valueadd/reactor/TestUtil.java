///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.upa.valueadd.reactor;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CopyMsgFlags;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryClose;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryMsg;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryStatus;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryClose;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryConsumerStatus;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsg;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryStatus;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryUpdate;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.*;

import static org.junit.Assert.fail;

/* Utility functions for use with Reactor Junit tests. */
public class TestUtil
{
    /** Copies a ReactorErrorInfo object. */
    public static void copyErrorInfo(ReactorErrorInfo srcErrorInfo, ReactorErrorInfo destErrorInfo)
    {
        destErrorInfo.location(srcErrorInfo.location());
        destErrorInfo.error().channel(srcErrorInfo.error().channel());
        destErrorInfo.error().errorId(srcErrorInfo.error().errorId());
        destErrorInfo.error().sysError(srcErrorInfo.error().sysError());
        if (srcErrorInfo.error().text() != null)
            destErrorInfo.error().text(srcErrorInfo.error().text());
        destErrorInfo.code(srcErrorInfo.code());
    }

    /** Copy ReactorMsgEvent parts. */
    public static void copyMsgEvent(ReactorMsgEvent otherEvent, ReactorMsgEvent event)
    {
        if (otherEvent.msg() != null)
        {
            event.msg(CodecFactory.createMsg());
            otherEvent.msg().copy(event.msg(), CopyMsgFlags.ALL_FLAGS);
        }
        
        /* Copy transport buffer if present. */
        if (otherEvent.transportBuffer() != null)
            event.transportBuffer(new CopiedTransportBuffer(otherEvent.transportBuffer())); 
        
        if (otherEvent.streamInfo() != null)
        {
            event.streamInfo().userSpecObject(otherEvent.streamInfo().userSpecObject());

            if (otherEvent.streamInfo().serviceName() != null)
                event.streamInfo().serviceName(otherEvent.streamInfo().serviceName());
        }

        event.reactorChannel(otherEvent.reactorChannel());
        TestUtil.copyErrorInfo(otherEvent.errorInfo(), event.errorInfo());
    }

    /** Copies a LoginMsg. Used for both RDMLoginMsgEvent and TunnelStreamAuthInfo. */
    public static void copyLoginMsg(LoginMsg srcMsg, LoginMsg destMsg)
    {
        switch(srcMsg.rdmMsgType())
        {
            case REQUEST: 
                destMsg.rdmMsgType(LoginMsgType.REQUEST);
                ((LoginRequest)srcMsg).copy((LoginRequest)destMsg);
                break;
            case CLOSE:
                destMsg.rdmMsgType(LoginMsgType.CLOSE);
                ((LoginClose)srcMsg).copy((LoginClose)destMsg);
                break;
            case REFRESH:
                destMsg.rdmMsgType(LoginMsgType.REFRESH);
                ((LoginRefresh)srcMsg).copy((LoginRefresh)destMsg);
                break;
            case STATUS: 
                destMsg.rdmMsgType(LoginMsgType.STATUS);
                ((LoginStatus)srcMsg).copy((LoginStatus)destMsg);
                break;
            case CONSUMER_CONNECTION_STATUS:
                destMsg.rdmMsgType(LoginMsgType.CONSUMER_CONNECTION_STATUS);
                ((LoginConsumerConnectionStatus)srcMsg).copy((LoginConsumerConnectionStatus)destMsg); 
                break;
            case RTT:
                destMsg.rdmMsgType(LoginMsgType.RTT);
                ((LoginRTT) srcMsg).copy((LoginRTT) destMsg);
                break;
            default: 
                fail("Unknown LoginMsgType."); 
                break;
        }
    }

    /** Copies a DirectoryMsg. */
    public static void copyDirectoryMsg(DirectoryMsg srcMsg, DirectoryMsg destMsg)
    {
        switch(srcMsg.rdmMsgType())
        {
            case REQUEST:
                destMsg.rdmMsgType(DirectoryMsgType.REQUEST);
                ((DirectoryRequest)srcMsg).copy((DirectoryRequest)destMsg);
                break;
            case CLOSE:
                destMsg.rdmMsgType(DirectoryMsgType.CLOSE);
                ((DirectoryClose)srcMsg).copy((DirectoryClose)destMsg);
                break;
            case REFRESH:
                destMsg.rdmMsgType(DirectoryMsgType.REFRESH);
                ((DirectoryRefresh)srcMsg).copy((DirectoryRefresh)destMsg);
                break;
            case UPDATE:
                destMsg.rdmMsgType(DirectoryMsgType.UPDATE);
                ((DirectoryUpdate)srcMsg).copy((DirectoryUpdate)destMsg);
                break;
            case STATUS:
                destMsg.rdmMsgType(DirectoryMsgType.STATUS);
                ((DirectoryStatus)srcMsg).copy((DirectoryStatus)destMsg);
                break;
            case CONSUMER_STATUS: 
                destMsg.rdmMsgType(DirectoryMsgType.CONSUMER_STATUS);
                ((DirectoryConsumerStatus)srcMsg).copy((DirectoryConsumerStatus)destMsg);
                break;
            default: 
                fail("Unknown DirectoryMsgType.");
                break;
        }
    }

    /** Copies a DictionaryMsg. */
    public static void copyDictionaryMsg(DictionaryMsg srcMsg, DictionaryMsg destMsg)
    {
        switch(srcMsg.rdmMsgType())
        {
            case REQUEST:
                destMsg.rdmMsgType(DictionaryMsgType.REQUEST);
                ((DictionaryRequest)srcMsg).copy((DictionaryRequest)destMsg);
                break;
            case CLOSE:
                destMsg.rdmMsgType(DictionaryMsgType.CLOSE);
                ((DictionaryClose)srcMsg).copy((DictionaryClose)destMsg);
                break;
            case REFRESH:
                destMsg.rdmMsgType(DictionaryMsgType.REFRESH);
                ((DictionaryRefresh)srcMsg).copy((DictionaryRefresh)destMsg);
                break;
            case STATUS:
                destMsg.rdmMsgType(DictionaryMsgType.STATUS);
                ((DictionaryStatus)srcMsg).copy((DictionaryStatus)destMsg);
                break;
            default:
                fail("Unknown DictionaryMsgType.");
                break;
        }
    }
}
