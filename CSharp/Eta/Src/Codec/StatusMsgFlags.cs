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
	/// Specifies a combination of bit values indicating special behaviors and the presence of optional content.
	/// </summary>
	/// <seealso cref="IStatusMsg"/>
	public class StatusMsgFlags
	{
		/// <summary>
		/// This class is not instantiated
		/// </summary>
		private StatusMsgFlags()
		{
            throw new System.NotImplementedException();
        }

		/// <summary>
		/// (0x000) No Flags set </summary>
		public const int NONE = 0x000;

		/// <summary>
		/// (0x001) Indicates that Status Message has Extended Header </summary>
		public const int HAS_EXTENDED_HEADER = 0x001;

		/// <summary>
		/// (0x002) Indicates that Status Message has Permission Expression.
		/// When present, the message might be changing the stream's permission information.
		/// </summary>
		public const int HAS_PERM_DATA = 0x002;

		/// <summary>
		/// (0x008) Indicates that Status Message has a msgKey </summary>
		public const int HAS_MSG_KEY = 0x008;

		/// <summary>
		/// (0x010) Indicates that Status Message has Group Id.
		/// When present, the message might be changing the stream's groupId.
		/// </summary>
		public const int HAS_GROUP_ID = 0x010;

		/// <summary>
		/// (0x020) Indicates that Status Message has State. If state information is
		/// not present, the message might be changing the stream's permission information or groupId.
		/// </summary>
		public const int HAS_STATE = 0x020;

		/// <summary>
		/// (0x040) Indicates that the application should clear stored header or
		/// payload information associated with the stream.
		/// This can happen if some portion of data is invalid.
		/// </summary>
		public const int CLEAR_CACHE = 0x040;

		/// <summary>
		/// (0x080) Acknowledges the establishment of a private stream, or when
		/// combined with a streamState value of <see cref="StreamStates.REDIRECTED"/>,
		/// indicates that a stream can be opened only as private.
		/// </summary>
		public const int PRIVATE_STREAM = 0x080;

		/// <summary>
		/// (0x100) Indicates that this data was posted by the user with this
		/// identifying information
		/// </summary>
		public const int HAS_POST_USER_INFO = 0x100;

		/* (0x200) reserved */

		/// <summary>
		/// (0x400) Acknowledges the establishment of a qualified stream.
		/// </summary>
		public const int QUALIFIED_STREAM = 0x400;
	}

}