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
	/// <seealso cref="IGenericMsg"/>
	public class GenericMsgFlags
	{
		/// <summary>
		/// This class is not instantiated
		/// </summary>
		private GenericMsgFlags()
		{
            throw new System.NotImplementedException();
        }

		/// <summary>
		/// (0x000) No Flags set </summary>
		public const int NONE = 0x000;

		/// <summary>
		/// (0x001) Indicates the presence of extended header </summary>
		public const int HAS_EXTENDED_HEADER = 0x001;

		/// <summary>
		/// (0x002) Indicates the presence of permission expression </summary>
		public const int HAS_PERM_DATA = 0x002;

		/// <summary>
		/// (0x004) Indicates the presence of a populated <see cref="IMsgKey"/>. Use of a
		/// <see cref="IMsgKey"/> differentiates a generic message from the msgKey
		/// information specified for other messages within the stream. Contents and
		/// semantics associated with a <see cref="IMsg.MsgKey"/> should be
		/// defined by the domain model specification that employs them.
		/// </summary>
		public const int HAS_MSG_KEY = 0x004;

		/// <summary>
		/// (0x008) Indicates the presence of sequence number </summary>
		public const int HAS_SEQ_NUM = 0x008;

		/// <summary>
		/// (0x010) Indicates that the message is the final part of a
		/// <seealso cref="IGenericMsg"/>. This flag should be set on:
		/// <ul>
		/// <li>Single-part generic messages (i.e., an atomic generic message).</li>
		/// <li>The last message (final part) in a multi-part generic message</li>
		/// </ul>
		/// </summary>
		public const int MESSAGE_COMPLETE = 0x010;

		/// <summary>
		/// (0x020) Indicates the presence of the secondary sequence number </summary>
		public const int HAS_SECONDARY_SEQ_NUM = 0x020;

		/// <summary>
		/// (0x040) Indicates the presence of the partNum </summary>
		public const int HAS_PART_NUM = 0x040;

		/// <summary>
		/// (0x4000) Indicates that the message is sent from a provider and is not in response to a specific request
		/// </summary>
		public const int PROVIDER_DRIVEN = 0x4000;
	}

}