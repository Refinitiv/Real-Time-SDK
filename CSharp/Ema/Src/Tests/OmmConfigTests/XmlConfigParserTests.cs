/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using static LSEG.Ema.Access.EmaConfig;

namespace LSEG.Ema.Access.Tests.OmmConfigTests;

public class XmlConfigParserTests : IDisposable
{
    public const string EMA_BLANK_CONFIG = "../../../OmmConfigTests/EmaBlankConfig.xml";
    // this is a copy of the EmaConfig.xml from the CSharp/Ema directory
    public const string EMA_DEFAULT_CONFIG = "../../../OmmConfigTests/EmaDefaultConfig.xml";
    public const string EMA_INVALID_CONFIG = "../../../OmmConfigTests/EmaInvalidConfig.xml";
    public const string EMA_MINIMAL_CONFIG = "../../../OmmConfigTests/EmaMinimalConfig.xml";

    public const string MALFORMED_XML = "../../../OmmConfigTests/Malformed.xml";

    public const string EMA_INCOMPLETE_CONFIG = "../../../OmmConfigTests/EmaIncompleteConfig.xml";

    // this is a copy of the EmaConfig.xsd from the CSharp/Ema directory
    public const string TEST_SCHEMA_FILE = "../../../OmmConfigTests/EmaDefaultConfig.xsd";

    public XmlConfigParserTests()
    {
        XmlConfigParser.DEFAULT_SCHEMA_FILE = TEST_SCHEMA_FILE;
    }

    public void Dispose()
    {
        XmlConfigParser.DEFAULT_SCHEMA_FILE = "EmaConfig.xsd";
        EtaGlobalPoolTestUtil.Clear();
    }

    [Theory]
    [InlineData(EMA_BLANK_CONFIG)]
    [InlineData(EMA_DEFAULT_CONFIG)]
    [InlineData(EMA_MINIMAL_CONFIG)]
    public void LoadValidConfig_Test(string configName)
    {
        // valid config files are loaded, no exceptions are expected
        OmmConsumerConfig consumerConfig = new OmmConsumerConfig(configName);
        OmmNiProviderConfig niProviderConfig = new OmmNiProviderConfig(configName);
        OmmIProviderConfig iProviderConfig = new OmmIProviderConfig(configName);

        Assert.NotNull(consumerConfig.OmmConsConfigImpl);
        Assert.NotNull(niProviderConfig.OmmNiProvConfigImpl);
        Assert.NotNull(iProviderConfig.OmmIProvConfigImpl);
    }

    [Fact]
    public void LoadInvalidConfig_Test()
    {
        OmmInvalidConfigurationException ex = Assert.Throws<OmmInvalidConfigurationException>(() =>
        {
            OmmIProviderConfig iProviderConfig = new OmmIProviderConfig(EMA_INVALID_CONFIG);
        });

        Assert.Contains("Error validating XML configuration", ex.Message);
        Assert.Contains("The element 'Consumer' has invalid child element 'ConsumerName'", ex.Message);
    }

    [Fact]
    public void MissingSchemaFile_Test()
    {
        // specify schema file that does not exist anywhere, let alone cwd
        XmlConfigParser.DEFAULT_SCHEMA_FILE = "This-File-Does-Not-Exist.xsd";

        OmmIProviderConfig iProviderConfig = new OmmIProviderConfig(EMA_BLANK_CONFIG);

        // schema file is totally optional, no exceptions should be thrown when it is missing
        Assert.NotNull(iProviderConfig);
    }

    [Fact]
    public void MalformedSchemaFile_Test()
    {
        XmlConfigParser.DEFAULT_SCHEMA_FILE = MALFORMED_XML;

        // load malformed XML Schema file, which is not even a valid XML Document at all
        OmmInvalidConfigurationException ex = Assert.Throws<OmmInvalidConfigurationException>(() =>
        {
            OmmConsumerConfig iProviderConfig = new OmmConsumerConfig(EMA_BLANK_CONFIG);
        });

        Assert.StartsWith("XML Configuration validation failed:", ex.Message);
    }

    [Fact]
    public void MalformedConfigFile_Test()
    {
        // load malformed XML Config file, which is not even a valid XML Document at all
        OmmInvalidConfigurationException ex = Assert.Throws<OmmInvalidConfigurationException>(() =>
        {
            OmmConsumerConfig iProviderConfig = new OmmConsumerConfig(MALFORMED_XML);
        });

        Assert.StartsWith("Error parsing XML file", ex.Message);
    }

    [Fact]
    public void OverlayedConfigFile_Test()
    {
        // an incomplete XML Config file where Consumer refers to a Logger which is
        // defined later in the Programmatic config here
        OmmConsumerConfig consumerConfig = new OmmConsumerConfig(EMA_INCOMPLETE_CONFIG);
        OmmConsumerConfigImpl consConfigImpl = consumerConfig.OmmConsConfigImpl;

        // ensure that there are no Loggers defined indeed
        Assert.Empty(consConfigImpl.LoggerConfigMap);

        OmmInvalidConfigurationException oicEx = Assert.Throws<OmmInvalidConfigurationException>(
            () => consConfigImpl.VerifyConfiguration());

        Assert.Contains("Logger ProgLogger_1 in Consumer ", oicEx.Message);

        // Overlay with Programmatic config of the previously mentioned Logger

        // Top level map
        Map outerMap = new Map();
        // Middle map for consumers, channels, loggers, dictionaries
        Map innerMap = new Map();
        // Outer element list
        ElementList encodeGroupList = new ElementList();
        ElementList encodeObjectList = new ElementList();

        // Start encoding the Logger information
        encodeGroupList.Clear();
        innerMap.Clear();
        encodeObjectList.Clear();

        encodeObjectList.AddAscii("FileName", "ProgLogFile")
            .AddUInt("IncludeDateInLoggerOutput", 1)
            .AddUInt("NumberOfLogFiles", 20)
            .AddUInt("MaxLogFileSize", 100)
            .AddEnum("LoggerSeverity", LoggerLevelEnum.INFO)
            .AddEnum("LoggerType", LoggerTypeEnum.STDOUT)
            .MarkForClear().Complete();

        innerMap.AddKeyAscii("ProgLogger_1", MapAction.ADD, encodeObjectList);

        encodeGroupList.AddMap("LoggerList", innerMap.MarkForClear().Complete())
            .MarkForClear().Complete();

        outerMap.AddKeyAscii("LoggerGroup", MapAction.ADD, encodeGroupList);
        outerMap.MarkForClear().Complete();

        consConfigImpl.Config(outerMap);

        // Inspect overlayed config
        Assert.NotEmpty(consConfigImpl.LoggerConfigMap);
        Assert.True(consConfigImpl.LoggerConfigMap.ContainsKey("ProgLogger_1"));

        consConfigImpl.VerifyConfiguration();
    }

}
