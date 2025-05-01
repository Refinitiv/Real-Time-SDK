/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

/* Defines all the valid states of a ReadBuffer, used by the ReadBufferStateMachine */
enum ReadBufferState
{
    /* The buffer contains no data */
    NO_DATA,

    /* The length of the next message is unknown */
    UNKNOWN_INCOMPLETE,

    /* The length of the next message is unknown,
     * and there is insufficient space in the buffer to read the message length
     */
    UNKNOWN_INSUFFICIENT,

    /* The length of the next message is known, but the message is incomplete */
    KNOWN_INCOMPLETE,

    /* The length of the next message is known, but there is insufficient space to read it completely */
    KNOWN_INSUFFICENT,

    /* A complete message is ready to be read */
    KNOWN_COMPLETE,

    /* The reader reached end of stream */
    END_OF_STREAM
}
