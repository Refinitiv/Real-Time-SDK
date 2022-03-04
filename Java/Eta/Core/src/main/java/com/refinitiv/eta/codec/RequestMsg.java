/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.Qos;

/**
 * ETA {@link RequestMsg} is used by an OMM consumer to express interest in a
 * particular information stream. The request's msgKey members help identify the
 * stream and priority information can be used to indicate the streams
 * importance to the consumer. {@link Qos} information can be used to express
 * either a specific desired QoS or a range of acceptable qualities of service
 * that can satisfy the request.
 * <p>
 * When a {@link RequestMsg} is issued with a new streamId, this is considered
 * a request to open the stream. If requested information is available and the
 * consumer is entitled to receive the information, this will typically result
 * in a {@link RefreshMsg} being delivered to the consumer, though a
 * {@link StatusMsg} is also possible - either message can be used to indicate a
 * stream is open. If information is not available or the user is not entitled,
 * a {@link StatusMsg} is typically delivered to provide more detailed
 * information to the consumer.
 * <p>
 * Issuing a {@link RequestMsg} on an existing stream allows a consumer to
 * modify some parameters associated with the stream. Also known as a reissue,
 * this can be used to Pause or Resume a stream, change a Dynamic View, increase
 * or decrease the streams priority or request that a new refresh is delivered.
 * 
 * 
 * @see Msg
 * @see RequestMsgFlags
 */
public interface RequestMsg extends Msg
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
     * Checks the presence of the Priority presence flag.
     *
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasPriority();

    /**
     * Checks the presence of the Streaming indication flag.
     *
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkStreaming();

    /**
     * Checks the presence of the Message Key presence flag.
     *
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkMsgKeyInUpdates();

    /**
     * Checks the presence of the Conflation Info presence flag.
     *
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkConfInfoInUpdates();

    /**
     * Checks the presence of the No Refresh indication flag.
     *
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkNoRefresh();

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
     * Checks the presence of the Worst Quality of Service presence flag.
     *
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasWorstQos();

    /**
     ** Checks the presence of the Private Stream indication flag.
     *
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     ** 
     ** @return true - if exists; false if does not exist.
     **/
    public boolean checkPrivateStream();

    /**
     * Checks the presence of the Pause indication flag.
     *
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkPause();

    /**
     * Checks the presence of the View indication flag.
     *
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasView();

    /**
     * Checks the presence of the Batch indication flag.
     *
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasBatch();

    /**
     ** Checks the presence of the Qualified Stream indication flag.
     *
     * <p>Flags may also be bulk-get via {@link Msg#flags()}.
     * 
     * @see Msg#flags()
     ** 
     ** @return true - if exists; false if does not exist.
     **/
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
     * Applies the Priority presence flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasPriority();

    /**
     * Applies the Streaming indication flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyStreaming();

    /**
     * Applies the Message Key presence flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyMsgKeyInUpdates();

    /**
     * Applies the Conflation Information in Updates indication flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyConfInfoInUpdates();

    /**
     * Applies the No Refresh indication flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyNoRefresh();

    /**
     * Applies the Quality of Service presence flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasQos();

    /**
     * Applies the Worst Quality of Service presence flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasWorstQos();

    /**
     ** Applies the Private Stream indication flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     **/
    public void applyPrivateStream();

    /**
     * Applies the Pause indication flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyPause();

    /**
     * Applies the View indication flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasView();

    /**
     * Applies the Batch indication flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     */
    public void applyHasBatch();

    /**
     ** Applies the Qualified Stream indication flag.
     *
     * <p>Flags may also be bulk-set via {@link Msg#flags(int)}.
     * 
     * @see Msg#flags(int)
     **/
    public void applyQualifiedStream();

    /**
     * Priority Structure (priority for the requested stream).
     * 
     * @return the priority
     */
    public Priority priority();

    /**
     * QoS for the requested stream.
     * <ul>
     * <li>When specified without a worstQos member, this is the only allowable
     * QoS for the requested stream. If this QoS is unavailable, the stream is not opened.</li>
     * <li>When specified with a worstQos, this is the best in the range of
     * allowable QoSs. When a QoS range is specified, any QoS within the range
     * is acceptable for servicing the stream.</li>
     * </ul>
     * <p>
     * If neither qos nor worstQos are present on the request, this indicates
     * that any available QoS will satisfy the request. Some components may
     * require qos on initial request and reissue messages.
     * 
     * @return the qos
     */
    public Qos qos();

    /**
     * The least acceptable QoS for the requested stream. When specified with a
     * qos value, this is the worst in the range of allowable QoSs. When a QoS
     * range is specified, any QoS within the range is acceptable for servicing the stream.
     * 
     * @return the worstQos
     */
    public Qos worstQos();
}