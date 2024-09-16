/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|              Copyright (C) 2024 LSEG. All rights reserved.                --
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"
#include "OmmConsumerConfigImpl.h"
#include "OmmIProviderConfigImpl.h"
#include "OmmNiProviderConfigImpl.h"
#include "EmaConfig.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

/// Tests XML Config file validation with XML Schema

/// Config file copied from Cpp-C/Ema/EmaConfig.xml, the default configuration file
static const char* EMA_DEFAULT_CONFIG_FILE = "./EmaConfigDefault.xml";
static const char* EMA_BLANK_CONFIG_FILE = "./EmaBlankConfig.xml";
static const char* EMA_INVALID_CONFIG_FILE = "./EmaInvalidConfig.xml";
static const char* EMA_MINIMAL_CONFIG_FILE = "./EmaMinimalConfig.xml";
static const char* EMA_MALFORMED_CONFIG_FILE = "./Malformed.xml";

static const char* EMA_SCHEMA_FILE = "EmaConfigDefault.xsd";

// This file is used in other config unit-tests, it does not match the schema but is
// accepted by the EMA XML config parser
static const char* EMA_TEST_CONFIG_FILE = "./EmaConfigTest.xml";

class EmaXmlConfigValidationTest : public ::testing::Test {
public:
	void SetUp()
	{
		EmaString customizedDefaultName{ EMA_DEFAULT_CONFIG_FILE };
		EmaConfigBaseImpl::setDefaultConfigFileName(customizedDefaultName);
	}

	void TearDown()
	{
		EmaString empty{};
		EmaConfigBaseImpl::setDefaultConfigFileName(empty);
		EmaConfigBaseImpl::setDefaultSchemaFileName("EmaConfig.xsd");
	}
};

TEST_F(EmaXmlConfigValidationTest, testLoadValidConfig)
{
	static const char* paths[] = { EMA_BLANK_CONFIG_FILE, EMA_MINIMAL_CONFIG_FILE, EMA_DEFAULT_CONFIG_FILE };

	EmaString workingDir;
	ASSERT_EQ(getCurrentDir(workingDir), true)
		<< "Error: failed to load config file from current working dir "
		<< workingDir.c_str();

	int failuresCount = 0;
	for (const char* path : paths)
	{
		try
		{
			EmaConfigBaseImpl::setDefaultSchemaFileName(EMA_SCHEMA_FILE);

			OmmConsumerConfigImpl config{ path };
			OmmIProviderConfigImpl iprovConfig{ path };
			OmmNiProviderConfigImpl niprovConfig{ path };
			EXPECT_FALSE(config.getConfiguredName() == nullptr);
			EXPECT_FALSE(iprovConfig.getConfiguredName() == nullptr);
			EXPECT_FALSE(niprovConfig.getConfiguredName() == nullptr);
		}
		catch (const OmmInvalidConfigurationException& ice)
		{
			ADD_FAILURE() << "Failed to validate file: " << path
				<< " With the following error: " << ice.getText();
			failuresCount++;
		}
	}
	EXPECT_EQ(0, failuresCount) << "Some files have failed validation";
}


TEST_F(EmaXmlConfigValidationTest, testLoadDefaultValidConfig)
{
	// The schema file has non-default file name and won't take part in the schema
	// validation when not specified
	try
	{
		EmaString localConfigPath{ "" };
		OmmConsumerConfigImpl config{ localConfigPath };

		EmaString retrievedValue = config.getConfiguredName();
		bool debugResult = config.get<EmaString>("ConsumerGroup|DefaultConsumer", retrievedValue);
		EXPECT_TRUE(debugResult);
		ASSERT_STREQ("Consumer_1", retrievedValue.c_str());
	}
	catch (const OmmInvalidConfigurationException& ice)
	{
		FAIL() << "Failed to load default config: " << ice.getText();
	}
}

TEST_F(EmaXmlConfigValidationTest, testLoadInvalidConfig)
{
	// Load valid XML file that can't be validated with the schema -- test that the
	// validation error is reported as expected
	EmaString workingDir;
	ASSERT_EQ(getCurrentDir(workingDir), true)
		<< "Error: failed to load config file from current working dir "
		<< workingDir.c_str();

	try
	{
		EmaConfigBaseImpl::setDefaultSchemaFileName(EMA_SCHEMA_FILE);

		OmmConsumerConfigImpl ommConsumerConfig{ EMA_INVALID_CONFIG_FILE };
		FAIL() << "An exception is expected to be thrown here";
	}
	catch (const OmmInvalidConfigurationException& ice)
	{
		ASSERT_STREQ("error validating XML configuration", ice.getText().c_str());
		SUCCEED() << ice;
	}
	catch (...)
	{
		FAIL() << "Only OmmInvalidConfigurationException is expected";
	}
}

TEST_F(EmaXmlConfigValidationTest, testLoadInvalidConfigNotValidate)
{
	// Load XML Config that is a valid XML document, but can't be validated with the
	// schema. At the same time, XML Schema file name is not provided to the config parser
	// so it must skip XML Schema validation and accept this file
	EmaString workingDir;
	ASSERT_EQ(getCurrentDir(workingDir), true)
		<< "Error: failed to load config file from current working dir "
		<< workingDir.c_str();

	try
	{
		OmmConsumerConfigImpl ommConsumerConfig{ EMA_TEST_CONFIG_FILE };
		SUCCEED() << "An exception is not expected to be thrown here";
	}
	catch (const OmmInvalidConfigurationException& ice)
	{
		FAIL() << ice;
	}
	catch (...)
	{
		FAIL() << "No exceptions are expected at all";
	}
}

TEST_F(EmaXmlConfigValidationTest, testLoadInvalidConfigDefaultValidate)
{
	// Load XML Config that is a valid XML document, but can't be validated with the
	// schema. At the same time, XML Schema file name is not provided to the config parser
	// so it must try to load the default XML Schema file and then fail validation
	EmaString workingDir;
	ASSERT_EQ(getCurrentDir(workingDir), true)
		<< "Error: failed to load config file from current working dir "
		<< workingDir.c_str();

	// despite not specifying the Schema file explicitly, the library will have to load it
	// as it is in the current working directory
	EmaConfigBaseImpl::setDefaultSchemaFileName(EMA_SCHEMA_FILE);

	try
	{
		OmmConsumerConfigImpl ommConsumerConfig{ EMA_TEST_CONFIG_FILE };
		FAIL() << "XML Schema validation must fail and generate an exception";
	}
	catch (const OmmInvalidConfigurationException& ice)
	{
		SUCCEED() << ice;
	}
	catch (...)
	{
		FAIL() << "No other exceptions are expected";
	}

	// revert to defaults
	{
		EmaString empty{};
		EmaConfigBaseImpl::setDefaultSchemaFileName(empty);
	}
}

TEST_F(EmaXmlConfigValidationTest, testLoadMalformedFile)
{
	// Attempt to load a configuration file which is not a valid XML document at all
	EmaString workingDir;
	ASSERT_EQ(getCurrentDir(workingDir), true)
		<< "Error: failed to load config file from current working dir "
		<< workingDir.c_str();

	try
	{
		EmaConfigBaseImpl::setDefaultSchemaFileName(EMA_SCHEMA_FILE);

		OmmConsumerConfigImpl config{ EMA_MALFORMED_CONFIG_FILE };
		FAIL() << "Exception had to be thrown above";
	}
	catch (const OmmInvalidConfigurationException& ice)
	{
		ASSERT_STREQ("error validating XML configuration", ice.getText().c_str());
		SUCCEED() << ice.getText();
	}
	catch (...)
	{
		FAIL() << "Unexpected exception";
	}
}
