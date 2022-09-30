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
    /// ETA Post Message allows a Consumer application to push content to upstream
    /// components. This information can be applied to an Enterprise Platform cache
    /// or routed further upstream to the source of data. Once received, the upstream
    /// components can republish data to downstream consumers. Post messages can be
    /// routed along a specific item stream, referred to as on-stream posting, or
    /// along a user's Login stream, referred to as off-stream posting.
    /// A <seealso cref="IPostMsg"/> can contain any ETA container type, including other messages.
    /// User identification information can be associated with a post message and can
    /// be provided along with the content that was posted.
    /// </summary>
    /// <seealso cref="IMsg"/>
    /// <seealso cref="Refinitiv.Eta.Codec.PostUserRights"/>
    /// <seealso cref="PostMsgFlags"/>
    public interface IPostMsg : IMsg
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
        /// Checks the presence of the Post Id presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckHasPostId();

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
        /// Checks the presence of the Post Complete indication flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckPostComplete();

        /// <summary>
        /// Checks the presence of the Acknowledgment indication flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckAck();

        /// <summary>
        /// Checks the presence of the Permission Expression presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckHasPermData();

        /// <summary>
        /// Checks the presence of the Post User Rights presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckHasPostUserRights();

        /// <summary>
        /// Applies the Extended Header presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyHasExtendedHdr();

        /// <summary>
        /// Applies the Post Id presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyHasPostId();

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
        /// Applies the Post Complete indication flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyPostComplete();

        /// <summary>
        /// Applies the Acknowledgment indication flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyAck();

        /// <summary>
        /// Applies the Permission Expression presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyHasPermData();

        /// <summary>
        /// Applies the Post User Rights presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyHasPostUserRights();

		/// <summary>
		/// Gets or sets the part number for this post message, typically used with
		/// multi-part post messages. If sent on a single-part post message, use a
		/// partNum of 0. On multi-part post messages, use a partNum of 0, on the
		/// initial part and increment partNum in each subsequent part by 1. Must
		/// be in the range of 0 - 32767.
		/// </summary>
		int PartNum { get; set; }

		/// <summary>
		/// Gets or sets a user-defined sequence number (typically corresponding to the
		/// sequencing of the message). To help with temporal ordering, seqNum should
		/// increase, though gaps might exist depending on the sequencing algorithm
		/// in use. Details about seqNum use should be defined in the domain model
		/// specification or any documentation for products that use seqNum. When
		/// acknowledgments are requested, the seqNum will be provided back in the
		/// <see cref="IAckMsg"/> to help identify the <see cref="IPostMsg"/> being acknowledged.
		/// Must be in the range of 0 - 4294967296 (2^32).
		/// </summary>
		long SeqNum { get; set; }

		/// <summary>
		/// Gets or sets a consumer-assigned identifier that distinguishes different
		/// post messages. Each part in a multi-part post message should use the same
		/// postId value. Must be in the range of 0 - 4294967296 (2^32).
		/// </summary>
		long PostId { get; set; }

		/// <summary>
		/// Gets or sets authorization information for this stream. When permData is
		/// specified, it indicates authorization information for only the content
		/// within this message, though this can be overridden for specific content
		/// within the message (e.g. MapEntry.permData).
		/// </summary>
		Buffer PermData { get; set; }


        /// <summary>
        /// Gets or sets the rights or abilities of the user posting this content. This
        /// can indicate whether the user is permissioned to:
        /// <ul>
        /// <li>Create items in the cache of record. (0x01)</li>
        /// <li>Delete items from the cache of record. (0x02)</li>
        /// <li>Modify the permData on items already present in the cache of record. (0x03)</li>
        /// </ul>
        /// Must be in the range of 0 - 32767.
        /// </summary>
        /// <seealso cref="Refinitiv.Eta.Codec.PostUserRights"/>
        int PostUserRights { get; set; }

		/// <summary>
		/// The ETA Post User Info Structure. This information can optionally be
		/// provided along with the posted content via the postUserInfo on the
		/// <seealso cref="IRefreshMsg"/>, <seealso cref="IUpdateMsg"/>, and <seealso cref="IStatusMsg"/>.
		/// </summary>
		/// <returns> the postUserInfo </returns>
		PostUserInfo PostUserInfo { get; }
	}
}