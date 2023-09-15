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
	/// ETA close message is used by a Consumer to indicate no further interest in a
	/// stream. The stream should be closed as a result. The streamId indicates the
	/// item stream to which <see cref="ICloseMsg"/> applies.
	/// </summary>
	/// <seealso cref="IMsg"/>
	/// <seealso cref="CloseMsgFlags"/>
	public interface ICloseMsg : IMsg
	{
		/// <summary>
		/// Checks the presence of the Extended Header presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
		/// </summary>
		/// <returns> <c>true</c> - if exists; <c>false</c> if does not exist.
		/// </returns>
		/// <seealso cref="IMsg.Flags"/>
		bool CheckHasExtendedHdr();

        /// <summary>
        /// Checks the presence of the Acknowledgment indication flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <returns> <c>true</c> - if exists; <c>false</c> if does not exist.
        /// </returns>
        /// <seealso cref="IMsg.Flags"/>
        bool CheckAck();

        /// <summary>
        /// Sets the Extended Header presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyHasExtendedHdr();

        /// <summary>
        /// Sets the Acknowledgment indication flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="IMsg.Flags"/>.
        /// </summary>
        /// <seealso cref="IMsg.Flags"/>
        void ApplyAck();
	}
}