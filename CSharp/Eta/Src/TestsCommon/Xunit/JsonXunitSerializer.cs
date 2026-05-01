/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Text.Json;
using Xunit.Sdk;

namespace LSEG.Eta.Tests.Common.Xunit
{
    public class JsonXunitSerializer : IXunitSerializer
    {
        public object Deserialize(Type type, string serializedValue) => JsonSerializer.Deserialize(serializedValue, type)!;

        public string Serialize(object value) => JsonSerializer.Serialize(value);

        public bool IsSerializable(Type type, object? value, out string failureReason)
        {
            failureReason = null!;
            return true;
        }
    }
}
