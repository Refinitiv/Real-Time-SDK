/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using Xunit.Sdk;
using LSEG.Eta.Tests.Common.Xunit;

[assembly: RegisterXunitSerializer(
    typeof(JsonXunitSerializer),
    typeof(LSEG.Ema.Access.OmmConsumerConfigImpl),
    typeof(LSEG.Ema.Access.Tests.OmmConsumerTests.ModifyIOCtlTest.IOCtlSetting),
    typeof(LSEG.Ema.Access.Tests.OmmIProviderTests.ModifyIOCtlTest.IOCtlSetting),
    typeof(LSEG.Ema.Access.OmmConsumerConfigImpl)
)]