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
    /// presence of optional content for an <see cref="IAckMsg"/>.
    /// </summary>
    /// <seealso cref="LSEG.Eta.Codec.IAckMsg"/>
    sealed public class AckMsgFlags
	{
		/// <summary>
		/// This class is not instantiated
		/// </summary>
		private AckMsgFlags()
		{
            throw new System.NotImplementedException();
		}

		/// <summary>
		/// (0x00) No Flags set </summary>
		public const int NONE = 0x00;

		/// <summary>
		/// (0x01) Indicates that Ack Message has Extended Header. </summary>
		public const int HAS_EXTENDED_HEADER = 0x01;

		/// <summary>
		/// (0x02) Indicates that Ack Message Has Text </summary>
		public const int HAS_TEXT = 0x02;

		/// <summary>
		/// (0x04) Indicates that Ack message acknowledges private stream establishment.
		/// </summary>
		public const int PRIVATE_STREAM = 0x04;

		/// <summary>
		/// (0x08) Indicates that Ack Message contains a sequence number. </summary>
		public const int HAS_SEQ_NUM = 0x08;

		/// <summary>
		/// (0x10) Indicates that Ack Message contains a msgKey. When present, this
		/// is typically populated to match the information being acknowledged.
		/// </summary>
		public const int HAS_MSG_KEY = 0x10;

		/// <summary>
		/// (0x20) Indicates that Ack Message contains a NAK Code <see cref="NakCodes"/>. </summary>
		public const int HAS_NAK_CODE = 0x20;

		/// <summary>
		/// (0x40) Indicates that Ack message acknowledges qualified stream establishment.
		/// </summary>
		public const int QUALIFIED_STREAM = 0x40;
	}

}