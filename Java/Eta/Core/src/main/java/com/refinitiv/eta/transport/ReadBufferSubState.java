/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2021,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

/* Represents sub-states of the ReadBufferStateMachine */
enum ReadBufferSubState
{
    /* No special processing is being performed */
    NORMAL,

    /* We are processing a packed message.
     * ASSUMPTION: It is not possible to have a message that is both packed and fragmented
     */
    PROCESSING_PACKED_MESSAGE,

    /* We are processing a complete fragmented message */
    PROCESSING_COMPLETE_FRAGMENTED_MESSAGE,

    /* We are processing a compressed message */
    PROCESSING_COMPRESSED_MESSAGE,

    /* We are processing a fragmented compressed message */
    PROCESSING_FRAGMENTED_COMPRESSED_MESSAGE,

    /* We are processing a packed compressed message */
    PROCESSING_PACKED_COMPRESSED_MESSAGE,

    /* We are processing a fragmented message.
     * ASSUMPTION: It is not possible to have a message that is both packed and fragmented
     */
    PROCESSING_FRAGMENTED_MESSAGE,

    /* We are processing a complete fragmented JSON message over the websocket connection. */
    PROCESSING_COMPLETE_FRAGMENTED_JSON_MESSAGE
}
