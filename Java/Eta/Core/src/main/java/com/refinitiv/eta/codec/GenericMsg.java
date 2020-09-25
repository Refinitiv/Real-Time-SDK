package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.Buffer;

/**
 * A bi-directional UPA message that does not have any implicit interaction
 * semantics associated with it, thus the name generic. Once a stream is
 * established via a request-refresh/status interaction, this message can be
 * sent from consumer to provider as well as from provider to consumer, and can
 * also be leveraged by non-interactive provider applications. Generic messages
 * are transient and are typically not cached by any Refinitiv Real-Time Distibution 
 * System components. The msgKey of a {@link GenericMsg} does not need to match the
 * {@link MsgKey} information associated with the stream the generic message is
 * flowing on. This allows for the key information to be used independently of
 * the stream. Any specific message usage, msgKey usage, expected interactions,
 * and handling instructions are typically defined by a domain message model
 * specification.
 * 
 * @see Msg
 * @see GenericMsgFlags
 */
public interface GenericMsg extends Msg
{
    /**
     * Checks the presence of the Extended Header presence flag.
     * 
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasExtendedHdr();

    /**
     * Checks the presence of the Permission Expression presence flag.
     * 
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasPermData();

    /**
     * Checks the presence of the Message Key presence flag.
     * 
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasMsgKey();

    /**
     * Checks the presence of the Sequence Number presence flag.
     * 
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasSeqNum();

    /**
     * Checks the presence of the Part Number presence flag.
     * 
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasPartNum();

    /**
     * Checks the presence of the Message Complete indication flag.
     * 
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkMessageComplete();

    /**
     * Checks the presence of the Secondary Sequence Number presence flag.
     * 
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasSecondarySeqNum();
    /**
     * Checks the presence of the Provider Driven flag.
     *
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     *
     * @see Msg#flags()
     *
     * @return true - if the message is provider driven; false if it is not.
     */
    public boolean checkIsProviderDriven();

    /**
     * Applies the Extended Header presence flag.
     * 
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasExtendedHdr();

    /**
     * Applies the Permission Expression presence flag.
     * 
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasPermData();

    /**
     * Applies the Message Key presence flag.
     * 
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasMsgKey();

    /**
     * Applies the Sequence Number presence flag.
     * 
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasSeqNum();

    /**
     * Applies the Part Number presence flag.
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasPartNum();

    /**
     * Applies the Message Complete indication flag.
     * 
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyMessageComplete();

    /**
     * Applies the Secondary Sequence Number presence flag.
     * 
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasSecondarySeqNum();

    /**
     * Applies the Provider Driven indication flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     *
     * @see Msg#flags(int)
     */
    public void applyProviderDriven();

    /**
     * Specifies the part number of this generic message, typically used with
     * multi-part generic messages. If sent on a single-part post message, use a
     * partNum of 0. On multi-part post messages, use a partNum of 0 on the
     * initial part and increment partNum in each subsequent part by 1.
     * 
     * @param partNum the partNum to set
     */
    public void partNum(int partNum);

    /**
     * Specifies the part number of this generic message, typically used with
     * multi-part generic messages. If sent on a single-part post message, use a
     * partNum of 0. On multi-part post messages, use a partNum of 0 on the
     * initial part and increment partNum in each subsequent part by 1.
     * 
     * @return the partNum
     */
    public int partNum();

    /**
     * Specifies a user-defined sequence number and typically corresponds to the
     * sequencing of this message. To help with temporal ordering, seqNum should
     * increase across messages, but can have gaps depending on the sequencing
     * algorithm in use. Details about using seqNum should be defined in the
     * domain model specification or the documentation for products which
     * require the use of seqNum.
     * 
     * @param seqNum the seqNum to set. Valid range is 0-4294967296 (2^32).
     */
    public void seqNum(long seqNum);

    /**
     * Specifies a user-defined sequence number and typically corresponds to the
     * sequencing of this message. To help with temporal ordering, seqNum should
     * increase across messages, but can have gaps depending on the sequencing
     * algorithm in use. Details about using seqNum should be defined in the
     * domain model specification or the documentation for products which
     * require the use of seqNum.
     * 
     * @return the seqNum
     */
    public long seqNum();

    /**
     * Specifies an additional user-defined sequence number. When using
     * {@link GenericMsg} on a stream in a bi-directional manner,
     * secondarySeqNum is often used as an acknowledgment sequence number. For
     * example, a consumer sends a generic message with seqNum populated to
     * indicate the sequence of this message in the stream and secondarySeqNum
     * set to the seqNum last received from the provider. This effectively
     * acknowledges all messages received up to that point while also sending
     * additional information. Sequence number use should be defined within the
     * domain model specification or any documentation for products that use
     * secondarySeqNum.
     * 
     * @param secondarySeqNum the secondarySeqNum to set. Valid range is
     *            0-4294967296 (2^32).
     */
    public void secondarySeqNum(long secondarySeqNum);

    /**
     * Specifies an additional user-defined sequence number. When using
     * {@link GenericMsg} on a stream in a bi-directional manner,
     * secondarySeqNum is often used as an acknowledgment sequence number. For
     * example, a consumer sends a generic message with seqNum populated to
     * indicate the sequence of this message in the stream and secondarySeqNum
     * set to the seqNum last received from the provider. This effectively
     * acknowledges all messages received up to that point while also sending
     * additional information. Sequence number use should be defined within the
     * domain model specification or any documentation for products that use
     * secondarySeqNum.
     * 
     * @return the secondarySeqNum
     */
    public long secondarySeqNum();

    /**
     * Specifies authorization information for this stream. When permData is
     * specified, it indicates authorization information for only the content
     * within this message, though this can be overridden for specific content
     * within the message (for example, MapEntry.permData).
     * 
     * @param permData the permData to set
     */
    public void permData(Buffer permData);

    /**
     * Specifies authorization information for this stream. When permData is
     * specified, it indicates authorization information for only the content
     * within this message, though this can be overridden for specific content
     * within the message (for example, MapEntry.permData).
     * 
     * @return the permData
     */
    public Buffer permData();
}
