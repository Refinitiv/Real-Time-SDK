/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Transports;
using System;

namespace LSEG.Eta.Example.Common;
public class EncryptionProtocolCommandLineArg: CommandLineArg<EncryptionProtocolFlags>
{
    public override string Name { get; } = "encryptionProtocol";
    public override string Description { get; } = "Specifies accepted Tls versions(TlsV1.2 TlsV1.3 or TlsV1.2;TlsV1.3)";
    protected override EncryptionProtocolFlags Parse(string? value)
    {
        if (string.IsNullOrEmpty(value))
        {
            return EncryptionProtocolFlags.ENC_NONE;
        }
        if (EncryptionProtocolFlagsExtension.TryParse(value, out var tls))
        {
            return tls;
        }

        throw new NotSupportedException($"Tls value(s) not supported: {value}");
    }
}
