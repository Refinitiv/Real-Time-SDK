/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "DataDictionaryImpl.h"
#include "DictionaryEntryImpl.h"
#include "ExceptionTranslator.h"
#include "OmmLoggerClient.h"
#include "SeriesEncoder.h"
#include "StaticDecoder.h"
#include "Rdm/Include/EmaRdm.h"
#include "Utilities.h"
#include "OmmInvalidUsageException.h"

#include <new>

#define MAX_ERROR_TEXT_SIZE 255
#define DEFAULT_FRAGMENTATION_SIZE 12800
#define DEFAULT_DICTIONARY_ENTRY_SIZE 40
#define DEFAULT_ENUM_TABLE_ENTRY_SIZE 1024
#define DEFAULT_ENCODE_ITERATOR_BUFFER_SIZE 4096

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;

DataDictionaryImpl::DataDictionaryImpl(bool ownRsslDataDictionary) :
	_loadedFieldDictionary(false),
	_loadedEnumTypeDef(false),
	_pDictionaryEntryList(0),
	_pEnumTypeTableList(0),
	_ownRsslDataDictionary(ownRsslDataDictionary),
	_pfieldNameToIdHash(0)
{
	_errorText.length = MAX_ERROR_TEXT_SIZE;
	_errorText.data = (char*)malloc(sizeof(char) * _errorText.length);

	if (!_errorText.data)
	{
		throwMeeException("Failed to allocate memory in DataDictionaryImpl::DataDictionaryImpl()");
	}

	if (_ownRsslDataDictionary)
	{
		try
		{
			_pRsslDataDictionary = new RsslDataDictionary();
		}
		catch (std::bad_alloc)
		{
			throwMeeException("Failed to allocate memory in DataDictionaryImpl::DataDictionaryImpl()");
		}

		rsslClearDataDictionary(_pRsslDataDictionary);
	}
}

DataDictionaryImpl::DataDictionaryImpl(const DataDictionaryImpl& other) :
	_loadedFieldDictionary(false),
	_loadedEnumTypeDef(false),
	_pDictionaryEntryList(0),
	_pEnumTypeTableList(0),
	_ownRsslDataDictionary(true),
	_pfieldNameToIdHash(0)
{
	_errorText.length = MAX_ERROR_TEXT_SIZE;
	_errorText.data = (char*)malloc(sizeof(char) * _errorText.length);

	if ( !_errorText.data )
	{
		throwMeeException("Failed to allocate memory in DataDictionaryImpl::DataDictionaryImpl()");
	}

	if ( _ownRsslDataDictionary )
	{
		try
		{
			_pRsslDataDictionary = new RsslDataDictionary();
		}
		catch (std::bad_alloc)
		{
			throwMeeException("Failed to allocate memory in DataDictionaryImpl::DataDictionaryImpl()");
		}

		rsslClearDataDictionary(_pRsslDataDictionary);
	}

	if (!other._loadedEnumTypeDef && !other._loadedFieldDictionary)
	{
		return;
	}

	Series series;

	if ( other._loadedFieldDictionary )
	{
		const_cast<DataDictionaryImpl&>(other).encodeFieldDictionary(series, DICTIONARY_VERBOSE);

		StaticDecoder::setData(&series, 0);

		decodeFieldDictionary(series, DICTIONARY_VERBOSE);

		series.clear();
	}

	if ( other._loadedEnumTypeDef )
	{
		const_cast<DataDictionaryImpl&>(other).encodeEnumTypeDictionary(series, DICTIONARY_VERBOSE);

		StaticDecoder::setData(&series, 0);

		decodeEnumTypeDictionary(series, DICTIONARY_VERBOSE);
	}
}

void DataDictionaryImpl::setRsslDataDictionary(const RsslDataDictionary* rsslDataDictionary)
{
	if (_pDictionaryEntryList)
	{
		_pDictionaryEntryList->clear();
	}

	if (_pEnumTypeTableList)
	{
		_pEnumTypeTableList->clear();
	}

	if (_pfieldNameToIdHash)
	{
		_pfieldNameToIdHash->clear();
	}

	if (!_ownRsslDataDictionary)
	{
		if (rsslDataDictionary->isInitialized)
		{
			_loadedFieldDictionary = true;
			_loadedEnumTypeDef = true;
		}

		_pRsslDataDictionary = const_cast<RsslDataDictionary*>(rsslDataDictionary);
	}
}

DataDictionaryImpl::~DataDictionaryImpl()
{
	if (_errorText.data)
	{
		free(_errorText.data);
		_errorText.data = 0;
	}

	if ( _pDictionaryEntryList )
	{
		delete _pDictionaryEntryList;
		_pDictionaryEntryList = 0;
	}

	if (_pEnumTypeTableList)
	{
		delete _pEnumTypeTableList;
		_pEnumTypeTableList = 0;
	}

	if (_pfieldNameToIdHash)
	{
		delete _pfieldNameToIdHash;
		_pfieldNameToIdHash = 0;
	}

	if (_ownRsslDataDictionary && _pRsslDataDictionary)
	{
		rsslDeleteDataDictionary(_pRsslDataDictionary);

		delete _pRsslDataDictionary;
		_pRsslDataDictionary = 0;
	}
}

void DataDictionaryImpl::clear()
{
	if (_ownRsslDataDictionary)
	{
		_loadedFieldDictionary = false;
		_loadedEnumTypeDef = false;

		rsslClearDataDictionary(_pRsslDataDictionary);

		if (_pDictionaryEntryList)
		{
			_pDictionaryEntryList->clear();
		}

		if (_pEnumTypeTableList)
		{
			_pEnumTypeTableList->clear();
		}

		if (_pfieldNameToIdHash)
		{
			_pfieldNameToIdHash->clear();
		}
	}
}

Int32 DataDictionaryImpl::getMinFid() const
{
	return _pRsslDataDictionary->minFid;
}

Int32 DataDictionaryImpl::getMaxFid() const
{
	return _pRsslDataDictionary->maxFid;
}

const EmaVector<DictionaryEntry>& DataDictionaryImpl::getEntries() const
{
	if ( !_pDictionaryEntryList )
	{
		_pDictionaryEntryList = new EmaVector<DictionaryEntry>(_pRsslDataDictionary->numberOfEntries);
	}
	else
	{
		if ( _pDictionaryEntryList->size() != _pRsslDataDictionary->numberOfEntries )
		{
			_pDictionaryEntryList->clear();
		}
		else
		{
			return *_pDictionaryEntryList;
		}
	}

	if ( _loadedFieldDictionary )
	{
		RsslDictionaryEntry* rsslDictionaryEntry = 0;
		DictionaryEntry dictionaryEntry;

		for (Int32 index = _pRsslDataDictionary->minFid ; index <= _pRsslDataDictionary->maxFid; index++)
		{
			rsslDictionaryEntry = *(_pRsslDataDictionary->entriesArray + index);

			if (rsslDictionaryEntry)
			{
				dictionaryEntry._pImpl->rsslDictionaryEntry(rsslDictionaryEntry);

				_pDictionaryEntryList->push_back(dictionaryEntry);
			}
		}
	}

	return *_pDictionaryEntryList;
}

const thomsonreuters::ema::access::EmaVector<EnumTypeTable>& DataDictionaryImpl::getEnumTables() const
{
	if (!_pEnumTypeTableList)
	{
		_pEnumTypeTableList = new EmaVector<EnumTypeTable>(_pRsslDataDictionary->enumTableCount);
	}
	else
	{
		if (_pEnumTypeTableList->size() != _pRsslDataDictionary->enumTableCount)
		{
			_pEnumTypeTableList->clear();
		}
		else
		{
			return *_pEnumTypeTableList;
		}
	}

	if ( _loadedEnumTypeDef )
	{
		RsslEnumTypeTable* rsslEnumTypeTable = 0;
		EnumTypeTable enumTypeTable;

		for (UInt16 index = 0; index <= _pRsslDataDictionary->enumTableCount; index++)
		{
			rsslEnumTypeTable = *(_pRsslDataDictionary->enumTables + index);

			if (rsslEnumTypeTable)
			{
				enumTypeTable._pImpl->rsslEnumTypeTable(rsslEnumTypeTable);

				_pEnumTypeTableList->push_back(enumTypeTable);
			}
		}
	}

	return *_pEnumTypeTableList;
}

thomsonreuters::ema::access::Int32 DataDictionaryImpl::getInfoDictionaryId() const
{
	return _pRsslDataDictionary->info_DictionaryId;
}

const thomsonreuters::ema::access::EmaString& DataDictionaryImpl::getFieldVersion() const
{
	_stringInfoFieldVersion.setInt(_pRsslDataDictionary->infoField_Version.data, _pRsslDataDictionary->infoField_Version.length,
		_pRsslDataDictionary->infoField_Version.length > 0 ? true : false );
	return _stringInfoFieldVersion.toString();
}

const thomsonreuters::ema::access::EmaString& DataDictionaryImpl::getEnumRecordTemplateVersion() const
{
	_stringInfoEnumRTVersion.setInt(_pRsslDataDictionary->infoEnum_RT_Version.data, _pRsslDataDictionary->infoEnum_RT_Version.length,
		_pRsslDataDictionary->infoEnum_RT_Version.length > 0 ? true : false);
	return _stringInfoEnumRTVersion.toString();
}

const thomsonreuters::ema::access::EmaString& DataDictionaryImpl::getEnumDisplayTemplateVersion() const
{
	_stringInfoEnumDTVersion.setInt(_pRsslDataDictionary->infoEnum_DT_Version.data, _pRsslDataDictionary->infoEnum_DT_Version.length,
		_pRsslDataDictionary->infoEnum_DT_Version.length > 0 ? true : false);
	return _stringInfoEnumDTVersion.toString();
}

const thomsonreuters::ema::access::EmaString& DataDictionaryImpl::getFieldFilename() const
{
	_stringInfoFieldFilename.setInt(_pRsslDataDictionary->infoField_Filename.data, _pRsslDataDictionary->infoField_Filename.length, 
		_pRsslDataDictionary->infoField_Filename.length > 0 ? true : false);
	return _stringInfoFieldFilename.toString();
}

const thomsonreuters::ema::access::EmaString& DataDictionaryImpl::getFieldDescription() const
{
	_stringInfoFieldDesc.setInt(_pRsslDataDictionary->infoField_Desc.data, _pRsslDataDictionary->infoField_Desc.length,
		_pRsslDataDictionary->infoField_Desc.length > 0 ? true : false);
	return _stringInfoFieldDesc.toString();
}

const thomsonreuters::ema::access::EmaString& DataDictionaryImpl::getFieldBuild() const
{
	_stringInfoFieldBuild.setInt(_pRsslDataDictionary->infoField_Build.data, _pRsslDataDictionary->infoField_Build.length, 
		_pRsslDataDictionary->infoField_Build.length > 0 ? true : false);
	return _stringInfoFieldBuild.toString();
}

const thomsonreuters::ema::access::EmaString& DataDictionaryImpl::getFieldDate() const
{
	_stringInfoFieldDate.setInt(_pRsslDataDictionary->infoField_Date.data, _pRsslDataDictionary->infoField_Date.length, 
		_pRsslDataDictionary->infoField_Date.length > 0 ? true : false);
	return _stringInfoFieldDate.toString();
}

const thomsonreuters::ema::access::EmaString& DataDictionaryImpl::getEnumFilename() const
{
	_stringInfoEnumFilename.setInt(_pRsslDataDictionary->infoEnum_Filename.data, _pRsslDataDictionary->infoEnum_Filename.length,
		_pRsslDataDictionary->infoEnum_Filename.length > 0 ? true : false);
	return _stringInfoEnumFilename;
}

const thomsonreuters::ema::access::EmaString& DataDictionaryImpl::getEnumDescription() const
{
	_stringInfoEnumDesc.setInt(_pRsslDataDictionary->infoEnum_Desc.data, _pRsslDataDictionary->infoEnum_Desc.length, 
		_pRsslDataDictionary->infoEnum_Desc.length > 0 ? true : false);
	return _stringInfoEnumDesc.toString();
}

const thomsonreuters::ema::access::EmaString& DataDictionaryImpl::getEnumDate() const
{
	_stringInfoEnumDate.setInt(_pRsslDataDictionary->infoEnum_Date.data, _pRsslDataDictionary->infoEnum_Date.length, 
		_pRsslDataDictionary->infoEnum_Date.length > 0 ? true : false);

	return _stringInfoEnumDate.toString();
}

bool DataDictionaryImpl::hasEntry(thomsonreuters::ema::access::Int32 fieldId) const
{
	if ( _loadedFieldDictionary )
	{
		if ( getDictionaryEntry(_pRsslDataDictionary, fieldId) != 0 )
		{
			return true;
		}
	}
	
	return false;
}

const DictionaryEntry& DataDictionaryImpl::getEntry(Int32 fieldId) const
{
	if (!_loadedFieldDictionary)
	{
		throwIueException( "The field dictionary information was not loaded", OmmInvalidUsageException::InvalidOperationEnum );
	}

	RsslDictionaryEntry* rsslDictionaryEntry = getDictionaryEntry(_pRsslDataDictionary, fieldId);

	if ( rsslDictionaryEntry )
	{
		_dictionaryEntry._pImpl->rsslDictionaryEntry(rsslDictionaryEntry);

		return _dictionaryEntry;
	}

	EmaString errorText("The Field ID ");
	errorText.append(fieldId).append(" does not exist in the field dictionary");
	throwIueException( errorText, OmmInvalidUsageException::InvalidArgumentEnum );
	
	return _dictionaryEntry;
}

bool DataDictionaryImpl::hasEntry(const thomsonreuters::ema::access::EmaString& fieldName) const
{
	if ( !_loadedFieldDictionary )
	{
		return false;
	}

	FieldNameToIdHash*	 pNameToIdMap = fieldNameToIdMap();

	return pNameToIdMap != 0 ? pNameToIdMap->find(fieldName) != 0 : false;
}

const DictionaryEntry& DataDictionaryImpl::getEntry(const thomsonreuters::ema::access::EmaString& fieldName) const
{
	if ( !_loadedFieldDictionary )
	{
		throwIueException( "The field dictionary information was not loaded", OmmInvalidUsageException::InvalidOperationEnum );
	}

	FieldNameToIdHash*	 pNameToIdMap = fieldNameToIdMap();

	if ( pNameToIdMap )
	{
		thomsonreuters::ema::access::Int16* pFid = fieldNameToIdMap()->find(fieldName);

		if ( pFid )
		{
			return getEntry(*pFid);
		}
	}

	EmaString errorText("The Field name ");
	errorText.append(fieldName).append(" does not exist in the field dictionary");
	throwIueException( errorText, OmmInvalidUsageException::InvalidArgumentEnum );

	return _dictionaryEntry;
}

bool DataDictionaryImpl::hasEnumType(thomsonreuters::ema::access::Int32 fieldId, thomsonreuters::ema::access::Int32 value) const
{
	if ( _loadedFieldDictionary && _loadedEnumTypeDef )
	{
		RsslDictionaryEntry* rsslDictionaryEntry = getDictionaryEntry(_pRsslDataDictionary, fieldId);

		if (rsslDictionaryEntry != 0)
		{
			if (getFieldEntryEnumType(rsslDictionaryEntry, value) != 0)
			{
				return true;
			}
		}
	}

	return false;
}

const EnumType& DataDictionaryImpl::getEnumType(Int32 fieldId, Int32 value) const
{
	if ( !_loadedEnumTypeDef )
	{
		throwIueException( "The enumerated types dictionary was not loaded", OmmInvalidUsageException::InvalidOperationEnum );
	}

	RsslDictionaryEntry* rsslDictionaryEntry = getDictionaryEntry(_pRsslDataDictionary, fieldId);

	if (rsslDictionaryEntry != 0)
	{
		RsslEnumType* rsslEnumType = getFieldEntryEnumType(rsslDictionaryEntry, value);

		if (rsslEnumType != 0)
		{
			_enumType._pImpl->rsslEnumType(rsslEnumType);
			return _enumType;
		}
	}

	EmaString errorText("The enum value ");
	errorText.append(value).append(" for the Field ID ");
	errorText.append(fieldId).append(" does not exist in enumerated type definitions");
	throwIueException( errorText, OmmInvalidUsageException::InvalidArgumentEnum );
	
	return _enumType;
}

void DataDictionaryImpl::loadFieldDictionary(const thomsonreuters::ema::access::EmaString& filename)
{
	if ( _ownRsslDataDictionary )
	{
		if (rsslLoadFieldDictionary(filename.c_str(), _pRsslDataDictionary, &_errorText) < RSSL_RET_SUCCESS)
		{
			thomsonreuters::ema::access::EmaString errorText, workingDir;
			getCurrentDir(workingDir);
			errorText.set("Unable to load field dictionary from file named ").append(filename).append(CR)
				.append("Current working directory ").append(workingDir).append(CR)
				.append("Reason='").append(_errorText.data).append("'");

			throwIueException( errorText, OmmInvalidUsageException::FailureEnum );
		}
		else
		{
			_loadedFieldDictionary = true;
		}
	}
	else
	{
		throwIueForQueryOnly();
	}
}

void DataDictionaryImpl::loadEnumTypeDictionary(const thomsonreuters::ema::access::EmaString& filename)
{
	if ( _ownRsslDataDictionary )
	{
		if (rsslLoadEnumTypeDictionary(filename.c_str(), _pRsslDataDictionary, &_errorText) < RSSL_RET_SUCCESS)
		{
			thomsonreuters::ema::access::EmaString errorText, workingDir;
			getCurrentDir(workingDir);
			errorText.set("Unable to load enumerated type definition from file named ").append(filename).append(CR)
				.append("Current working directory ").append(workingDir).append(CR)
				.append("Reason='").append(_errorText.data).append("'");

			throwIueException( errorText, OmmInvalidUsageException::FailureEnum );
		}
		else
		{
			_loadedEnumTypeDef = true;
		}
	}
	else
	{
		throwIueForQueryOnly();
	}
}

void DataDictionaryImpl::encodeFieldDictionary(thomsonreuters::ema::access::Series& series,
	thomsonreuters::ema::access::UInt32 verbosity)
{
	if (!_loadedFieldDictionary)
	{
		throwIueException( "The field dictionary information was not loaded", OmmInvalidUsageException::InvalidOperationEnum );
	}

	SeriesEncoder& seriesEncoder = static_cast<SeriesEncoder&>(const_cast<Encoder&>(series.getEncoder()));

	UInt32 fieldDictionarySize = _pRsslDataDictionary->numberOfEntries > 0 ? (_pRsslDataDictionary->numberOfEntries * DEFAULT_DICTIONARY_ENTRY_SIZE)
		: DEFAULT_ENCODE_ITERATOR_BUFFER_SIZE;

	EncodeIterator* pEncodeIterator;

	if (!seriesEncoder.hasEncIterator())
	{
		seriesEncoder.acquireEncIterator(fieldDictionarySize);
		pEncodeIterator = seriesEncoder._pEncodeIter;
	}
	else
	{
		pEncodeIterator = seriesEncoder._pEncodeIter;
		pEncodeIterator->reallocate(fieldDictionarySize);
	}

	thomsonreuters::ema::access::Int32 minFid = _pRsslDataDictionary->minFid;
	RsslRet ret;

	while ((ret = rsslEncodeFieldDictionary(&pEncodeIterator->_rsslEncIter, _pRsslDataDictionary, &minFid,
		(RDMDictionaryVerbosityValues)verbosity, &_errorText)) == RSSL_RET_DICT_PART_ENCODED)
	{
		pEncodeIterator->reallocate( pEncodeIterator->_allocatedSize * 2 );
		minFid = _pRsslDataDictionary->minFid;
	}

	if (ret != RSSL_RET_SUCCESS)
	{
		thomsonreuters::ema::access::EmaString errorText("Failed to encode the field dictionary information");
		errorText.append(CR).append("Reason='").append(_errorText.data).append("'");

		throwIueException( errorText, ret );
	}

	pEncodeIterator->setEncodedLength(rsslGetEncodedBufferLength(&(pEncodeIterator->_rsslEncIter)));

	seriesEncoder._containerComplete = true;
}

bool DataDictionaryImpl::encodeFieldDictionary(Series& series, Int32& currentFid, UInt32 verbosity, UInt32 fragmentationSize)
{
	if (!_loadedFieldDictionary)
	{
		throwIueException( "The field dictionary information was not loaded", OmmInvalidUsageException::InvalidOperationEnum );
	}

	SeriesEncoder& seriesEncoder = static_cast<SeriesEncoder&>(const_cast<Encoder&>(series.getEncoder()));

	UInt32 fieldDictionarySize = fragmentationSize != 0 ? fragmentationSize : DEFAULT_FRAGMENTATION_SIZE;

	EncodeIterator* pEncodeIterator;

	if (!seriesEncoder.hasEncIterator())
	{
		seriesEncoder.acquireEncIterator(fieldDictionarySize);
		pEncodeIterator = seriesEncoder._pEncodeIter;
	}
	else
	{
		pEncodeIterator = seriesEncoder._pEncodeIter;
		pEncodeIterator->clear();
		pEncodeIterator->reallocate(fieldDictionarySize);
	}

	RsslRet ret;

	if (pEncodeIterator->_allocatedSize != fieldDictionarySize)
	{
		RsslBuffer* rsslBuffer = pEncodeIterator->_rsslEncBuffer1.data ? &pEncodeIterator->_rsslEncBuffer1 : &pEncodeIterator->_rsslEncBuffer2;

		rsslBuffer->length = fieldDictionarySize;

		RsslRet ret = rsslSetEncodeIteratorBuffer(&pEncodeIterator->_rsslEncIter, rsslBuffer);
		if (ret != RSSL_RET_SUCCESS)
		{
			throwIueException( "Failed to set RsslEncodeIterator buffer in DataDictionaryImpl::encodeFieldDictionary(fragmentationSize).", ret );
		}
	}

	ret = rsslEncodeFieldDictionary(&pEncodeIterator->_rsslEncIter, _pRsslDataDictionary, &currentFid, 
		(RDMDictionaryVerbosityValues)verbosity, &_errorText);

	if (ret == RSSL_RET_SUCCESS)
	{
		pEncodeIterator->setEncodedLength(rsslGetEncodedBufferLength(&(pEncodeIterator->_rsslEncIter)));
		seriesEncoder._containerComplete = true;

		return true;
	}
	else if ( ret == RSSL_RET_DICT_PART_ENCODED )
	{
		pEncodeIterator->setEncodedLength(rsslGetEncodedBufferLength(&(pEncodeIterator->_rsslEncIter)));
		seriesEncoder._containerComplete = true;

		return false;
	}

	thomsonreuters::ema::access::EmaString errorText("Failed to encode the field dictionary information with fragmentation size ");
	errorText.append(fieldDictionarySize).append(CR).append("Reason='").append(_errorText.data).append("'");

	throwIueException( errorText, ret );

	return false;
}

void DataDictionaryImpl::decodeFieldDictionary(const Series& series, UInt32 verbosity)
{
	if (_ownRsslDataDictionary)
	{
		RsslBuffer rsslBuffer;
		rsslBuffer.data = (char *)series.getAsHex().c_buf();
		rsslBuffer.length = series.getAsHex().length();

		RsslDecodeIterator rsslDecodeIterator;
		rsslClearDecodeIterator(&rsslDecodeIterator);

		RsslRet retCode;
		if ((retCode = rsslSetDecodeIteratorRWFVersion(&rsslDecodeIterator, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION)) != RSSL_RET_SUCCESS)
		{
			throwIueException( "Failed to set decode iterator RWF version in DataDictionaryImpl::decodeFieldDictionary()", retCode );
		}

		if ((retCode = rsslSetDecodeIteratorBuffer(&rsslDecodeIterator, &rsslBuffer)) != RSSL_RET_SUCCESS)
		{
			throwIueException( "Failed to set decode iterator buffer in DataDictionaryImpl::decodeFieldDictionary()", retCode );
		}

		if (  (retCode = rsslDecodeFieldDictionary(&rsslDecodeIterator, _pRsslDataDictionary, (RDMDictionaryVerbosityValues)verbosity, &_errorText) ) < RSSL_RET_SUCCESS)
		{
			EmaString errorText("Failed to decode the field dictionary information");
			errorText.append(CR).append("Reason='").append(_errorText.data).append("'");

			throwIueException( errorText, retCode );
		}

		_loadedFieldDictionary = true;
	}
	else
	{
		throwIueForQueryOnly();
	}
}

void DataDictionaryImpl::encodeEnumTypeDictionary(Series& series, UInt32 verbosity)
{
	if (!_loadedEnumTypeDef)
	{
		throwIueException( "The enumerated types dictionary was not loaded", OmmInvalidUsageException::InvalidOperationEnum );
	}

	SeriesEncoder& seriesEncoder = static_cast<SeriesEncoder&>(const_cast<Encoder&>(series.getEncoder()));

	UInt32 enumTypeDictionarySize = _pRsslDataDictionary->enumTableCount > 0 ? (_pRsslDataDictionary->enumTableCount * DEFAULT_ENUM_TABLE_ENTRY_SIZE ) 
		: DEFAULT_ENCODE_ITERATOR_BUFFER_SIZE;

	EncodeIterator* pEncodeIterator;

	if (!seriesEncoder.hasEncIterator())
	{
		seriesEncoder.acquireEncIterator(enumTypeDictionarySize);
		pEncodeIterator = seriesEncoder._pEncodeIter;
	}
	else
	{
		pEncodeIterator = seriesEncoder._pEncodeIter;
		pEncodeIterator->clear(enumTypeDictionarySize);
	}

	RsslRet ret;

	while ((ret = rsslEncodeEnumTypeDictionary(&pEncodeIterator->_rsslEncIter, _pRsslDataDictionary,
		(RDMDictionaryVerbosityValues)verbosity, &_errorText)) == RSSL_RET_DICT_PART_ENCODED)
	{
		pEncodeIterator->reallocate(pEncodeIterator->_allocatedSize * 2);
	}

	if (ret != RSSL_RET_SUCCESS)
	{
		thomsonreuters::ema::access::EmaString errorText("Failed to encode the enumerated type definition");
		errorText.append(CR).append("Reason='").append(_errorText.data).append("'");

		throwIueException( errorText, ret );
	}

	pEncodeIterator->setEncodedLength(rsslGetEncodedBufferLength(&(pEncodeIterator->_rsslEncIter)));

	seriesEncoder._containerComplete = true;
}

bool DataDictionaryImpl::encodeEnumTypeDictionary(thomsonreuters::ema::access::Series& series, thomsonreuters::ema::access::Int32& currenCount,
	thomsonreuters::ema::access::UInt32 verbosity, thomsonreuters::ema::access::UInt32 fragmentationSize)
{
	if (!_loadedEnumTypeDef)
	{
		throwIueException( "The enumerated types dictionary was not loaded", OmmInvalidUsageException::InvalidOperationEnum );
	}

	SeriesEncoder& seriesEncoder = static_cast<SeriesEncoder&>(const_cast<Encoder&>(series.getEncoder()));

	UInt32 enumTypeDictionarySize = fragmentationSize > 0 ? fragmentationSize : DEFAULT_FRAGMENTATION_SIZE;

	EncodeIterator* pEncodeIterator;

	if (!seriesEncoder.hasEncIterator())
	{
		seriesEncoder.acquireEncIterator(enumTypeDictionarySize);
		pEncodeIterator = seriesEncoder._pEncodeIter;
	}
	else
	{
		pEncodeIterator = seriesEncoder._pEncodeIter;
		pEncodeIterator->clear(enumTypeDictionarySize);
	}

	RsslRet ret;

	if (pEncodeIterator->_allocatedSize != enumTypeDictionarySize)
	{
		RsslBuffer* rsslBuffer = pEncodeIterator->_rsslEncBuffer1.data ? &pEncodeIterator->_rsslEncBuffer1 : &pEncodeIterator->_rsslEncBuffer2;

		rsslBuffer->length = enumTypeDictionarySize;

		RsslRet ret = rsslSetEncodeIteratorBuffer(&pEncodeIterator->_rsslEncIter, rsslBuffer);
		if (ret != RSSL_RET_SUCCESS)
		{
			throwIueException( "Failed to set RsslEncodeIterator buffer in DataDictionaryImpl::encodeEnumTypeDictionary(fragmentationSize).", ret );
		}
	}

	ret = rsslEncodeEnumTypeDictionaryAsMultiPart(&pEncodeIterator->_rsslEncIter, _pRsslDataDictionary, &currenCount, 
		(RDMDictionaryVerbosityValues)verbosity, &_errorText);

	if (ret == RSSL_RET_SUCCESS)
	{
		pEncodeIterator->setEncodedLength(rsslGetEncodedBufferLength(&(pEncodeIterator->_rsslEncIter)));
		seriesEncoder._containerComplete = true;

		return true;
	}
	else if (ret == RSSL_RET_DICT_PART_ENCODED)
	{
		pEncodeIterator->setEncodedLength(rsslGetEncodedBufferLength(&(pEncodeIterator->_rsslEncIter)));
		seriesEncoder._containerComplete = true;

		return false;
	}
	
	thomsonreuters::ema::access::EmaString errorText("Failed to set encode enumeration types definition with fragmentation size ");
	errorText.append(enumTypeDictionarySize).append(CR).append("Reason='").append(_errorText.data).append("'");

	throwIueException( errorText, ret );

	return false;
}

void DataDictionaryImpl::decodeEnumTypeDictionary(const Series& series, UInt32 verbosity)
{
	if (_ownRsslDataDictionary)
	{
		RsslBuffer rsslBuffer;
		rsslBuffer.data = (char *)series.getAsHex().c_buf();
		rsslBuffer.length = series.getAsHex().length();

		RsslDecodeIterator rsslDecodeIterator;
		rsslClearDecodeIterator(&rsslDecodeIterator);
		RsslRet ret;

		if ((ret = rsslSetDecodeIteratorBuffer(&rsslDecodeIterator, &rsslBuffer)) != RSSL_RET_SUCCESS)
		{
			throwIueException( "Failed to set decode iterator buffer in DataDictionaryImpl::decodeEnumTypeDictionary()", ret );
		}

		if ((ret = rsslSetDecodeIteratorRWFVersion(&rsslDecodeIterator, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION)) != RSSL_RET_SUCCESS)
		{
			throwIueException( "Failed to set decode iterator RWF version in DataDictionaryImpl::decodeEnumTypeDictionary()", ret );
		}

		if ((ret = rsslDecodeEnumTypeDictionary(&rsslDecodeIterator, _pRsslDataDictionary, (RDMDictionaryVerbosityValues)verbosity, &_errorText)) < 0)
		{
			EmaString errorText("Failed to decode the enumerated types dictionary");
			errorText.append(CR).append("Reason='").append(_errorText.data).append("'");

			throwIueException( errorText, ret );
		}

		_loadedEnumTypeDef = true;
	}
	else
	{
		throwIueForQueryOnly();
	}
}

thomsonreuters::ema::access::UInt32 DataDictionaryImpl::extractDictionaryType(const thomsonreuters::ema::access::Series& series)
{
	RsslBuffer rsslBuffer;
	rsslBuffer.data = (char *)series.getAsHex().c_buf();
	rsslBuffer.length = series.getAsHex().length();

	RsslDecodeIterator rsslDecodeIterator;
	rsslClearDecodeIterator(&rsslDecodeIterator);
	RsslRet ret;

	if ((ret = rsslSetDecodeIteratorBuffer(&rsslDecodeIterator, &rsslBuffer)) != RSSL_RET_SUCCESS)
	{
		throwIueException( "Failed to set decode iterator buffer in DataDictionaryImpl::extractDictionaryType()", ret );
	}

	if ((ret = rsslSetDecodeIteratorRWFVersion(&rsslDecodeIterator, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION)) != RSSL_RET_SUCCESS)
	{
		throwIueException( "Failed to set decode iterator RWF version in DataDictionaryImpl::extractDictionaryType()", ret );
	}

	RDMDictionaryTypes rdmDitionaryTypes;

	if ((ret = rsslExtractDictionaryType(&rsslDecodeIterator, &rdmDitionaryTypes, &_errorText)) < 0)
	{
		EmaString errorText("Failed to extract dictionary type");
		errorText.append(CR).append("Reason='").append(_errorText.data).append("'");

		throwIueException( errorText, ret );
	}

	return (UInt64)rdmDitionaryTypes;
}

DataDictionaryImpl::FieldNameToIdHash* DataDictionaryImpl::fieldNameToIdMap() const
{
	if ( _loadedFieldDictionary )
	{
		if ( _pfieldNameToIdHash == 0 )
		{
			_pfieldNameToIdHash = new FieldNameToIdHash(_pRsslDataDictionary->numberOfEntries);
		}

		if ( _pfieldNameToIdHash->empty() )
		{
			RsslDictionaryEntry* rsslDictionaryEntry = 0;
			EmaString fieldName;

			for (Int32 index = _pRsslDataDictionary->minFid; index <= _pRsslDataDictionary->maxFid; index++)
			{
				rsslDictionaryEntry = *(_pRsslDataDictionary->entriesArray + index);

				if (rsslDictionaryEntry)
				{
					fieldName.set(rsslDictionaryEntry->acronym.data, rsslDictionaryEntry->acronym.length);
					_pfieldNameToIdHash->insert(fieldName, rsslDictionaryEntry->fid);
				}
			}
		}
	}

	return _pfieldNameToIdHash;
}

void DataDictionaryImpl::throwIueForQueryOnly()
{
	throwIueException( "This DataDictionary instance is used for query data dictionary information only", OmmInvalidUsageException::InvalidOperationEnum );
}

const thomsonreuters::ema::access::EmaString&  DataDictionaryImpl::toString() const
{	  
	if (!_pRsslDataDictionary->isInitialized)
	{
		_stringToString.clear().append("DataDictionary is not initialized");
		return _stringToString;
	}

	_stringToString.set(0, 2000000);

	_stringToString.append("Data Dictionary Dump: MinFid=").append(getMinFid()).append(" MaxFid=").append(getMaxFid()).
		append(" NumEntries ").append(_pRsslDataDictionary->numberOfEntries).append("\n\n");

	_stringToString.append("Tags:\n  DictionaryId=\"").append(getInfoDictionaryId()).append("\"\n\n");

	_stringToString.append("  [Field Dictionary Tags]\n").
		append("      Filename=\"").append(getFieldFilename()).append("\"\n").
		append("          Desc=\"").append(getFieldDescription()).append("\"\n").
		append("       Version=\"").append(getFieldVersion()).append("\"\n").
		append("         Build=\"").append(getFieldBuild()).append("\"\n").
		append("          Date=\"").append(getFieldDate()).append("\"\n\n");

	_stringToString.append("  [Enum Type Dictionary Tags]\n").
		append("      Filename=\"").append(getEnumFilename()).append("\"\n").
		append("          Desc=\"").append(getEnumDescription()).append("\"\n").
		append("    RT_Version=\"").append(getEnumRecordTemplateVersion()).append("\"\n").
		append("    DT_Version=\"").append(getEnumDisplayTemplateVersion()).append("\"\n").
		append("          Date=\"").append(getEnumDate()).append("\"\n\n");

	_stringToString.append("Field Dictionary:\n");

	RsslDictionaryEntry* rsslDictionaryEntry = 0;
	DictionaryEntry	dictionaryEntry;

	for (Int32 index = 0; index <= _pRsslDataDictionary->maxFid; index++)
	{
		rsslDictionaryEntry = *(_pRsslDataDictionary->entriesArray + index);

		if (rsslDictionaryEntry)
		{
			dictionaryEntry._pImpl->rsslDictionaryEntry(rsslDictionaryEntry);
			_stringToString.append("  Fid=").append(dictionaryEntry.getFid()).append(" '").append(dictionaryEntry.getAcronym()).
				append("' '").append(dictionaryEntry.getDDEAcronym()).
				append("' Type=").append(dictionaryEntry.getFieldType()).
				append(" RippleTo=").append(dictionaryEntry.getRippleToField()).append(" Len=").append(dictionaryEntry.getLength()).
				append(" EnumLen=").append(dictionaryEntry.getEnumLength()).
				append(" RwfType=").append(dictionaryEntry.getRwfType()).append(" RwfLen=").append(dictionaryEntry.getRwfLength()).append("\n");
		}
	}

	_stringToString.append("\nEnum Type Tables:\n");

	RsslEnumTypeTable* rsslEnumTypeTable = 0;
	EnumTypeTable enumTypeTable;

	for (UInt16 index = 0; index <= _pRsslDataDictionary->enumTableCount; index++)
	{
		rsslEnumTypeTable = *(_pRsslDataDictionary->enumTables + index);

		if ( rsslEnumTypeTable )
		{
			enumTypeTable._pImpl->rsslEnumTypeTable( rsslEnumTypeTable );

			_stringToString.append(enumTypeTable.toString());

			_stringToString.append("\n");
		}
	}

	return _stringToString;
}
