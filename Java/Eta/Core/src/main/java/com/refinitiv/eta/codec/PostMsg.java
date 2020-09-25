package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.Buffer;

/**
 * UPA Post Message allows a Consumer application to push content to upstream
 * components. This information can be applied to a Refinitiv Real-Time 
 * Distribution System cache or routed further upstream to the source of data. 
 * Once received, the upstream components can republish data to downstream consumers. 
 * Post messages can be routed along a specific item stream, referred to 
 * as on-stream posting, or along a user's Login stream, referred to as off-stream posting.
 * A {@link PostMsg} can contain any UPA container type, including other messages.
 * User identification information can be associated with a post message and can
 * be provided along with the content that was posted.
 * 
 * @see Msg
 * @see PostUserRights
 * @see PostMsgFlags
 */
public interface PostMsg extends Msg
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
     * Checks the presence of the Post Id presence flag.
     * 
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasPostId();

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
     * Checks the presence of the Post Complete indication flag.
     * 
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkPostComplete();

    /**
     * Checks the presence of the Acknowledgment indication flag.
     * 
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkAck();

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
     * Checks the presence of the Post User Rights presence flag.
     * 
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasPostUserRights();

    /**
     * Applies the Extended Header presence flag.
     * 
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasExtendedHdr();

    /**
     * Applies the Post Id presence flag.
     * 
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasPostId();

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
     * 
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasPartNum();

    /**
     * Applies the Post Complete indication flag.
     * 
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyPostComplete();

    /**
     * Applies the Acknowledgment indication flag.
     * 
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyAck();

    /**
     * Applies the Permission Expression presence flag.
     * 
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasPermData();

    /**
     * Applies the Post User Rights presence flag.
     * 
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasPostUserRights();

    /**
     * Specifies the part number for this post message, typically used with
     * multi-part post messages. If sent on a single-part post message, use a
     * partNum of 0. On multi-part post messages, use a partNum of 0, on the
     * initial part and increment partNum in each subsequent part by 1. Must
     * be in the range of 0 - 32767.
     * 
     * @param partNum the partNum to set
     */
    public void partNum(int partNum);

    /**
     * Specifies the part number for this post message, typically used with
     * multi-part post messages. If sent on a single-part post message, use a
     * partNum is 0. On multi-part post messages, use a partNum is 0, on the
     * initial part and incremented in each subsequent part by 1.
     * 
     * 
     * @return the partNum
     */
    public int partNum();

    /**
     * Specifies a user-defined sequence number (typically corresponding to the
     * sequencing of the message). To help with temporal ordering, seqNum should
     * increase, though gaps might exist depending on the sequencing algorithm
     * in use. Details about seqNum use should be defined in the domain model
     * specification or any documentation for products that use seqNum. When
     * acknowledgments are requested, the seqNum will be provided back in the
     * {@link AckMsg} to help identify the {@link PostMsg} being acknowledged.
     * Must be in the range of 0 - 4294967296 (2^32).
     * 
     * @param seqNum the seqNum to set
     */
    public void seqNum(long seqNum);

    /**
     * Specifies a user-defined sequence number (typically corresponding to the
     * sequencing of the message). To help with temporal ordering, seqNum should
     * increase, though gaps might exist depending on the sequencing algorithm
     * in use. Details about seqNum use should be defined in the domain model
     * specification or any documentation for products that use seqNum. When
     * acknowledgments are requested, the seqNum will be provided back in the
     * {@link AckMsg} to help identify the {@link PostMsg} being acknowledged.
     * 
     * @return the seqNum
     */
    public long seqNum();

    /**
     * Specifies a consumer-assigned identifier that distinguishes different
     * post messages. Each part in a multi-part post message should use the same
     * postId value. Must be in the range of 0 - 4294967296 (2^32).
     * 
     * @param postId the postId to set
     */
    public void postId(long postId);

    /**
     * Specifies a consumer-assigned identifier that distinguishes different
     * post messages. Each part in a multi-part post message should use the same
     * postId value.
     * 
     * @return the postId
     */
    public long postId();

    /**
     * Specifies authorization information for this stream. When permData is
     * specified, it indicates authorization information for only the content
     * within this message, though this can be overridden for specific content
     * within the message (e.g. MapEntry.permData).
     * 
     * @param permData the permData to set
     */
    public void permData(Buffer permData);

    /**
     * Permission data.
     * 
     * @return the permData
     */
    public Buffer permData();

    /**
     * Conveys the rights or abilities of the user posting this content. This
     * can indicate whether the user is permissioned to:
     * <ul>
     * <li>Create items in the cache of record. (0x01)</li>
     * <li>Delete items from the cache of record. (0x02)</li>
     * <li>Modify the permData on items already present in the cache of record. (0x03)</li>
     * </ul>
     * Must be in the range of 0 - 32767.
     * 
     * @param postUserRights the permData to set
     * 
     * @see PostUserRights
     */
    public void postUserRights(int postUserRights);

    /**
     * Conveys the rights or abilities of the user posting this content. This
     * can indicate whether the user is permissioned to:
     * <ul>
     * <li>Create items in the cache of record. (0x01)</li>
     * <li>Delete items from the cache of record. (0x02)</li>
     * <li>Modify the permData on items already present in the cache of record. (0x03)</li>
     * </ul>
     * 
     * @return the postUserRights
     * 
     * @see PostUserRights
     */
    public int postUserRights();

    /**
     * The UPA Post User Info Structure. This information can optionally be
     * provided along with the posted content via the postUserInfo on the
     * {@link RefreshMsg}, {@link UpdateMsg}, and {@link StatusMsg}.
     * 
     * @return the postUserInfo
     */
    public PostUserInfo postUserInfo();
}
