/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2024 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access;

internal interface XmlTraceConfigurable
{
    bool XmlTraceToFile { get; set; }

    bool XmlTraceToStdout { get; set; }

    string XmlTraceFileName { get; set; }

    ulong XmlTraceMaxFileSize { get; set; }

    bool XmlTraceToMultipleFiles { get; set; }

    bool XmlTraceWrite { get; set; }

    bool XmlTraceRead { get; set; }

    bool XmlTracePing { get; set; }
}
