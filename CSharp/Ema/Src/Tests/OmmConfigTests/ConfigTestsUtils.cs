/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2024 Refinitiv. All rights reserved.        --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access.Tests.OmmConfigTests
{
    public static class ConfigTestsUtils
    {
        public static OmmConsumerConfig LoadEmaTestConfig() => new OmmConsumerConfig("../../../OmmConfigTests/EmaTestConfig.xml");
        public static OmmConsumerConfig LoadEmaBlankConfig() => new OmmConsumerConfig("../../../OmmConfigTests/EmaBlankConfig.xml");
    }
}
