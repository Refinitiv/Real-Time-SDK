/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.Buffer;

/**
 * ETA acknowledgment message is sent from a provider to a consumer to indicate receipt of a specific message.
 * The acknowledgment carries success or failure (negative acknowledgment or Nak) information to the consumer.
 * Currently, a consumer can request acknowledgment for a {@link PostMsg} or a {@link CloseMsg}.
 * 
 * @see Msg
 * @see CloseMsg
 * @see PostMsg
 * @see AckMsgFlags
 * @see NakCodes
 */
public interface AckMsg extends Msg
{
    /**
     * Checks the presence of the Extended Header flag.
     * 
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasExtendedHdr();

    /**
     * Checks the presence of the Text flag.
     * 
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasText();

    /**
     * Checks the presence of the Private Stream flag.
     * 
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkPrivateStream();

    /**
     * Checks the presence of the Sequence Number flag.
     * 
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasSeqNum();

    /**
     * Checks the presence of the Message Key flag.
     * 
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasMsgKey();

    /**
     * Checks the presence of the NAK Code flag.
     * 
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasNakCode();

    /**
     * Checks the presence of the Qualified Stream flag.
     * 
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkQualifiedStream();

    /**
     * Applies the Extended Header indication flag.
     * 
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasExtendedHdr();

    /**
     * Applies the Text indication flag.
     * 
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasText();

    /**
     ** Applies the Private Stream indication flag.
     * 
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     **/
    public void applyPrivateStream();

    /**
     * Applies the Sequence Number indication flag.
     * 
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasSeqNum();

    /**
     * Applies the Message Key indication flag.
     * 
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasMsgKey();

    /**
     * Applies the NAK Code indication flag.
     * 
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasNakCode();

    /**
     * Applies the Qualified Stream indication flag.
     * 
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     **/
    public void applyQualifiedStream();

    /**
     * ID used to associate this Ack with the message it is acknowledging.
     * 
     * @param ackId the ackId to set. Valid range is 0-4294967296 (2^32).
     */
    public void ackId(long ackId);

    /**
     * ID used to associate this Ack with the message it is acknowledging.
     * 
     * @return the ackId
     */
    public long ackId();

    /**
     * If presents, this Ack indicates negative acknowledgment, if 0 or not
     * present, Ack indicates positive acknowledgment.
     * 
     * @param nakCode the nakCode to set
     * 
     * @see NakCodes
     */
    public void nakCode(int nakCode);

    /**
     * If presents, this Ack indicates negative acknowledgment, if 0 or not
     * present, Ack indicates positive acknowledgment.
     * 
     * @return the nakCode
     * 
     * @see NakCodes
     */
    public int nakCode();

    /**
     * A user-defined sequence number.
     * <p>
     * To help with temporal ordering, seqNum should increase, though gaps might
     * exist depending on the sequencing algorithm in use. The acknowledgment
     * message may populate this with the seqNum of the {@link PostMsg} being
     * acknowledged. This helps correlate the message being acknowledged when
     * the postId alone is not sufficient (e.g., multi-part post messages).
     * Must be in the range of 0 - 4294967296 (2^32).
     * 
     * @param seqNum the seqNum to set
     */
    public void seqNum(long seqNum);

    /**
     * A user-defined sequence number.
     * <p>
     * To help with temporal ordering, seqNum should increase, though gaps might
     * exist depending on the sequencing algorithm in use. The acknowledgment
     * message may populate this with the seqNum of the {@link PostMsg} being
     * acknowledged. This helps correlate the message being acknowledged when
     * the postId alone is not sufficient (e.g., multi-part post messages).
     * 
     * @return the seqNum
     */
    public long seqNum();

    /**
     * Text describes additional information about the acceptance or rejection
     * of the message being acknowledged.
     * 
     * @param text the text to set
     */
    public void text(Buffer text);

    /**
     * Text describes additional information about the acceptance or rejection
     * of the message being acknowledged.
     * 
     * @return the text
     */
    public Buffer text();
}