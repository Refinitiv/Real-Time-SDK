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
