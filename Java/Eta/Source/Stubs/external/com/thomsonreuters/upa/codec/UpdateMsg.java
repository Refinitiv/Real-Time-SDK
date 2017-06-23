package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.rdm.UpdateEventTypes;

/**
 * UPA Update Message is used by Interactive and Non-Interactive Providers to
 * convey changes to data associated with an item stream. When streaming, update
 * messages typically flow after an initial refresh has been delivered. Update
 * messages can be delivered between parts of a multi-part refresh message, even
 * in response to a non-streaming request.
 * 
 * <p>
 * Some providers can aggregate the information from multiple update messages
 * into a single update message. This is known as conflation, and typically
 * occurs if a conflated quality of service is requested, a stream is paused, or
 * a consuming application is unable to keep up with the data rates associated
 * with the stream.
 * 
 * @see Msg
 * @see UpdateMsgFlags
 */
public interface UpdateMsg extends Msg
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
     * Checks the presence of the Conflation Information presence flag.
     *
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasConfInfo();

    /**
     * Checks the presence of the Do Not Cache indication flag.
     *
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkDoNotCache();

    /**
     * Checks the presence of the Do Not Conflate indication flag.
     *
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkDoNotConflate();

    /**
     * Checks the presence of the Do Not Ripple indication flag.
     *
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkDoNotRipple();

    /**
     * Checks the presence of the Post User Information presence flag.
     *
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasPostUserInfo();

    /**
     * Checks the presence of the Discardable presence flag.
     *
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkDiscardable();

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
     * Applies the Conflation Info presence flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasConfInfo();

    /**
     * Applies the Do Not Cache indication flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyDoNotCache();

    /**
     * Applies the Do Not Conflate indication flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyDoNotConflate();

    /**
     * Applies the Do Not Ripple indication flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyDoNotRipple();

    /**
     * Applies the Post User Info presence flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasPostUserInfo();

    /**
     * Applies the Discardable presence flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyDiscardable();

    /**
     * Specifies the type of data in the {@link UpdateMsg}. Examples of possible
     * update types include: Trade, Quote, or Closing Run.
     * Must be in the range of 0 - 255.
     * <ul>
     * <li>
     * Domain message model specifications define available update types</li>
     * <li>
     * For Thomson Reuters's provided domain models, see
     * {@link UpdateEventTypes}</li>
     * </ul>
     * 
     * @param updateType the updateType to set
     */
    public void updateType(int updateType);

    /**
     * Specifies the type of data in the {@link UpdateMsg}. Examples of possible
     * update types include: Trade, Quote, or Closing Run.
     * <ul>
     * <li>
     * Domain message model specifications define available update types</li>
     * <li>
     * For Thomson Reuters's provided domain models, see {@link UpdateEventTypes}</li>
     * </ul>
     * 
     * @return the updateType
     */
    public int updateType();

    /**
     * Specifies a user-defined sequence number. To help with temporal ordering,
     * seqNum should increase across messages, but can have gaps depending on
     * the sequencing algorithm in use. Details about sequence number use should
     * be defined within the domain model specification or any documentation for
     * products which require the use of seqNum.
     * Must be in the range of 0 - 4294967296 (2^32).
     * 
     * @param seqNum the seqNum to set
     */
    public void seqNum(long seqNum);

    /**
     * Specifies a user-defined sequence number. To help with temporal ordering,
     * seqNum should increase across messages, but can have gaps depending on
     * the sequencing algorithm in use. Details about sequence number use should
     * be defined within the domain model specification or any documentation for
     * products which require the use of seqNum.
     * 
     * @return the seqNum
     */
    public long seqNum();

    /**
     * When conflation is used, this value indicates the number of updates
     * conflated or aggregated into this {@link UpdateMsg}.
     * Must be in the range of 0 - 32767.
     * 
     * @param conflationCount the conflationCount to set
     */
    public void conflationCount(int conflationCount);

    /**
     * When conflation is used, this value indicates the number of updates
     * conflated or aggregated into this {@link UpdateMsg}.
     * 
     * @return the conflationCount
     */
    public int conflationCount();

    /**
     * When conflation is used, this value indicates the period of time over
     * which individual updates were conflated or aggregated into this
     * {@link UpdateMsg} (typically in milliseconds).
     * Must be in the range of 0 - 65535.
     * 
     * @param conflationTime the conflationTime to set
     */
    public void conflationTime(int conflationTime);

    /**
     * When conflation is used, this value indicates the period of time over
     * which individual updates were conflated or aggregated into this
     * {@link UpdateMsg} (typically in milliseconds).
     * 
     * @return the conflationTime
     */
    public int conflationTime();

    /**
     * Specifies authorization information for this stream. When specified,
     * permData indicates authorization information for only the content within
     * this message, though this can be overridden for specific content within
     * the message (e.g. MapEntry.permData).
     * 
     * @param permData the permData to set
     */
    public void permData(Buffer permData);

    /**
     * Specifies authorization information for this stream. When specified,
     * permData indicates authorization information for only the content within
     * this message, though this can be overridden for specific content within
     * the message (e.g. MapEntry.permData).
     * 
     * @return the permData
     */
    public Buffer permData();

    /**
     * Identifies the user that posted this information.
     * 
     * @return the postUserInfo
     */
    public PostUserInfo postUserInfo();
}