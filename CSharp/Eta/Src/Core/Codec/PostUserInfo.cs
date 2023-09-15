/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Text;
using System.Runtime.CompilerServices;

namespace LSEG.Eta.Codec
{
    /// <summary>
    /// The PostUserInfo contains information that identifies the user posting this
    /// information.If present on a <see cref="IRefreshMsg"/>, this implies that this
    /// refresh was posted to the system by the user described in postUserInfo.
    /// </summary>
    /// <seealso cref="IPostMsg"/>
    /// <seealso cref="IRefreshMsg"/>
    /// <seealso cref="IStatusMsg"/>
    /// <seealso cref="IUpdateMsg"/>
	sealed public class PostUserInfo
	{
		private StringBuilder _strBldr = new StringBuilder();


        /// <summary>
        /// Creates <see cref="PostUserInfo"/>.
        /// </summary>
        /// <seealso cref="PostUserInfo"/>
        public PostUserInfo()
		{
		}

		/// <summary>
		/// Clears this object
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Clear()
		{
		    UserAddr = 0;
			UserId = 0;
		}

        /// <summary>
        /// Gets or sets IP Address of user that posted this data.
        /// Must be in the range of 0 - 4294967295 (2^32).
        /// Converts dotted-decimal IP address string(e.g. "127.0.0.1") to integer equivalent.
        /// </summary>
		public long UserAddr { get; set; }

		/// <summary>
		/// Converts dotted-decimal IP address string(e.g. "127.0.0.1") to integer equivalent.
		/// 
		/// userAddrString is the IP address string.
		/// 
		/// The output integer value, in host byte order, goes into _userAddr.
		/// </summary>
		/// <param name="userAddrString"></param>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void UserAddrFromString(string userAddrString)
		{
			int ipPart = 0;
			int byteCount = 0;
			for (int i = 0; i < userAddrString.Length; i++)
			{
				char c = userAddrString[i];
				if (c != '.')
				{
					int digit = (int)c - (int)'0';
					ipPart *= 10;
					ipPart += digit;
				}
				else
				{
                    UserAddr = (UserAddr << 8) + ipPart;
					byteCount++;
					ipPart = 0;
				}
			}

            // final part
            UserAddr = (UserAddr << 8) + ipPart;
			byteCount++;
        }

        /// <summary>
        /// Gets or sets identifier of the specific user that posted this data.
        /// Must be in the range of 0 - 4294967295 (2^32).
        /// </summary>
		public long UserId { get; set; }

		/// <summary>
		/// Converts IP address in integer format to string equivalent.
		/// Must be in the range of 0 - 4294967295 (2^32).
		/// </summary>
		/// <param name="addrInt">The input integer value</param>
		/// <returns>The IP address string
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public string UserAddrToString(long addrInt)
		{
            _strBldr.Clear();
            _strBldr.Append((addrInt >> 24) & 0xFF);
			_strBldr.Append(".");
			_strBldr.Append((addrInt >> 16) & 0xFF);
			_strBldr.Append(".");
			_strBldr.Append((addrInt >> 8) & 0xFF);
			_strBldr.Append(".");
			_strBldr.Append(addrInt & 0xFF);

			return _strBldr.ToString();
		}
	}
}