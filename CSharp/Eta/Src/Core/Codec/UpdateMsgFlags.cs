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
	/// A combination of bit values that indicate special behaviors and the presence of optional content.
	/// </summary>
	/// <seealso cref="IUpdateMsg"/>
	sealed public class UpdateMsgFlags
	{
		/// <summary>
		/// This class is not instantiated
		/// </summary>
		private UpdateMsgFlags()
		{
            throw new System.NotImplementedException();
        }

		/// <summary>
		/// (0x000) No Flags set </summary>
		public const int NONE = 0x000;

		/// <summary>
		/// (0x001) Indicates that Update Message has Extended Header </summary>
		public const int HAS_EXTENDED_HEADER = 0x001;

		/// <summary>
		/// (0x002) Indicates that Update Message has Permission Expression </summary>
		public const int HAS_PERM_DATA = 0x002;

		/// <summary>
		/// (0x008) Indicates that Update Message has Update Key </summary>
		public const int HAS_MSG_KEY = 0x008;

		/// <summary>
		/// (0x010) Indicates that Update Message has Sequence Number </summary>
		public const int HAS_SEQ_NUM = 0x010;

		/// <summary>
		/// (0x020) Indicates that Update Message contains conflation information </summary>
		public const int HAS_CONF_INFO = 0x020;

		/// <summary>
		/// (0x040) Indicates that Message is transient and should not be cached </summary>
		public const int DO_NOT_CACHE = 0x040;

		/// <summary>
		/// (0x080) Indicates that Update Message should not be conflated </summary>
		public const int DO_NOT_CONFLATE = 0x080;

		/// <summary>
		/// (0x100) Indicates that Update message should not ripple </summary>
		public const int DO_NOT_RIPPLE = 0x100;

		/// <summary>
		/// (0x200) Indicates that this data was posted by the user with this
		/// identifying information
		/// </summary>
		public const int HAS_POST_USER_INFO = 0x200;

		/// <summary>
		/// (0x400) Indicates that this update can be discarded. Common for options
		/// with no open interest
		/// </summary>
		public const int DISCARDABLE = 0x400;
	}

}