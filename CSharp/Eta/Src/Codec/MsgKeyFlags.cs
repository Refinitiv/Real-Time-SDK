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
	/// Combination of bit values to indicate the presence of optional <see cref="IMsgKey"/> members.
	/// </summary>
	/// <seealso cref="IMsgKey"/>
	public class MsgKeyFlags
	{
		/// <summary>
		/// This class is not instantiated
		/// </summary>
		private MsgKeyFlags()
		{
            throw new System.NotImplementedException();
        }

		/// <summary>
		/// (0x0000) No Key Flags </summary>
		public const int NONE = 0x0000;

		/// <summary>
		/// (0x0001) Indicates the presence of the serviceId field. </summary>
		public const int HAS_SERVICE_ID = 0x0001;

		/// <summary>
		/// (0x0002) Indicates the presence of the name field. </summary>
		public const int HAS_NAME = 0x0002;

		/// <summary>
		/// (0x0004) Indicates the presence of the nameType field. </summary>
		public const int HAS_NAME_TYPE = 0x0004;

		/// <summary>
		/// (0x0008) Indicates the presence of the filter field. </summary>
		public const int HAS_FILTER = 0x0008;

		/// <summary>
		/// (0x0010) Indicates the presence of the identifier field. </summary>
		public const int HAS_IDENTIFIER = 0x0010;

		/// <summary>
		/// (0x0020) Indicates key has attribute information, this includes msgKey.attrib and
		/// msgKey.attribContainerType
		/// </summary>
		public const int HAS_ATTRIB = 0x0020;
	}

}