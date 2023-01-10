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
	/// A combination of bit values that indicate special behaviors and the presence
	/// of optional <see cref="IRefreshMsg"/> content.
	/// </summary>
	/// <seealso cref="IRefreshMsg"/>
	public class RefreshMsgFlags
	{
		/// <summary>
		/// This class is not instantiated
		/// </summary>
		private RefreshMsgFlags()
		{
            throw new System.NotImplementedException();
        }

		/// <summary>
		/// (0x0000) No Flags set </summary>
		public const int NONE = 0x0000;

		/// <summary>
		/// (0x0001) Indicates the presence of the Extended Header </summary>
		public const int HAS_EXTENDED_HEADER = 0x0001;

		/// <summary>
		/// (0x0002) Indicates the presence of Permission Expression </summary>
		public const int HAS_PERM_DATA = 0x0002;

		/// <summary>
		/// (0x0008) Indicates that the Refresh Message contains a populated msgKey.
		/// This can associate a request with a refresh or identify an item sent from
		/// an NIP application.
		/// </summary>
		public const int HAS_MSG_KEY = 0x0008;

		/// <summary>
		/// (0x0010) Indicates the presence of Sequence Number </summary>
		public const int HAS_SEQ_NUM = 0x0010;

		/// <summary>
		/// (0x0020) Indicates that the refresh is sent as a response to a request,
		/// referred to as a solicited refresh. A refresh sent to inform a consumer
		/// of an upstream change in information (i.e., an unsolicited refresh) must
		/// not include this flag.
		/// </summary>
		public const int SOLICITED = 0x0020;

		/// <summary>
		/// (0x0040) Indicates that the message is the final part of the
		/// <see cref="IRefreshMsg"/>. This flag value should be set when:
		/// <ul>
		/// <li>The message is a single-part refresh (i.e., atomic refresh).</li>
		/// <li>The message is the final part of a multi-part refresh.</li>
		/// </ul>
		/// </summary>
		public const int REFRESH_COMPLETE = 0x0040;

		/// <summary>
		/// (0x0080) Indicates the presence of Qos </summary>
		public const int HAS_QOS = 0x0080;

		/// <summary>
		/// (0x0100) Indicates that the stream's stored payload information should be
		/// cleared. This might occur if some portion of data is known to be invalid.
		/// </summary>
		public const int CLEAR_CACHE = 0x0100;

		/// <summary>
		/// (0x0200) Indicates that the message's payload information should not be
		/// cached. This flag value only applies to the message on which it is present.
		/// </summary>
		public const int DO_NOT_CACHE = 0x0200;

		/// <summary>
		/// (0x0400) Acknowledges the initial establishment of a private stream or,
		/// when combined with a streamState value of <see cref="StreamStates.REDIRECTED"/>,
		/// indicates that a stream can only be opened as private.
		/// </summary>
		public const int PRIVATE_STREAM = 0x0400;

		/// <summary>
		/// (0x0800) Indicates that this message includes postUserInfo, implying that
		/// this <see cref="IRefreshMsg"/> was posted by the user described in postUserInfo.
		/// </summary>
		public const int HAS_POST_USER_INFO = 0x0800;

		/// <summary>
		/// (0x1000) Indicates the presence of a Part Number </summary>
		public const int HAS_PART_NUM = 0x1000;

		/* (0x2000) reserved */

		/// <summary>
		/// (0x4000) Acknowledges the initial establishment of a qualified stream.
		/// </summary>
		public const int QUALIFIED_STREAM = 0x4000;
	}

}