package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.State;

/**
 * <p>
 * UPA {@link RefreshMsg} contains payload information along with state, QoS,
 * permissioning, and group information. {@link RefreshMsg} is often provided as
 * an initial response or when an upstream source requires a data
 * resynchronization point.
 * <ul>
 * <li>
 * If provided as a response to a {@link RefreshMsg}, the refresh is a
 * solicited refresh. Typically, solicited refresh messages are delivered only
 * to the requesting consumer application</li>
 * <li>If some kind of information change occurs (e.g., some kind of error is
 * detected on a stream), an upstream provider can push out a
 * {@link RefreshMsg} to downstream consumers. The refresh is an unsolicited
 * refresh. Typically, unsolicited refresh messages are delivered to all
 * consumers using the stream.</li>
 * </ul>
 * <p>
 * When the OMM Interactive Provider sends a {@link RefreshMsg}, the streamId
 * should match the streamId on the corresponding {@link RequestMsg}. The msgKey
 * should be populated appropriately, and also often matches the msgKey of the
 * request. When an OMM NIP sends a {@link RefreshMsg}, the provider should
 * assign a negative streamId (when establishing a new stream, the streamId
 * should be unique). In this scenario, the msgKey should define the information
 * that the stream provides.
 * <p>
 * Using {@link RefreshMsg}, an application can fragment the contents of a
 * message payload and deliver the content across multiple messages, with the
 * final message indicating that the refresh is complete. This is useful when
 * providing large sets of content that may require multiple cache look-ups or
 * be too large for an underlying transport layer. Additionally, an application
 * receiving multiple parts of a response can potentially begin processing
 * received portions of data before all content has been received.
 * 
 * 
 * @see Msg
 * @see RefreshMsgFlags
 */
public interface RefreshMsg extends Msg
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
     * <p>* Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasSeqNum();

    /**
     * Checks the presence of the Part Number presence flag.
     *
     * <p>* Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasPartNum();

    /**
     * Checks the presence of the Solicited indication flag.
     *
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkSolicited();

    /**
     * Checks the presence of the Refresh Complete indication flag.
     *
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkRefreshComplete();

    /**
     * Checks the presence of the Quality of Service presence flag.
     *
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasQos();

    /**
     * Checks the presence of the Clear Cache indication flag.
     *
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkClearCache();

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
     * Checks the presence of the Private Stream indication flag.
     *
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkPrivateStream();

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
     * Checks the presence of the Qualified Stream indication flag.
     *
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkQualifiedStream();

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
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasPartNum();

    /**
     * Applies the Solicited indication flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applySolicited();

    /**
     * Applies the Refresh Complete indication flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyRefreshComplete();

    /**
     * Applies the Quality of Service presence flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasQos();

    /**
     * Applies the Clear Cache indication flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyClearCache();

    /**
     * Applies the Do Not Cache indication flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyDoNotCache();

    /**
     * Applies the Private Stream indication flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     **/
    public void applyPrivateStream();

    /**
     * Applies the Post User Information presence flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasPostUserInfo();

    /**
     * Applies the Qualified Stream indication flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     **/
    public void applyQualifiedStream();

    /**
     * Sets the part number of this refresh. Must be in the range of 0 - 32767.
     * <ul>
     * <li>On multi-part refresh messages, partNum should start at 0 (to
     * indicate the initial part) and increment by by 1 for each subsequent
     * message in the multi-part message.</li>
     * <li>If sent on a single-part refresh, a partNum of should be used.</li>
     * </ul>
     * 
     * 
     * @param partNum the partNum to set
     */
    public void partNum(int partNum);

    /**
     * Part number of this refresh.
     * <ul>
     * <li>On multi-part refresh messages, partNum should start at 0 (to
     * indicate the initial part) and increment by by 1 for each subsequent
     * message in the multi-part message.</li>
     * <li>If sent on a single-part refresh, a partNum of should be used.</li>
     * </ul>
     * 
     * @return the partNum
     */
    public int partNum();

    /**
     * A user-defined sequence number. Should typically be increasing to help
     * with temporal ordering, but may have gaps depending on the sequencing
     * algorithm being used. Details about sequence number use should be defined
     * within the domain model specification or any documentation for products
     * which require the use of seqNum. Must be in the range of 0 - 4294967296 (2^32).
     * 
     * @param seqNum the seqNum to set
     */
    public void seqNum(long seqNum);

    /**
     * A user-defined sequence number. Should typically be increasing to help
     * with temporal ordering, but may have gaps depending on the sequencing
     * algorithm being used. Details about sequence number use should be defined
     * within the domain model specification or any documentation for products
     * which require the use of seqNum.
     * 
     * @return the seqNum
     */
    public long seqNum();

    /**
     * State. Conveys stream and data state information which can change over
     * time via subsequent refresh or status messages, or group status
     * notifications.
     * 
     * @return the state
     */
    public State state();

    /**
     * Group identifier {@link Buffer} containing information about the item
     * group to which this stream belongs. You can change the associated groupId
     * via a subsequent {@link StatusMsg} or {@link RefreshMsg}. Group status
     * notifications can change the state of an entire group of items.
     * 
     * @param groupId the groupId to set
     */
    public void groupId(Buffer groupId);

    /**
     * Group identifier {@link Buffer} containing information about the item
     * group to which this stream belongs. You can change the associated groupId
     * via a subsequent {@link StatusMsg} or {@link RefreshMsg}. Group status
     * notifications can change the state of an entire group of items.
     * 
     * @return the groupId
     */
    public Buffer groupId();

    /**
     * Permission data.Specifies authorization information for this stream. When
     * specified, permData indicates authorization information for only the
     * content within this message, though this can be overridden for specific
     * content within the message (e.g., MapEntry.permData).
     * 
     * @param permData the permData to set
     */
    public void permData(Buffer permData);

    /**
     * Permission data.Specifies authorization information for this stream. When
     * specified, permData indicates authorization information for only the
     * content within this message, though this can be overridden for specific
     * content within the message (e.g., MapEntry.permData).
     * 
     * @return the permData
     */
    public Buffer permData();

    /**
     * The QoS of the stream. If a range was requested by the {@link RequestMsg}
     * , the qos should fall somewhere in this range, otherwise qos should
     * exactly match what was requested..
     * 
     * @return the qos
     */
    public Qos qos();

    /**
     * Contains information that identifies the user posting this information.
     * If present on a {@link RefreshMsg}, this implies that this refresh was
     * posted to the system by the user described in postUserInfo.
     * 
     * @return the postUserInfo
     */
    public PostUserInfo postUserInfo();
}
