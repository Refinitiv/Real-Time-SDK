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
	/// ETA Update Message is used by Interactive and Non-Interactive Providers to
	/// convey changes to data associated with an item stream. When streaming, update
	/// messages typically flow after an initial refresh has been delivered. Update
	/// messages can be delivered between parts of a multi-part refresh message, even
	/// in response to a non-streaming request.
	/// 
	/// <para>
	/// Some providers can aggregate the information from multiple update messages
	/// into a single update message. This is known as conflation, and typically
	/// occurs if a conflated quality of service is requested, a stream is paused, or
	/// a consuming application is unable to keep up with the data rates associated
	/// with the stream.
	/// 
	/// </para>
	/// </summary>
	/// <seealso cref="IMsg"/>
	/// <seealso cref="UpdateMsgFlags" />
	public interface IUpdateMsg : IMsg
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
		/// Checks the presence of the Permission Expression presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <seealso cref="IMsg.Flags"/>.
		/// </summary>
		/// <seealso cref="IMsg.Flags"/>
		/// <returns> true - if exists; false if does not exist. </returns>
		bool CheckHasPermData();

		/// <summary>
		/// Checks the presence of the Message Key presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
		/// </summary>
		/// <seealso cref="IMsg.Flags"/>
		/// <returns> true - if exists; false if does not exist. </returns>
		bool CheckHasMsgKey();

        /// <summary>
        /// Checks the presence of the Sequence Number presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckHasSeqNum();

        /// <summary>
        /// Checks the presence of the Conflation Information presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckHasConfInfo();

        /// <summary>
        /// Checks the presence of the Do Not Cache indication flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckDoNotCache();

        /// <summary>
        /// Checks the presence of the Do Not Conflate indication flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckDoNotConflate();

        /// <summary>
        /// Checks the presence of the Do Not Ripple indication flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckDoNotRipple();

        /// <summary>
        /// Checks the presence of the Post User Information presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckHasPostUserInfo();

        /// <summary>
        /// Checks the presence of the Discardable presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckDiscardable();

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
        /// Applies the Conflation Info presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyHasConfInfo();

        /// <summary>
        /// Applies the Do Not Cache indication flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyDoNotCache();

        /// <summary>
        /// Applies the Do Not Conflate indication flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyDoNotConflate();

        /// <summary>
        /// Applies the Do Not Ripple indication flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyDoNotRipple();

        /// <summary>
        /// Applies the Post User Info presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyHasPostUserInfo();

        /// <summary>
        /// Applies the Discardable presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyDiscardable();

		/// <summary>
		/// Gets or sets the type of data in the <seealso cref="IUpdateMsg"/>. Examples of possible
		/// update types include: Trade, Quote, or Closing Run.
		/// Must be in the range of 0 - 255.
		/// <ul>
		/// <li>
		/// Domain message model specifications define available update types</li>
		/// <li>
		/// For Thomson Reuters's provided domain models, see
		/// <seealso cref="Rdm.UpdateEventTypes"/></li>
		/// </ul>
		/// </summary>
		int UpdateType { get; set; }

		/// <summary>
		/// Gets or sets a user-defined sequence number. To help with temporal ordering,
		/// seqNum should increase across messages, but can have gaps depending on
		/// the sequencing algorithm in use. Details about sequence number use should
		/// be defined within the domain model specification or any documentation for
		/// products which require the use of seqNum.
		/// Must be in the range of 0 - 4294967296 (2^32).
		/// </summary>
		long SeqNum { get; set; }

		/// <summary>
		/// When conflation is used, this value indicates the number of updates
		/// conflated or aggregated into this <seealso cref="IUpdateMsg"/>.
		/// Must be in the range of 0 - 32767.
		/// </summary>
		int ConflationCount { get; set; }

		/// <summary>
		/// When conflation is used, this value indicates the period of time over
		/// which individual updates were conflated or aggregated into this
		/// <seealso cref="IUpdateMsg"/> (typically in milliseconds).
		/// Must be in the range of 0 - 65535.
		/// </summary>
		int ConflationTime { get; set; }

		/// <summary>
		/// Gets or sets authorization information for this stream. When specified,
		/// permData indicates authorization information for only the content within
		/// this message, though this can be overridden for specific content within
		/// the message (e.g. MapEntry.permData).
		/// </summary>
		Buffer PermData { get; set; }

		/// <summary>
		/// Identifies the user that posted this information.
		/// </summary>
		/// <returns> the postUserInfo </returns>
		PostUserInfo PostUserInfo { get; }
	}
}