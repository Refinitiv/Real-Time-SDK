/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Codec
{

	/// <summary>
	/// ETA <see cref="IRefreshMsg"/> contains payload information along with state, QoS,
	/// permissioning, and group information. <see cref="IRefreshMsg"/> is often provided as
	/// an initial response or when an upstream source requires a data
	/// resynchronization point.
	/// </summary>
	/// <remarks>
	///
	/// <para>
	/// <ul>
	/// <li>
	/// If provided as a response to a <see cref="IRefreshMsg"/>, the refresh is a
	/// solicited refresh. Typically, solicited refresh messages are delivered only
	/// to the requesting consumer application</li>
	/// <li>If some kind of information change occurs (e.g., some kind of error is
	/// detected on a stream), an upstream provider can push out a
	/// <see cref="IRefreshMsg"/> to downstream consumers. The refresh is an unsolicited
	/// refresh. Typically, unsolicited refresh messages are delivered to all
	/// consumers using the stream.</li>
	/// </ul>
	/// </para>
	/// <para>
	/// When the OMM Interactive Provider sends a <see cref="IRefreshMsg"/>, the streamId
	/// should match the streamId on the corresponding <see cref="IRequestMsg"/>. The msgKey
	/// should be populated appropriately, and also often matches the msgKey of the
	/// request. When an OMM NIP sends a <see cref="IRefreshMsg"/>, the provider should
	/// assign a negative streamId (when establishing a new stream, the streamId
	/// should be unique). In this scenario, the msgKey should define the information
	/// that the stream provides.
	/// </para>
	/// <para>
	/// Using <see cref="IRefreshMsg"/>, an application can fragment the contents of a
	/// message payload and deliver the content across multiple messages, with the
	/// final message indicating that the refresh is complete. This is useful when
	/// providing large sets of content that may require multiple cache look-ups or
	/// be too large for an underlying transport layer. Additionally, an application
	/// receiving multiple parts of a response can potentially begin processing
	/// received portions of data before all content has been received.
	///
	/// </para>
	/// </remarks>
	/// <seealso cref="IMsg"/>
	/// <seealso cref="RefreshMsgFlags"/>
	public interface IRefreshMsg : IMsg
	{
		/// <summary>
		/// Checks the presence of the Extended Header presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
		/// </summary>
		/// <returns> <c>true</c> - if exists; <c>false</c>if does not exist.
		/// </returns>
		/// <seealso cref="IMsg.Flags"/>
		bool CheckHasExtendedHdr();

        /// <summary>
        /// Checks the presence of the Permission Expression presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <returns> <c>true</c> - if exists; <c>false</c>if does not exist.
        /// </returns>
        /// <seealso cref="IMsg.Flags"/>
        bool CheckHasPermData();

        /// <summary>
        /// Checks the presence of the Message Key presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <returns> <c>true</c> - if exists; <c>false</c>if does not exist.
        /// </returns>
        /// <seealso cref="IMsg.Flags"/>
        bool CheckHasMsgKey();

        /// <summary>
        /// Checks the presence of the Sequence Number presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <returns> <c>true</c> - if exists; <c>false</c>if does not exist.
        /// </returns>
        /// <seealso cref="IMsg.Flags"/>
        bool CheckHasSeqNum();

        /// <summary>
        /// Checks the presence of the Part Number presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <returns> <c>true</c> - if exists; <c>false</c>if does not exist.
        /// </returns>
        /// <seealso cref="IMsg.Flags"/>
        bool CheckHasPartNum();

        /// <summary>
        /// Checks the presence of the Solicited indication flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <returns> <c>true</c> - if exists; <c>false</c>if does not exist.
        /// </returns>
        /// <seealso cref="IMsg.Flags"/>
        bool CheckSolicited();

        /// <summary>
        /// Checks the presence of the Refresh Complete indication flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <returns> <c>true</c> - if exists; <c>false</c>if does not exist.
        /// </returns>
        /// <seealso cref="IMsg.Flags"/>
        bool CheckRefreshComplete();

        /// <summary>
        /// Checks the presence of the Quality of Service presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <returns> <c>true</c> - if exists; <c>false</c>if does not exist.
        /// </returns>
        /// <seealso cref="IMsg.Flags"/>
        bool CheckHasQos();

        /// <summary>
        /// Checks the presence of the Clear Cache indication flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <returns> <c>true</c> - if exists; <c>false</c>if does not exist.
        /// </returns>
        /// <seealso cref="IMsg.Flags"/>
        bool CheckClearCache();

        /// <summary>
        /// Checks the presence of the Do Not Cache indication flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <returns> <c>true</c> - if exists; <c>false</c>if does not exist.
        /// </returns>
        /// <seealso cref="IMsg.Flags"/>
        bool CheckDoNotCache();

        /// <summary>
        /// Checks the presence of the Private Stream indication flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <returns> <c>true</c> - if exists; <c>false</c>if does not exist.
        /// </returns>
        /// <seealso cref="IMsg.Flags"/>
        bool CheckPrivateStream();

        /// <summary>
        /// Checks the presence of the Post User Information presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <returns> <c>true</c> - if exists; <c>false</c>if does not exist.
        /// </returns>
        /// <seealso cref="IMsg.Flags"/>
        bool CheckHasPostUserInfo();

        /// <summary>
        /// Checks the presence of the Qualified Stream indication flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <returns> <c>true</c> - if exists; <c>false</c>if does not exist.
        /// </returns>
        /// <seealso cref="IMsg.Flags"/>
        bool CheckQualifiedStream();

        /// <summary>
        /// Applies the Extended Header presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyHasExtendedHdr();

        /// <summary>
        /// Applies the Permission Expression presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyHasPermData();

        /// <summary>
        /// Applies the Message Key presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyHasMsgKey();

        /// <summary>
        /// Applies the Sequence Number presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyHasSeqNum();

        /// <summary>
        /// Applies the Part Number presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyHasPartNum();

        /// <summary>
        /// Applies the Solicited indication flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplySolicited();

        /// <summary>
        /// Applies the Refresh Complete indication flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyRefreshComplete();

        /// <summary>
        /// Applies the Quality of Service presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyHasQos();

        /// <summary>
        /// Applies the Clear Cache indication flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyClearCache();

        /// <summary>
        /// Applies the Do Not Cache indication flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyDoNotCache();

        /// <summary>
        /// Applies the Private Stream indication flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyPrivateStream();

        /// <summary>
        /// Applies the Post User Information presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyHasPostUserInfo();

        /// <summary>
        /// Applies the Qualified Stream indication flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyQualifiedStream();

		/// <summary>
		/// Sets the part number of this refresh. Must be in the range of 0 - 32767.
		/// <ul>
		/// <li>On multi-part refresh messages, partNum should start at 0 (to
		/// indicate the initial part) and increment by by 1 for each subsequent
		/// message in the multi-part message.</li>
		/// <li>If sent on a single-part refresh, a partNum of should be used.</li>
		/// </ul>
		///
		/// </summary>
		int PartNum { get; set; }

		/// <summary>
		/// Gets or sets a user-defined sequence number. Should typically be increasing to help
		/// with temporal ordering, but may have gaps depending on the sequencing
		/// algorithm being used. Details about sequence number use should be defined
		/// within the domain model specification or any documentation for products
		/// which require the use of seqNum. Must be in the range of 0 - 4294967296 (2^32).
		/// </summary>
		long SeqNum { get; set; }

		/// <summary>
		/// State. Conveys stream and data state information which can change over
		/// time via subsequent refresh or status messages, or group status
		/// notifications.
		/// </summary>
		/// <value> the state </value>
		State State { get; }

		/// <summary>
		/// Gets or sets group identifier <see cref="Buffer"/> containing information about the item
		/// group to which this stream belongs. You can change the associated groupId
		/// via a subsequent <see cref="IStatusMsg"/> or <see cref="IRefreshMsg"/>. Group status
		/// notifications can change the state of an entire group of items.
		/// </summary>
		Buffer GroupId { get; set; }

        /// <summary>
        /// Gets or sets permission data.Specifies authorization information for this stream. When
        /// specified, permData indicates authorization information for only the
        /// content within this message, though this can be overridden for specific
        /// content within the message (e.g., MapEntry.permData).
        /// </summary>
        Buffer PermData { get; set; }

		/// <summary>
		/// The QoS of the stream. If a range was requested by the <see cref="IRequestMsg"/>
		/// , the qos should fall somewhere in this range, otherwise qos should
		/// exactly match what was requested..
		/// </summary>
		/// <value> the qos </value>
		Qos Qos { get; }

		/// <summary>
		/// Contains information that identifies the user posting this information.
		/// If present on a <see cref="IRefreshMsg"/>, this implies that this refresh was
		/// posted to the system by the user described in postUserInfo.
		/// </summary>
		/// <value> the postUserInfo </value>
		PostUserInfo PostUserInfo { get; }
	}
}