/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Codec
{

	/// <summary>
	/// ETA acknowledgment message is sent from a provider to a consumer to indicate receipt of a specific message.
	/// The acknowledgment carries success or failure (negative acknowledgment or Nak) information to the consumer.
	/// Currently, a consumer can request acknowledgment for a <see cref="IPostMsg"/> or a <see cref="ICloseMsg"/>.
	/// </summary>
	/// <seealso cref="IMsg"/>
	/// <seealso cref="ICloseMsg"/>
	/// <seealso cref="IPostMsg"/>
	/// <seealso cref="AckMsgFlags"/>
	/// <seealso cref="NakCodes"/>
	public interface IAckMsg : IMsg
	{
		/// <summary>
		/// Checks the presence of the Extended Header flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
		/// </summary>
		/// <returns> <c>true</c> - if exists; <c>false</c> if does not exist.
		/// </returns>
		/// <seealso cref="IMsg.Flags"/>
		bool CheckHasExtendedHdr();

		/// <summary>
		/// Checks the presence of the Text flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
		/// </summary>
		/// <returns> <c>true</c> - if exists; <c>false</c> if does not exist.
		/// </returns>
		/// <seealso cref="IMsg.Flags"/>
		bool CheckHasText();

		/// <summary>
		/// Checks the presence of the Private Stream flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
		/// </summary>
		/// <returns> <c>true</c> - if exists; <c>false</c> if does not exist.
		/// </returns>
		/// <seealso cref="IMsg.Flags"/>
		bool CheckPrivateStream();

		/// <summary>
		/// Checks the presence of the Sequence Number flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
		/// </summary>
		/// <returns> <c>true</c> - if exists; <c>false</c> if does not exist.
		/// </returns>
		/// <seealso cref="IMsg.Flags"/>
		bool CheckHasSeqNum();

		/// <summary>
		/// Checks the presence of the Message Key flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
		/// </summary>
		/// <returns> <c>true</c> - if exists; <c>false</c> if does not exist.
		/// </returns>
		/// <seealso cref="IMsg.Flags"/>
		bool CheckHasMsgKey();

		/// <summary>
		/// Checks the presence of the NAK Code flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
		/// </summary>
		/// <returns> <c>true</c> - if exists; <c>false</c> if does not exist.
		/// </returns>
		/// <seealso cref="IMsg.Flags"/>
		bool CheckHasNakCode();

		/// <summary>
		/// Checks the presence of the Qualified Stream flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
		/// </summary>
		/// <returns> <c>true</c> - if exists; <c>false</c> if does not exist.
		/// </returns>
		/// <seealso cref="IMsg.Flags"/>
		bool CheckQualifiedStream();

		/// <summary>
		/// Applies the Extended Header indication flag.<br />
		/// <br />
		/// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
		/// </summary>
		/// <seealso cref="IMsg.Flags"/>
		void ApplyHasExtendedHdr();

        /// <summary>
        /// Applies the Text indication flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyHasText();

		/// <summary>
		/// Applies the Private Stream indication flag.<br />
		/// <br />
		/// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
		/// </summary>
		/// <seealso cref="IMsg.Flags"/>
		void ApplyPrivateStream();

        /// <summary>
        /// Applies the Sequence Number indication flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyHasSeqNum();

        /// <summary>
        /// Applies the Message Key indication flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyHasMsgKey();

        /// <summary>
        /// Applies the NAK Code indication flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyHasNakCode();

        /// <summary>
        /// Applies the Qualified Stream indication flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyQualifiedStream();

        /// <summary>
        /// Gets or sets ID used to associate this Ack with the message it is acknowledging.
        /// Valid range is 0-4294967296 (2^32).
        /// </summary>
        long AckId { get; set; }

		/// <summary>
		/// Gets or sets this Ack indicates negative acknowledgment, if 0 or not
		/// present, Ack indicates positive acknowledgment.
		/// </summary>
		/// <seealso cref="NakCodes"/>
		int NakCode { get; set; }

		/// <summary>
		/// Gets or sets a user-defined sequence number.
		/// </summary>
		/// <remarks>
		/// <para>
		/// To help with temporal ordering, seqNum should increase, though gaps might
		/// exist depending on the sequencing algorithm in use.</para>
		///
		/// <para>
        /// The acknowledgment message may populate this with the seqNum of the
		/// <see cref="IPostMsg"/> being acknowledged.</para>
		///
		/// <para>
		/// This helps correlate the message being acknowledged when the postId alone
		/// is not sufficient (e.g., multi-part post messages).</para>
        ///
		/// <para>
		/// Must be in the range of 0 - 4294967296 (2<sup>32</sup>).</para>
		/// 
		/// </remarks>
		long SeqNum { get; set; }

		/// <summary>
		/// Gets or sets text describes additional information about the acceptance or rejection
		/// of the message being acknowledged.
		/// </summary>
		Buffer Text { get; set; }
	}
}