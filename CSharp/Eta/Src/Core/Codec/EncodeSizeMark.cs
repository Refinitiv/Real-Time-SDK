/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Codec
{
	internal class EncodeSizeMark
	{
		/* Position of size to encode */
		internal int _sizePos;

		/* Number of bytes reserved for the size */
		internal int _sizeBytes;
	}

}