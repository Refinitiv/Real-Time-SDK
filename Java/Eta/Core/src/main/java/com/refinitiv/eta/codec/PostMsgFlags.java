/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

/**
 * The Post Message flags.
 * 
 * @see PostMsg
 */
public class PostMsgFlags
{
    /**
     * This class is not instantiated
     */
    private PostMsgFlags()
    {
        throw new AssertionError();
    }

    /** (0x000) No Flags set */
    public static final int NONE = 0x000;

    /** (0x001) Indicates presence of the Extended Header */
    public static final int HAS_EXTENDED_HEADER = 0x001;

    /** (0x002) Indicates the presence of the Post Id */
    public static final int HAS_POST_ID = 0x002;

    /**
     * (0x004) Indicates that the {@link PostMsg} contains a populated msgKey that
     * identifies the stream on which the information is posted. A msgKey is
     * typically required for off-stream posting and is not necessary when on-stream posting.
     */
    public static final int HAS_MSG_KEY = 0x004;

    /** (0x008) Indicates the presence of Sequence Number */
    public static final int HAS_SEQ_NUM = 0x008;

    /**
     * (0x020) Post Message is last message in a post exchange. If post message
     * is multipart, this is the last part. If message is atomic, both INIT and
     * COMPLETE should be set on same message.
     */
    public static final int POST_COMPLETE = 0x020;

    /**
     * (0x040) Specifies that the consumer wants the provider to send an {@link AckMsg}
     * to indicate that the {@link PostMsg} was processed properly. When
     * acknowledging a {@link PostMsg}, the provider will include the postId in
     * the ackId and communicate any associated seqNum.
     */
    public static final int ACK = 0x040;

    /** (0x080) Indicates the presence of the Permission Expressions */
    public static final int HAS_PERM_DATA = 0x080;

    /** (0x100) Indicates the presence of Part Number */
    public static final int HAS_PART_NUM = 0x100;

    /** (0x200) Indicates that post Message includes which rights a user has */
    public static final int HAS_POST_USER_RIGHTS = 0x200;
}
