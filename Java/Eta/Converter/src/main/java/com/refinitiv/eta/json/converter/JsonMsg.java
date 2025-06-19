/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CopyMsgFlags;
import com.refinitiv.eta.codec.Msg;

/**
 * Representation of JSON message which is incoming to the application.
 */
public interface JsonMsg {

    /**
     * Class of this message (PING, PONG, ERROR, etc).
     * Populated from {@link JsonMsgClasses} constants.
     *
     * @return the class of JSON msg.
     */
    int jsonMsgClass();

    /**
     * Class of this message (PING, PONG, ERROR, etc).
     * Populated from {@link JsonMsgClasses} constants.
     *
     * @param jsonMsgClass the class of JSON msg to set.
     */
    void jsonMsgClass(int jsonMsgClass);

    /**
     * @return data of the incoming or outgoing JSON msg.
     */
    Buffer jsonMsgData();

    /**
     * RWF message which has been converted to/from JSON.
     * Relevant for {@link #jsonMsgClass()} is equivalent to {@link JsonMsgClasses#RSSL_MESSAGE}.
     *
     * @return rwf message.
     */
    Msg rwfMsg();

    /**
     * Clears the current contents of the message and prepares it for re-use.
     */
    void clear();

    /**
     * Performs a deep copy of a {@link JsonMsg} structure.
     *
     * @param copyTo          - JSON msg which
     * @param rwfMsgCopyFlags - controls which parameters of RWF message are copied to destination message.
     * @see CopyMsgFlags
     */
    void copy(JsonMsg copyTo, int rwfMsgCopyFlags);
}
