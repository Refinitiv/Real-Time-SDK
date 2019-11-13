///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.upa.valueadd.reactor;

import static org.junit.Assert.assertEquals;

import com.thomsonreuters.upa.rdm.ClassesOfService.DataIntegrityTypes;
import com.thomsonreuters.upa.rdm.ClassesOfService.FlowControlTypes;

/** This provider always accepts tunnel streams. */
class TunnelStreamProvider extends Provider
{
    TunnelStreamAcceptOptions _acceptOpts = ReactorFactory.createTunnelStreamAcceptOptions();

    int _maxMsgSize = CosCommon.DEFAULT_MAX_MSG_SIZE;
    
    public TunnelStreamProvider(TestReactor reactor)
    {
        super(reactor);
    }
    
    @Override
    public int listenerCallback(TunnelStreamRequestEvent event)
    {
        super.listenerCallback(event);
        
        /* Accept the tunnel stream request. */
        _acceptOpts.clear();
        _acceptOpts.statusEventCallback(this);
        _acceptOpts.defaultMsgCallback(this); 
        _acceptOpts.classOfService().common().maxMsgSize(_maxMsgSize);           
        _acceptOpts.classOfService().dataIntegrity().type(DataIntegrityTypes.RELIABLE);
        _acceptOpts.classOfService().flowControl().type(FlowControlTypes.BIDIRECTIONAL);
        _acceptOpts.classOfService().authentication().type(event.classOfService().authentication().type());
        assertEquals(ReactorReturnCodes.SUCCESS, reactorChannel().acceptTunnelStream(event, _acceptOpts, _errorInfo));
        
        return ReactorReturnCodes.SUCCESS;
    }
    
    void maxMsgSize(int maxMsgSize)
    {
        _maxMsgSize = maxMsgSize;
    }
    
}
