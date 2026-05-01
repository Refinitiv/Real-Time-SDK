/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024,2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access.Tests.OmmConfigTests
{
    public static class ConfigTestsUtils
    {
        public const string BASE_TEST_CONFIG_PATH = "../../../../Src/Tests";

        public const string TEST_CONFIG_PATH = BASE_TEST_CONFIG_PATH + "/OmmConfigTests";
        public const string TEST_CONFIG_FILE_PATH = TEST_CONFIG_PATH + "/EmaTestConfig.xml";
        public const string TEST_BLANK_CONFIG_FILE_PATH = TEST_CONFIG_PATH + "/EmaBlankConfig.xml";
        public const string TEST_DICTIONARIES_CONFIG_FILE_PATH = TEST_CONFIG_PATH + "/EmaBlankConfig.xml";

        public const string OMM_CONSUMER_CONFIG_PATH = BASE_TEST_CONFIG_PATH + "/OmmConsumerTests";
        public const string EMA_DICTIONARIES_CONFIG_CONFIG_FILE_PATH = OMM_CONSUMER_CONFIG_PATH + "/EmaDictionariesConfig.xml";

        public static OmmConsumerConfig LoadEmaTestConfig() => new OmmConsumerConfig(TEST_CONFIG_FILE_PATH);
        public static OmmConsumerConfig LoadEmaBlankConfig() => new OmmConsumerConfig(TEST_BLANK_CONFIG_FILE_PATH);
    }
}
