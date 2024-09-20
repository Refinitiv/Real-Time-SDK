/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm;

import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Msg;

/**
 * This message structure contains the information that is common across all RDM
 * Message formats. It is included in all Value Added RDM Components.
 */
public interface MsgBase
{
    /**
     * The Stream Id for the given item.
     * 
     * @return the streamId
     */
    public int streamId();

    /**
     * The Stream Id for the given item.
     *
     * @param streamId the streamId to set
     * 
     */
    public void streamId(int streamId);
    
    /**
     * Returns the domain type of the RDM message.
     *
     * @return the int
     */
    public int domainType();

    /**
     * Encode an RDM message.
     * 
     * @param eIter The Encode Iterator
     * 
     * @return ETA return value
     */
    public int encode(EncodeIterator eIter);

    /**
     * Decode a ETA message into an RDM message.
     *
     * @param dIter The Decode Iterator
     * @param msg the msg
     * @return ETA return value
     */
    public int decode(DecodeIterator dIter, Msg msg);

    /**
     * Clears the current contents of the message and prepares it for re-use.
     */
    public void clear();
}