package com.rtsdk.eta.transport;

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
}
