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
	/// Collection of interfaces to query supported RWF versions for encoder/decoder
	/// as well as RWF protocol type being used by the connection.
	/// </summary>
	sealed public class Codec
	{
		/// <summary>
		/// The RWF protocol type. </summary>
		public const int RWF_PROTOCOL_TYPE = 0;

		/// <summary>
		/// Protocol type definition. This can be used in conjunction transport layer
		/// to indicate protocol being used on the connection. Codec only supports RWF protocol type.
		/// </summary>
		/// <returns> protocol type. </returns>
		public static int ProtocolType()
		{
			return RWF_PROTOCOL_TYPE;
		}

		/// <summary>
		/// Version Major number for the version of RWF supported.
		/// </summary>
		/// <returns> RWF major version </returns>
		public static int MajorVersion()
		{
			return RwfDataConstants.MAJOR_VERSION_1;
		}

		/// <summary>
		/// Version Minor number for the version of RWF supported.
		/// </summary>
		/// <returns> RWF minor version </returns>
		public static int MinorVersion()
		{
			return RwfDataConstants.MINOR_VERSION_1;
		}

	}
}
