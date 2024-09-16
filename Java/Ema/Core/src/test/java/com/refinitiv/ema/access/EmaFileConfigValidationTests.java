///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2024 Refinitiv. All rights reserved.              --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import com.refinitiv.ema.unittest.TestUtilities;

import java.io.File;
import java.util.Arrays;
import java.util.Set;

import javax.xml.XMLConstants;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.validation.Schema;
import javax.xml.validation.SchemaFactory;
import javax.xml.validation.Validator;

import org.junit.After;
import org.junit.Rule;
import org.junit.Test;
import org.junit.experimental.theories.DataPoints;
import org.junit.experimental.theories.Theories;
import org.junit.experimental.theories.Theory;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;

import org.w3c.dom.DOMException;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;

/**
 * Test optional XML Configuration validation with XML Schema.
 */
@RunWith(Theories.class)
public class EmaFileConfigValidationTests
{
	private static final String EMA_TEST_CONFIG_FILE = "./src/test/resources/com/refinitiv/ema/unittest/EmaFileConfigTests/EmaConfigTest.xml";
	private static final String EMA_BLANK_CONFIG_FILE = "./src/test/resources/com/refinitiv/ema/unittest/EmaFileConfigTests/EmaBlankConfig.xml";
	private static final String EMA_INVALID_CONFIG_FILE = "./src/test/resources/com/refinitiv/ema/unittest/EmaFileConfigTests/EmaInvalidConfig.xml";
	private static final String EMA_MINIMAL_CONFIG_FILE = "./src/test/resources/com/refinitiv/ema/unittest/EmaFileConfigTests/EmaMinimalConfig.xml";
	private static final String EMA_MALFORMED_CONFIG_FILE = "./src/test/resources/com/refinitiv/ema/unittest/EmaFileConfigTests/Malformed.xml";
	private static final String EMA_DEFAULT_CONFIG_FILE = "../EmaConfig.xml";

	private static final String EMA_SCHEMA_FILE = "../EmaConfig.xsd";

	@DataPoints
	public static String[] paths = { EMA_BLANK_CONFIG_FILE, EMA_MINIMAL_CONFIG_FILE, EMA_TEST_CONFIG_FILE, EMA_DEFAULT_CONFIG_FILE };

	@Rule
	@SuppressWarnings("deprecation")
	public ExpectedException thrownException = ExpectedException.none();


	@After
	public void tearDown()
	{
		JUnitTestConnect.setDefaultXmlSchemaName("EmaConfig.xsd");
	}

	@Theory
	public void testLoadValidConfig(String pathToConfig)
	{
		JUnitTestConnect.setDefaultXmlSchemaName(EMA_SCHEMA_FILE);

		OmmConsumerConfigImpl ommConsumerConfig = (OmmConsumerConfigImpl) EmaFactory
				.createOmmConsumerConfig(pathToConfig);

		TestUtilities.checkResult("ommConsumerConfig value != null", ommConsumerConfig != null);
	}

	@Test
	public void testLoadDefaultValidConfig()
	{
		OmmConsumerConfigImpl ommConsumerConfig = (OmmConsumerConfigImpl) EmaFactory.createOmmConsumerConfig();

		TestUtilities.checkResult("ommConsumerConfig value != null", ommConsumerConfig != null);
	}

	@Test
	public void testLoadInvalidConfig()
	{
		JUnitTestConnect.setDefaultXmlSchemaName(EMA_SCHEMA_FILE);

		thrownException.expect(OmmInvalidConfigurationException.class);
		thrownException.expectMessage(
				"cvc-complex-type.2.4.a: Invalid content was found starting with element 'ConsumerName'. One of");

		@SuppressWarnings("unused")
		OmmConsumerConfigImpl ommConsumerConfig = (OmmConsumerConfigImpl) EmaFactory
		.createOmmConsumerConfig(EMA_INVALID_CONFIG_FILE);
	}

	@Test
	public void testLoadMalformedFile()
	{
		JUnitTestConnect.setDefaultXmlSchemaName(EMA_SCHEMA_FILE);

		thrownException.expect(OmmInvalidConfigurationException.class);
		thrownException.expectMessage("Content is not allowed in prolog.");

		@SuppressWarnings("unused")
		OmmConsumerConfigImpl ommConsumerConfig = (OmmConsumerConfigImpl) EmaFactory
		.createOmmConsumerConfig(EMA_MALFORMED_CONFIG_FILE);
	}

	@Test
	public void testSchemaCompleteness()
			throws Exception
	{
		DocumentBuilder builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
		Document doc = builder.newDocument();
		// traverse defined tags and create a document with them all

		Element emaConfig = doc.createElement("EmaConfig");
		doc.appendChild(emaConfig);

		// BEGIN ConsumerGroup

		Element consumerGroup = doc.createElement("ConsumerGroup");
		emaConfig.appendChild(consumerGroup);

		Element defaultConsumer = doc.createElement("DefaultConsumer");
		defaultConsumer.setAttribute("value", "1");
		consumerGroup.appendChild(defaultConsumer);

		Element consumerList = doc.createElement("ConsumerList");
		consumerGroup.appendChild(consumerList);

		Element consumer = doc.createElement("Consumer");
		consumerList.appendChild(consumer);

		// create all known elements below the "Consumer" element
		populateGroup(doc, consumer, ConfigManager.ConsumerTagDict.dict.keySet(),
				new String[] { "ConsumerGroup", "DefaultConsumer", "ConsumerList", "Consumer", "Logger" });

		// BEGIN IProviderGroup

		Element iproviderGroup = doc.createElement("IProviderGroup");
		emaConfig.appendChild(iproviderGroup);

		Element defaultIProvider = doc.createElement("DefaultIProvider");
		defaultIProvider.setAttribute("value", "1");
		iproviderGroup.appendChild(defaultIProvider);

		Element iproviderList = doc.createElement("IProviderList");
		iproviderGroup.appendChild(iproviderList);

		Element iprovider = doc.createElement("IProvider");
		iproviderList.appendChild(iprovider);

		// create all known elements below the "IProvider" element
		populateGroup(doc, iprovider, ConfigManager.IProviderTagDict.dict.keySet(),
				new String[] { "IProviderGroup", "DefaultIProvider", "IProviderList", "IProvider" });

		// BEGIN NiProviderGroup

		Element niproviderGroup = doc.createElement("NiProviderGroup");
		emaConfig.appendChild(niproviderGroup);

		Element defaultNiProvider = doc.createElement("DefaultNiProvider");
		defaultNiProvider.setAttribute("value", "1");
		niproviderGroup.appendChild(defaultNiProvider);

		Element niproviderList = doc.createElement("NiProviderList");
		niproviderGroup.appendChild(niproviderList);

		Element niprovider = doc.createElement("NiProvider");
		niproviderList.appendChild(niprovider);

		// create all known elements below the "NiProvider" element
		populateGroup(doc, niprovider, ConfigManager.NiProviderTagDict.dict.keySet(),
				new String[] { "NiProviderGroup", "DefaultNiProvider", "NiProviderList", "NiProvider" });

		// BEGIN ChannelGroup

		Element channelGroup = doc.createElement("ChannelGroup");
		emaConfig.appendChild(channelGroup);

		Element channelList = doc.createElement("ChannelList");
		channelGroup.appendChild(channelList);

		Element channel = doc.createElement("Channel");
		channelList.appendChild(channel);

		populateGroup(doc, channel, ConfigManager.ChannelTagDict.dict.keySet(),
				new String[] { "ChannelGroup", "ChannelList", "Channel" });

		// BEGIN ServerGroup

		Element serverGroup = doc.createElement("ServerGroup");
		emaConfig.appendChild(serverGroup);

		Element serverList = doc.createElement("ServerList");
		serverGroup.appendChild(serverList);

		Element server = doc.createElement("Server");
		serverList.appendChild(server);

		populateGroup(doc, server, ConfigManager.ServerTagDict.dict.keySet(),
				new String[] { "ServerGroup", "ServerList", "Server" });

		// BEGIN DirectoryGroup

		Element directoryGroup = doc.createElement("DirectoryGroup");
		emaConfig.appendChild(directoryGroup);

		Element defaultDirectory = doc.createElement("DefaultDirectory");
		defaultDirectory.setAttribute("value", "1");
		directoryGroup.appendChild(defaultDirectory);

		Element directoryList = doc.createElement("DirectoryList");
		directoryGroup.appendChild(directoryList);

		Element directory = doc.createElement("Directory");
		directoryList.appendChild(directory);

		populateGroup(doc, directory, ConfigManager.DirectoryTagDict.dict.keySet(),
				new String[] { "DirectoryGroup", "DefaultDirectory", "DirectoryList", "Directory" });

		// BEGIN DictionaryGroup

		Element dictionaryGroup = doc.createElement("DictionaryGroup");
		emaConfig.appendChild(dictionaryGroup);

		Element dictionaryList = doc.createElement("DictionaryList");
		dictionaryGroup.appendChild(dictionaryList);

		Element dictionary = doc.createElement("Dictionary");
		dictionaryList.appendChild(dictionary);

		populateGroup(doc, dictionary, ConfigManager.DictionaryTagDict.dict.keySet(), new String[] { "DictionaryGroup",
				"DefaultDictionary", "DictionaryList", "Dictionary", "DictionaryID" });

		// BEGIN GlobalConfig

		Element globalConfig = doc.createElement("GlobalConfig");
		emaConfig.appendChild(globalConfig);

		populateGroup(doc, globalConfig, ConfigManager.GlobalConfigDict.dict.keySet(), new String[] { "GlobalConfig" });

		// BEGIN WarmStanbdyGroup

		Element warmStanbdyGroup = doc.createElement("WarmStandbyGroup");
		emaConfig.appendChild(warmStanbdyGroup);

		Element warmStandbyList = doc.createElement("WarmStandbyList");
		warmStanbdyGroup.appendChild(warmStandbyList);

		Element warmStandbyChannel = doc.createElement("WarmStandbyChannel");
		warmStandbyList.appendChild(warmStandbyChannel);

		populateGroup(doc, warmStandbyChannel, ConfigManager.WarmStandbyGroupDict.dict.keySet(),
				new String[] { "WarmStandbyGroup", "WarmStandbyList", "WarmStandbyChannel" });

		// BEGIN WarmStandbyServerInfoGroup

		Element warmStandbyServerGroup = doc.createElement("WarmStandbyServerInfoGroup");
		emaConfig.appendChild(warmStandbyServerGroup);

		Element warmStandbyServerInfoList = doc.createElement("WarmStandbyServerInfoList");
		warmStandbyServerGroup.appendChild(warmStandbyServerInfoList);

		Element warmStandbyServerInfo = doc.createElement("WarmStandbyServerInfo");
		warmStandbyServerInfoList.appendChild(warmStandbyServerInfo);

		populateGroup(doc, warmStandbyServerInfo, ConfigManager.WarmStandbyServerDict.dict.keySet(),
				new String[] { "WarmStandbyServerInfoGroup", "WarmStandbyServerInfoList", "WarmStandbyServerInfo" });

		// Load schema and validate this generated document
		SchemaFactory factory = SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);
		Schema schema = factory.newSchema(new File(EMA_SCHEMA_FILE));
		Validator validator = schema.newValidator();

		validator.validate(new DOMSource(doc));
	}

	private void populateGroup(Document doc, Element groupElement, Set<String> keys, String[] skippedElements)
			throws DOMException
	{
		for (String key : keys) {
			// skip higher-level elements
			if (Arrays.asList(skippedElements).contains(key))
				continue;

			String val;

			switch (key) {
			case "ChannelType":
				val = "ChannelType::RSSL_WEBSOCKET";
				break;
			case "CompressionType":
				val = "CompressionType::None";
				break;
			case "EncryptedProtocolType":
				val = "EncryptedProtocolType::RSSL_WEBSOCKET";
				break;
			case "DataState":
				val = "DataState::Suspect";
				break;
			case "DictionaryType":
				val = "DictionaryType::ChannelDictionary";
				break;
			case "ServerType":
				val = "ServerType::RSSL_WEBSOCKET";
				break;
			case "StatusCode":
				val = "StatusCode::AppAuthorizationFailed";
				break;
			case "StreamState":
				val = "StreamState::ClosedRedirected";
				break;
			case "WarmStandbyMode":
				val = "WarmStandbyMode::LOGIN_BASED";
				break;
			default:
				// check that the Schema accepts values of this type
				if (Arrays.asList(ConfigManager.AsciiValues).contains(key))
					val = "text value";
				if (Arrays.asList(ConfigManager.Int64Values).contains(key))
					val = "-1";
				if (Arrays.asList(ConfigManager.UInt64Values).contains(key))
					val = "100";
				if (Arrays.asList(ConfigManager.DoubleValues).contains(key))
					val = "1.2";
				else
					val = "1";
				break;

			}
			Element keyEl = doc.createElement(key);
			keyEl.setAttribute("value", val);

			groupElement.appendChild(keyEl);
		}
	}
}
