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
	/// A bi-directional ETA message that does not have any implicit interaction
	/// semantics associated with it, thus the name generic. Once a stream is
	/// established via a request-refresh/status interaction, this message can be
	/// sent from consumer to provider as well as from provider to consumer, and can
	/// also be leveraged by non-interactive provider applications. Generic messages
	/// are transient and are typically not cached by any Enterprise Platform
	/// components. The msgKey of a <see cref="IGenericMsg"/> does not need to match the
	/// <see cref="IMsgKey"/> information associated with the stream the generic message is
	/// flowing on. This allows for the key information to be used independently of
	/// the stream. Any specific message usage, msgKey usage, expected interactions,
	/// and handling instructions are typically defined by a domain message model
	/// specification.
	/// </summary>
	/// <seealso cref="IMsg"/>
	/// <seealso cref="GenericMsgFlags"/>
	public interface IGenericMsg : IMsg
	{
		/// <summary>
		/// Checks the presence of the Extended Header presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
		/// </summary>
		/// <seealso cref="IMsg.Flags"/>
		/// <returns> true - if exists; false if does not exist. </returns>
		bool CheckHasExtendedHdr();

        /// <summary>
        /// Checks the presence of the Permission Expression presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
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
        /// Checks the presence of the Part Number presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckHasPartNum();

        /// <summary>
        /// Checks the presence of the Message Complete indication flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckMessageComplete();

        /// <summary>
        /// Checks the presence of the Secondary Sequence Number presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckHasSecondarySeqNum();

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
        /// Applies the Message Complete indication flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyMessageComplete();

        /// <summary>
        /// Applies the Secondary Sequence Number presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyHasSecondarySeqNum();

		/// <summary>
		/// Gets or sets the part number of this generic message, typically used with
		/// multi-part generic messages. If sent on a single-part post message, use a
		/// partNum of 0. On multi-part post messages, use a partNum of 0 on the
		/// initial part and increment partNum in each subsequent part by 1.
		/// </summary>
		int PartNum { get; set; }

		/// <summary>
		/// Gets or sets a user-defined sequence number and typically corresponds to the
		/// sequencing of this message. To help with temporal ordering, seqNum should
		/// increase across messages, but can have gaps depending on the sequencing
		/// algorithm in use. Details about using seqNum should be defined in the
		/// domain model specification or the documentation for products which
		/// require the use of seqNum.
		/// </summary>
		long SeqNum { get; set; }

		/// <summary>
		/// Gets or sets an additional user-defined sequence number. When using
		/// <seealso cref="IGenericMsg"/> on a stream in a bi-directional manner,
		/// secondarySeqNum is often used as an acknowledgment sequence number. For
		/// example, a consumer sends a generic message with seqNum populated to
		/// indicate the sequence of this message in the stream and secondarySeqNum
		/// set to the seqNum last received from the provider. This effectively
		/// acknowledges all messages received up to that point while also sending
		/// additional information. Sequence number use should be defined within the
		/// domain model specification or any documentation for products that use
		/// secondarySeqNum.
		/// </summary>
		long SecondarySeqNum { get; set; }

		/// <summary>
		/// Gets or sets authorization information for this stream. When permData is
		/// specified, it indicates authorization information for only the content
		/// within this message, though this can be overridden for specific content
		/// within the message (for example, MapEntry.permData).
		/// </summary>
		Buffer PermData { get; set; }
	}
}