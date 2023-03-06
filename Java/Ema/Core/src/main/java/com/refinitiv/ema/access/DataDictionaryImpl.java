///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.concurrent.locks.ReentrantLock;

import com.refinitiv.ema.access.SeriesImpl;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.ema.rdm.DictionaryEntry;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.rdm.EnumType;
import com.refinitiv.ema.rdm.EnumTypeTable;
import com.refinitiv.eta.codec.Int;
import com.refinitiv.eta.valueadd.common.VaPool;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;

class DataDictionaryImpl implements DataDictionary
{
    private com.refinitiv.eta.codec.DataDictionary 		rsslDataDictionary;
    private boolean												loadedFieldDictionary;
    private boolean												loadedEnumTypeDef;
    private OmmInvalidUsageExceptionImpl                        ommIUExcept;
    private ArrayList<DictionaryEntry>                          dictionaryEntryList;
    private ArrayList<EnumTypeTable>                            enumTypeTableList;
    private StringBuilder                                       errorString;
    private com.refinitiv.eta.codec.DecodeIterator 		rsslDecodeIterator;
    private boolean												ownRsslDataDictionary;
    private HashMap<String,Integer>								fieldNametoIdMap;
    
    private DictionaryEntryImpl									dictionaryEntryImpl = new DictionaryEntryImpl();
    private EnumTypeImpl										enumTypeImpl = new EnumTypeImpl();

    private Int                                                 rsslInt = CodecFactory.createInt();
    private com.refinitiv.eta.codec.Enum 					rsslEnumValue = com.refinitiv.eta.codec.CodecFactory.createEnum();
    private com.refinitiv.eta.transport.Error 				rsslError = com.refinitiv.eta.transport.TransportFactory.createError();
    private VaPool 												dictionaryEntryPool = new VaPool(false);
    private VaPool												enumTypeTablePool = new VaPool(false);
    
    private static final int									DEFAULT_DICTIONARY_ENTRY_SIZE = 40;
    private static final int									DEFAULT_ENUM_TABLE_ENTRY_SIZE = 1024;
    private static final int                                    DEFAULT_FRAGMENTATION_SIZE = 12800;
    private static final String									queryingOnlyText = "This DataDictionary instance is used for query data dictionary information only";

    private ReentrantLock                                       dictionaryLock = new ReentrantLock();
   
    
	DataDictionaryImpl(boolean ownDataDictionary)
	{
		ownRsslDataDictionary = ownDataDictionary;
		
		if ( ownRsslDataDictionary )
		{
			rsslDataDictionary = com.refinitiv.eta.codec.CodecFactory.createDataDictionary();
		}
		
		clearFlags();
	}
	
	DataDictionaryImpl(DataDictionaryImpl other)
	{
		ownRsslDataDictionary = true;
		
		rsslDataDictionary = com.refinitiv.eta.codec.CodecFactory.createDataDictionary();
	
		clearFlags();
		
		if ( !other.loadedFieldDictionary && !other.loadedEnumTypeDef )
		{
			return;
		}
		
		Series series = EmaFactory.createSeries();

		if ( other.loadedFieldDictionary )
		{
			other.encodeFieldDictionary(series, EmaRdm.DICTIONARY_VERBOSE);

			decodeFieldDictionary(series, EmaRdm.DICTIONARY_VERBOSE);

			series.clear();
		}

		if ( other.loadedEnumTypeDef )
		{
			other.encodeEnumTypeDictionary(series, EmaRdm.DICTIONARY_VERBOSE);

			decodeEnumTypeDictionary(series, EmaRdm.DICTIONARY_VERBOSE);
		}
	}
	
	void rsslDataDictionary(com.refinitiv.eta.codec.DataDictionary dataDictionary)
	{
		try {
			dictionaryLock.lock();

			clearDictionaryEntryList();

			clearEnumTypeTableList();

			if (fieldNametoIdMap != null)
			 {
				fieldNametoIdMap.clear();
			}

			if( !ownRsslDataDictionary )
			{
				loadedFieldDictionary = true;
				loadedEnumTypeDef = true;

				rsslDataDictionary = dataDictionary;
			}
		} finally {
			dictionaryLock.unlock();
		}
	}
	
	void clearDictionaryEntryList()
	{
		try {
			dictionaryLock.lock();

			if ( dictionaryEntryList != null && dictionaryEntryList.size() != 0)
			{
				for(int index = 0; index < dictionaryEntryList.size(); index++ )
				{
					((DictionaryEntryImpl) dictionaryEntryList.get(index)).returnToPool();
				}

				dictionaryEntryList.clear();
			}
		} finally {
			dictionaryLock.unlock();
		}
	}
	
	void clearEnumTypeTableList()
	{
		try {
			dictionaryLock.lock();
			if ( enumTypeTableList != null && enumTypeTableList.size() != 0)
			{
				for(int index = 0; index < enumTypeTableList.size(); index++)
				{
					((EnumTypeTableImpl) enumTypeTableList.get(index)).clear().returnToPool();
				}

				enumTypeTableList.clear();
			}
		} finally {
			dictionaryLock.unlock();
		}
	}
	
	@Override
	public void clear() {
		try {
			dictionaryLock.lock();

			clearFlags();

			clearDictionaryEntryList();

			clearEnumTypeTableList();

			if ( fieldNametoIdMap != null)
			{
				fieldNametoIdMap.clear();
			}

			if ( rsslDataDictionary != null)
			{
				rsslDataDictionary.clear();
			}
		} finally {
			dictionaryLock.unlock();
		}
	}
	
	@Override
	public int minFid() {
		try {
			dictionaryLock.lock();
			return rsslDataDictionary.minFid();
		} finally {
			dictionaryLock.unlock();
		}
	}

	@Override
	public int maxFid() {
		try {
			dictionaryLock.lock();
			return rsslDataDictionary.maxFid();
		} finally {
			dictionaryLock.unlock();
		}
	}

	@Override
	public List<DictionaryEntry> entries() {
		try {
			dictionaryLock.lock();

			if ( dictionaryEntryList == null)
			{
				dictionaryEntryList = new ArrayList<>(rsslDataDictionary.numberOfEntries());
			}

			if ( dictionaryEntryList.size() != rsslDataDictionary.numberOfEntries())
			{
				clearDictionaryEntryList();
			}
			else
			{
				return dictionaryEntryList;
			}

			if ( loadedFieldDictionary)
			{
				com.refinitiv.eta.codec.DictionaryEntry dictionaryEntry;

				for( int fieldId = rsslDataDictionary.minFid(); fieldId <= rsslDataDictionary.maxFid(); fieldId++ )
				{
					dictionaryEntry = rsslDataDictionary.entry(fieldId);

					if ( dictionaryEntry != null )
					{
						dictionaryEntryList.add(getDictionaryEntry(this, dictionaryEntry));
					}
				}
			}

			return dictionaryEntryList;
		} finally {
			dictionaryLock.unlock();
		}
	}
	
	@Override
	public List<EnumTypeTable> enumTables() {
		try {
			dictionaryLock.lock();

			if ( enumTypeTableList == null )
			{
				enumTypeTableList = new ArrayList<>(rsslDataDictionary.enumTableCount());
			}

			if ( enumTypeTableList.size() != rsslDataDictionary.enumTableCount())
			{
				clearEnumTypeTableList();
			}
			else
			{
				return enumTypeTableList;
			}

			if( loadedEnumTypeDef )
			{
				com.refinitiv.eta.codec.EnumTypeTable enumTypeTable;

				for( int index = 0; index < rsslDataDictionary.enumTableCount(); index++ )
				{
					enumTypeTable = rsslDataDictionary.enumTables()[index];

					if ( enumTypeTable != null )
					{
						enumTypeTableList.add(getEnumTypeTable(enumTypeTable));
					}
				}
			}
			return enumTypeTableList;
		} finally {
			dictionaryLock.unlock();
		}
	}

	@Override
	public int dictionaryId() {
		try {
			dictionaryLock.lock();
			return rsslDataDictionary.infoDictionaryId();
		} finally {
			dictionaryLock.unlock();
		}
	}

	@Override
	public String fieldVersion() {
		try {
			dictionaryLock.lock();
			return rsslDataDictionary.infoFieldVersion().data() != null ? rsslDataDictionary.infoFieldVersion().toString() : "";
		} finally {
			dictionaryLock.unlock();
		}
	}

	@Override
	public String enumRecordTemplateVersion() {
		try {
			dictionaryLock.lock();
			return rsslDataDictionary.infoEnumRTVersion().data() != null ? rsslDataDictionary.infoEnumRTVersion().toString() : "";
		} finally {
			dictionaryLock.unlock();
		}
	}

	@Override
	public String enumDisplayTemplateVersion() {
		try {
			dictionaryLock.lock();
			return rsslDataDictionary.infoEnumDTVersion().data() != null ? rsslDataDictionary.infoEnumDTVersion().toString() : "";
		} finally {
			dictionaryLock.unlock();
		}
	}

	@Override
	public String fieldFilename() {
		try {
			dictionaryLock.lock();
			return rsslDataDictionary.infoFieldFilename().data() != null ? rsslDataDictionary.infoFieldFilename().toString() : "";
		} finally {
			dictionaryLock.unlock();
		}
	}

	@Override
	public String fieldDescription() {
		try {
			dictionaryLock.lock();
			return rsslDataDictionary.infoFieldDesc().data() != null ? rsslDataDictionary.infoFieldDesc().toString() : "";
		} finally {
			dictionaryLock.unlock();
		}
	}

	@Override
	public String fieldBuild() {
		try {
			dictionaryLock.lock();
			return rsslDataDictionary.infoFieldBuild().data() != null ? rsslDataDictionary.infoFieldBuild().toString() : "";
		} finally {
			dictionaryLock.unlock();
		}
	}

	@Override
	public String fieldDate() {
		try {
			dictionaryLock.lock();
			return rsslDataDictionary.infoFieldDate().data() != null ? rsslDataDictionary.infoFieldDate().toString() : "";
		} finally {
			dictionaryLock.unlock();
		}
	}

	@Override
	public String enumFilename() {
		try {
			dictionaryLock.lock();
			return rsslDataDictionary.infoEnumFilename().data() != null ? rsslDataDictionary.infoEnumFilename().toString() : "";
		} finally {
			dictionaryLock.unlock();
		}
	}

	@Override
	public String enumDescription() {
		try {
			dictionaryLock.lock();
			return rsslDataDictionary.infoEnumDesc().data() != null ? rsslDataDictionary.infoEnumDesc().toString() : "";
		} finally {
			dictionaryLock.unlock();
		}
	}

	@Override
	public String enumDate() {
		try {
			dictionaryLock.lock();
			return rsslDataDictionary.infoEnumDate().data() != null ? rsslDataDictionary.infoEnumDate().toString() : "";
		} finally {
			dictionaryLock.unlock();
		}
	}

	@Override
	public void loadFieldDictionary(String filename) {
		
		try {
			dictionaryLock.lock();
			if ( ownRsslDataDictionary )
			{
				if ( rsslDataDictionary.loadFieldDictionary(filename, rsslError) < 0 )
				{
					String errText = errorString().append("Unable to load field dictionary from file named ")
							.append(filename).append(OmmLoggerClient.CR)
							.append("Current working directory ")
							.append(System.getProperty("user.dir"))
							.append(OmmLoggerClient.CR)
							.append("Reason='")
							.append(rsslError.text())
							.append("'").toString();
					throw ommIUExcept().message(errText, rsslError.errorId());
				}
				else
				{
					loadedFieldDictionary = true;
				}
			}
			else
			{
				throw ommIUExcept().message(queryingOnlyText, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			}
		} finally {
			dictionaryLock.unlock();
		}
	}

	@Override
	public void loadEnumTypeDictionary(String filename) {
		
		try {
			dictionaryLock.lock();
			if ( ownRsslDataDictionary )
			{
				if ( rsslDataDictionary.loadEnumTypeDictionary(filename, rsslError) < 0 )
				{
					String errText = errorString().append("Unable to load enumerated type definition from file named ")
							.append(filename).append(OmmLoggerClient.CR)
							.append("Current working directory ")
							.append(System.getProperty("user.dir"))
							.append(OmmLoggerClient.CR)
							.append("Reason='")
							.append(rsslError.text())
							.append("'").toString();
					throw ommIUExcept().message(errText, rsslError.errorId());
				}
				else
				{
					loadedEnumTypeDef = true;
				}
			}
			else
			{
				throw ommIUExcept().message(queryingOnlyText, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			}
		} finally {
			dictionaryLock.unlock();
		}
	}

	@Override
	public void encodeFieldDictionary(Series series, long verbosity) {
		try {
			dictionaryLock.lock();

			if( !loadedFieldDictionary )
			{
				throw ommIUExcept().message("The field dictionary information was not loaded", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			}

			if ( series == null )
			{
				String errText = errorString().append("Passed in series parameter is null").toString();
				throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
			}

			series.clear();

			SeriesImpl seriesImpl = (SeriesImpl) series;

			int fieldDictionarySize = rsslDataDictionary.numberOfEntries() > 0 ? (rsslDataDictionary.numberOfEntries() * DEFAULT_DICTIONARY_ENTRY_SIZE) :
					CollectionDataImpl.ENCODE_RSSL_BUFFER_INIT_SIZE;

			Utilities.reallocate(seriesImpl._rsslEncodeIter, fieldDictionarySize);

			seriesImpl._rsslBuffer = seriesImpl._rsslEncodeIter.buffer();

			rsslInt.value(rsslDataDictionary.minFid());

			int ret;

			while ( (ret = rsslDataDictionary.encodeFieldDictionary(seriesImpl._rsslEncodeIter, rsslInt, (int) verbosity, rsslError)) == CodecReturnCodes.DICT_PART_ENCODED )
			{
				Utilities.reallocate(seriesImpl._rsslEncodeIter, seriesImpl._rsslEncodeIter.buffer().capacity() * 2);
				rsslInt.value(rsslDataDictionary.minFid());
			}

			if ( ret != CodecReturnCodes.SUCCESS)
			{
				seriesImpl.clear();
				String errText = errorString().append("Failed to encode the field dictionary information")
						.append(OmmLoggerClient.CR)
						.append("Reason='")
						.append(rsslError.text())
						.append("'").toString();

				throw ommIUExcept().message(errText, ret);
			}

			seriesImpl._encodeComplete = true;
		} finally {
			dictionaryLock.unlock();
		}
	}

	@Override
	public void decodeFieldDictionary(Series series, long verbosity) {
		try {
			dictionaryLock.lock();

			if (ownRsslDataDictionary)
			{
				if ( series == null )
				{
					String errText = errorString().append("Passed in series parameter is null").toString();
					throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
				}

				Buffer encodedBuffer = ((SeriesImpl) series).encodedData();

				com.refinitiv.eta.codec.DecodeIterator decodeIterator = decodeIterator();

				int ret = decodeIterator.setBufferAndRWFVersion(encodedBuffer, com.refinitiv.eta.codec.Codec.majorVersion(),
						com.refinitiv.eta.codec.Codec.minorVersion());

				if (ret != CodecReturnCodes.SUCCESS)
				{
					String errText = errorString().append("Failed to setBufferAndRWFVersion on rssl decode iterator. Reason='")
							.append(CodecReturnCodes.toString(ret))
							.append("'").toString();
					throw ommIUExcept().message(errText, ret);
				}

				ret = rsslDataDictionary.decodeFieldDictionary(decodeIterator, (int) verbosity, rsslError);

				if ( ret < CodecReturnCodes.SUCCESS )
				{
					String errText = errorString().append("Failed to decode the field dictionary information. Reason='")
							.append(rsslError.text())
							.append("'").toString();
					throw ommIUExcept().message(errText, ret);
				}

				loadedFieldDictionary = true;
			}
			else
			{
				throw ommIUExcept().message(queryingOnlyText, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			}
		} finally {
			dictionaryLock.unlock();
		}
	}

	@Override
	public void encodeEnumTypeDictionary(Series series, long verbosity) {
		try {
			dictionaryLock.lock();

			if (!loadedEnumTypeDef)
			{
				throw ommIUExcept().message("The enumerated types dictionary was not loaded", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			}

			if ( series == null)
			{
				String errText = errorString().append("Passed in series parameter is null").toString();
				throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
			}

			series.clear();

			SeriesImpl seriesImpl = (SeriesImpl) series;

			int enumTypeDictionarySize = rsslDataDictionary.enumTableCount() > 0 ? (rsslDataDictionary.enumTableCount() * DEFAULT_ENUM_TABLE_ENTRY_SIZE) :
					CollectionDataImpl.ENCODE_RSSL_BUFFER_INIT_SIZE;

			Utilities.reallocate(seriesImpl._rsslEncodeIter, enumTypeDictionarySize);

			seriesImpl._rsslBuffer = seriesImpl._rsslEncodeIter.buffer();

			int ret;

			while ( (ret = rsslDataDictionary.encodeEnumTypeDictionary(seriesImpl._rsslEncodeIter, (int) verbosity, rsslError)) == CodecReturnCodes.DICT_PART_ENCODED )
			{
				Utilities.reallocate(seriesImpl._rsslEncodeIter, seriesImpl._rsslEncodeIter.buffer().capacity() * 2);
			}

			if ( ret != CodecReturnCodes.SUCCESS)
			{
				seriesImpl.clear();
				String errText = errorString().append("Failed to encode the enumerated type definition")
						.append(OmmLoggerClient.CR)
						.append("Reason='")
						.append(rsslError.text())
						.append("'").toString();

				throw ommIUExcept().message(errText, ret);
			}

			seriesImpl._encodeComplete = true;
		} finally {
			dictionaryLock.unlock();
		}
	}
	
	@Override
	public int encodeEnumTypeDictionary(Series series, int currentEnumTableEntry, long verbosity, int fragmentationSize) {
		try {
			dictionaryLock.lock();

			if ( series == null )
			{
				String errText = errorString().append("Passed in series parameter is null").toString();
				throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
			}

			if( !loadedEnumTypeDef )
			{
				throw ommIUExcept().message("The enumerated types dictionary was not loaded", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			}

			series.clear();

			SeriesImpl seriesImpl = (SeriesImpl) series;

			int enumTypeDictionarySize = fragmentationSize > 0 ? fragmentationSize : DEFAULT_FRAGMENTATION_SIZE;

			Utilities.reallocate(seriesImpl._rsslEncodeIter, enumTypeDictionarySize);

			seriesImpl._rsslBuffer = seriesImpl._rsslEncodeIter.buffer();

			seriesImpl._rsslEncodeIter.buffer().data(seriesImpl._rsslEncodeIter.buffer().data(), 0, enumTypeDictionarySize);

			rsslInt.value(currentEnumTableEntry);

			int ret = rsslDataDictionary.encodeEnumTypeDictionaryAsMultiPart(seriesImpl._rsslEncodeIter, rsslInt, (int) verbosity, rsslError);

			if ( ret == CodecReturnCodes.DICT_PART_ENCODED)
			{
				seriesImpl._encodeComplete = true;
				return (int) rsslInt.toLong();
			}
			else if (ret == CodecReturnCodes.SUCCESS )
			{
				seriesImpl._encodeComplete = true;
				return enumTables().size();
			}

			seriesImpl.clear();
			String errText = errorString().append("Failed to encode the enumerated type definition")
					.append(OmmLoggerClient.CR)
					.append("Reason='")
					.append(rsslError.text())
					.append("'").toString();

			throw ommIUExcept().message(errText, ret);
		} finally {
			dictionaryLock.unlock();
		}
	}
	
	@Override
	public int encodeFieldDictionary(Series series, int currentFid, long verbosity, int fragmentationSize) {
		try {
			dictionaryLock.lock();

			if ( series == null )
			{
				String errText = errorString().append("Passed in series parameter is null").toString();
				throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
			}

			if(!loadedFieldDictionary)
			{
				throw ommIUExcept().message("The field dictionary information was not loaded", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			}

			series.clear();

			SeriesImpl seriesImpl = (SeriesImpl) series;

			int fieldDictionarySize = fragmentationSize > 0 ? fragmentationSize : DEFAULT_FRAGMENTATION_SIZE;

			Utilities.reallocate(seriesImpl._rsslEncodeIter, fieldDictionarySize);

			seriesImpl._rsslBuffer = seriesImpl._rsslEncodeIter.buffer();

			seriesImpl._rsslEncodeIter.buffer().data(seriesImpl._rsslEncodeIter.buffer().data(), 0, fieldDictionarySize);

			rsslInt.value(currentFid);

			int ret;

			ret = rsslDataDictionary.encodeFieldDictionary(seriesImpl._rsslEncodeIter, rsslInt, (int) verbosity, rsslError);

			if ( ret == CodecReturnCodes.SUCCESS )
			{
				seriesImpl._encodeComplete = true;
				return rsslDataDictionary.maxFid();
			}
			if ( ret == CodecReturnCodes.DICT_PART_ENCODED )
			{
				seriesImpl._encodeComplete = true;
				return (int) rsslInt.toLong();
			}

			String errText = errorString().append("Failed to encode the field dictionary information. Reason='")
					.append(CodecReturnCodes.toString(ret))
					.append("'").toString();
			throw ommIUExcept().message(errText, ret);
		} finally {
			dictionaryLock.unlock();
		}
	}

	@Override
	public int extractDictionaryType(Series series) {
		try {
			dictionaryLock.lock();

			if ( series == null )
			{
				String errText = errorString().append("Passed in series parameter is null").toString();
				throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
			}

			Buffer encodedBuffer = ((SeriesImpl) series).encodedData();

			com.refinitiv.eta.codec.DecodeIterator decodeIterator = decodeIterator();

			int ret = decodeIterator.setBufferAndRWFVersion(encodedBuffer, com.refinitiv.eta.codec.Codec.majorVersion(),
					com.refinitiv.eta.codec.Codec.minorVersion());

			if (ret != CodecReturnCodes.SUCCESS)
			{
				String errText = errorString().append("Failed to setBufferAndRWFVersion on rssl decode iterator. Reason='")
						.append(CodecReturnCodes.toString(ret))
						.append("'").toString();
				throw ommIUExcept().message(errText, ret);
			}

			com.refinitiv.eta.codec.Int dictionaryType = com.refinitiv.eta.codec.CodecFactory.createInt();

			if ( rsslDataDictionary.extractDictionaryType(decodeIterator, dictionaryType, rsslError) < CodecReturnCodes.SUCCESS )
			{
				String errText = errorString().append("Failed to extract dictionary type. Reason='")
						.append(CodecReturnCodes.toString(ret))
						.append("'").toString();
				throw ommIUExcept().message(errText, rsslError.errorId());
			}

			return (int) dictionaryType.toLong();
		} finally {
			dictionaryLock.unlock();
		}
	}

	@Override
	public void decodeEnumTypeDictionary(Series series, long verbosity) {
		try {
			dictionaryLock.lock();

			if ( ownRsslDataDictionary)
			{
				if ( series == null )
				{
					String errText = errorString().append("Passed in series parameter is null").toString();
					throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
				}

				Buffer encodedBuffer = ((SeriesImpl) series).encodedData();

				com.refinitiv.eta.codec.DecodeIterator decodeIterator = decodeIterator();

				int ret = decodeIterator.setBufferAndRWFVersion(encodedBuffer, com.refinitiv.eta.codec.Codec.majorVersion(),
						com.refinitiv.eta.codec.Codec.minorVersion());

				if (ret != CodecReturnCodes.SUCCESS)
				{
					String errText = errorString().append("Failed to setBufferAndRWFVersion on rssl decode iterator. Reason='")
							.append(CodecReturnCodes.toString(ret))
							.append("'").toString();
					throw ommIUExcept().message(errText, ret);
				}

				ret = rsslDataDictionary.decodeEnumTypeDictionary(decodeIterator, (int) verbosity, rsslError);
				if ( ret < CodecReturnCodes.SUCCESS )
				{
					String errText = errorString().append("Failed to decode the enumerated types dictionary. Reason='")
							.append(rsslError.text())
							.append("'").toString();
					throw ommIUExcept().message(errText, ret);
				}

				loadedEnumTypeDef = true;
			}
			else
			{
				throw ommIUExcept().message(queryingOnlyText, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			}
		} finally {
			dictionaryLock.unlock();
		}
	}

	@Override
	public DictionaryEntry entry(int fieldId) {
		getEntry(fieldId, dictionaryEntryImpl);
		return dictionaryEntryImpl;
	}

	@Override
	public void entry(int fieldId, DictionaryEntry entryDst) {
		if (entryDst == null) {
			throw ommIUExcept().message("DictionaryEntry entryDst parameter is null", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
		}
		if (!((DictionaryEntryImpl)entryDst).isManagedByUser()) {
			throw  ommIUExcept().message("DictionaryEntry entryDst parameter should be created by EmaFactory.createDictionaryEntry call", OmmInvalidUsageException.ErrorCode.INVALID_USAGE);
		}
		getEntry(fieldId, (DictionaryEntryImpl) entryDst);
	}

	private void getEntry(int fieldId, DictionaryEntryImpl entryDst) {
		try {
			dictionaryLock.lock();

			if( !loadedFieldDictionary )
			{
				throw ommIUExcept().message("The field dictionary information was not loaded", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			}

			com.refinitiv.eta.codec.DictionaryEntry dictionaryEntry = rsslDataDictionary.entry(fieldId);

			if ( dictionaryEntry != null )
			{
				entryDst.dictionaryEntry(this, dictionaryEntry);
				return;
			}

			throw ommIUExcept().message("The Field ID " + fieldId + " does not exist in the field dictionary", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
		} finally {
			dictionaryLock.unlock();
		}
	}


	@Override
	public EnumType enumType(int fieldId, int value) {
		try {
			dictionaryLock.lock();

			if ( !loadedEnumTypeDef )
			{
				throw ommIUExcept().message("The enumerated types dictionary was not loaded", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			}

			com.refinitiv.eta.codec.DictionaryEntry dictionaryEntry;

			dictionaryEntry = rsslDataDictionary.entry(fieldId);

			if ( dictionaryEntry != null )
			{
				rsslEnumValue.value(value);

				com.refinitiv.eta.codec.EnumType enumType = rsslDataDictionary.entryEnumType(dictionaryEntry, rsslEnumValue);

				if ( enumType != null )
				{
					return enumTypeImpl.enumType(enumType);
				}
			}

			throw ommIUExcept().message("The enum value " + value + " for the Field ID " + fieldId + " does not exist in enumerated type definitions",
					OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
		} finally {
			dictionaryLock.unlock();
		}
	}

	@Override
	public boolean isFieldDictionaryLoaded() {
		return loadedFieldDictionary;
	}

	@Override
	public boolean isEnumTypeDefLoaded() {
		return loadedEnumTypeDef;
	}

	@Override
	public boolean hasEntry(int fieldId) {
		try {
			dictionaryLock.lock();

			if ( !loadedFieldDictionary )
			{
				return false;
			}

			return rsslDataDictionary.entry(fieldId) != null ? true : false;
		} finally {
			dictionaryLock.unlock();
		}
	}

	@Override
	public boolean hasEnumType(int fieldId, int value) {
		try {
			dictionaryLock.lock();

			if( !loadedFieldDictionary &&  !loadedEnumTypeDef )
			{
				return false;
			}

			com.refinitiv.eta.codec.DictionaryEntry dictionaryEntry;

			dictionaryEntry = rsslDataDictionary.entry(fieldId);

			if ( dictionaryEntry != null )
			{
				rsslEnumValue.value(value);
				return rsslDataDictionary.entryEnumType(dictionaryEntry, rsslEnumValue) != null ? true : false;
			}

			return false;
		} finally {
			dictionaryLock.unlock();
		}
	}
	
	@Override
	public boolean hasEntry(String fieldName) {
		try {
			dictionaryLock.lock();

			if( !loadedFieldDictionary )
			{
				return false;
			}

			HashMap<String, Integer> nameToIdMap = fieldNameToIdMap();

			return nameToIdMap != null ? nameToIdMap.containsKey(fieldName) : false;
		} finally {
			dictionaryLock.unlock();
		}
	}

	@Override
	public DictionaryEntry entry(String fieldName) {
		getEntry(fieldName, dictionaryEntryImpl);
		return dictionaryEntryImpl;
	}

	@Override
	public void entry(String fieldName, DictionaryEntry entryDst) {
		if (entryDst == null) {
			throw ommIUExcept().message("DictionaryEntry entryDst parameter is null", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
		}
		if (!((DictionaryEntryImpl)entryDst).isManagedByUser()) {
			throw  ommIUExcept().message("DictionaryEntry entryDst parameter should be created by EmaFactory. call", OmmInvalidUsageException.ErrorCode.INVALID_USAGE);
		}
		getEntry(fieldName, (DictionaryEntryImpl) entryDst);
	}

	private void getEntry(String fieldName, DictionaryEntryImpl entryDst) {
		try {
			dictionaryLock.lock();

			if( !loadedFieldDictionary )
			{
				throw ommIUExcept().message("The field dictionary information was not loaded", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			}

			if ( !hasEntry(fieldName) )
			{
				throw ommIUExcept().message("The Field name " + fieldName + " does not exist in the field dictionary", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
			}

			getEntry(fieldNameToIdMap().get(fieldName), entryDst);
		} finally {
			dictionaryLock.unlock();
		}
	}

	@Override
	public String toString() {
		try {
			dictionaryLock.lock();

			if ( rsslDataDictionary.toString() == null || ( !loadedFieldDictionary && !loadedEnumTypeDef ) )
			{
				return "DataDictionary is not initialized";
			}

			return rsslDataDictionary.toString();
		} finally {
			dictionaryLock.unlock();
		}
	}

	com.refinitiv.eta.codec.DataDictionary rsslDataDictionary() {
		return rsslDataDictionary;
	}
	
	
	private HashMap<String,Integer> fieldNameToIdMap()
	{
		if ( loadedFieldDictionary )
		{
			if ( fieldNametoIdMap == null )
			{
				fieldNametoIdMap = new HashMap<>(rsslDataDictionary.numberOfEntries());
			}

			if ( fieldNametoIdMap.size() == 0 )
			{
				com.refinitiv.eta.codec.DictionaryEntry dictionaryEntry;

				for( int fieldId = rsslDataDictionary.minFid(); fieldId <= rsslDataDictionary.maxFid(); fieldId++ )
				{
					dictionaryEntry = rsslDataDictionary.entry(fieldId);

					if ( dictionaryEntry != null && dictionaryEntry.acronym().data() != null )
					{
						fieldNametoIdMap.put(dictionaryEntry.acronym().toString(), dictionaryEntry.fid());
					}
				}
			}
		}

		return fieldNametoIdMap;
	}
	
	private DictionaryEntryImpl getDictionaryEntry(DataDictionaryImpl dataDictionary, 
			com.refinitiv.eta.codec.DictionaryEntry dictionaryEntry)
	{
		DictionaryEntryImpl dictionaryEntryImpl = (DictionaryEntryImpl)dictionaryEntryPool.poll();

		if ( dictionaryEntryImpl == null )
		{
			dictionaryEntryImpl = new DictionaryEntryImpl();
			dictionaryEntryPool.updatePool(dictionaryEntryImpl);
		}

		dictionaryEntryImpl.dictionaryEntry(dataDictionary, dictionaryEntry);

		return dictionaryEntryImpl;
	}
	
	private EnumTypeTableImpl getEnumTypeTable(com.refinitiv.eta.codec.EnumTypeTable enumTypeTable)
	{
		EnumTypeTableImpl enumTypeTableImpl = (EnumTypeTableImpl)enumTypeTablePool.poll();

		if ( enumTypeTableImpl == null )
		{
			enumTypeTableImpl = new EnumTypeTableImpl();
			enumTypeTablePool.updatePool(enumTypeTableImpl);
		}

		enumTypeTableImpl.enumTypeTable(enumTypeTable);

		return enumTypeTableImpl;
	}
	
	void clearFlags()
	{
		loadedFieldDictionary = false;
		loadedEnumTypeDef = false;
	}
	
	private StringBuilder errorString()
	{
		if ( errorString == null )
		{
			errorString = new StringBuilder(255);
		}
		else
		{
			errorString.setLength(0);
		}

		return errorString;
	}
	
	private OmmInvalidUsageExceptionImpl ommIUExcept()
	{
		if (ommIUExcept == null)
			ommIUExcept = new OmmInvalidUsageExceptionImpl();

		return ommIUExcept;
	}
	
	private com.refinitiv.eta.codec.DecodeIterator decodeIterator()
	{
		if ( rsslDecodeIterator == null )
		{
			rsslDecodeIterator = com.refinitiv.eta.codec.CodecFactory.createDecodeIterator();
		}
		
		rsslDecodeIterator.clear();
		
		return rsslDecodeIterator;
	}
}
