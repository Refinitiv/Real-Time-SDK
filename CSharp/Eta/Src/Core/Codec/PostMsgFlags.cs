/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Codec
{
	/// <summary>
	/// The Post Message flags.
	/// </summary>
	/// <seealso cref="IPostMsg"/>
	sealed public class PostMsgFlags
	{
		/// <summary>
		/// This class is not instantiated
		/// </summary>
		private PostMsgFlags()
		{
            throw new System.NotImplementedException();
        }

		/// <summary>
		/// (0x000) No Flags set </summary>
		public const int NONE = 0x000;

		/// <summary>
		/// (0x001) Indicates presence of the Extended Header </summary>
		public const int HAS_EXTENDED_HEADER = 0x001;

		/// <summary>
		/// (0x002) Indicates the presence of the Post Id </summary>
		public const int HAS_POST_ID = 0x002;

		/// <summary>
		/// (0x004) Indicates that the <see cref="IPostMsg"/> contains a populated msgKey that
		/// identifies the stream on which the information is posted. A msgKey is
		/// typically required for off-stream posting and is not necessary when on-stream posting.
		/// </summary>
		public const int HAS_MSG_KEY = 0x004;

		/// <summary>
		/// (0x008) Indicates the presence of Sequence Number </summary>
		public const int HAS_SEQ_NUM = 0x008;

		/// <summary>
		/// (0x020) Post Message is last message in a post exchange. If post message
		/// is multipart, this is the last part. If message is atomic, both INIT and
		/// COMPLETE should be set on same message.
		/// </summary>
		public const int POST_COMPLETE = 0x020;

		/// <summary>
		/// (0x040) Specifies that the consumer wants the provider to send an <see cref="IAckMsg"/>
		/// to indicate that the <see cref="IPostMsg"/> was processed properly. When
		/// acknowledging a <see cref="IPostMsg"/>, the provider will include the postId in
		/// the ackId and communicate any associated seqNum.
		/// </summary>
		public const int ACK = 0x040;

		/// <summary>
		/// (0x080) Indicates the presence of the Permission Expressions </summary>
		public const int HAS_PERM_DATA = 0x080;

		/// <summary>
		/// (0x100) Indicates the presence of Part Number </summary>
		public const int HAS_PART_NUM = 0x100;

		/// <summary>
		/// (0x200) Indicates that post Message includes which rights a user has </summary>
		public const int HAS_POST_USER_RIGHTS = 0x200;
	}

}