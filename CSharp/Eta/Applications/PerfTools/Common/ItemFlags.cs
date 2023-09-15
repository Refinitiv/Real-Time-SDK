/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.PerfTools.Common
{
    public enum ItemFlags
    {
		// Provider should send updates
		IS_STREAMING_REQ = 0x04,

		// Item was requested(not published)
		IS_SOLICITED = 0x10,

		// Consumer should send posts
		IS_POST = 0x20,

		// Consumer should send generic msgs
		IS_GEN_MSG = 0x40,

		// Consumer should request private stream
		IS_PRIVATE = 0x80
	}
}
