package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.State;

/**
 * UPA Status Message is used to indicate changes to the stream or data
 * properties. This message can convey changes in streamState or dataState,
 * changes in a stream's permissioning information, and changes to the item
 * group that the stream is part of. A Provider application uses the
 * {@link StatusMsg} to close streams to a Consumer, both in conjunction with an
 * initial request or at some point after the stream has been established. A
 * {@link StatusMsg} can also be used to indicate successful establishment of a
 * stream, even though the message may not contain any data - this can be useful
 * when establishing a stream solely to exchange bi-directional {@link GenericMsg}.
 * 
 * @see Msg
 * @see StatusMsgFlags
 */
public interface StatusMsg extends Msg
{
    /**
     * Checks the presence of the Extended Header presence flag.<br />
     * <br />
     * Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasExtendedHdr();

    /**
     * Checks the presence of the Permission Expression presence flag.<br />
     * <br />
     * Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasPermData();

    /**
     * Checks the presence of the Message Key presence flag.<br />
     * <br />
     * Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasMsgKey();

    /**
     * Checks the presence of the Group Id presence flag.<br />
     * <br />
     * Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasGroupId();

    /**
     * Checks the presence of the State presence flag.<br />
     * <br />
     * Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasState();

    /**
     * Checks the presence of the Clear Cache indication flag.<br />
     * <br />
     * Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkClearCache();

    /**
     * Checks the presence of the Private Stream indication flag.<br />
     * <br />
     * Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     **/
    public boolean checkPrivateStream();

    /**
     * Checks the presence of the Post User Information presence flag.<br />
     * <br />
     * Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasPostUserInfo();

    /**
     * Checks the presence of the Qualified Stream indication flag.<br />
     * <br />
     * Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkQualifiedStream();

    /**
     * Applies the Extended Header presence flag.<br />
     * <br />
     * Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasExtendedHdr();

    /**
     * Applies the Permission Expression presence flag.<br />
     * <br />
     * Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasPermData();

    /**
     * Applies the Message Key presence flag.<br />
     * <br />
     * Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasMsgKey();

    /**
     * Applies the Group Id presence flag.<br />
     * <br />
     * Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasGroupId();

    /**
     * Applies the State presence flag.<br />
     * <br />
     * Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasState();

    /**
     * Applies the Clear Cache indication flag.<br />
     * <br />
     * Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyClearCache();

    /**
     ** Applies the Private Stream indication flag.<br />
     * <br />
     * Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     **/
    public void applyPrivateStream();

    /**
     * Applies the Post User Information presence flag.<br />
     * <br />
     * Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasPostUserInfo();

    /**
     * Applies the Qualified Stream indication flag.<br />
     * <br />
     * Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     **/
    public void applyQualifiedStream();

    /*
     * Sets the state field containing information about stream and data state.
     * 
     * @param state the state to set
     */
    public void state(State state);
    
    /**
     * Conveys stream and data state information, which can change over time via
     * subsequent refresh or status messages, or group status notifications.
     * 
     * @return the state
     */
    public State state();

    /**
     * A {@link Buffer} containing information about the item group to which
     * this stream belongs. A subsequent {@link StatusMsg} or {@link RefreshMsg}
     * can change the item group's associated groupId, while group status
     * notifications can change the state of an entire group of items.
     * 
     * @param groupId the groupId to set
     */
    public void groupId(Buffer groupId);

    /**
     * A {@link Buffer} containing information about the item group to which
     * this stream belongs. A subsequent {@link StatusMsg} or {@link RefreshMsg}
     * can change the item group's associated groupId, while group status
     * notifications can change the state of an entire group of items.
     * 
     * @return the groupId
     */
    public Buffer groupId();

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
     * Identifies the user who posted this information.
     * 
     * @return the postUserInfo
     */
    public PostUserInfo postUserInfo();
}