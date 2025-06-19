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
	/// Specifies a combination of bit values indicating how <see cref="IMsg"/> is copied.
	/// </summary>
	/// <seealso cref="IMsg.Copy(IMsg, int)"/>
	sealed public class CopyMsgFlags
	{
		/// <summary>
		/// This class is not instantiated
		/// </summary>
		private CopyMsgFlags()
		{
            throw new System.NotImplementedException();
        }

		/// <summary>
		/// (0x000) No Flags set; no sub-structs will be copied </summary>
		public const int NONE = 0x000;

		/// <summary>
		/// (0x001) State test will be copied </summary>
		public const int STATE_TEXT = 0x001;

		/// <summary>
		/// (0x002) Perm exp will be copied </summary>
		public const int PERM_DATA = 0x002;

		/// <summary>
		/// (0x004) Key name will be copied </summary>
		public const int KEY_NAME = 0x004;

		/// <summary>
		/// (0x008) Key attrib will be copied </summary>
		public const int KEY_ATTRIB = 0x008;

		/// <summary>
		/// (0x00C) Entire key will be copied </summary>
		public const int KEY = 0x00C;

		/// <summary>
		/// (0x010) Extended header will be copied </summary>
		public const int EXTENDED_HEADER = 0x010;

		/// <summary>
		/// (0x020) Data body will be copied </summary>
		public const int DATA_BODY = 0x020;

		/// <summary>
		/// (0x040) Encoded message buffer will be copied </summary>
		public const int MSG_BUFFER = 0x040;

		/// <summary>
		/// (0x080) Group Id will be copied </summary>
		public const int GROUP_ID = 0x080;

		/// <summary>
		/// (0x100) Nak Text will be copied </summary>
		public const int NAK_TEXT = 0x100;

		/// <summary>
		/// (0xFFF) Everything will be copied </summary>
		public const int ALL_FLAGS = 0xFFF;
	}

}