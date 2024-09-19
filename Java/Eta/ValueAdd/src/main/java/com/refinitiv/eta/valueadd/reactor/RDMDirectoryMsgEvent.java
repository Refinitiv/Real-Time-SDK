/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg;

/**
 * Event provided to RDMDirectoryMsgCallback methods.
 * 
 * @see ReactorMsgEvent
 */
public class RDMDirectoryMsgEvent extends ReactorMsgEvent
{
    DirectoryMsg _directoryMsg;

    RDMDirectoryMsgEvent()
    {
        super();
    }

    void rdmDirectoryMsg(DirectoryMsg rdmDirectoryMsg)
    {
        _directoryMsg = rdmDirectoryMsg;
    }

    /**
     * The DirectoryMsg associated with this message event.
     * 
     * @return DirectoryMsg
     */
    public DirectoryMsg rdmDirectoryMsg()
    {
        return _directoryMsg;
    }
}
