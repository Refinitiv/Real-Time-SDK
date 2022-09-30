/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.Codec
{

    /// <summary>
    /// ETA <seealso cref="IRequestMsg"/> is used by an OMM consumer to express interest in a
    /// particular information stream. The request's msgKey members help identify the
    /// stream and priority information can be used to indicate the streams
    /// importance to the consumer. <seealso cref="Refinitiv.Eta.Codec.Qos"/> information can be used to express
    /// either a specific desired QoS or a range of acceptable qualities of service
    /// that can satisfy the request.
    /// <para>
    /// When a <seealso cref="IRequestMsg"/> is issued with a new streamId, this is considered
    /// a request to open the stream. If requested information is available and the
    /// consumer is entitled to receive the information, this will typically result
    /// in a <seealso cref="IRefreshMsg"/> being delivered to the consumer, though a
    /// <seealso cref="IStatusMsg"/> is also possible - either message can be used to indicate a
    /// stream is open. If information is not available or the user is not entitled,
    /// a <seealso cref="IStatusMsg"/> is typically delivered to provide more detailed
    /// information to the consumer.
    /// </para>
    /// <para>
    /// Issuing a <seealso cref="IRequestMsg"/> on an existing stream allows a consumer to
    /// modify some parameters associated with the stream. Also known as a reissue,
    /// this can be used to Pause or Resume a stream, change a Dynamic View, increase
    /// or decrease the streams priority or request that a new refresh is delivered.
    /// 
    /// 
    /// </para>
    /// </summary>
    /// <seealso cref="IMsg"/>
    /// <seealso cref="RequestMsgFlags"/>
    public interface IRequestMsg : IMsg
	{
		/// <summary>
		/// Checks the presence of the Extended Header presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <seealso cref="IMsg.Flags"/>.
		/// </summary>
		/// <seealso cref="IMsg.Flags"/>
		/// <returns> true - if exists; false if does not exist. </returns>
		bool CheckHasExtendedHdr();

        /// <summary>
        /// Checks the presence of the Priority presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckHasPriority();

        /// <summary>
        /// Checks the presence of the Streaming indication flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckStreaming();

        /// <summary>
        /// Checks the presence of the Message Key presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckMsgKeyInUpdates();

        /// <summary>
        /// Checks the presence of the Conflation Info presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckConfInfoInUpdates();

        /// <summary>
        /// Checks the presence of the No Refresh indication flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckNoRefresh();

        /// <summary>
        /// Checks the presence of the Quality of Service presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckHasQos();

        /// <summary>
        /// Checks the presence of the Worst Quality of Service presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckHasWorstQos();

        /// <summary>
        /// ** Checks the presence of the Private Stream indication flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist.
        ///  </returns>
        bool CheckPrivateStream();

        /// <summary>
        /// Checks the presence of the Pause indication flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckPause();

        /// <summary>
        /// Checks the presence of the View indication flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckHasView();

        /// <summary>
        /// Checks the presence of the Batch indication flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckHasBatch();

        /// <summary>
        /// Checks the presence of the Qualified Stream indication flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist.
        ///  </returns>
        bool CheckQualifiedStream();

        /// <summary>
        /// Applies the Extended Header presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyHasExtendedHdr();

        /// <summary>
        /// Applies the Priority presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyHasPriority();

        /// <summary>
        /// Applies the Streaming indication flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyStreaming();

        /// <summary>
        /// Applies the Message Key presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyMsgKeyInUpdates();

        /// <summary>
        /// Applies the Conflation Information in Updates indication flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyConfInfoInUpdates();

        /// <summary>
        /// Applies the No Refresh indication flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyNoRefresh();

        /// <summary>
        /// Applies the Quality of Service presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyHasQos();

        /// <summary>
        /// Applies the Worst Quality of Service presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyHasWorstQos();

        /// <summary>
        /// ** Applies the Private Stream indication flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyPrivateStream();

        /// <summary>
        /// Applies the Pause indication flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyPause();

        /// <summary>
        /// Applies the View indication flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyHasView();

        /// <summary>
        /// Applies the Batch indication flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyHasBatch();

        /// <summary>
        /// Applies the Qualified Stream indication flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyQualifiedStream();

		/// <summary>
		/// Priority Structure (priority for the requested stream).
		/// </summary>
		/// <returns> the priority </returns>
		Priority Priority { get; }

		/// <summary>
		/// QoS for the requested stream.
		/// <ul>
		/// <li>When specified without a worstQos member, this is the only allowable
		/// QoS for the requested stream. If this QoS is unavailable, the stream is not opened.</li>
		/// <li>When specified with a worstQos, this is the best in the range of
		/// allowable QoSs. When a QoS range is specified, any QoS within the range
		/// is acceptable for servicing the stream.</li>
		/// </ul>
		/// <para>
		/// If neither qos nor worstQos are present on the request, this indicates
		/// that any available QoS will satisfy the request. Some components may
		/// require qos on initial request and reissue messages.
		/// 
		/// </para>
		/// </summary>
		/// <returns> the qos </returns>
		Qos Qos { get; }

		/// <summary>
		/// The least acceptable QoS for the requested stream. When specified with a
		/// qos value, this is the worst in the range of allowable QoSs. When a QoS
		/// range is specified, any QoS within the range is acceptable for servicing the stream.
		/// </summary>
		/// <returns> the worstQos </returns>
		Qos WorstQos { get; }
	}
}