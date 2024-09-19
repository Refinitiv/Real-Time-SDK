/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Codec
{
	/// <summary>
	/// Specifies a combination of bit values indicating special behaviors and the
	/// presence of optional content for a <see cref="ICloseMsg"/>.
	/// </summary>
	/// <seealso cref="ICloseMsg"/>
	sealed public class CloseMsgFlags
	{
		/// <summary>
		/// This class is not instantiated
		/// </summary>
		private CloseMsgFlags()
		{
            throw new System.NotImplementedException();
        }

		/// <summary>
		/// (0x00) No Flags set </summary>
		public const int NONE = 0x00;

		/// <summary>
		/// (0x01) Indicates that Close Message has Extended Header </summary>
		public const int HAS_EXTENDED_HEADER = 0x01;

		/// <summary>
		/// (0x02) If present, the consumer wants the provider to send an
		/// <see cref="IAckMsg"/> to indicate that the <see cref="ICloseMsg"/> has been processed
		/// properly and the stream is properly closed. This functionality might not
		/// be available with some components; for details, refer to the component's documentation.
		/// </summary>
		public const int ACK = 0x02;
	}

}