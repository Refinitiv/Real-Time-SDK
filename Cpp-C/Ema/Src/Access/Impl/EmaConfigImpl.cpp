/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2022 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#else
#pragma warning( disable : 4355)
#endif

#include <new>

#include "ActiveConfig.h"
#include "EmaConfigImpl.h"
#include "DefaultXML.h"
#include "DictionaryCallbackClient.h"
#include "ReqMsg.h"
#include "DataType.h"
#include "ReqMsgEncoder.h"
#include "RefreshMsg.h"
#include "RefreshMsgEncoder.h"
#include "ProgrammaticConfigure.h"
#include "OmmOAuth2CredentialImpl.h"
#include "OmmException.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

static const char* strEmaConfigXMLFileName = "EmaConfig.xml";

EmaString EmaConfigBaseImpl::defaultEmaConfigXMLFileName(strEmaConfigXMLFileName);

EmaConfigBaseImpl::EmaConfigBaseImpl( const EmaString & path ) :
	_pEmaConfig(new XMLnode("EmaConfig", 0, 0)),
	_pProgrammaticConfigure(0),
	_instanceNodeName(),
	_configSessionName()
{
	createNameToValueHashTable();

	OmmLoggerClient::Severity result(readXMLconfiguration(path));
	if (result == OmmLoggerClient::ErrorEnum || result == OmmLoggerClient::VerboseEnum)
	{
		EmaString errorMsg("failed to extract configuration from path [");
		errorMsg.append(path.c_str()).append("]");
		_pEmaConfig->appendErrorMessage(errorMsg, result);
	}
}

EmaConfigBaseImpl::~EmaConfigBaseImpl()
{
	delete _pEmaConfig;
	delete _pProgrammaticConfigure;
}

void EmaConfigBaseImpl::clear()
{
	_instanceNodeName.clear();
	_configSessionName.clear();
}

const XMLnode* EmaConfigBaseImpl::getNode(const EmaString& itemToRetrieve) const
{
	if (itemToRetrieve.empty())
		return 0;

	EmaString remainingPart;
	EmaString currentPart;
	EmaString name;
	const char* nodeSeparator("|");
	const char* nameSeparator(".");

	Int32 pos(itemToRetrieve.find(nodeSeparator, 0));
	if (pos == -1)
		currentPart = itemToRetrieve;
	else
	{
		currentPart = itemToRetrieve.substr(0, pos);
		remainingPart = itemToRetrieve.substr(pos + 1, EmaString::npos);
	}

	Int32 dotPos(currentPart.find(nameSeparator, 0));
	if (dotPos != -1)
	{
		currentPart = currentPart.substr(0, dotPos);
		name = currentPart.substr(dotPos + 1, EmaString::npos);
	}

	xmlList<XMLnode>* children(_pEmaConfig->getChildren());
	if (children->empty())
		return 0;

	const XMLnode* node = children->getFirst();

	while (node)
	{
		bool match(false);

		if (name.empty())
			match = (node->name() == currentPart);
		else
		{
			if (node->name() == currentPart)
			{
				ConfigElementList* attributes = node->attributes();
				ConfigElement* attribute = attributes->first();
				while (attribute)
				{
					if (attribute->name() == "Name" &&
						*static_cast<XMLConfigElement<EmaString>*>(attribute)->value() == name)
					{
						match = true;
						break;
					}
					attribute = attributes->next(attribute);
				}
			}
		}

		if (match)
		{
			if (remainingPart.empty())
				return node;

			children = node->getChildren();
			if (!children)
				return 0;
			node = children->getFirst();

			pos = remainingPart.find(nodeSeparator, 0);
			if (pos == -1)
			{
				currentPart = remainingPart;
				remainingPart.clear();
			}
			else
			{
				currentPart = remainingPart.substr(0, pos);
				remainingPart = remainingPart.substr(pos + 1, EmaString::npos);
			}

			dotPos = currentPart.find(nameSeparator, 0);
			if (dotPos == -1)
				name.clear();
			else
			{
				name = currentPart.substr(dotPos + 1, EmaString::npos);
				currentPart = currentPart.substr(0, dotPos);
			}
		}
		else
			node = children->getNext(node);
	}
	return 0;
}

void EmaConfigBaseImpl::setDefaultConfigFileName(const EmaString& pathConfigFileName)
{
	if (!pathConfigFileName.empty())
		defaultEmaConfigXMLFileName = pathConfigFileName;
	else
		defaultEmaConfigXMLFileName = strEmaConfigXMLFileName;
}

//  helper function for handling errors in readXMLconfiguration
static OmmLoggerClient::Severity handleConfigurationPathError(const EmaString& errorMsg, bool userSpecifiedPath) {
	if (userSpecifiedPath)
		throwIceException(errorMsg);
	return OmmLoggerClient::ErrorEnum; // only happens if no path was specified
}

/*
 * read and parse a configuration file
 *
 * if parameter path is empty, attempt to read and parse file EmaConfig.xml in the current working directory. Return
 * OmmLoggerClient::SuccessEnum or OmmLoggerClient::ErrorEnum
 *
 * if parameter path is not empty, read the configuration from path if it is a file or file path/EmaConfig.xml if path is a directory.
 * If a configuration can be constructed from the file, return OmmLoggerClient::SuccessEnum. Otherwise, throw an
 * OmmInvalidConfigurationException exception
 */
OmmLoggerClient::Severity EmaConfigBaseImpl::readXMLconfiguration(const EmaString& path)
{
	EmaString fileName;		// eventual location of config file
	const EmaString defaultFileName( defaultEmaConfigXMLFileName ); // used if path is empty or contains a directory

	if ( path.empty() )
		fileName = defaultFileName;
	else {						// user specified a path
		int statResult;
#ifdef WIN32
		struct _stat statBuffer;
		statResult = _stat(path.c_str(), &statBuffer);
#define getcwd _getcwd
#else
		struct stat statBuffer;
		statResult = stat(path.c_str(), &statBuffer);
#endif
		if (statResult == -1) {
			EmaString errorMsg( "configuration path [" );
			errorMsg.append(path).append("] does not exist;")
				.append("working directory was [").append(getcwd(0, 0)).append("];")
				.append("system error message [").append(strerror(errno)).append("]");
			throwIceException(errorMsg);
		}

		// path must be a file or directory
		if (statBuffer.st_mode & S_IFREG) // path is a file
			fileName = path;

		// if path is a directory, create fileName and verify existence, and verify that it is a file
		else if (statBuffer.st_mode & S_IFDIR) {
			fileName = path;
			fileName.append("/").append(defaultFileName);
#ifdef WIN32
			statResult = _stat(fileName.c_str(), &statBuffer);
#else
			statResult = stat(fileName.c_str(), &statBuffer);
#endif
			// file must exist
			if (statResult == -1) {
				EmaString errorMsg( "fileName [" );
				errorMsg.append(fileName).append("] does not exist;")
					.append("working directory was [").append(getcwd(0, 0)).append("];")
					.append("system error message [").append(strerror(errno)).append("]");
				throwIceException(errorMsg);
			}
			// file must be a file
			if ( ! (statBuffer.st_mode & S_IFREG) ) {
				EmaString errorMsg( "fileName [" );
				errorMsg.append(fileName).append("] is not a file;")
					.append("working directory was [").append(getcwd(0, 0)).append("]");
				throwIceException(errorMsg);
			}
		}

		else {
			EmaString errorMsg( "configuration path [" );
			errorMsg.append(path).append("] must be either a file or directory;")
				.append("working directory was [").append(getcwd(0, 0)).append("]");
			throwIceException(errorMsg);
		}
	}

	/* at this point:
	 *    if the user specified a filename, then a path with that name exists, and that path is a file
	 *    if the user did not specify a filename, we are using the default filename and will use
	 *    the result of stat to determine whether or not the file exists
	 */
	int statResult;
#ifdef WIN32
	struct _stat statBuffer;
	statResult = _stat(fileName.c_str(), &statBuffer);
#else
	struct stat statBuffer;
	statResult = stat(fileName.c_str(), &statBuffer);
#endif
	if (statResult == -1 || !statBuffer.st_size)
	{
		EmaString errorMsg("error reading configuration file [");
		errorMsg.append(fileName.c_str()).append("]; file is empty");
		_pEmaConfig->appendErrorMessage(errorMsg, OmmLoggerClient::ErrorEnum);
		return handleConfigurationPathError(errorMsg, !path.empty());
	}

	FILE* fp;
	fp = fopen(fileName.c_str(), "r");
	if (!fp)
	{
		EmaString errorMsg("error reading configuration file [");
		errorMsg.append(fileName.c_str()).append("]; could not open file; system error message [").append(strerror(errno)).append("]");
		_pEmaConfig->appendErrorMessage(errorMsg, OmmLoggerClient::ErrorEnum);
		return handleConfigurationPathError(errorMsg, !path.empty());
	}

	char* xmlData = reinterpret_cast<char*>(malloc(statBuffer.st_size + 1));
	if (!xmlData)
	{
		EmaString errorMsg("Failed to allocate memory for reading configuration file[");
		errorMsg.append(fileName.c_str()).append("]");
		_pEmaConfig->appendErrorMessage(errorMsg, OmmLoggerClient::ErrorEnum);
		return handleConfigurationPathError(errorMsg, !path.empty());
	}

	size_t bytesRead(fread(reinterpret_cast<void*>(xmlData), sizeof(char), statBuffer.st_size, fp));
	if (!bytesRead)
	{
		EmaString errorMsg("error reading configuration file [");
		errorMsg.append(fileName.c_str()).append("]; fread failed; system error message [").append(strerror(errno)).append("]");
		_pEmaConfig->appendErrorMessage(errorMsg, OmmLoggerClient::ErrorEnum);
		free(xmlData);
		return handleConfigurationPathError(errorMsg, !path.empty());
	}
	fclose(fp);
	xmlData[bytesRead] = 0;
	bool retVal(extractXMLdataFromCharBuffer(fileName.c_str(), xmlData, static_cast<int>(bytesRead)));
	free(xmlData);

	if ( retVal )
		return OmmLoggerClient::SuccessEnum;
	else {
		EmaString errorMsg;
		if (!path.empty())
			errorMsg.append("could not construct configuration from file [").append(fileName).append("]");
		return handleConfigurationPathError(errorMsg, !path.empty());
	}
}

bool EmaConfigBaseImpl::extractXMLdataFromCharBuffer(const EmaString& what, const char* xmlData, int length)
{
	LIBXML_TEST_VERSION

		EmaString note("extracting XML data from ");
	note.append(what);
	_pEmaConfig->appendErrorMessage(note, OmmLoggerClient::VerboseEnum);

	xmlDocPtr xmlDoc = xmlReadMemory(xmlData, length, NULL, "notnamed.xml", XML_PARSE_HUGE);
	if (xmlDoc == NULL)
	{
		EmaString errorMsg("extractXMLdataFromCharBuffer: xmlReadMemory failed while processing ");
		errorMsg.append(what);
		_pEmaConfig->appendErrorMessage(errorMsg, OmmLoggerClient::ErrorEnum);
		xmlFreeDoc(xmlDoc);
		return false;
	}

	xmlNodePtr _xmlNodePtr = xmlDocGetRootElement(xmlDoc);
	if (_xmlNodePtr == NULL)
	{
		EmaString errorMsg("extractXMLdataFromCharBuffer: xmlDocGetRootElement failed while processing ");
		errorMsg.append(what);
		_pEmaConfig->appendErrorMessage(errorMsg, OmmLoggerClient::ErrorEnum);
		xmlFreeDoc(xmlDoc);
		return false;
	}

	processXMLnodePtr(_pEmaConfig, _xmlNodePtr);
	_pEmaConfig->name(reinterpret_cast<const char*>(_xmlNodePtr->name));
	xmlFreeDoc(xmlDoc);
	return true;
}

void EmaConfigBaseImpl::processXMLnodePtr(XMLnode* theNode, const xmlNodePtr& nodePtr)
{
	// add attibutes
	if (nodePtr->properties)
	{
		xmlChar* value(0);
		for (xmlAttrPtr attrPtr = nodePtr->properties; attrPtr != NULL; attrPtr = attrPtr->next)
		{
			if (!xmlStrcmp(attrPtr->name, reinterpret_cast<const xmlChar*>("value")))
				value = xmlNodeListGetString(attrPtr->doc, attrPtr->children, 1);
			else
			{
				EmaString errorMsg("got unexpected name [");
				errorMsg.append(reinterpret_cast<const char*>(attrPtr->name)).append("] while processing XML data; ignored");
				theNode->appendErrorMessage(errorMsg, OmmLoggerClient::VerboseEnum);
			}
		}

		static EmaString errorMsg;
		ConfigElement* e(createConfigElement(reinterpret_cast<const char*>(nodePtr->name), theNode->parent(),
			reinterpret_cast<const char*>(value), errorMsg));
		if (e)
			theNode->addAttribute(e);
		else if (!errorMsg.empty())
		{
			theNode->appendErrorMessage(errorMsg, OmmLoggerClient::ErrorEnum);
			errorMsg.clear();
		}

		if (value)
			xmlFree(value);
	}

	for (xmlNodePtr childNodePtr = nodePtr->children; childNodePtr; childNodePtr = childNodePtr->next)
	{
		if (xmlIsBlankNode(childNodePtr))
			continue;
		if (childNodePtr->type == XML_COMMENT_NODE)
			continue;

		switch (childNodePtr->type)
		{
		case XML_TEXT_NODE:
		case XML_PI_NODE:
			break;
		case XML_ELEMENT_NODE:
		{
			XMLnode* child(new XMLnode(reinterpret_cast<const char*>(childNodePtr->name), theNode->level() + 1, theNode));
			static int instance(0);
			++instance;
			processXMLnodePtr(child, childNodePtr);
			if (child->errorCount())
			{
				theNode->errors().add(child->errors());
				child->errors().clear();
			}
			--instance;

			if (childNodePtr->properties && !childNodePtr->children)
			{
				theNode->appendAttributes(child->attributes(), true);
				delete child;
			}
			else if (!childNodePtr->properties && childNodePtr->children)
			{
				if (theNode->addChild(child))
				{
					delete child;
				}
			}
			else if (!childNodePtr->properties && !childNodePtr->children)
			{
				EmaString errorMsg("node [");
				errorMsg.append(reinterpret_cast<const char*>(childNodePtr->name)).append("has neither children nor attributes");
				theNode->appendErrorMessage(errorMsg, OmmLoggerClient::VerboseEnum);
			}
			else
			{
				EmaString errorMsg("node [");
				errorMsg.append(reinterpret_cast<const char*>(childNodePtr->name)).append("has both children and attributes; node was ignored");
				theNode->appendErrorMessage(errorMsg, OmmLoggerClient::ErrorEnum);
			}
			break;
		}
		default:
			EmaString errorMsg("childNodePtr has unhandled type [");
			errorMsg.append(childNodePtr->type).append("]");
			theNode->appendErrorMessage(errorMsg, OmmLoggerClient::VerboseEnum);
		}
	}
}

void EmaConfigBaseImpl::createNameToValueHashTable()
{
	for (int i = 0; i < sizeof AsciiValues / sizeof(EmaString); ++i)
		nameToValueHashTable.insert(AsciiValues[i], ConfigElement::ConfigElementTypeAscii);

	for (int i = 0; i < sizeof EnumeratedValues / sizeof(EmaString); ++i)
		nameToValueHashTable.insert(EnumeratedValues[i], ConfigElement::ConfigElementTypeEnum);

	for (int i = 0; i < sizeof Int64Values / sizeof(EmaString); ++i)
		nameToValueHashTable.insert(Int64Values[i], ConfigElement::ConfigElementTypeInt64);

	for (int i = 0; i < sizeof UInt64Values / sizeof(EmaString); ++i)
		nameToValueHashTable.insert(UInt64Values[i], ConfigElement::ConfigElementTypeUInt64);

	for (int i = 0; i < sizeof DoubleValues / sizeof(EmaString); ++i)
		nameToValueHashTable.insert(DoubleValues[i], ConfigElement::ConfigElementTypeDouble);
}

ConfigElement* EmaConfigBaseImpl::createConfigElement(const char* name, XMLnode* parent, const char* value, EmaString& errorMsg)
{
	ConfigElement* e(0);
	ConfigElement::ConfigElementType* elementType = nameToValueHashTable.find(name);
	if (elementType == 0)
		errorMsg.append("unsupported configuration element [").append(name).append("]; element ignored");
	else switch (*elementType)
	{
	case ConfigElement::ConfigElementTypeAscii:
		e = new XMLConfigElement<EmaString>(name, parent, ConfigElement::ConfigElementTypeAscii, value);
		break;
	case ConfigElement::ConfigElementTypeEnum:
		e = convertEnum(name, parent, value, errorMsg);
		break;
	case ConfigElement::ConfigElementTypeInt64:
	{
		if (!validateConfigElement(value, ConfigElement::ConfigElementTypeInt64))
		{
			errorMsg.append("value [").append(value).append("] for config element [").append(name).append("] is not a signed integer; element ignored");
			break;
		}
		Int64 converted;
#ifdef WIN32
		converted = _strtoi64(value, 0, 0);
#else
		converted = strtoll(value, 0, 0);
#endif
		e = new XMLConfigElement<Int64>(name, parent, ConfigElement::ConfigElementTypeInt64, converted);
	}
	break;
	case ConfigElement::ConfigElementTypeUInt64:
	{
		if (!validateConfigElement(value, ConfigElement::ConfigElementTypeUInt64))
		{
			errorMsg.append("value [").append(value).append("] for config element [").append(name).append("] is not an unsigned integer; element ignored");
			break;
		}
		UInt64 converted;
#ifdef WIN32
		converted = _strtoui64(value, 0, 0);
#else
		converted = strtoull(value, 0, 0);
#endif
		e = new XMLConfigElement<UInt64>(name, parent, ConfigElement::ConfigElementTypeUInt64, converted);
	}
	break;
	case ConfigElement::ConfigElementTypeDouble:
	{
		if (!validateConfigElement(value, ConfigElement::ConfigElementTypeDouble))
		{
			errorMsg.append("value [").append(value).append("] for config element [").append(name).append("] is not an double; element ignored");
			break;
		}
		Double converted;
		char* endStr;

		converted = strtod(value, &endStr);

		e = new XMLConfigElement<Double>(name, parent, ConfigElement::ConfigElementTypeDouble, converted);
	}
	break;
	default:
		errorMsg.append("config element [").append(name).append("] had unexpected elementType [").append(*elementType).append("; element ignored");
		break;
	}
	return e;
}

bool EmaConfigBaseImpl::validateConfigElement(const char* value, ConfigElement::ConfigElementType valueType) const
{
	if (!strlen(value))
		return false;

	const char* p = value;

	if (valueType == ConfigElement::ConfigElementTypeInt64 || valueType == ConfigElement::ConfigElementTypeUInt64)
	{
		if (valueType == ConfigElement::ConfigElementTypeInt64 && !isdigit(*p))
		{
			if (*p++ != '-')
				return false;
			if (!*p)
				return false;
		}
		for (; *p; ++p)
			if (!isdigit(*p))
				return false;
		return true;
	}
	else if (valueType == ConfigElement::ConfigElementTypeDouble)
	{
		for (; *p; ++p)
			if (!isdigit(*p) && *p != '.')
				return false;
		return true;
	}

	return false;
}

ConfigElement* EmaConfigBaseImpl::convertEnum(const char* name, XMLnode* parent, const char* value, EmaString& errorMsg)
{
	EmaString enumValue(value);
	int colonPosition(enumValue.find("::"));
	if (colonPosition == -1)
	{
		errorMsg.append("configuration attribute [").append(name)
			.append("] has an invalid Enum value format [").append(value)
			.append("]; expected typename::value (e.g., OperationModel::ApiDispatch)");
		return 0;
	}

	EmaString enumType(enumValue.substr(0, colonPosition));
	enumValue = enumValue.substr(colonPosition + 2, enumValue.length() - colonPosition - 2);

	if (!strcmp(enumType, "LoggerSeverity"))
	{
		static struct
		{
			const char* configInput;
			OmmLoggerClient::Severity convertedValue;
		} converter[] =
		{
			{ "Verbose", OmmLoggerClient::VerboseEnum },
			{ "Success", OmmLoggerClient::SuccessEnum },
			{ "Warning", OmmLoggerClient::WarningEnum },
			{ "Error", OmmLoggerClient::ErrorEnum },
			{ "NoLogMsg", OmmLoggerClient::NoLogMsgEnum }
		};

		for (int i = 0; i < sizeof converter / sizeof converter[0]; ++i)
			if (!strcmp(converter[i].configInput, enumValue))
				return new XMLConfigElement<OmmLoggerClient::Severity>(name, parent, ConfigElement::ConfigElementTypeEnum, converter[i].convertedValue);
	}
	else if (!strcmp(enumType, "LoggerType"))
	{
		static struct
		{
			const char* configInput;
			OmmLoggerClient::LoggerType convertedValue;
		} converter[] =
		{
			{ "File", OmmLoggerClient::FileEnum },
			{ "Stdout", OmmLoggerClient::StdoutEnum },
		};
		for (int i = 0; i < sizeof converter / sizeof converter[0]; i++)
			if (!strcmp(converter[i].configInput, enumValue))
				return new XMLConfigElement<OmmLoggerClient::LoggerType>(name, parent, ConfigElement::ConfigElementTypeEnum, converter[i].convertedValue);
	}
	else if (!strcmp(enumType, "DictionaryType"))
	{
		static struct
		{
			const char* configInput;
			Dictionary::DictionaryType convertedValue;
		} converter[] =
		{
			{ "FileDictionary", Dictionary::FileDictionaryEnum },
			{ "ChannelDictionary", Dictionary::ChannelDictionaryEnum },
		};

		for (int i = 0; i < sizeof converter / sizeof converter[0]; i++)
			if (!strcmp(converter[i].configInput, enumValue))
				return new XMLConfigElement<Dictionary::DictionaryType>(name, parent, ConfigElement::ConfigElementTypeEnum, converter[i].convertedValue);
	}
	else if (!strcmp(enumType, "ChannelType"))
	{
		static struct
		{
			const char* configInput;
			RsslConnectionTypes convertedValue;
		} converter[] =
		{
			{ "RSSL_SOCKET", RSSL_CONN_TYPE_SOCKET },
			{ "RSSL_HTTP", RSSL_CONN_TYPE_HTTP },
			{ "RSSL_ENCRYPTED", RSSL_CONN_TYPE_ENCRYPTED },
			{ "RSSL_RELIABLE_MCAST", RSSL_CONN_TYPE_RELIABLE_MCAST },
			{ "RSSL_WEBSOCKET", RSSL_CONN_TYPE_WEBSOCKET },
		};

		for (int i = 0; i < sizeof converter / sizeof converter[0]; i++)
			if (!strcmp(converter[i].configInput, enumValue))
				return new XMLConfigElement<RsslConnectionTypes>(name, parent, ConfigElement::ConfigElementTypeEnum, converter[i].convertedValue);
	}
	else if (!strcmp(enumType, "EncryptedProtocolType"))
	{
		static struct
		{
			const char* configInput;
			RsslConnectionTypes convertedValue;
		} converter[] =
		{
			{ "RSSL_SOCKET", RSSL_CONN_TYPE_SOCKET },
			{ "RSSL_HTTP", RSSL_CONN_TYPE_HTTP },
			{ "RSSL_WEBSOCKET", RSSL_CONN_TYPE_WEBSOCKET },
		};

		for (int i = 0; i < sizeof converter / sizeof converter[0]; i++)
			if (!strcmp(converter[i].configInput, enumValue))
				return new XMLConfigElement<RsslConnectionTypes>(name, parent, ConfigElement::ConfigElementTypeEnum, converter[i].convertedValue);
	}
	else if (!strcmp(enumType, "ServerType"))
	{
		static struct
		{
			const char* configInput;
			RsslConnectionTypes convertedValue;
		} converter[] =
		{
			{ "RSSL_SOCKET", RSSL_CONN_TYPE_SOCKET },
			{ "RSSL_ENCRYPTED", RSSL_CONN_TYPE_ENCRYPTED },
			{ "RSSL_WEBSOCKET", RSSL_CONN_TYPE_WEBSOCKET }
		};

		for (int i = 0; i < sizeof converter / sizeof converter[0]; i++)
			if (!strcmp(converter[i].configInput, enumValue))
				return new XMLConfigElement<RsslConnectionTypes>(name, parent, ConfigElement::ConfigElementTypeEnum, converter[i].convertedValue);
	}
	else if (!strcmp(enumType, "CompressionType"))
	{
		static struct
		{
			const char* configInput;
			RsslCompTypes convertedValue;
		} converter[] =
		{
			{ "None", RSSL_COMP_NONE },
			{ "ZLib", RSSL_COMP_ZLIB },
			{ "LZ4", RSSL_COMP_LZ4 },
		};

		for (int i = 0; i < sizeof converter / sizeof converter[0]; i++)
			if (!strcmp(converter[i].configInput, enumValue))
				return new XMLConfigElement<RsslCompTypes>(name, parent, ConfigElement::ConfigElementTypeEnum, converter[i].convertedValue);
	}
	else if (!strcmp(enumType, "StreamState"))
	{
		for (int i = 0; i < sizeof streamStateConverter / sizeof streamStateConverter[0]; i++)
			if (!strcmp(streamStateConverter[i].configInput, enumValue))
				return new XMLConfigElement<OmmState::StreamState>(name, parent, ConfigElement::ConfigElementTypeEnum, streamStateConverter[i].convertedValue);
	}
	else if (!strcmp(enumType, "DataState"))
	{
		for (int i = 0; i < sizeof dataStateConverter / sizeof dataStateConverter[0]; i++)
			if (!strcmp(dataStateConverter[i].configInput, enumValue))
				return new XMLConfigElement<OmmState::DataState>(name, parent, ConfigElement::ConfigElementTypeEnum, dataStateConverter[i].convertedValue);
	}
	else if (!strcmp(enumType, "StatusCode"))
	{
		for (int i = 0; i < sizeof statusCodeConverter / sizeof statusCodeConverter[0]; i++)
			if (!strcmp(statusCodeConverter[i].configInput, enumValue))
				return new XMLConfigElement<OmmState::StatusCode>(name, parent, ConfigElement::ConfigElementTypeEnum, statusCodeConverter[i].convertedValue);
	}
	else if (!strcmp(enumType, "WarmStandbyMode"))
	{
		for (int i = 0; i < sizeof warmStandbyModeConverter / sizeof warmStandbyModeConverter[0]; i++)
			if (!strcmp(warmStandbyModeConverter[i].configInput, enumValue))
				return new XMLConfigElement<RsslReactorWarmStandbyMode>(name, parent, ConfigElement::ConfigElementTypeEnum, warmStandbyModeConverter[i].convertedValue);
	}
	else
	{
		errorMsg.append("no implementation in convertEnum for enumType [").append(enumType.c_str()).append("]");
		return 0;
	}

	errorMsg.append("convertEnum has an implementation for enumType [").append(enumType.c_str()).append("] but no appropriate conversion for value [").append(enumValue.c_str()).append("]");
	return 0;
}

void EmaConfigBaseImpl::config(const Data& config)
{
	if (config.getDataType() == DataType::MapEnum)
	{
		if (!_pProgrammaticConfigure)
		{
			_pProgrammaticConfigure = new ProgrammaticConfigure(static_cast<const Map&>(config), _pEmaConfig->errors());
		}
		else
		{
			_pProgrammaticConfigure->addConfigure(static_cast<const Map&>(config));
		}
	}
	else
	{
		EmaString temp("Invalid Data type='");
		temp.append(getDTypeAsString(config.getDataType())).append("' for Programmatic Configure.");
		EmaConfigError* mce(new EmaConfigError(temp, OmmLoggerClient::ErrorEnum));
		_pEmaConfig->errors().add(mce);
	}
}

void EmaConfigBaseImpl::getLoggerName(const EmaString& instanceName, EmaString& retVal) const
{
	if (_pProgrammaticConfigure && _pProgrammaticConfigure->getActiveLoggerName(instanceName, retVal))
		return;

	EmaString nodeName(_instanceNodeName);
	nodeName.append(instanceName).append("|Logger");

	get<EmaString>(nodeName, retVal);
}

void EmaConfigBaseImpl::getAsciiAttributeValueList(const EmaString& nodeName, const EmaString& attributeName, EmaVector< EmaString >& entryValues)
{
	_pEmaConfig->getAsciiAttributeValueList(nodeName, attributeName, entryValues);
}

void EmaConfigBaseImpl::getEntryNodeList(const EmaString& nodeName, const EmaString& entryName, EmaVector< XMLnode* >& entryNodeList)
{
	_pEmaConfig->getEntryNodeList(nodeName, entryName, entryNodeList);
}

void EmaConfigBaseImpl::getServiceNames(const EmaString& directoryName, EmaVector< EmaString >& serviceNames)
{
	_pEmaConfig->getServiceNameList(directoryName, serviceNames);
}


EmaConfigImpl::EmaConfigImpl(const EmaString& path) :
	EmaConfigBaseImpl( path ),
	configFilePath(path),
	_loginRdmReqMsg(),
	_pDirectoryRsslRequestMsg( 0 ),
	_pRdmFldRsslRequestMsg( 0 ),
	_pEnumDefRsslRequestMsg( 0 ),
	_pDirectoryRsslRefreshMsg( 0 ),
	_hostnameSetViaFunctionCall(),
	_portSetViaFunctionCall(),
	_proxyHostnameSetViaFunctionCall(),
	_proxyPortSetViaFunctionCall(),
	_securityProtocolSetViaFunctionCall(OmmConsumerConfig::ENC_NONE),
	_proxyUserNameSetViaFunctionCall(),
	_proxyPasswdSetViaFunctionCall(),
	_proxyDomainSetViaFunctionCall(),
	_objectName(),
	_libSslName(),
	_libCryptoName(),
	_libcurlName(),
	_tokenServiceUrlV1(),
	_tokenServiceUrlV2(),
	_sslCAStoreSetViaFunctionCall()
{
}

EmaConfigImpl::~EmaConfigImpl()
{
	if ( _pDirectoryRsslRefreshMsg )
		delete _pDirectoryRsslRefreshMsg;

	if ( _pDirectoryRsslRequestMsg )
		delete _pDirectoryRsslRequestMsg;

	if ( _pRdmFldRsslRequestMsg )
		delete _pRdmFldRsslRequestMsg;

	if ( _pEnumDefRsslRequestMsg )
		delete _pEnumDefRsslRequestMsg;

	if (_oAuth2Credentials.size() != 0)
	{
		OmmOAuth2CredentialImpl* tmpCredentials;
		
		for (int i = 0; i < (int)_oAuth2Credentials.size(); i++)
		{
			tmpCredentials = _oAuth2Credentials[i];
			delete tmpCredentials;
		}
	}

	_oAuthCredential.clear();

	if (_LoginRequestMsgs.size() != 0)
	{
		LoginRdmReqMsgImpl* tmpMsg;

		for (int i = 0; i < (int)_LoginRequestMsgs.size(); i++)
		{
			tmpMsg = _LoginRequestMsgs[i];
			delete tmpMsg;
		}
	}

	_LoginRequestMsgs.clear();
}

void EmaConfigImpl::clear()
{
	_loginRdmReqMsg.clear();
	_oAuthCredential.clear();

	if ( _pDirectoryRsslRefreshMsg )
		_pDirectoryRsslRefreshMsg->clear();

	if ( _pDirectoryRsslRequestMsg )
		_pDirectoryRsslRequestMsg->clear();

	if ( _pRdmFldRsslRequestMsg )
		_pRdmFldRsslRequestMsg->clear();

	if ( _pEnumDefRsslRequestMsg )
		_pEnumDefRsslRequestMsg->clear();

	if ( _pProgrammaticConfigure )
		_pProgrammaticConfigure->clear();

	_instanceNodeName.clear();
	_proxyHostnameSetViaFunctionCall.clear();
	_proxyPortSetViaFunctionCall.clear();
	_securityProtocolSetViaFunctionCall = OmmConsumerConfig::ENC_NONE;
	_proxyUserNameSetViaFunctionCall.clear();
	_proxyPasswdSetViaFunctionCall.clear();
	_proxyDomainSetViaFunctionCall.clear();
	_objectName.clear();
	_libSslName.clear();
	_libCryptoName.clear();
	_tokenServiceUrlV1.clear();
	_tokenServiceUrlV2.clear();
	_serviceDiscoveryUrl.clear();
}

void EmaConfigImpl::username( const EmaString& username )
{
	_loginRdmReqMsg.username( username );
	_oAuthCredential.userName(username);
}

void EmaConfigImpl::password( const EmaString& password )
{
	_loginRdmReqMsg.password( password );
	_oAuthCredential.password(password);
}

void EmaConfigImpl::position( const EmaString& position )
{
	_loginRdmReqMsg.position( position );
}

void EmaConfigImpl::applicationId( const EmaString& applicationId )
{
	_loginRdmReqMsg.applicationId( applicationId );
}

void EmaConfigImpl::applicationName( const EmaString& applicationName )
{
	_loginRdmReqMsg.applicationName( applicationName );
}

void EmaConfigImpl::instanceId( const EmaString& instanceId )
{
	_loginRdmReqMsg.instanceId( instanceId );
}

void EmaConfigImpl::clientId( const EmaString& clientId )
{
	_oAuthCredential.clientId(clientId);
}

void EmaConfigImpl::clientSecret( const EmaString& clientSecret )
{
	_oAuthCredential.clientSecret(clientSecret);
}

void EmaConfigImpl::tokenScope(const EmaString& tokenScope)
{
	_oAuthCredential.tokenScope(tokenScope);
}

void EmaConfigImpl::takeExclusiveSignOnControl( bool takeExclusiveSignOnControl )
{
	_oAuthCredential.takeExclusiveSignOnControl(takeExclusiveSignOnControl);
}

void EmaConfigImpl::tokenServiceUrl( const EmaString& tokenServiceUrl )
{
	_tokenServiceUrlV1 = tokenServiceUrl;
}

void EmaConfigImpl::tokenServiceUrlV1(const EmaString& tokenServiceUrl)
{
	_tokenServiceUrlV1 = tokenServiceUrl;
}

void EmaConfigImpl::tokenServiceUrlV2(const EmaString& tokenServiceUrl)
{
	_tokenServiceUrlV2 = tokenServiceUrl;
}

void EmaConfigImpl::serviceDiscoveryUrl( const EmaString& serviceDiscoveryUrl )
{
	_serviceDiscoveryUrl = serviceDiscoveryUrl;
}

void EmaConfigImpl::host( const EmaString& host )
{
	Int32 index = host.find( ":", 0 );
	if ( index == -1 )
	{
		if ( host.length() )
			_hostnameSetViaFunctionCall = host;
		else
			_hostnameSetViaFunctionCall = DEFAULT_HOST_NAME;
	}

	else if ( index == 0 )
	{
		_hostnameSetViaFunctionCall = DEFAULT_HOST_NAME;

		if (host.length() > 1)
		{
			_portSetViaFunctionCall.userSet = true;
			_portSetViaFunctionCall.userSpecifiedValue = host.substr(1, host.length() - 1);
		}
	}

	else
	{
		_hostnameSetViaFunctionCall = host.substr( 0, index );
		if (host.length() > static_cast<UInt32>(index + 1))
		{
			_portSetViaFunctionCall.userSet = true;
			_portSetViaFunctionCall.userSpecifiedValue = host.substr(index + 1, host.length() - index - 1);
		}
			
	}
}

void EmaConfigImpl::addAdminMsg( const ReqMsg& reqMsg )
{
	RsslRequestMsg* pRsslRequestMsg = static_cast<const ReqMsgEncoder&>( reqMsg.getEncoder() ).getRsslRequestMsg();

	switch ( pRsslRequestMsg->msgBase.domainType )
	{
	case RSSL_DMT_LOGIN:
		addLoginReqMsg( pRsslRequestMsg );
		break;
	case RSSL_DMT_DICTIONARY:
		addDictionaryReqMsg( pRsslRequestMsg, static_cast<const ReqMsgEncoder&>( reqMsg.getEncoder() ).hasServiceName() ?
			&static_cast<const ReqMsgEncoder&>( reqMsg.getEncoder() ).getServiceName() : 0 );
		break;
	case RSSL_DMT_SOURCE:
		addDirectoryReqMsg( pRsslRequestMsg );
		break;
	default:
	{
		EmaString temp( "Request message with unhandled domain passed into addAdminMsg( const ReqMsg& ). Domain type='" );
		temp.append( pRsslRequestMsg->msgBase.domainType ).append( "'. " );
		EmaConfigError* mce( new EmaConfigError( temp, OmmLoggerClient::ErrorEnum ) );
		_pEmaConfig->errors().add( mce );
	}
	break;
	}
}

void EmaConfigImpl::addAdminMsg( const RefreshMsg& refreshMsg )
{
	RsslRefreshMsg* pRsslRefreshMsg = static_cast<const RefreshMsgEncoder&>( refreshMsg.getEncoder() ).getRsslRefreshMsg();

	switch ( pRsslRefreshMsg->msgBase.domainType )
	{
	case RSSL_DMT_LOGIN:
		break;
	case RSSL_DMT_DICTIONARY:
		break;
	case RSSL_DMT_SOURCE:
	{
		if ( pRsslRefreshMsg->msgBase.streamId > 0 )
		{
			EmaString temp( "Refresh passed into addAdminMsg( const RefreshMsg& ) contains unhandled stream id. StreamId='" );
			temp.append( pRsslRefreshMsg->msgBase.streamId ).append( "'. " );
			EmaConfigError* mce( new EmaConfigError( temp, OmmLoggerClient::ErrorEnum ) );
			_pEmaConfig->errors().add( mce );
			return;
		}

		if ( pRsslRefreshMsg->msgBase.containerType != RSSL_DT_MAP )
		{
			EmaString temp( "RefreshMsg with SourceDirectory passed into addAdminMsg( const RefreshMsg& ) contains a container with wrong data type. Expected container data type is Map. Passed in is " );
			temp += DataType((DataType::DataTypeEnum)pRsslRefreshMsg->msgBase.containerType).toString();
			EmaConfigError* mce( new EmaConfigError( temp, OmmLoggerClient::ErrorEnum ) );
			_pEmaConfig->errors().add( mce );
			return;
		}

		addDirectoryRefreshMsg( pRsslRefreshMsg );
	}
	break;
	default:
	{
		EmaString temp( "Refresh message passed into addAdminMsg( const RefreshMsg& ) contains unhandled domain type. Domain type='" );
		temp.append( pRsslRefreshMsg->msgBase.domainType ).append( "'. " );
		EmaConfigError* mce( new EmaConfigError( temp, OmmLoggerClient::ErrorEnum ) );
		_pEmaConfig->errors().add( mce );
	}
	break;
	}
}

void EmaConfigImpl::getChannelName( const EmaString& instanceName, EmaString& retVal ) const
{
	if ( _pProgrammaticConfigure && _pProgrammaticConfigure->getActiveChannelName( instanceName, retVal ) )
		return;

	EmaString nodeName( _instanceNodeName );
	nodeName.append( instanceName );
	nodeName.append( "|Channel" );

	if ( !get<EmaString>(nodeName, retVal) )
	{
		EmaString nodeName( _instanceNodeName );
		nodeName.append( instanceName );
		nodeName.append( "|ChannelSet" );

		get<EmaString>( nodeName, retVal );
	}
}

void EmaConfigImpl::getWarmStandbyChannelName(const EmaString& instanceName, EmaString& retVal, bool& foundProgrammaticCfg) const
{
	if (_pProgrammaticConfigure && _pProgrammaticConfigure->getActiveWSBChannelSetName(instanceName, retVal))
	{
		foundProgrammaticCfg = true;
		return;
	}

	EmaString nodeName(_instanceNodeName);
	nodeName.append(instanceName);
	nodeName.append("|WarmStandbyChannelSet");

	get<EmaString>(nodeName, retVal);

	foundProgrammaticCfg = false;
}

bool EmaConfigImpl::getDictionaryName(const EmaString& instanceName, EmaString& retVal) const
{
	if (!_pProgrammaticConfigure || !_pProgrammaticConfigure->getActiveDictionaryName(instanceName, retVal))
	{
		EmaString nodeName(_instanceNodeName);
		nodeName.append(instanceName);
		nodeName.append("|Dictionary");
		get<EmaString>(nodeName, retVal);
	}

	return true;
}

void EmaConfigImpl::addLoginReqMsg( RsslRequestMsg* pRsslRequestMsg )
{
	_loginRdmReqMsg.set(this,  pRsslRequestMsg );
}

void EmaConfigImpl::addDictionaryReqMsg( RsslRequestMsg* pRsslRequestMsg, const EmaString* serviceName )
{
	if ( !( pRsslRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME ) )
	{
		EmaString temp( "Received dicionary request message contains no dictionary name. Message ignored." );
		_pEmaConfig->appendErrorMessage( temp, OmmLoggerClient::ErrorEnum );
		return;
	}
	if ( !( pRsslRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID ) )
	{
		if ( !serviceName )
		{
			EmaString temp( "Received dicionary request message contains no serviceId or service name. Message ignored." );
			_pEmaConfig->appendErrorMessage( temp, OmmLoggerClient::ErrorEnum );
			return;
		}
	}
	else if ( !( pRsslRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_FILTER ) )
	{
		EmaString temp( "Received dicionary request message contains no filter. Message ignored." );
		_pEmaConfig->appendErrorMessage( temp, OmmLoggerClient::ErrorEnum );
		return;
	}
	else if ( pRsslRequestMsg->flags & RSSL_RQMF_NO_REFRESH )
	{
		EmaString temp( "Received dicionary request message contains no_refresh flag. Message ignored." );
		_pEmaConfig->appendErrorMessage( temp, OmmLoggerClient::ErrorEnum );
		return;
	}

	RsslBuffer rdmFldDictionaryName;
	rdmFldDictionaryName.data = ( char* ) "RWFFld";
	rdmFldDictionaryName.length = 6;

	RsslBuffer enumtypedefName;
	enumtypedefName.data = ( char* ) "RWFEnum";
	enumtypedefName.length = 7;

	if ( rsslBufferIsEqual( &pRsslRequestMsg->msgBase.msgKey.name, &rdmFldDictionaryName ) )
	{
		if ( !_pRdmFldRsslRequestMsg )
			_pRdmFldRsslRequestMsg = new AdminReqMsg( *this );

		_pRdmFldRsslRequestMsg->set( pRsslRequestMsg );

		if ( serviceName )
			_pRdmFldRsslRequestMsg->setServiceName( *serviceName );
	}
	else if ( rsslBufferIsEqual( &pRsslRequestMsg->msgBase.msgKey.name, &enumtypedefName ) )
	{
		if ( !_pEnumDefRsslRequestMsg )
			_pEnumDefRsslRequestMsg = new AdminReqMsg( *this );

		_pEnumDefRsslRequestMsg->set( pRsslRequestMsg );

		if ( serviceName )
			_pEnumDefRsslRequestMsg->setServiceName( *serviceName );
	}
	else
	{
		EmaString temp( "Received dicionary request message contains unrecognized dictionary name. Message ignored." );
		_pEmaConfig->appendErrorMessage( temp, OmmLoggerClient::ErrorEnum );
	}
}

void EmaConfigImpl::addDirectoryReqMsg( RsslRequestMsg* pRsslRequestMsg )
{
	if ( !_pDirectoryRsslRequestMsg )
		_pDirectoryRsslRequestMsg = new AdminReqMsg( *this );

	_pDirectoryRsslRequestMsg->set( pRsslRequestMsg );
}

void EmaConfigImpl::addDirectoryRefreshMsg( RsslRefreshMsg* pRsslRefreshMsg )
{
	if ( !_pDirectoryRsslRefreshMsg )
		_pDirectoryRsslRefreshMsg = new AdminRefreshMsg( this );

	_pDirectoryRsslRefreshMsg->set( pRsslRefreshMsg );
}

RsslRDMLoginRequest* EmaConfigImpl::getLoginReq()
{
	return _loginRdmReqMsg.get();
}

LoginRdmReqMsgImpl& EmaConfigImpl::getLoginRdmReqMsg()
{
	return _loginRdmReqMsg;
}

RsslRequestMsg* EmaConfigImpl::getDirectoryReq()
{
	return _pDirectoryRsslRequestMsg ? _pDirectoryRsslRequestMsg->get() : 0;
}

OAuth2Credential& EmaConfigImpl::getOAuthCredential()
{
	return _oAuthCredential;
}

AdminReqMsg* EmaConfigImpl::getRdmFldDictionaryReq()
{
	return _pRdmFldRsslRequestMsg ? _pRdmFldRsslRequestMsg : 0;
}

AdminReqMsg* EmaConfigImpl::getEnumDefDictionaryReq()
{
	return _pEnumDefRsslRequestMsg ? _pEnumDefRsslRequestMsg : 0;
}

AdminRefreshMsg* EmaConfigImpl::getDirectoryRefreshMsg()
{
	AdminRefreshMsg* pTemp = _pDirectoryRsslRefreshMsg;
	_pDirectoryRsslRefreshMsg = 0;
	return pTemp;
}

void EmaConfigImpl::proxyHostName(const EmaString& proxyHostName)
{
	if (proxyHostName.length())
		_proxyHostnameSetViaFunctionCall = proxyHostName;
	else
		_proxyHostnameSetViaFunctionCall = "";
}

void EmaConfigImpl::proxyPort(const EmaString& proxyPort)
{
	if (proxyPort.length())
		_proxyPortSetViaFunctionCall = proxyPort;
	else
		_proxyPortSetViaFunctionCall = "";
}

void EmaConfigImpl::securityProtocol(int securityProtocol)
{
	_securityProtocolSetViaFunctionCall = securityProtocol;
}

void EmaConfigImpl::proxyUserName(const EmaString& proxyUserName)
{
	if (proxyUserName.length())
		_proxyUserNameSetViaFunctionCall = proxyUserName;
	else
		_proxyUserNameSetViaFunctionCall = "";
}

void EmaConfigImpl::proxyPasswd(const EmaString& proxyPasswd)
{
	if (proxyPasswd.length())
		_proxyPasswdSetViaFunctionCall = proxyPasswd;
	else
		_proxyPasswdSetViaFunctionCall = "";
}

void EmaConfigImpl::proxyDomain(const EmaString& proxyDomain)
{
	if (proxyDomain.length())
		_proxyDomainSetViaFunctionCall = proxyDomain;
	else
		_proxyDomainSetViaFunctionCall = "";
}

void EmaConfigImpl::sslCAStore(const EmaString& sslCAStore)
{
	if (sslCAStore.length())
		_sslCAStoreSetViaFunctionCall = sslCAStore;
	else
		_sslCAStoreSetViaFunctionCall = "";
}

void EmaConfigImpl::objectName(const EmaString& objectName)
{
	_objectName = objectName;
}

void EmaConfigImpl::libsslName(const EmaString& libsslName)
{
	_libSslName = libsslName;
}

void EmaConfigImpl::libcryptoName(const EmaString& libcryptoName)
{
	_libCryptoName = libcryptoName;
}

void EmaConfigImpl::libcurlName(const EmaString& libcurlName)
{
	_libcurlName = libcurlName;
}

void EmaConfigImpl::addOAuth2Credential(const OAuth2Credential& credential)
{
	OmmOAuth2CredentialImpl* newCredential = new OmmOAuth2CredentialImpl((OAuth2Credential&)credential);

	_oAuth2Credentials.push_back(newCredential);
}

void EmaConfigImpl::addOAuth2Credential(const OAuth2Credential& credential, const OmmOAuth2ConsumerClient& client)
{
	OmmOAuth2CredentialImpl* newCredential = new OmmOAuth2CredentialImpl((OAuth2Credential&)credential, (OmmOAuth2ConsumerClient&)client);

	_oAuth2Credentials.push_back(newCredential);
}

void EmaConfigImpl::addOAuth2Credential(const OAuth2Credential& credential, const OmmOAuth2ConsumerClient& client, void* closure)
{
	OmmOAuth2CredentialImpl* newCredential = new OmmOAuth2CredentialImpl((OAuth2Credential&)credential, (OmmOAuth2ConsumerClient&)client, closure);

	_oAuth2Credentials.push_back(newCredential);
}

void EmaConfigImpl::addLoginMsgCredential(const ReqMsg& reqMsg, const EmaString& channelList)
{
	LoginRdmReqMsgImpl* newCredential = new LoginRdmReqMsgImpl();

	newCredential->set(this, static_cast<const ReqMsgEncoder&>(reqMsg.getEncoder()).getRsslRequestMsg());
	newCredential->setChannelList(channelList);

	_LoginRequestMsgs.push_back(newCredential);
}


void EmaConfigImpl::addLoginMsgCredential(const ReqMsg& reqMsg, const EmaString& channelList, const OmmLoginCredentialConsumerClient& client)
{
	LoginRdmReqMsgImpl* newCredential = new LoginRdmReqMsgImpl(const_cast<OmmLoginCredentialConsumerClient&>(client));

	newCredential->set(this, static_cast<const ReqMsgEncoder&>(reqMsg.getEncoder()).getRsslRequestMsg());
	newCredential->setChannelList(channelList);

	_LoginRequestMsgs.push_back(newCredential);
}

void EmaConfigImpl::addLoginMsgCredential(const ReqMsg& reqMsg, const EmaString& channelList, const OmmLoginCredentialConsumerClient& client, void* closure)
{
	LoginRdmReqMsgImpl* newCredential = new LoginRdmReqMsgImpl(const_cast<OmmLoginCredentialConsumerClient&>(client), closure);

	newCredential->set(this, static_cast<const ReqMsgEncoder&>(reqMsg.getEncoder()).getRsslRequestMsg());
	newCredential->setChannelList(channelList);

	_LoginRequestMsgs.push_back(newCredential);
}

EmaConfigServerImpl::EmaConfigServerImpl( const EmaString & path ) :
	EmaConfigBaseImpl( path ),
	_portSetViaFunctionCall(),
	_pDirectoryRsslRefreshMsg(0)
{
}

EmaConfigServerImpl::~EmaConfigServerImpl()
{
}

void EmaConfigServerImpl::clear()
{
	EmaConfigBaseImpl::clear();
	_portSetViaFunctionCall.userSet = false;
	_portSetViaFunctionCall.userSpecifiedValue.clear();

	if (_pDirectoryRsslRefreshMsg)
		_pDirectoryRsslRefreshMsg->clear();

	if (_pProgrammaticConfigure)
		_pProgrammaticConfigure->clear();

	_libCryptoName.clear();
	_libSslName.clear();
	_libCurlName.clear();
	_serverCert.clear();
	_serverPrivateKey.clear();
	_dhParams.clear();
	_cipherSuite.clear();
}

void EmaConfigServerImpl::addAdminMsg( const RefreshMsg& refreshMsg )
{
	RsslRefreshMsg* pRsslRefreshMsg = static_cast<const RefreshMsgEncoder&>(refreshMsg.getEncoder()).getRsslRefreshMsg();

	switch ( pRsslRefreshMsg->msgBase.domainType )
	{
		case RSSL_DMT_LOGIN:
			break;
		case RSSL_DMT_DICTIONARY:
		{
			// Todo: add code to handle to store dictionary message
		}
		break;
		case RSSL_DMT_SOURCE:
		{
			if (pRsslRefreshMsg->msgBase.streamId > 0)
			{
				EmaString temp("Refresh passed into addAdminMsg( const RefreshMsg& ) contains unhandled stream id. StreamId='");
				temp.append(pRsslRefreshMsg->msgBase.streamId).append("'. ");
				EmaConfigError* mce(new EmaConfigError(temp, OmmLoggerClient::ErrorEnum));
				_pEmaConfig->errors().add(mce);
				return;
			}

			if (pRsslRefreshMsg->msgBase.containerType != RSSL_DT_MAP)
			{
				EmaString temp("RefreshMsg with SourceDirectory passed into addAdminMsg( const RefreshMsg& ) contains a container with wrong data type. Expected container data type is Map. Passed in is ");
				temp += DataType((DataType::DataTypeEnum)pRsslRefreshMsg->msgBase.containerType).toString();
				EmaConfigError* mce(new EmaConfigError(temp, OmmLoggerClient::ErrorEnum));
				_pEmaConfig->errors().add(mce);
				return;
			}

			addDirectoryRefreshMsg(pRsslRefreshMsg);
		}
		break;
		default:
		{
			EmaString temp("Refresh message passed into addAdminMsg( const RefreshMsg& ) contains unhandled domain type. Domain type='");
			temp.append(pRsslRefreshMsg->msgBase.domainType).append("'. ");
			EmaConfigError* mce(new EmaConfigError(temp, OmmLoggerClient::ErrorEnum));
			_pEmaConfig->errors().add(mce);
		}
		break;
	}
}

void EmaConfigServerImpl::port(const EmaString& port)
{
	_portSetViaFunctionCall.userSet = true;
	_portSetViaFunctionCall.userSpecifiedValue = port;
}

const PortSetViaFunctionCall& EmaConfigServerImpl::getUserSpecifiedPort() const
{
	return _portSetViaFunctionCall;
}

void EmaConfigServerImpl::getServerName( const EmaString& instanceName, EmaString& retVal) const
{
	if (_pProgrammaticConfigure && _pProgrammaticConfigure->getActiveServerName(instanceName, retVal))
		return;
	
	EmaString nodeName(_instanceNodeName);
	nodeName.append(instanceName);
	nodeName.append("|Server");

	get<EmaString>(nodeName, retVal);
}

bool EmaConfigServerImpl::getDirectoryName(const EmaString& instanceName, EmaString& retVal) const
{
	if (!_pProgrammaticConfigure || !_pProgrammaticConfigure->getActiveDirectoryName(instanceName, retVal))
	{
		EmaString nodeName(_instanceNodeName);
		nodeName.append(instanceName);
		nodeName.append("|Directory");

		get<EmaString>(nodeName, retVal);
	}

	return true;
}

void EmaConfigServerImpl::addDirectoryRefreshMsg(RsslRefreshMsg* pRsslRefreshMsg)
{
	if (!_pDirectoryRsslRefreshMsg)
		_pDirectoryRsslRefreshMsg = new AdminRefreshMsg(this);

	_pDirectoryRsslRefreshMsg->set(pRsslRefreshMsg);
}

AdminRefreshMsg* EmaConfigServerImpl::getDirectoryRefreshMsg()
{
	AdminRefreshMsg* pTemp = _pDirectoryRsslRefreshMsg;
	_pDirectoryRsslRefreshMsg = 0;
	return pTemp;
}

void EmaConfigServerImpl::libsslName(const EmaString& libsslName)
{
	_libSslName = libsslName;
}

void EmaConfigServerImpl::libcryptoName(const EmaString& libcryptoName)
{
	_libCryptoName = libcryptoName;
}

void EmaConfigServerImpl::libcurlName(const EmaString& libcurlName)
{
	_libCurlName = libcurlName;
}

void EmaConfigServerImpl::serverCert(const EmaString& serverCert)
{
	_serverCert = serverCert;
}

void EmaConfigServerImpl::serverPrivateKey(const EmaString& serverPrivateKey)
{
	_serverPrivateKey = serverPrivateKey;
}

void EmaConfigServerImpl::cipherSuite(const EmaString& cipherSuite)
{
	_cipherSuite = cipherSuite;
}

void EmaConfigServerImpl::dhParams(const EmaString& dhParams)
{
	_dhParams = dhParams;
}

void XMLnode::print( int tabs )
{
	printf( "%s (level %d, this %p, parent %p)\n", _name.c_str(), _level, this, _parent );
	fflush( stdout );
	++tabs;
	_attributes->print( tabs );
	_children->print( tabs );
	--tabs;
}

void XMLnode::appendErrorMessage( const EmaString& errorMsg, OmmLoggerClient::Severity severity )
{
	if ( _parent )
		_parent->appendErrorMessage( errorMsg, severity );
	else
	{
		EmaConfigError* mce( new EmaConfigError( errorMsg, severity ) );
		_errors.add( mce );
	}
}

void XMLnode::verifyDefaultConsumer()
{
	const EmaString* defaultName( find< EmaString >( EmaString( "ConsumerGroup|DefaultConsumer" ) ) );
	if ( defaultName )
	{
		XMLnode* consumerList( find< XMLnode >( "ConsumerGroup|ConsumerList" ) );
		if ( consumerList )
		{
			EmaList< NameString* > theNames;
			consumerList->getNames( theNames );

			if ( theNames.empty() && *defaultName == "EmaConsumer" )
				return;

			NameString* name( theNames.pop_front() );
			bool foundDefaultName = false;
			while ( name )
			{
				if ( *name == *defaultName )
				{
					foundDefaultName = true;
				}

				delete name;
				name = theNames.pop_front();
			}

		    if (foundDefaultName)
				return;	

			EmaString errorMsg( "specified default consumer name [" );
			errorMsg.append( *defaultName ).append( "] was not found in the configured consumers" );
			throwIceException( errorMsg );
		}
		else if ( *defaultName != "EmaConsumer" )
		{
			EmaString errorMsg( "default consumer name [" );
			errorMsg.append( *defaultName ).append( "] was specified, but no consumers with this name were configured" );
			throwIceException( errorMsg );
		}
	}
}

void XMLnode::verifyDefaultNiProvider()
{
	const EmaString* defaultName( find< EmaString >( EmaString( "NiProviderGroup|DefaultNiProvider" ) ) );
	if ( defaultName )
	{
		XMLnode* niProviderList( find< XMLnode >( "NiProviderGroup|NiProviderList" ) );
		if ( niProviderList )
		{
			EmaList< NameString* > theNames;
			niProviderList->getNames( theNames );

			if ( theNames.empty() && *defaultName == "EmaNiProvider" )
				return;

            NameString* name( theNames.pop_front() );
            bool foundDefaultName = false;
            while ( name )
            {
                if ( *name == *defaultName )
                {
                    foundDefaultName = true;
                }

                delete name;
                name = theNames.pop_front();
            }

            if (foundDefaultName)
                return;

			EmaString errorMsg( "specified default ni provider name [" );
			errorMsg.append( *defaultName ).append( "] was not found in the configured ni providers" );
			throwIceException( errorMsg );
		}
		else if ( *defaultName != "EmaNiProvider" )
		{
			EmaString errorMsg( "default ni provider name [" );
			errorMsg.append( *defaultName ).append( "] was specified, but no ni providers with this name were configured" );
			throwIceException( errorMsg );
		}
	}
}

void XMLnode::verifyDefaultDirectory()
{
	const EmaString* defaultName( find< EmaString >( EmaString( "DirectoryGroup|DefaultDirectory" ) ) );
	if ( defaultName )
	{
		XMLnode* niProviderList( find< XMLnode >( "DirectoryGroup|DirectoryList" ) );
		if ( niProviderList )
		{
			EmaList< NameString* > theNames;
			niProviderList->getNames( theNames );

			if ( theNames.empty() && *defaultName == "EmaDirectory" )
				return;

            NameString* name( theNames.pop_front() );
            bool foundDefaultName = false;
            while ( name )
            {
                if ( *name == *defaultName )
                {
                    foundDefaultName = true;
                }

                delete name;
                name = theNames.pop_front();
            }

            if (foundDefaultName)
                return;

			EmaString errorMsg( "specified default directory name [" );
			errorMsg.append( *defaultName ).append( "] was not found in the configured directories" );
			throwIceException( errorMsg );
		}
		else if ( *defaultName != "EmaDirectory" )
		{
			EmaString errorMsg( "default ni directory name [" );
			errorMsg.append( *defaultName ).append( "] was specified, but no directories with this name were configured" );
			throwIceException( errorMsg );
		}
	}
}

void XMLnode::getServiceNameList( const EmaString& directoryName, EmaVector< EmaString >& serviceNames )
{
	serviceNames.clear();
	XMLnode* serviceList( find< XMLnode >( EmaString( "DirectoryGroup|DirectoryList|Directory." ) + directoryName ) );
	if ( serviceList )
	{
		EmaList< NameString* > theNames;
		serviceList->getNames( theNames );

		NameString* pTempName = theNames.pop_back();

		while ( pTempName )
		{
			serviceNames.push_back( *pTempName );
			delete pTempName;
			pTempName = theNames.pop_back();
		}
	}
}

void XMLnode::getAsciiAttributeValueList( const EmaString& nodeName, const EmaString& attributeName, EmaVector< EmaString >& valueList )
{
	valueList.clear();

	XMLnode* pNode = find< XMLnode >( nodeName );
	if ( pNode )
		pNode->getValues( attributeName, valueList );
}

void XMLnode::getEntryNodeList( const EmaString& nodeName, const EmaString& entryName, EmaVector< XMLnode* >& entryNodeList )
{
	entryNodeList.clear();

	XMLnode* pNode = find< XMLnode >( nodeName );

	if ( !pNode ) return;

	xmlList< XMLnode >* pChildren = pNode->getChildren();

	if ( !pChildren ) return;

	XMLnode* pChild = pChildren->getFirst();

	while ( pChild )
	{
		if ( entryName.empty() || pChild->name() == entryName )
			entryNodeList.push_back( pChild );

		pChild = pChildren->getNext( pChild );
	}
}

bool XMLnode::addChild( XMLnode* child )
{
	for ( int i = 0; i < sizeof NodesThatRequireName / sizeof NodesThatRequireName[0]; ++i )
		if ( child->_name == NodesThatRequireName[i] )
		{

			const ConfigElement* tmp( child->_attributes->first() );
			while ( tmp )
			{
				if ( tmp->name() == "Name" )
					return _children->insert( child, true );
				tmp = child->_attributes->next( tmp );
			}

			EmaString errorMsg( "cannot add node with name [" );
			errorMsg += child->_name
				+ "] because node is missing \"name\" attribute; node will be ignored";
			appendErrorMessage( errorMsg, OmmLoggerClient::WarningEnum );
			return false;
		}

	return _children->insert( child, false );
}

EmaConfigError::EmaConfigError( const EmaString& errorMsg, OmmLoggerClient::Severity severity ) :
	_errorMsg( errorMsg ),
	_severity( severity )
{
}

EmaConfigError::~EmaConfigError()
{
}

OmmLoggerClient::Severity EmaConfigError::severity() const
{
	return _severity;
}

const EmaString& EmaConfigError::errorMsg() const
{
	return _errorMsg;
}

void EmaConfigErrorList::add( EmaConfigError* mce )
{
	ListElement* le( new ListElement( mce ) );
	if ( _pList )
	{
		ListElement* p;
		for ( p = _pList; p; p = p->next )
			if ( !p->next )
				break;
		p->next = le;
	}
	else
		_pList = le;
	++_count;
}

EmaConfigErrorList::EmaConfigErrorList() :
	_pList( 0 ),
	_count( 0 )
{
}

EmaConfigErrorList::~EmaConfigErrorList()
{
	clear();
}

UInt32 EmaConfigErrorList::count() const
{
	return _count;
}

void EmaConfigErrorList::add( EmaConfigErrorList& eL )
{
	if ( eL._count )
	{
		if ( _pList )
		{
			ListElement* p = _pList;
			while ( p->next )
				p = p->next;
			p->next = eL._pList;
		}
		else
			_pList = eL._pList;
		_count += eL.count();
	}
};

void EmaConfigErrorList::clear()
{
	if ( _pList )
	{
		ListElement* q;
		for ( ListElement* p = _pList; p; p = q )
		{
			q = p->next;
			delete( p->error );
			delete( p );
		}
		_pList = 0;

	}
	_count = 0;
}

void EmaConfigErrorList::printErrors( OmmLoggerClient::Severity severity )
{
	bool printed( false );
	if ( _pList )
	{
		for ( ListElement* p = _pList; p; p = p->next )
			if ( p->error->severity() >= severity )
			{
				if ( !printed )
				{
					printf( "begin configuration errors:\n" );
					printed = true;
				}
				printf( "\t[%s] %s\n", OmmLoggerClient::loggerSeverityString( p->error->severity() ), p->error->errorMsg().c_str() );
			}
		if ( printed )
			printf( "end configuration errors\n" );
		else
			printf( "no configuration errors existed with level equal to or exceeding %s\n", OmmLoggerClient::loggerSeverityString( severity ) );
	}
	else
		printf( "no configuration errors found\n" );

}

void EmaConfigErrorList::log( OmmLoggerClient* logger, OmmLoggerClient::Severity severity )
{
	for ( ListElement* p = _pList; p; p = p->next )
		if ( p->error->severity() >= severity )
			logger->log( "EmaConfig", p->error->severity(), p->error->errorMsg().c_str() );
}


namespace refinitiv {

namespace ema {

namespace access {

template<>
void XMLConfigElement<Double>::print() const
{
	printf("%s (parent %p)", _name.c_str(), _parent);
	printf(": %f", _values[0]);
	for (unsigned int i = 1; i < _values.size(); ++i)
		printf(", %f", _values[i]);
}

template<>
void XMLConfigElement<UInt64>::print() const
{
	printf("%s (parent %p)", _name.c_str(), _parent);
	printf(": %llu", _values[0]);
	for (unsigned int i = 1; i < _values.size(); ++i)
		printf(", %llu", _values[i]);
}

template<>
void XMLConfigElement<Int64>::print() const
{
	printf("%s (parent %p)", _name.c_str(), _parent);
	printf(": %lld", _values[0]);
	for (unsigned int i = 1; i < _values.size(); ++i)
		printf(", %lld", _values[i]);
}

template<>
void XMLConfigElement< EmaString >::print() const
{
	printf( "%s (parent %p)", _name.c_str(), _parent );
	printf( ": \"%s\"", _values[0].c_str() );
	for ( unsigned int i = 1; i < _values.size(); ++i )
		printf( ", \"%s\"", _values[i].c_str() );
}

template<>
void XMLConfigElement< bool >::print() const
{
	printf( "%s (parent %p)", _name.c_str(), _parent );
	printf( _values[0] == true ? ": true" : ": false" );
	for ( unsigned int i = 0; i < _values.size(); ++i )
		printf( _values[i] == true ? ", true" : ", false" );
}

template<>
void XMLConfigElement<OmmLoggerClient::Severity>::print() const
{
	printf("%s (parent %p)", _name.c_str(), _parent);
	printf(": %d", _values[0]);
	for (unsigned int i = 1; i < _values.size(); ++i)
		printf(", %d", _values[i]);
}

template<>
void XMLConfigElement<OmmLoggerClient::LoggerType>::print() const
{
	printf("%s (parent %p)", _name.c_str(), _parent);
	printf(": %d", _values[0]);
	for (unsigned int i = 1; i < _values.size(); ++i)
		printf(", %d", _values[i]);
}

template<>
void XMLConfigElement<Dictionary::DictionaryType>::print() const
{
	printf("%s (parent %p)", _name.c_str(), _parent);
	printf(": %d", _values[0]);
	for (unsigned int i = 1; i < _values.size(); ++i)
		printf(", %d", _values[i]);
}

template<>
void XMLConfigElement<RsslConnectionTypes>::print() const
{
	printf("%s (parent %p)", _name.c_str(), _parent);
	printf(": %d", _values[0]);
	for (unsigned int i = 1; i < _values.size(); ++i)
		printf(", %d", _values[i]);
}

template<>
void XMLConfigElement<RsslCompTypes>::print() const
{
	printf("%s (parent %p)", _name.c_str(), _parent);
	printf(": %d", _values[0]);
	for (unsigned int i = 1; i < _values.size(); ++i)
		printf(", %d", _values[i]);
}

template<>
void XMLConfigElement<OmmState::StreamState>::print() const
{
	printf("%s (parent %p)", _name.c_str(), _parent);
	printf(": %d", _values[0]);
	for (unsigned int i = 1; i < _values.size(); ++i)
		printf(", %d", _values[i]);
}

template<>
void XMLConfigElement<OmmState::DataState>::print() const
{
	printf("%s (parent %p)", _name.c_str(), _parent);
	printf(": %d", _values[0]);
	for (unsigned int i = 1; i < _values.size(); ++i)
		printf(", %d", _values[i]);
}

template<>
void XMLConfigElement<OmmState::StatusCode>::print() const
{
	printf("%s (parent %p)", _name.c_str(), _parent);
	printf(": %d", _values[0]);
	for (unsigned int i = 1; i < _values.size(); ++i)
		printf(", %d", _values[i]);
}

template<>
void XMLConfigElement<RsslReactorWarmStandbyMode>::print() const
{
	printf("%s (parent %p)", _name.c_str(), _parent);
	printf(": %d", _values[0]);
	for (unsigned int i = 1; i < _values.size(); ++i)
		printf(", %d", _values[i]);
}

template<>
bool XMLConfigElement<Int64>::operator== ( const XMLConfigElement<Int64>& rhs ) const
{
	for ( unsigned int i = 0; i < _values.size(); ++i )
		if ( _values[0] != rhs._values[0] )
			return false;
	return true;
}

template<>
bool XMLConfigElement<EmaString>::operator== ( const XMLConfigElement<EmaString>& rhs ) const
{
	for ( unsigned int i = 0; i < _values.size(); ++i )
		if ( _values[0] != rhs._values[0] )
			return false;
	return true;
}

}

}

}

bool ConfigElement::operator== ( const ConfigElement& rhs ) const
{
	if ( _name == rhs._name && type() == rhs.type() )
		switch ( type() )
		{
		case ConfigElementTypeInt64:
		{
			const XMLConfigElement<Int64>& l = dynamic_cast<const XMLConfigElement<Int64> &>( *this );
			const XMLConfigElement<Int64>& r = dynamic_cast<const XMLConfigElement<Int64> &>( rhs );
			return l == r ? true : false;
		}
		case ConfigElementTypeAscii:
		{
			const XMLConfigElement<EmaString>& l = dynamic_cast<const XMLConfigElement<EmaString> &>( *this );
			const XMLConfigElement<EmaString>& r = dynamic_cast<const XMLConfigElement<EmaString> &>( rhs );
			return l == r ? true : false;
		}
		case ConfigElementTypeEnum:
		{
			break;
		}
		}

	return false;
}

void ConfigElement::appendErrorMessage( EmaString& errorMsg, OmmLoggerClient::Severity severity )
{
	_parent->appendErrorMessage( errorMsg, severity );
}

namespace refinitiv
{

namespace ema
{

namespace access
{

template< typename T >
EmaString
XMLConfigElement< T >::changeMessage( const EmaString& actualName, const XMLConfigElement< T >& newElement ) const
{
	EmaString msg( "value for element [" );
	if ( actualName.empty() )
		msg.append( newElement.name() );
	else
		msg.append( actualName ).append( "|" ).append( newElement.name() );
	msg.append( "] changing from [" ).append( *( const_cast<XMLConfigElement<T> *>( this )->value() ) ).append( "] to [" )
		.append( *( const_cast<XMLConfigElement<T> &>( newElement ).value() ) ).append( "]" );
	return msg;
}

}

}

}

EmaString ConfigElement::changeMessage( const EmaString& actualName, const ConfigElement& newElement ) const
{
	switch ( type() )
	{
	case ConfigElementTypeAscii:
	{
		const XMLConfigElement<EmaString>& l = dynamic_cast<const XMLConfigElement<EmaString> &>( *this );
		const XMLConfigElement<EmaString>& r = dynamic_cast<const XMLConfigElement<EmaString> &>( newElement );
		EmaString retVal( l.changeMessage( actualName, r ) );
		return retVal;
	}
	case ConfigElementTypeInt64:
	{
		const XMLConfigElement< Int64 >& l = dynamic_cast<const XMLConfigElement< Int64 > &>( *this );
		const XMLConfigElement< Int64 >& r = dynamic_cast<const XMLConfigElement< Int64 > &>( newElement );
		EmaString retVal( l.changeMessage( actualName, r ) );
		return retVal;
	}
	case ConfigElementTypeUInt64:
	{
		const XMLConfigElement< UInt64 >& l = dynamic_cast<const XMLConfigElement< UInt64 > &>( *this );
		const XMLConfigElement< UInt64 >& r = dynamic_cast<const XMLConfigElement< UInt64 > &>( newElement );
		EmaString retVal( l.changeMessage( actualName, r ) );
		return retVal;
	}
	default:
	{
		EmaString defaultMsg( "element [" );
		defaultMsg.append( newElement.name() ).append( "] change; no information on exact change in values" );
		return defaultMsg;
	}
	}
}

AdminReqMsg::AdminReqMsg( EmaConfigImpl& configImpl ) :
	_emaConfigImpl( configImpl ),
	_hasServiceName( false ),
	_serviceName()
{
	rsslClearRequestMsg( &_rsslMsg );
	rsslClearBuffer( &_name );
	rsslClearBuffer( &_header );
	rsslClearBuffer( &_attrib );
	rsslClearBuffer( &_payload );
}

AdminReqMsg::~AdminReqMsg()
{
	if ( _payload.data )
		free( _payload.data );

	if ( _attrib.data )
		free( _attrib.data );

	if ( _header.data )
		free( _header.data );

	if ( _name.data )
		free( _name.data );
}

AdminReqMsg& AdminReqMsg::set( RsslRequestMsg* pRsslRequestMsg )
{
	_rsslMsg = *pRsslRequestMsg;

	if ( _rsslMsg.flags & RSSL_RQMF_HAS_EXTENDED_HEADER )
	{
		if ( _rsslMsg.extendedHeader.length > _header.length )
		{
			if ( _header.data ) free( _header.data );
			_header.data = 0;

			_header.data = (char*) malloc( _rsslMsg.extendedHeader.length );
			if (!_header.data)
			{
				throwMeeException("Failed to allocate memory for encoded Admin Request extended header in AdminReqMsg::set()");			
				return *this;
			}
			_header.length = _rsslMsg.extendedHeader.length;
		}

		memcpy( _header.data, _rsslMsg.extendedHeader.data, _header.length );

		_rsslMsg.extendedHeader = _header;
	}
	else
	{
		rsslClearBuffer( &_rsslMsg.extendedHeader );
	}

	if ( _rsslMsg.msgBase.containerType != RSSL_DT_NO_DATA )
	{
		if ( _rsslMsg.msgBase.encDataBody.length > _payload.length )
		{
			if ( _payload.data ) free( _payload.data );
			_payload.data = 0;

			_payload.data = (char*) malloc( _rsslMsg.msgBase.encDataBody.length );
			if (!_payload.data)
			{
				throwMeeException("Failed to allocate memory for encoded Admin Request payload in AdminReqMsg::set()");
				return *this;
			}
			_payload.length = _rsslMsg.msgBase.encDataBody.length;
		}

		memcpy( _payload.data, _rsslMsg.msgBase.encDataBody.data, _payload.length );

		_rsslMsg.msgBase.encDataBody = _payload;
	}
	else
	{
		rsslClearBuffer( &_rsslMsg.msgBase.encDataBody );
	}

	if ( _rsslMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB )
	{
		if ( _rsslMsg.msgBase.msgKey.encAttrib.length > _attrib.length )
		{
			if ( _attrib.data ) free( _attrib.data );
			_attrib.data = 0;

			_attrib.data = (char*) malloc( _rsslMsg.msgBase.msgKey.encAttrib.length );

			if (!_attrib.data)
			{
				throwMeeException("Failed to allocate memory for encoded Admin Request msgKey.attrib in AdminReqMsg::set()");
				return *this;
			}
			_attrib.length = _rsslMsg.msgBase.msgKey.encAttrib.length;
		}

		memcpy( _attrib.data, _rsslMsg.msgBase.msgKey.encAttrib.data, _attrib.length );

		_rsslMsg.msgBase.msgKey.encAttrib = _attrib;
	}
	else
	{
		rsslClearBuffer( &_rsslMsg.msgBase.msgKey.encAttrib );
	}

	if ( _rsslMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_NAME )
	{
		if ( _rsslMsg.msgBase.msgKey.name.length > _name.length )
		{
			if ( _name.data ) free( _name.data );
			_name.data = 0;

			_name.data = (char*) malloc( _rsslMsg.msgBase.msgKey.name.length );
			
			if (!_name.data)
			{
				throwMeeException("Failed to allocate memory for encoded Admin Request msgKey.name in AdminReqMsg::set()");
				return *this;
			}

			_name.length = _rsslMsg.msgBase.msgKey.name.length;
		}

		memcpy( _name.data, _rsslMsg.msgBase.msgKey.name.data, _name.length );

		_rsslMsg.msgBase.msgKey.name = _name;
	}
	else
	{
		rsslClearBuffer( &_rsslMsg.msgBase.msgKey.name );
	}

	return *this;
}

AdminReqMsg& AdminReqMsg::clear()
{
	rsslClearRequestMsg( &_rsslMsg );
	_hasServiceName = false;

	return *this;
}

RsslRequestMsg* AdminReqMsg::get()
{
	return &_rsslMsg;
}

bool AdminReqMsg::hasServiceName()
{
	return _hasServiceName;
}

void AdminReqMsg::setServiceName( const EmaString& serviceName )
{
	_serviceName = serviceName;
	_hasServiceName = true;
}

const EmaString& AdminReqMsg::getServiceName()
{
	return _serviceName;
}

AdminRefreshMsg::AdminRefreshMsg( EmaConfigBaseImpl* pConfigImpl ) :
	_pEmaConfigImpl( pConfigImpl )
{
	rsslClearRefreshMsg( &_rsslMsg );
	rsslClearBuffer( &_name );
	rsslClearBuffer( &_header );
	rsslClearBuffer( &_attrib );
	rsslClearBuffer( &_payload );
	rsslClearBuffer( &_statusText );
}

AdminRefreshMsg::AdminRefreshMsg( const AdminRefreshMsg& other ) :
	_pEmaConfigImpl( 0 )
{
	rsslClearRefreshMsg( &_rsslMsg );
	rsslClearBuffer( &_name );
	rsslClearBuffer( &_header );
	rsslClearBuffer( &_attrib );
	rsslClearBuffer( &_payload );
	rsslClearBuffer( &_statusText );

	_rsslMsg = other._rsslMsg;

	if ( _rsslMsg.flags & RSSL_RQMF_HAS_EXTENDED_HEADER )
	{
		if ( _header.data ) free( _header.data );
		_header.data = 0;

		_header.data = (char*) malloc( _rsslMsg.extendedHeader.length );
		if (!_header.data)
		{
			throwMeeException("Failed to allocate memory for encoded Admin Refresh Extended Header in AdminRefreshMsg::AdminRefreshMsg()");
		}
		_header.length = _rsslMsg.extendedHeader.length;

		memcpy( _header.data, _rsslMsg.extendedHeader.data, _header.length );

		_rsslMsg.extendedHeader = _header;
	}
	else
	{
		rsslClearBuffer( &_rsslMsg.extendedHeader );
	}

	if ( _rsslMsg.msgBase.containerType != RSSL_DT_NO_DATA )
	{
		if ( _payload.data ) free( _payload.data );
		_payload.data = 0;

		_payload.data = (char*) malloc( _rsslMsg.msgBase.encDataBody.length );
		if (!_payload.data)
		{
			throwMeeException("Failed to allocate memory for encoded Admin Refresh Payload in AdminRefreshMsg::AdminRefreshMsg()");
		}
		_payload.length = _rsslMsg.msgBase.encDataBody.length;

		memcpy( _payload.data, _rsslMsg.msgBase.encDataBody.data, _payload.length );

		_rsslMsg.msgBase.encDataBody = _payload;
	}
	else
	{
		rsslClearBuffer( &_rsslMsg.msgBase.encDataBody );
	}

	if ( _rsslMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB )
	{
		if ( _attrib.data ) free( _attrib.data );
		_attrib.data = 0;

		_attrib.data = (char*) malloc( _rsslMsg.msgBase.msgKey.encAttrib.length );
		if (!_attrib.data)
		{
			throwMeeException("Failed to allocate memory for encoded Admin Refresh msgKey.attrib in AdminRefreshMsg::AdminRefreshMsg()");
		}
		_attrib.length = _rsslMsg.msgBase.msgKey.encAttrib.length;

		memcpy( _attrib.data, _rsslMsg.msgBase.msgKey.encAttrib.data, _attrib.length );

		_rsslMsg.msgBase.msgKey.encAttrib = _attrib;
	}
	else
	{
		rsslClearBuffer( &_rsslMsg.msgBase.msgKey.encAttrib );
	}

	if ( _rsslMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_NAME )
	{
		if ( _name.data ) free( _name.data );
		_name.data = 0; 

		_name.data = (char*) malloc( _rsslMsg.msgBase.msgKey.name.length );
		if (!_name.data)
		{
			throwMeeException("Failed to allocate memory for encoded Admin Refresh msgKey.name in AdminRefreshMsg::AdminRefreshMsg()");
		}
		_name.length = _rsslMsg.msgBase.msgKey.name.length;

		memcpy( _name.data, _rsslMsg.msgBase.msgKey.name.data, _name.length );

		_rsslMsg.msgBase.msgKey.name = _name;
	}
	else
	{
		rsslClearBuffer( &_rsslMsg.msgBase.msgKey.name );
	}

	if ( _rsslMsg.state.text.data )
	{
		if ( _statusText.data ) free( _statusText.data );
		_statusText.data = 0;

		_statusText.data = (char*) malloc( _rsslMsg.state.text.length );
		if (!_statusText.data)
		{
			throwMeeException("Failed to allocate memory for encoded Admin Refresh state text in AdminRefreshMsg::AdminRefreshMsg()");
		}
		_statusText.length = _rsslMsg.state.text.length;

		memcpy( _statusText.data, _rsslMsg.state.text.data, _statusText.length );

		_rsslMsg.state.text = _statusText;
	}
	else
	{
		rsslClearBuffer( &_rsslMsg.state.text );
	}
}

AdminRefreshMsg& AdminRefreshMsg::operator=( const AdminRefreshMsg& other )
{
	_rsslMsg = other._rsslMsg;

	if ( _rsslMsg.flags & RSSL_RQMF_HAS_EXTENDED_HEADER )
	{
		if ( _rsslMsg.extendedHeader.length > _header.length )
		{
			if ( _header.data ) free( _header.data );
			_header.data = 0;

			_header.data = (char*) malloc( _rsslMsg.extendedHeader.length );
			if (!_header.data)
			{
				throwMeeException("Failed to allocate memory for encoded Admin Refresh extended header in AdminRefreshMsg::operator=");
				return *this;
			}
			_header.length = _rsslMsg.extendedHeader.length;
		}

		memcpy( _header.data, _rsslMsg.extendedHeader.data, _header.length );

		_rsslMsg.extendedHeader = _header;
	}
	else
	{
		rsslClearBuffer( &_rsslMsg.extendedHeader );
	}

	if ( _rsslMsg.msgBase.containerType != RSSL_DT_NO_DATA )
	{
		if ( _rsslMsg.msgBase.encDataBody.length > _payload.length )
		{
			if ( _payload.data ) free( _payload.data );
			_payload.data = 0;

			_payload.data = (char*) malloc( _rsslMsg.msgBase.encDataBody.length );
			if (!_payload.data)
			{
				throwMeeException("Failed to allocate memory for encoded Admin Refresh payload in AdminRefreshMsg::operator=");
				return *this;
			}
			_payload.length = _rsslMsg.msgBase.encDataBody.length;
		}

		memcpy( _payload.data, _rsslMsg.msgBase.encDataBody.data, _payload.length );

		_rsslMsg.msgBase.encDataBody = _payload;
	}
	else
	{
		rsslClearBuffer( &_rsslMsg.msgBase.encDataBody );
	}

	if ( _rsslMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB )
	{
		if ( _rsslMsg.msgBase.msgKey.encAttrib.length > _attrib.length )
		{
			if ( _attrib.data ) free( _attrib.data );
			_attrib.data = 0;

			_attrib.data = (char*) malloc( _rsslMsg.msgBase.msgKey.encAttrib.length );
			if (!_attrib.data)
			{
				throwMeeException("Failed to allocate memory for encoded Admin Refresh msgKey.attrib in AdminRefreshMsg::operator=");
				return *this;
			}
			_attrib.length = _rsslMsg.msgBase.msgKey.encAttrib.length;
		}

		memcpy( _attrib.data, _rsslMsg.msgBase.msgKey.encAttrib.data, _attrib.length );

		_rsslMsg.msgBase.msgKey.encAttrib = _attrib;
	}
	else
	{
		rsslClearBuffer( &_rsslMsg.msgBase.msgKey.encAttrib );
	}

	if ( _rsslMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_NAME )
	{
		if ( _rsslMsg.msgBase.msgKey.name.length > _name.length )
		{
			if ( _name.data ) free( _name.data );
			_name.data = 0;

			_name.data = (char*) malloc( _rsslMsg.msgBase.msgKey.name.length );
			if (!_name.data)
			{
				throwMeeException("Failed to allocate memory for encoded Admin Refresh name in AdminRefreshMsg::operator=");
				return *this;
			}
			_name.length = _rsslMsg.msgBase.msgKey.name.length;
		}

		memcpy( _name.data, _rsslMsg.msgBase.msgKey.name.data, _name.length );

		_rsslMsg.msgBase.msgKey.name = _name;
	}
	else
	{
		rsslClearBuffer( &_rsslMsg.msgBase.msgKey.name );
	}

	if ( _rsslMsg.state.text.data )
	{
		if ( _rsslMsg.state.text.length > _statusText.length )
		{
			if ( _statusText.data ) free( _statusText.data );
			_statusText.data = 0;

			_statusText.data = (char*) malloc( _rsslMsg.state.text.length );
			if (!_statusText.data)
			{
				throwMeeException("Failed to allocate memory for encoded Admin Refresh status text in AdminRefreshMsg::operator=");
				return *this;
			}
			_statusText.length = _rsslMsg.state.text.length;
		}

		memcpy( _statusText.data, _rsslMsg.state.text.data, _statusText.length );

		_rsslMsg.state.text = _statusText;
	}
	else
	{
		rsslClearBuffer( &_rsslMsg.state.text );
	}

	return *this;
}

AdminRefreshMsg::~AdminRefreshMsg()
{
	if ( _statusText.data )
		free( _statusText.data );

	if ( _payload.data )
		free( _payload.data );

	if ( _attrib.data )
		free( _attrib.data );

	if ( _header.data )
		free( _header.data );

	if ( _name.data )
		free( _name.data );
}

AdminRefreshMsg& AdminRefreshMsg::set( RsslRefreshMsg* pRsslRefreshMsg )
{
	_rsslMsg = *pRsslRefreshMsg;

	if ( _rsslMsg.flags & RSSL_RQMF_HAS_EXTENDED_HEADER )
	{
		if ( _rsslMsg.extendedHeader.length > _header.length )
		{
			if ( _header.data ) free( _header.data );
			_header.data = 0;

			_header.data = (char*) malloc( _rsslMsg.extendedHeader.length );
			if (!_header.data)
			{
				throwMeeException("Failed to allocate memory for encoded Admin Refresh Extended Header in AdminRefreshMsg::set");
				return *this;
			}
			_header.length = _rsslMsg.extendedHeader.length;
		}

		memcpy( _header.data, _rsslMsg.extendedHeader.data, _header.length );

		_rsslMsg.extendedHeader = _header;
	}
	else
	{
		rsslClearBuffer( &_rsslMsg.extendedHeader );
	}

	if ( _rsslMsg.msgBase.containerType != RSSL_DT_NO_DATA )
	{
		if ( _rsslMsg.msgBase.encDataBody.length > _payload.length )
		{
			if ( _payload.data ) free( _payload.data );
			_payload.data = 0;

			_payload.data = (char*) malloc( _rsslMsg.msgBase.encDataBody.length );
			if (!_payload.data)
			{
				throwMeeException("Failed to allocate memory for encoded Admin Refresh Payload in AdminRefreshMsg::set");
				return *this;
			}
			_payload.length = _rsslMsg.msgBase.encDataBody.length;
		}

		memcpy( _payload.data, _rsslMsg.msgBase.encDataBody.data, _payload.length );

		_rsslMsg.msgBase.encDataBody = _payload;
	}
	else
	{
		rsslClearBuffer( &_rsslMsg.msgBase.encDataBody );
	}

	if ( _rsslMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB )
	{
		if ( _rsslMsg.msgBase.msgKey.encAttrib.length > _attrib.length )
		{
			if ( _attrib.data ) free( _attrib.data );
			_attrib.data = 0;

			_attrib.data = (char*) malloc( _rsslMsg.msgBase.msgKey.encAttrib.length );
			if (!_attrib.data)
			{
				throwMeeException("Failed to allocate memory for encoded Admin Refresh msgKey.attrib in AdminRefreshMsg::set");
				return *this;
			}
			_attrib.length = _rsslMsg.msgBase.msgKey.encAttrib.length;
		}

		memcpy( _attrib.data, _rsslMsg.msgBase.msgKey.encAttrib.data, _attrib.length );

		_rsslMsg.msgBase.msgKey.encAttrib = _attrib;
	}
	else
	{
		rsslClearBuffer( &_rsslMsg.msgBase.msgKey.encAttrib );
	}

	if ( _rsslMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_NAME )
	{
		if ( _rsslMsg.msgBase.msgKey.name.length > _name.length )
		{
			if ( _name.data ) free( _name.data );
			_name.data = 0;

			_name.data = (char*) malloc( _rsslMsg.msgBase.msgKey.name.length );
			if (!_name.data)
			{
				throwMeeException("Failed to allocate memory for encoded Admin Refresh msgKey.name in AdminRefreshMsg::set");
				return *this;
			}
			_name.length = _rsslMsg.msgBase.msgKey.name.length;
		}

		memcpy( _name.data, _rsslMsg.msgBase.msgKey.name.data, _name.length );

		_rsslMsg.msgBase.msgKey.name = _name;
	}
	else
	{
		rsslClearBuffer( &_rsslMsg.msgBase.msgKey.name );
	}

	if ( _rsslMsg.state.text.data )
	{
		if ( _rsslMsg.state.text.length > _statusText.length )
		{
			if ( _statusText.data ) free( _statusText.data );
			_statusText.data = 0;

			_statusText.data = (char*) malloc( _rsslMsg.state.text.length );
			if (!_statusText.data)
			{
				throwMeeException("Failed to allocate memory for encoded Admin Refresh status text in AdminRefreshMsg::set");
				return *this;
			}
			_statusText.length = _rsslMsg.state.text.length;
		}

		memcpy( _statusText.data, _rsslMsg.state.text.data, _statusText.length );

		_rsslMsg.state.text = _statusText;
	}
	else
	{
		rsslClearBuffer( &_rsslMsg.state.text );
	}

	return *this;
}

AdminRefreshMsg& AdminRefreshMsg::clear()
{
	rsslClearRefreshMsg( &_rsslMsg );

	return *this;
}

RsslRefreshMsg* AdminRefreshMsg::get()
{
	return &_rsslMsg;
}

