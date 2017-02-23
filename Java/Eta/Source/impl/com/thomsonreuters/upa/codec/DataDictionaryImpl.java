package com.thomsonreuters.upa.codec;

import java.io.File;
import java.io.FileReader;
import java.nio.ByteBuffer;

import com.thomsonreuters.upa.rdm.Dictionary;
import com.thomsonreuters.upa.rdm.ElementNames;
import com.thomsonreuters.upa.transport.Error;

class DataDictionaryImpl implements DataDictionary
{
    private final int MIN_FID = -32768;
    private final int MAX_FID = 32767;
    private final int ENUM_TABLE_MAX_COUNT = ((MAX_FID) - (MIN_FID) + 1);
    private final int MAX_ENUM_TYPE_COUNT = 2500;

    // Dictionary - Element names that should be hidden
    private final Buffer ENUM_FID = CodecFactory.createBuffer();
    private final Buffer VALUES = CodecFactory.createBuffer();
    private final Buffer DISPLAYS = CodecFactory.createBuffer();

    int                         _minFid;
    int                         _maxFid;
    int                         _numberOfEntries;
    DictionaryEntryImpl[]       _entriesArray;
    boolean                     _isInitialized;

    EnumTypeTable[]             _enumTables;
    int                         _enumTableCount;

    /* Tags */
    int                         _infoDictionaryId;

    /* Field Dictionary Tags */
    final Buffer                _infoFieldVersion = CodecFactory.createBuffer();

    /* Enum Type Dictionary Tags */
    final Buffer                _infoEnumRTVersion = CodecFactory.createBuffer();
    final Buffer                _infoEnumDTVersion = CodecFactory.createBuffer();

    /* Field Dictionary Additional tags (currently these are not defined by the domain model and are not sent by the encode/decode methods) */
    final Buffer                _infoFieldFilename = CodecFactory.createBuffer();
    final Buffer                _infoFieldDesc = CodecFactory.createBuffer();
    final Buffer                _infoFieldBuild = CodecFactory.createBuffer();
    final Buffer                _infoFieldDate = CodecFactory.createBuffer();

    /* Enum Type Dictionary Additional Tags */
    final Buffer                _infoEnumFilename = CodecFactory.createBuffer();
    final Buffer                _infoEnumDesc = CodecFactory.createBuffer();
    final Buffer                _infoEnumDate = CodecFactory.createBuffer();
    
    // dictionary parsing variables
    private File _fieldDictFile;
    private File _enumTypeDefFile;
    private char[] _fieldDictFileLine, _enumTypeDefFileLine;
    private FileReader _fileInput;
    private int _startPosition, _lastPosition, _lineStartPosition;
    private final EnumTypeImpl[] _enumTypeArray = new EnumTypeImpl[MAX_ENUM_TYPE_COUNT];
    private int _enumTypeArrayCount = -1;
    private final int[] _referenceFidArray = new int[MAX_ENUM_TYPE_COUNT];
    private final Buffer[] _referenceFidAcronymArray = new Buffer[MAX_ENUM_TYPE_COUNT];
    
    // dictionary encoding variables
    private final Buffer NAME = CodecFactory.createBuffer();
    private final Buffer FID = CodecFactory.createBuffer();
    private final Buffer RIPPLETO = CodecFactory.createBuffer();
    private final Buffer TYPE = CodecFactory.createBuffer();
    private final Buffer LENGTH = CodecFactory.createBuffer();
    private final Buffer RWFTYPE = CodecFactory.createBuffer();
    private final Buffer RWFLEN = CodecFactory.createBuffer();
    private final Buffer ENUMLENGTH = CodecFactory.createBuffer();
    private final Buffer LONGNAME = CodecFactory.createBuffer();
    private final ElementSetDefEntry[] elementSetDefEntries = new ElementSetDefEntryImpl[9];
    private final ElementSetDef setDef0_Minimal = CodecFactory.createElementSetDef();
    private final ElementSetDef setDef0_Normal = CodecFactory.createElementSetDef();
    private final Buffer FIDS = CodecFactory.createBuffer();
    private final Buffer VALUE = CodecFactory.createBuffer();
    private final Buffer DISPLAY = CodecFactory.createBuffer();
    private final Buffer MEANING = CodecFactory.createBuffer();
    private final ElementSetDefEntry[] enumSetDefEntries = new ElementSetDefEntryImpl[4];
    private final ElementSetDef enumSetDef0_Normal = CodecFactory.createElementSetDef();
    private final ElementSetDef enumSetDef0_Verbose = CodecFactory.createElementSetDef();
    
    // dictionary encode/decode container variables
    SeriesImpl series = (SeriesImpl)CodecFactory.createSeries();
    VectorImpl vector = (VectorImpl)CodecFactory.createVector();
    LocalElementSetDefDbImpl setDb = (LocalElementSetDefDbImpl)CodecFactory.createLocalElementSetDefDb();
    VectorEntry vectorEntry = CodecFactory.createVectorEntry();
    SeriesEntry seriesEntry = CodecFactory.createSeriesEntry();
    ElementList elemList = CodecFactory.createElementList();
    ElementEntryImpl elemEntry = (ElementEntryImpl)CodecFactory.createElementEntry();
    Int tmpInt = CodecFactory.createInt();
    long tmpLong;
    ArrayImpl arr = (ArrayImpl)CodecFactory.createArray();
    ArrayEntry arrEntry = CodecFactory.createArrayEntry();
    Buffer rippleAcronym = CodecFactory.createBuffer();
    UInt tempUInt = CodecFactory.createUInt();
    Int tempInt = CodecFactory.createInt();
    Buffer tempBuffer = CodecFactory.createBuffer();
    DecodeIterator tempDecIter = CodecFactory.createDecodeIterator();
    Enum tempEnum = CodecFactory.createEnum();
    FieldSetDef newSetDef = CodecFactory.createFieldSetDef();
    LocalFieldSetDefDb fieldSetDef = CodecFactory.createLocalFieldSetDefDb();

    String dictionaryString; // for toString method

    {
        ENUM_FID.data("FID");
        VALUES.data("VALUES");
        DISPLAYS.data("DISPLAYS");

        for (int i = 0; i < MAX_ENUM_TYPE_COUNT; i++)
        {
            _referenceFidAcronymArray[i] = CodecFactory.createBuffer();
        }

        // set definitions for encode dictionary
        NAME.data("NAME");
        FID.data("FID");
        RIPPLETO.data("RIPPLETO");
        TYPE.data("TYPE");
        LENGTH.data("LENGTH");
        RWFTYPE.data("RWFTYPE");
        RWFLEN.data("RWFLEN");
        ENUMLENGTH.data("ENUMLENGTH");
        LONGNAME.data("LONGNAME");
        elementSetDefEntries[0] = CodecFactory.createElementSetDefEntry();
        elementSetDefEntries[0].name(NAME);
        elementSetDefEntries[0].dataType(DataTypes.ASCII_STRING);
        elementSetDefEntries[1] = CodecFactory.createElementSetDefEntry();
        elementSetDefEntries[1].name(FID);
        elementSetDefEntries[1].dataType(DataTypes.INT_2);
        elementSetDefEntries[2] = CodecFactory.createElementSetDefEntry();
        elementSetDefEntries[2].name(RIPPLETO);
        elementSetDefEntries[2].dataType(DataTypes.INT_2);
        elementSetDefEntries[3] = CodecFactory.createElementSetDefEntry();
        elementSetDefEntries[3].name(TYPE);
        elementSetDefEntries[3].dataType(DataTypes.INT_1);
        elementSetDefEntries[4] = CodecFactory.createElementSetDefEntry();
        elementSetDefEntries[4].name(LENGTH);
        elementSetDefEntries[4].dataType(DataTypes.UINT_2);
        elementSetDefEntries[5] = CodecFactory.createElementSetDefEntry();
        elementSetDefEntries[5].name(RWFTYPE);
        elementSetDefEntries[5].dataType(DataTypes.UINT_1);
        elementSetDefEntries[6] = CodecFactory.createElementSetDefEntry();
        elementSetDefEntries[6].name(RWFLEN);
        elementSetDefEntries[6].dataType(DataTypes.UINT_2);
        elementSetDefEntries[7] = CodecFactory.createElementSetDefEntry();
        elementSetDefEntries[7].name(ENUMLENGTH);
        elementSetDefEntries[7].dataType(DataTypes.UINT_1);
        elementSetDefEntries[8] = CodecFactory.createElementSetDefEntry();
        elementSetDefEntries[8].name(LONGNAME);
        elementSetDefEntries[8].dataType(DataTypes.ASCII_STRING);
        setDef0_Minimal.setId(0); /* SetID */
        setDef0_Minimal.count(7); /* count */
        setDef0_Minimal.entries(elementSetDefEntries);
        setDef0_Normal.setId(0); /* SetID */
        setDef0_Normal.count(9); /* count */
        setDef0_Normal.entries(elementSetDefEntries);
        FIDS.data("FIDS");
        VALUE.data("VALUE");
        DISPLAY.data("DISPLAY");
        MEANING.data("MEANING");
        enumSetDefEntries[0] = CodecFactory.createElementSetDefEntry();
        enumSetDefEntries[0].name(FIDS);
        enumSetDefEntries[0].dataType(DataTypes.ARRAY);
        enumSetDefEntries[1] = CodecFactory.createElementSetDefEntry();
        enumSetDefEntries[1].name(VALUE);
        enumSetDefEntries[1].dataType(DataTypes.ARRAY);
        enumSetDefEntries[2] = CodecFactory.createElementSetDefEntry();
        enumSetDefEntries[2].name(DISPLAY);
        enumSetDefEntries[2].dataType(DataTypes.ARRAY);
        enumSetDefEntries[3] = CodecFactory.createElementSetDefEntry();
        enumSetDefEntries[3].name(MEANING);
        enumSetDefEntries[3].dataType(DataTypes.ARRAY);
        enumSetDef0_Normal.setId(0); /* SetID */
        enumSetDef0_Normal.count(3); /* count */
        enumSetDef0_Normal.entries(enumSetDefEntries);
        enumSetDef0_Verbose.setId(0); /* SetID */
        enumSetDef0_Verbose.count(4); /* count */
        enumSetDef0_Verbose.entries(enumSetDefEntries);
    }

    private class RippleDefintion
    {
        final Buffer rippleAcronym = CodecFactory.createBuffer();
        int rippleFid;
        RippleDefintion next;
    }

    final int c_MfeedError = -2;
    final String c_defaultVersion = "";
    
    @Override
    public void clear()
    {
        _isInitialized = false;
    }
    
    @Override
    public DictionaryEntry entry(int fieldId)
    {
        if (_entriesArray == null)
            return null;

        return (fieldId - MIN_FID < MAX_FID - MIN_FID + 1) ? _entriesArray[fieldId - MIN_FID] : null;
    }

    @Override
    public EnumType entryEnumType(DictionaryEntry entryInt, Enum value)
    {
        DictionaryEntryImpl entry = (DictionaryEntryImpl)entryInt;
        return ((entry._enumTypeTable != null) &&
                (value.toInt() <= entry._enumTypeTable.maxValue())) ? entry._enumTypeTable.enumTypes()[value.toInt()] : null;
    }
    
    @Override
    public int loadFieldDictionary(String filename, Error error)
    {
        int lengthRead = 0;
        DictionaryEntryImpl newDictEntry;
        int fidNum;
        int lineNum = 0;
        int rippleFid = 0;
        RippleDefintion undefinedRipples = null;
        int tmpRwfType;

        try
        {
            _lastPosition = 0;
            rippleAcronym.clear();

            if (filename == null)
            {
                setError(error, "NULL Filename pointer.");
                return CodecReturnCodes.FAILURE;
            }

            _fieldDictFile = new File(filename);
            if (!_fieldDictFile.exists())
            {
                setError(error, "Can't open file: " + filename);
                return CodecReturnCodes.FAILURE;
            }

            if (!_isInitialized && initDictionary(error) != CodecReturnCodes.SUCCESS)
                return CodecReturnCodes.FAILURE;

            _fieldDictFileLine = new char[(int)_fieldDictFile.length()];
            _fileInput = new FileReader(_fieldDictFile);
            lengthRead = _fileInput.read(_fieldDictFileLine, 0, _fieldDictFileLine.length);
            while (_lastPosition < lengthRead - 1)
            {
                lineNum++;

                findLineStart(_fieldDictFileLine);
                if (_lineStartPosition >= lengthRead - 1)
                {
                    break;
                }

                if (_fieldDictFileLine[_lastPosition + 0] == '!' &&
                    _fieldDictFileLine[_lastPosition + 1] == 't' &&
                    _fieldDictFileLine[_lastPosition + 2] == 'a' &&
                    _fieldDictFileLine[_lastPosition + 3] == 'g')
                {
                    /* Tags */
                    if (copyDictionaryTag(tagName(_fieldDictFileLine), tagValue(_fieldDictFileLine), Dictionary.Types.FIELD_DEFINITIONS, error)
                            != CodecReturnCodes.SUCCESS)
                    {
                        _fileInput.close();
                        return CodecReturnCodes.FAILURE;
                    }
                    continue;
                }

                if (_fieldDictFileLine[_lastPosition + 0] == '!')
                {
                    nextLine(_fieldDictFileLine);
                    continue;
                }

                newDictEntry = new DictionaryEntryImpl();

                /* ACRONYM */
                ((BufferImpl)newDictEntry._acronym).data_internal(acronym(_fieldDictFileLine));
                if (newDictEntry._acronym.length() == 0)
                {
                    setError(error, "Cannot find Acronym (Line=" + lineNum + ").");
                    return CodecReturnCodes.FAILURE;
                }

                /* DDE ACRONYM */
                ((BufferImpl)newDictEntry._ddeAcronym).data_internal(ddeAcronym(_fieldDictFileLine));
                if (newDictEntry._ddeAcronym.length() == 0)
                {
                    setError(error, "Cannot find DDE Acronym (Line=" + lineNum + ").");
                    return CodecReturnCodes.FAILURE;
                }

                /* FID */
                fidNum = fid(_fieldDictFileLine);
                if ((fidNum < MIN_FID) || (fidNum > MAX_FID))
                {
                    setError(error, "Illegal fid number " + fidNum + " (Line=" + lineNum + ").");
                    return CodecReturnCodes.FAILURE;
                }
                newDictEntry._fid = fidNum;

                if (fidNum < _minFid)
                    _minFid = fidNum;
                else if (fidNum > _maxFid)
                    _maxFid = fidNum;

                /* RIPPLES TO */
                if (rippleAcronym.length() > 0)
                {
                    if (rippleAcronym.equals(newDictEntry._acronym))
                    {
                        _entriesArray[rippleFid - MIN_FID]._rippleToField = fidNum;
                        rippleAcronym.clear();
                        rippleFid = 0;
                    }
                }

                int ripplesToPos = ripplesToPosition(_fieldDictFileLine);

                if (ripplesToPos < 14)
                {
                    setError(error, "Cannot find Ripples To (Line=" + lineNum + ").");
                    return CodecReturnCodes.FAILURE;
                }

                /* Initialize to zero since will be filled in later, if exists */
                newDictEntry._rippleToField = 0;

                if (!(_fieldDictFileLine[ripplesToPos] == 'N' &&
                        _fieldDictFileLine[ripplesToPos + 1] == 'U' &&
                        _fieldDictFileLine[ripplesToPos + 2] == 'L' &&
                        _fieldDictFileLine[ripplesToPos + 3] == 'L'))
                {
                    if (rippleAcronym.length() > 0)
                    {
                        RippleDefintion newDef = new RippleDefintion();
                        ((BufferImpl)newDef.rippleAcronym).data(rippleAcronym.toString());
                        rippleAcronym.clear();

                        newDef.rippleFid = rippleFid;
                        newDef.next = undefinedRipples;
                        undefinedRipples = newDef;

                    }
                    ((BufferImpl)rippleAcronym).data_internal(new String(_fieldDictFileLine, _startPosition, _lastPosition - _startPosition));
                    rippleFid = fidNum;
                }

                /* FIELD TYPE */
                findFieldTypeStr(_fieldDictFileLine);
                newDictEntry._fieldType = fieldType(_fieldDictFileLine);
                if (newDictEntry._fieldType == c_MfeedError)
                {
                    setError(error, "Unknown Field Type '" + new String(_fieldDictFileLine, _startPosition, _lastPosition - _startPosition)
                            + "' (Line=" + lineNum + ").");
                    return CodecReturnCodes.FAILURE;
                }

                /* LENGTH */
                newDictEntry._length = length(_fieldDictFileLine);
                if (newDictEntry._length < 0)
                {
                    setError(error, "Cannot find Length (Line=" + lineNum + ").");
                    return CodecReturnCodes.FAILURE;
                }

                /* LENGTH ( ENUM ) */
                newDictEntry._enumLength = enumLength(_fieldDictFileLine);
                if (newDictEntry._enumLength == -1)
                {
                    setError(error, "Cannot find EnumLen (Line=" + lineNum + ").");
                    return CodecReturnCodes.FAILURE;
                }

                /* RWF TYPE */
                findRwfTypeStr(_fieldDictFileLine);
                tmpRwfType = rwfFieldType(_fieldDictFileLine);
                if (tmpRwfType < 0)
                {
                    setError(error, "Illegal Rwf Type '" + new String(_fieldDictFileLine, _startPosition, _lastPosition - _startPosition)
                            + "' (Line=" + lineNum + ").");
                    return CodecReturnCodes.FAILURE;
                }
                newDictEntry._rwfType = tmpRwfType;

                /* RWF LEN */
                newDictEntry._rwfLength = rwfLength(_fieldDictFileLine);
                if (newDictEntry._rwfLength < 0)
                {
                    setError(error, "Cannot find Rwf Length (Line=" + lineNum + ").");
                    return CodecReturnCodes.FAILURE;
                }

                if (addFieldToDictionary(newDictEntry, error, lineNum) != CodecReturnCodes.SUCCESS)
                    return CodecReturnCodes.FAILURE;
                newDictEntry = null;
            }

            if ((_minFid <= MAX_FID) && (_maxFid >= MIN_FID))
            {
                /* Go through the undefined ripplesTo fields and find */
                while (undefinedRipples != null)
                {
                    RippleDefintion tdef = undefinedRipples;
                    for (int j = _minFid; j <= _maxFid; j++)
                    {
                        if ((_entriesArray[j - MIN_FID] != null) && (tdef.rippleAcronym.equals(_entriesArray[j - MIN_FID]._acronym)))
                        {
                            _entriesArray[tdef.rippleFid - MIN_FID]._rippleToField = j;
                            break;
                        }
                    }
                    undefinedRipples = tdef.next;
                }
            }

            if (_infoFieldVersion.length() == 0) /* Set default if tag not found */
            {
                ((BufferImpl)_infoFieldVersion).data_internal(c_defaultVersion);
            }

            _fileInput.close();
        }
        catch (ArrayIndexOutOfBoundsException e)
        {
            setError(error, "ArrayIndexOutOfBoundsException");

            return CodecReturnCodes.FAILURE;
        }
        catch (Exception e)
        {
            setError(error, e.getMessage());

            return CodecReturnCodes.FAILURE;
        }

        return CodecReturnCodes.SUCCESS;
    }
    
    @Override
    public int loadEnumTypeDictionary(String filename, Error error)
    {
        int lengthRead = 0;
        int lineNum = 0;
        int value = 0;
        boolean success;
        int fidsCount = 0;
        int maxValue = 0;

        try
        {
            _lastPosition = 0;

            if (filename == null)
            {
                setError(error, "NULL Filename pointer.");
                return CodecReturnCodes.FAILURE;
            }

            _enumTypeDefFile = new File(filename);
            if (!_enumTypeDefFile.exists())
            {
                setError(error, "Can't open file: " + filename);
                return CodecReturnCodes.FAILURE;
            }

            if (!_isInitialized && initDictionary(error) != CodecReturnCodes.SUCCESS)
                return CodecReturnCodes.FAILURE;

            _enumTypeDefFileLine = new char[(int)_enumTypeDefFile.length()];
            _fileInput = new FileReader(_enumTypeDefFile);
            lengthRead = _fileInput.read(_enumTypeDefFileLine, 0, _enumTypeDefFileLine.length);
            while (_lastPosition < lengthRead - 1)
            {
                lineNum++;

                findLineStart(_enumTypeDefFileLine);
                if (_lineStartPosition > lengthRead - 1)
                {
                    break;
                }

                if (_enumTypeDefFileLine[_lastPosition + 0] == '!' &&
                        _enumTypeDefFileLine[_lastPosition + 1] == 't' &&
                        _enumTypeDefFileLine[_lastPosition + 2] == 'a' &&
                        _enumTypeDefFileLine[_lastPosition + 3] == 'g')
                {
                    /* Tags */
                    if (lengthRead < 14)
                    {
                        continue;
                    }

                    if (copyDictionaryTag(tagName(_enumTypeDefFileLine), tagValue(_enumTypeDefFileLine), Dictionary.Types.ENUM_TABLES, error)
                            != CodecReturnCodes.SUCCESS)
                    {
                        _fileInput.close();
                        return CodecReturnCodes.FAILURE;
                    }
                    continue;
                }

                if (_enumTypeDefFileLine[_lastPosition + 0] == '!')
                {
                    nextLine(_enumTypeDefFileLine);
                    continue;
                }

                /* Build a list of Fids. Once finished, make sure the fields point to the parsed enum table.
                 * If the field does not exist, create it with UNKNOWN type. */

                if (_enumTypeDefFileLine[0] == '"')
                {
                    setError(error, "Missing keyword (Line=" + lineNum + ").");
                    return CodecReturnCodes.FAILURE;
                }

                /* VALUE */
                _lastPosition = _lineStartPosition - 1;
                if ((value = intField(_enumTypeDefFileLine)) >= 0)
                {
                    success = true;
                }
                else if (value < MIN_FID)
                {
                    success = false;
                }
                else
                {
                    setError(error, "Enum value cannot be negative");
                    return CodecReturnCodes.FAILURE;
                }

                if (!success)
                {
                    /* Must be an acronym, so still working on fids. */

                    /* If we were working on a value table it's finished */
                    if (_enumTypeArrayCount >= 0)
                    {
                        if (addTableToDictionary(fidsCount, _referenceFidArray, _referenceFidAcronymArray, maxValue, _enumTypeArray,
                                                 _enumTypeArrayCount, error, lineNum) != CodecReturnCodes.SUCCESS)
                            return CodecReturnCodes.FAILURE;

                        maxValue = 0;
                        fidsCount = 0;
                        _enumTypeArrayCount = -1;
                    }

                    /* ACRONYM */
                    ((BufferImpl)_referenceFidAcronymArray[fidsCount]).data_internal(acronym(_enumTypeDefFileLine));

                    /* FID */
                    _referenceFidArray[fidsCount] = fid(_enumTypeDefFileLine);
                    if (_referenceFidArray[fidsCount] < MIN_FID)
                    {
                        setError(error, "Missing FID (Line=" + lineNum + ").");
                        return CodecReturnCodes.FAILURE;
                    }

                    fidsCount++;

                    continue;
                }
                else
                {
                    /* Working on values */
                    _enumTypeArrayCount++;
                    _enumTypeArray[_enumTypeArrayCount] = new EnumTypeImpl();
                }

                /* Parsing Enum Values */

                /* Since most value lists are likely to be 1) short, 2) fairly contiguous, and 3) on the low end
                 * Figure out the max value and then create an appproprately-sized table. */

                _enumTypeArray[_enumTypeArrayCount].value(value);
                if (_enumTypeArray[_enumTypeArrayCount].value() > maxValue)
                {
                    maxValue = _enumTypeArray[_enumTypeArrayCount].value();
                }

                if (!isDisplayHex(_enumTypeDefFileLine)) // display is not hex
                {
                    ((BufferImpl)_enumTypeArray[_enumTypeArrayCount].display()).data_internal(display(_enumTypeDefFileLine));
                    if (_enumTypeArray[_enumTypeArrayCount].display().length() == 0)
                    {
                        setError(error, "Missing DISPLAY (Line=" + lineNum + ").");
                        return CodecReturnCodes.FAILURE;
                    }
                }
                else
                // display is hex
                {
                    /* Special character -- store as binary */
                    int hexLen = hexLength(_enumTypeDefFileLine);

                    if ((hexLen & 0x1) > 0) /* Make sure it's even */
                    {
                        setError(error, "Odd-length hexadecimal input (Line=" + lineNum + ").");
                        return CodecReturnCodes.FAILURE;
                    }

                    if (!setDisplayToHex(_enumTypeDefFileLine, _enumTypeArray[_enumTypeArrayCount].display(), hexLen))
                    {
                        setError(error, "Invalid hexadecimal input (Line=" + lineNum + ").");
                        return CodecReturnCodes.FAILURE;
                    }
                }

                ((BufferImpl)_enumTypeArray[_enumTypeArrayCount].meaning()).data_internal(meaning(_enumTypeDefFileLine));
            }

            /* Finish last table */
            if (_enumTypeArrayCount == -1)
            {
                setError(error, "No EnumTable found (Line=" + lineNum + ").");
                return CodecReturnCodes.FAILURE;
            }

            if (addTableToDictionary(fidsCount, _referenceFidArray, _referenceFidAcronymArray, maxValue, _enumTypeArray,
                                     _enumTypeArrayCount, error, -1) != CodecReturnCodes.SUCCESS)
            {
                return CodecReturnCodes.FAILURE;
            }

            _enumTypeArrayCount = -1;
            maxValue = 0;
            fidsCount = 0;

            _fileInput.close();
        }
        catch (Exception e)
        {
            return CodecReturnCodes.FAILURE;
        }

        return CodecReturnCodes.SUCCESS;
    }

    private void setError(Error error, String errorStr)
    {
        if (error != null)
        {
            error.channel(null);
            error.errorId(CodecReturnCodes.FAILURE);
            error.sysError(0);
            error.text(errorStr);
        }
    }
    
    int initDictionary(Error error)
    {
        assert !_isInitialized : "Dictionary already initialized";

        _infoFieldFilename.clear();
        _infoFieldDesc.clear();
        _infoFieldVersion.clear();
        _infoFieldBuild.clear();
        _infoFieldDate.clear();

        _infoEnumFilename.clear();
        _infoEnumDate.clear();
        _infoEnumDesc.clear();
        _infoEnumRTVersion.clear();
        _infoEnumDTVersion.clear();

        _enumTableCount = 0;
        _numberOfEntries = 0;
        _infoDictionaryId = 0;

        _entriesArray = new DictionaryEntryImpl[MAX_FID - MIN_FID + 1];
        _minFid = MAX_FID + 1;
        _maxFid = MIN_FID - 1;

        /* The range of fids is a practical limit for the table, since no field can use more than one table. */
        _enumTables = new EnumTypeTableImpl[ENUM_TABLE_MAX_COUNT];

        _isInitialized = true;

        return CodecReturnCodes.SUCCESS;
    }

    /* Handle dictionary tags.
     * The logic is put here so the file-loading and wire-decoding versions can be kept close to each other. */
    int decodeDictionaryTag(DecodeIterator iter, ElementEntryImpl element, int type, Error error)
    {
        int ret;

        tempUInt.clear();
        tempInt.clear();

        switch (type)
        {
            case Dictionary.Types.FIELD_DEFINITIONS:
                if (element._name.equals(ElementNames.DICT_TYPE))
                {
                    if ((ret = Decoders.decodeUInt(iter, tempUInt)) < 0)
                    {
                        setError(error, "DecodeUInt failed - " + ret);
                        return CodecReturnCodes.FAILURE;
                    }

                    if (tempUInt.toLong() != Dictionary.Types.FIELD_DEFINITIONS)
                    {
                        setError(error, "Type '" + tempUInt.toLong() + "' indicates this is not a field definitions dictionary.");
                        return CodecReturnCodes.FAILURE;
                    }
                }
                else if (element._name.equals(ElementNames.DICTIONARY_ID))
                {
                    /* second element is dictionary id */
                    if ((ret = Decoders.decodeInt(iter, tempInt)) < 0)
                    {
                        setError(error, "DecodeUInt failed - " + ret);
                        return CodecReturnCodes.FAILURE;
                    }
                    if (tempInt.toLong() != 0 && _infoDictionaryId != 0 && tempInt.toLong() != _infoDictionaryId)
                    {
                        setError(error, "DictionaryId mismatch('" + tempInt.toLong() + "' vs. previously found '" + _infoDictionaryId + "').");
                        return CodecReturnCodes.FAILURE;
                    }
                    _infoDictionaryId = (int)tempInt.toLong();
                }
                else if (element._name.equals(ElementNames.DICT_VERSION))
                {
                    ((BufferImpl)_infoFieldVersion).data(element._encodedData.toString());
                }
                break;
            case Dictionary.Types.ENUM_TABLES:
                if (element._name.equals(ElementNames.DICT_TYPE))
                {
                    if ((ret = Decoders.decodeUInt(iter, tempUInt)) < 0)
                    {
                        setError(error, "DecodeUInt failed - " + ret);
                        return CodecReturnCodes.FAILURE;
                    }

                    if (tempUInt.toLong() != Dictionary.Types.ENUM_TABLES)
                    {
                        setError(error, "Type '" + tempUInt.toLong() + "' indicates this is not a set of enum tables .");
                        return CodecReturnCodes.FAILURE;
                    }
                }
                else if (element._name.equals(ElementNames.DICTIONARY_ID))
                {
                    /* second element is dictionary id */
                    if ((ret = Decoders.decodeInt(iter, tempInt)) < 0)
                    {
                        setError(error, "DecodeUInt failed - " + ret);
                        return CodecReturnCodes.FAILURE;
                    }
                    if (tempInt.toLong() != 0 && _infoDictionaryId != 0 && tempInt.toLong() != _infoDictionaryId)
                    {
                        setError(error, "DictionaryId mismatch('" + tempInt.toLong() + "' vs. previously found '" + _infoDictionaryId + "').");
                        return CodecReturnCodes.FAILURE;
                    }
                    _infoDictionaryId = (int)tempInt.toLong();
                }
                else if (element._name.equals(ElementNames.ENUM_RT_VERSION))
                {
                    ((BufferImpl)_infoEnumRTVersion).data(element._encodedData.toString());
                }
                else if (element._name.equals(ElementNames.ENUM_DT_VERSION))
                {
                    ((BufferImpl)_infoEnumDTVersion).data(element._encodedData.toString());
                }
                break;
            default:
                assert false : "Invalid Dictionary Type";
                break;
        }

        return CodecReturnCodes.SUCCESS;
    }

    int copyDictionaryTag(String tag, String value, int fieldDefinitions, Error error)
    {
        switch (fieldDefinitions)
        {
            case Dictionary.Types.FIELD_DEFINITIONS:
                if (0 == tag.compareTo(ElementNames.DICT_TYPE.toString()))
                {
                    /* No need to store, just make sure it's correct so we might avoid blowing up later. */
                    if (Integer.parseInt(value) != Dictionary.Types.FIELD_DEFINITIONS)
                    {
                        setError(error, "Type '" + value + "' indicates this is not a field definitions dictionary.");
                        return CodecReturnCodes.FAILURE;
                    }
                }
                else if (0 == tag.compareTo(ElementNames.DICT_VERSION.toString()))
                {
                    if (_infoFieldVersion.length() == 0)
                    {
                        ((BufferImpl)_infoFieldVersion).data_internal(value);
                    }
                }
                else if (0 == tag.compareTo(ElementNames.DICTIONARY_ID.toString()))
                {
                    int id = Integer.parseInt(value);
                    if (id != 0 && _infoDictionaryId != 0 && id != _infoDictionaryId)
                    {
                        setError(error, "DictionaryId mismatch('" + id + "' vs. previously found '" + _infoDictionaryId + "').");
                        return CodecReturnCodes.FAILURE;
                    }
                    _infoDictionaryId = id;
                }

                /* Other tags (not encoded or decoded by this package) */
                else if (0 == tag.compareTo("Filename"))
                {
                    if (_infoFieldFilename.length() == 0)
                    {
                        ((BufferImpl)_infoFieldFilename).data_internal(value);
                    }
                }
                else if (0 == tag.compareTo("Desc"))
                {
                    if (_infoFieldDesc.length() == 0)
                    {
                        ((BufferImpl)_infoFieldDesc).data_internal(value);
                    }
                }
                else if (0 == tag.compareTo("Build"))
                {
                    if (_infoFieldBuild.length() == 0)
                    {
                        ((BufferImpl)_infoFieldBuild).data_internal(value);
                    }
                }
                else if (0 == tag.compareTo("Date"))
                {
                    if (_infoFieldDate.length() == 0)
                    {
                        ((BufferImpl)_infoFieldDate).data_internal(value);
                    }
                }
                /* Ignore other tags */
                break;

            case Dictionary.Types.ENUM_TABLES:
                if (0 == tag.compareTo(ElementNames.DICT_TYPE.toString()))
                {
                    if (Integer.parseInt(value) != Dictionary.Types.ENUM_TABLES)
                    {
                        setError(error, "Type '" + value + "' indicates this is not a set of enum tables.");
                        return CodecReturnCodes.FAILURE;
                    }
                }
                else if (0 == tag.compareTo(ElementNames.DICTIONARY_ID.toString()))
                {
                    int id = Integer.parseInt(value);
                    if (id != 0 && _infoDictionaryId != 0 && id != _infoDictionaryId)
                    {
                        setError(error, "DictionaryId mismatch('" + id + "' vs. previously found '" + _infoDictionaryId + "').");
                        return CodecReturnCodes.FAILURE;
                    }
                }

                /* Other tags (not encoded or decoded by this package) */
                else if (0 == tag.compareTo("Filename"))
                {
                    if (_infoEnumFilename.length() == 0)
                    {
                        ((BufferImpl)_infoEnumFilename).data_internal(value);
                    }
                }
                else if (0 == tag.compareTo("Desc"))
                {
                    if (_infoEnumDesc.length() == 0)
                    {
                        ((BufferImpl)_infoEnumDesc).data_internal(value);
                    }
                }
                else if (0 == tag.compareTo("Date"))
                {
                    if (_infoEnumDate.length() == 0)
                    {
                        ((BufferImpl)_infoEnumDate).data_internal(value);
                    }
                }
                else if (0 == tag.compareTo("RT_Version"))
                {
                    if (_infoEnumRTVersion.length() == 0)
                    {
                        ((BufferImpl)_infoEnumRTVersion).data_internal(value);
                    }
                }
                else if (0 == tag.compareTo("DT_Version"))
                {
                    if (_infoEnumDTVersion.length() == 0)
                    {
                        ((BufferImpl)_infoEnumDTVersion).data_internal(value);
                    }
                }
                /* Ignore other tags */
                break;

            default:
                assert false : "Invalid Dictionary Type";
                break;
        }
        return CodecReturnCodes.SUCCESS;
    }
    
    int fieldType(char[] fileData)
    {
        if (compareTo(fileData, "INTEGER"))
            return MfFieldTypes.INTEGER;
        else if (compareTo(fileData, "ALPHANUMERIC"))
            return MfFieldTypes.ALPHANUMERIC;
        else if (compareTo(fileData, "ENUMERATED"))
            return MfFieldTypes.ENUMERATED;
        else if (compareTo(fileData, "TIME_SECONDS"))
            return MfFieldTypes.TIME_SECONDS;        
        else if (compareTo(fileData, "TIME"))
            return MfFieldTypes.TIME;
        else if (compareTo(fileData, "PRICE"))
            return MfFieldTypes.PRICE;
        else if (compareTo(fileData, "DATE"))
            return MfFieldTypes.DATE;
        else if (compareTo(fileData, "BINARY"))
            return MfFieldTypes.BINARY;
        else if (compareTo(fileData, "NONE"))
            return MfFieldTypes.NONE;
        return c_MfeedError;
    }

    int rwfFieldType(char[] fileData)
    {
        if (compareTo(fileData, "UINT"))
            return DataTypes.UINT;
        else if (compareTo(fileData, "INT"))
            return DataTypes.INT;
        else if (compareTo(fileData, "REAL"))
            return DataTypes.REAL;
        else if (compareTo(fileData, "FLOAT"))
            return DataTypes.FLOAT;
        else if (compareTo(fileData, "DOUBLE"))
            return DataTypes.DOUBLE;
        else if (compareTo(fileData, "DATETIME"))
            return DataTypes.DATETIME;
        else if (compareTo(fileData, "DATE_TIME"))
            return DataTypes.DATETIME;
        else if (compareTo(fileData, "DATE"))
            return DataTypes.DATE;
        else if (compareTo(fileData, "TIME"))
            return DataTypes.TIME;
        else if (compareTo(fileData, "QOS"))
            return DataTypes.QOS;
        else if (compareTo(fileData, "STATE"))
            return DataTypes.STATE;
        else if (compareTo(fileData, "STATUS"))
            return DataTypes.STATE;
        else if (compareTo(fileData, "ENUM"))
            return DataTypes.ENUM;
        else if (compareTo(fileData, "ARRAY"))
            return DataTypes.ARRAY;
        else if (compareTo(fileData, "BUFFER"))
            return DataTypes.BUFFER;
        else if (compareTo(fileData, "ASCII_STRING"))
            return DataTypes.ASCII_STRING;
        else if (compareTo(fileData, "UTF8_STRING"))
            return DataTypes.UTF8_STRING;
        else if (compareTo(fileData, "RMTES_STRING"))
            return DataTypes.RMTES_STRING;
        else if (compareTo(fileData, "VECTOR"))
            return DataTypes.VECTOR;
        else if (compareTo(fileData, "MAP"))
            return DataTypes.MAP;
        else if (compareTo(fileData, "SERIES"))
            return DataTypes.SERIES;
        else if (compareTo(fileData, "FIELD_LIST"))
            return DataTypes.FIELD_LIST;
        else if (compareTo(fileData, "FILTER_LIST"))
            return DataTypes.FILTER_LIST;
        else if (compareTo(fileData, "ELEMENT_LIST"))
            return DataTypes.ELEMENT_LIST;
        else if (compareTo(fileData, "ELEM_LIST"))
            return DataTypes.ELEMENT_LIST;
        else if (compareTo(fileData, "XML"))
            return DataTypes.XML;
        else if (compareTo(fileData, "ANSI_PAGE"))
            return DataTypes.ANSI_PAGE;
        else if (compareTo(fileData, "OPAQUE"))
            return DataTypes.OPAQUE;
        else if (compareTo(fileData, "MSG"))
            return DataTypes.MSG;

        return -1;
    }
    
    /* Adds field information to a dictionary entry.
     * Maintains a enumeration table reference if one is found.
     * Callers should not use the entry pointer afterwards -- if the entry is copied rather than used the pointer will be freed. */
    int addFieldToDictionary(DictionaryEntry entryInt, Error error, int lineNum)
    {
        DictionaryEntryImpl entry = (DictionaryEntryImpl)entryInt;
        int fidNum = entry._fid;

        /* fid 0 is reserved, & type cannot be UNKNOWN */
        if (entry._fid == 0)
        {
            if (lineNum > 0)
            {
                setError(error, "fid 0 is reserved (Line=" + lineNum + ").");
                return CodecReturnCodes.FAILURE;
            }
            else
            {
                setError(error, "fid 0 is reserved.");
                return CodecReturnCodes.FAILURE;
            }
        }
        else if (entry._rwfType == DataTypes.UNKNOWN)
        {
            if (lineNum > 0)
            {
                setError(error, "Invalid rwfType for fid " + entry._fid + " (Line=" + lineNum + ").");
                return CodecReturnCodes.FAILURE;
            }
            else
            {
                setError(error, "Invalid rwfType for fid " + entry._fid + ".");
                return CodecReturnCodes.FAILURE;
            }
        }

        if (_entriesArray[fidNum - MIN_FID] != null)
        {
            if (_entriesArray[fidNum - MIN_FID]._rwfType != DataTypes.UNKNOWN)
            {
                if (lineNum > 0)
                    setError(error, "Duplicate definition for fid " + fidNum + " (Line=" + lineNum + ").");
                else
                    setError(error, "Duplicate definition for fid " + fidNum + ".");
                return CodecReturnCodes.FAILURE;
            }
            else
            {
                /* Entry exists because it was loaded from an enumType def. Copy the fieldDict-related info. */
                if (copyEntryFieldDictInfo(_entriesArray[fidNum - MIN_FID], entry, error) != CodecReturnCodes.SUCCESS)
                    return CodecReturnCodes.FAILURE;
            }
        }
        else
        {
            entry._enumTypeTable = null;
            _entriesArray[fidNum - MIN_FID] = entry;
        }

        _numberOfEntries++;
        if (entry._fid > _maxFid)
            _maxFid = entry._fid;
        if (entry._fid < _minFid)
            _minFid = entry._fid;

        return CodecReturnCodes.SUCCESS;
    }
    
    /* Copies FieldDictionary-related information between entries.
     * Used for entries that were already initialized by the enumType dictionary. */
    int copyEntryFieldDictInfo(DictionaryEntry oEntryInt, DictionaryEntry iEntryInt, Error error)
    {
        DictionaryEntryImpl oEntry = (DictionaryEntryImpl)oEntryInt;
        DictionaryEntryImpl iEntry = (DictionaryEntryImpl)iEntryInt;
        /* oEntry has the enumType info. iEntry has the field dictionary info. */
        assert oEntry._rwfType == DataTypes.UNKNOWN : "Invalid Type";

        /* Match the acronym if present(for enum type dictionaries the files contain them, but domain-modeled messages do not). */
        if (oEntry._acronym.length() > 0)
        {
            if (oEntry._acronym.equals(iEntry._acronym) == false)
            {
                setError(error, "Acronym mismatch \"" + oEntry._acronym.toString() + "\" and \"" + iEntry._acronym.toString()
                        + "\" between Field Dictionary and Enum Type Dictionary");
                return CodecReturnCodes.FAILURE;
            }
        }
        else
        {
            ((BufferImpl)oEntry._acronym).data(iEntry._acronym.toString());
        }

        ((BufferImpl)oEntry._ddeAcronym).data(iEntry._ddeAcronym.toString());
        oEntry._enumLength = iEntry._enumLength;
        oEntry._fid = iEntry._fid;
        oEntry._fieldType = iEntry._fieldType;
        oEntry._length = iEntry._length;
        oEntry._rippleToField = iEntry._rippleToField;
        oEntry._rwfLength = iEntry._rwfLength;
        oEntry._rwfType = iEntry._rwfType;

        return CodecReturnCodes.SUCCESS;
    }
    
    int addTableToDictionary(int fidsCount, int[] fidArray, Buffer[] fidAcronymArray, int maxValue,
            EnumType[] enumTypeArray, int enumTypeArrayCount, Error error, int lineNum)
    {
        EnumTypeTable table;

        if (_enumTableCount == ENUM_TABLE_MAX_COUNT) /* Unlikely. */
        {
            setError(error, "Cannot add more tables to this dictionary.");
            return CodecReturnCodes.FAILURE;
        }

        if (fidsCount == 0)
        {
            if (lineNum > 0)
                setError(error, "No referencing FIDs found before enum table (Line=" + lineNum + ").");
            else
                setError(error, "No referencing FIDs found before enum table.");
            return CodecReturnCodes.FAILURE;
        }

        table = new EnumTypeTableImpl();
        ((EnumTypeTableImpl)table).maxValue(maxValue);
        ((EnumTypeTableImpl)table).enumTypes(new EnumType[maxValue + 1]);

        /* Create table and add it to dictionary */
        for (int i = 0; i <= enumTypeArrayCount; i++)
        {
            int value = enumTypeArray[i].value();
            assert enumTypeArray[enumTypeArrayCount].value() <= maxValue : "Invalid content";

            if (table.enumTypes()[value] != null)
            {
                setError(error, "Enum type table has Duplicate value: \"" + value + "\"");
                return CodecReturnCodes.FAILURE;
            }

            table.enumTypes()[value] = enumTypeArray[i];
        }

        ((EnumTypeTableImpl)table).fidReferences(new int[fidsCount]);
        ((EnumTypeTableImpl)table).fidReferenceCount(fidsCount);

        /* Point all referencing fields at it */
        for (int i = 0; i < fidsCount; i++)
        {
            if (addFieldTableReferenceToDictionary(fidArray[i], fidAcronymArray[i], table, error) != CodecReturnCodes.SUCCESS)
            {
                return CodecReturnCodes.FAILURE;
            }

            table.fidReferences()[i] = fidArray[i];
        }

        _enumTables[_enumTableCount++] = table;
        return CodecReturnCodes.SUCCESS;
    }

    /* Adds an enum table refrence to a dictionary entry
     * If the entry does not exist, a placeholder will be created */
    int addFieldTableReferenceToDictionary(int fid, Buffer fidAcronym, EnumTypeTable table, Error error)
    {
        DictionaryEntryImpl entry = _entriesArray[fid - MIN_FID];

        if (entry == null)
        {
            /* No field exists for this yet, so create one for purposes of referencing the table.
             * It's marked with type UNKNOWN and does not officially exist until the corresponding field is loaded (and finds this reference). */
            entry = new DictionaryEntryImpl();
            if (fidAcronym.length() > 0 /* Won't be present in decoded payload */)
            {
                ((BufferImpl)entry._acronym).copyReferences(fidAcronym);
            }
            entry._fid = fid;
            entry._rwfType = DataTypes.UNKNOWN;
            entry._enumTypeTable = table;

            _entriesArray[fid - MIN_FID] = entry;
        }
        else
        {

            if (fidAcronym.length() > 0 && (entry._acronym.equals(fidAcronym) == false))
            {
                setError(error, "Acronym mismatch \"" + entry._acronym.toString() + "\" and \"" + fidAcronym.toString()
                        + "\" between Field Dictionary and Enum Type Dictionary");
                return CodecReturnCodes.FAILURE;
            }

            /* Already exists, just point the field to the table. */
            if (entry._enumTypeTable != null)
            {
                setError(error, "FieldId " + fid + " has duplicate Enum Table reference");
                return CodecReturnCodes.FAILURE;
            }

            entry._enumTypeTable = table;
        }

        return CodecReturnCodes.SUCCESS;
    }
    
    @Override
    public int extractDictionaryType(DecodeIterator iterInt, Int dictionaryType, Error error)
    {
        DecodeIteratorImpl iter = (DecodeIteratorImpl)iterInt;
        int ret = 0;

        series.clear();
        elemList.clear();
        elemEntry.clear();
        tempUInt.clear();
        tempBuffer.clear();
        tempDecIter.clear();

        ((BufferImpl)tempBuffer).data_internal(iter._buffer.data(), iter._curBufPos,
                                               (iter._buffer.length() - iter._curBufPos));
        tempDecIter.setBufferAndRWFVersion(tempBuffer, iter._reader.majorVersion(), iter._reader.minorVersion());

        if ((ret = series.decode(tempDecIter)) < 0)
        {
            setError(error, "DecodeSeries failed " + ret);
            return CodecReturnCodes.FAILURE;
        }

        /* if this is not an element list, we should fail */
        if (series._containerType != DataTypes.ELEMENT_LIST)
        {
            setError(error, "Invalid container type of " + series._containerType + "; expecting " + DataTypes.ELEMENT_LIST + " (ELEMENT_LIST)");
            return CodecReturnCodes.FAILURE;
        }

        /* decode summary data */
        if (series.checkHasSummaryData())
        {
            if ((ret = elemList.decode(tempDecIter, null)) < 0)
            {
                setError(error, "DecodeElementList failed " + ret);
                return CodecReturnCodes.FAILURE;
            }

            while ((ret = elemEntry.decode(tempDecIter)) != CodecReturnCodes.END_OF_CONTAINER)
            {
                if (ret >= CodecReturnCodes.SUCCESS)
                {
                    if (elemEntry._name.equals(ElementNames.TYPE))
                    {
                        ret = Decoders.decodeUInt(tempDecIter, tempUInt);
                        if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
                        {
                            setError(error, "DecodeUInt failed " + ret);
                            return CodecReturnCodes.FAILURE;
                        }
                        dictionaryType.value(tempUInt.toLong());
                        break;
                    }
                }
                else
                {
                    setError(error, "DecodeElementEntry failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }
            }
        }
        else
        {
            setError(error, "No summary data present on message!");
            return CodecReturnCodes.FAILURE;
        }

        return CodecReturnCodes.SUCCESS;
    }
    
    @Override
    public int encodeFieldDictionary(EncodeIterator iter, Int currentFid, int verbosity, Error error)
    {
        int ret;
        long curFid = currentFid.toLong();

        if (!_isInitialized)
        {
            setError(error, "Dictionary not initialized");
            return CodecReturnCodes.FAILURE;
        }

        series.clear();
        setDb.clear();
        if (verbosity >= Dictionary.VerbosityValues.NORMAL)
        {
            setDb.definitions()[0].count(setDef0_Normal.count());
            setDb.definitions()[0].entries(setDef0_Normal.entries());
            setDb.definitions()[0].setId(setDef0_Normal.setId());
        }
        else
        {
            setDb.definitions()[0].count(setDef0_Minimal.count());
            setDb.definitions()[0].entries(setDef0_Minimal.entries());
            setDb.definitions()[0].setId(setDef0_Minimal.setId());
        }

        /* Set the data format */
        series.containerType(DataTypes.ELEMENT_LIST);

        /* Don't encode set definitions for info */
        if (verbosity > Dictionary.VerbosityValues.INFO)
            series.applyHasSetDefs();

        /* If first packet, then send hint and summary data */
        if (curFid <= _minFid)
        {
            /* Set the total count hint if exists */
            if (_numberOfEntries > 0)
            {
                series.applyHasTotalCountHint();
                series.totalCountHint(_numberOfEntries);
            }
            series.applyHasSummaryData();
        }

        if ((ret = series.encodeInit(iter, 0, 0)) < 0)
        {
            setError(error, "encodeSeriesInit failed " + ret);
            return CodecReturnCodes.FAILURE;
        }

        /* Don't encode set definitions for info */
        if (verbosity > Dictionary.VerbosityValues.INFO)
        {
            if ((ret = setDb.encode(iter)) < 0)
            {
                setError(error, "encodeLocalElementSetDefDb failed " + ret);
                return CodecReturnCodes.FAILURE;
            }

            if ((ret = series.encodeSetDefsComplete(iter, true)) < 0)
            {
                setError(error, "encodeSeriesSetDefsComplete failed " + ret);
                return CodecReturnCodes.FAILURE;
            }
        }

        /* If first packet, encode the summary data */
        if (curFid <= _minFid)
        {
            if ((ret = encodeDataDictSummaryData(iter, Dictionary.Types.FIELD_DEFINITIONS, series, error)) < 0)
                return CodecReturnCodes.FAILURE;
        }

        /* Don't encode actual entries for info */
        if (verbosity > Dictionary.VerbosityValues.INFO)
        {
            while (curFid <= _maxFid)
            {
                /* Entries with type UNKNOWN were loaded from an enumtype.
                 * Don't send them since they aren't officially defined yet. */
                if (_entriesArray[(int)curFid - MIN_FID] != null && _entriesArray[(int)curFid - MIN_FID].rwfType() != DataTypes.UNKNOWN)
                {
                    if ((ret = encodeDataDictEntry(iter, _entriesArray[(int)curFid - MIN_FID], verbosity, error, setDb)) < 0)
                        return CodecReturnCodes.FAILURE;

                    /* If we have filled the buffer, then complete */
                    if (ret == CodecReturnCodes.DICT_PART_ENCODED)
                        break;
                }
                (curFid)++;
            }
        }

        if ((ret = series.encodeComplete(iter, true)) < 0)
        {
            setError(error, "encodeSeriesComplete failed " + ret);
            return CodecReturnCodes.FAILURE;
        }

        currentFid.value(curFid);
        return (curFid > _maxFid ? CodecReturnCodes.SUCCESS : CodecReturnCodes.DICT_PART_ENCODED);
    }

    @Override
    public int decodeFieldDictionary(DecodeIterator iter, int verbosity, Error error)
    {
        int ret;
        @SuppressWarnings("unused")
        int fid = 0;
        DictionaryEntryImpl newDictEntry;

        if (!_isInitialized && initDictionary(error) != CodecReturnCodes.SUCCESS)
            return CodecReturnCodes.FAILURE;

        series.clear();
        elemList.clear();
        elemEntry.clear();
        seriesEntry.clear();
        tempInt.clear();
        tempUInt.clear();

        if (series.decode(iter) < 0)
            return CodecReturnCodes.FAILURE;

        /* if this is not an element list, we should fail for now */
        if (series._containerType != DataTypes.ELEMENT_LIST)
            return CodecReturnCodes.FAILURE;

        /* decode summary data */
        if (series.checkHasSummaryData())
        {
            /* decode summary data here */

            /* since we own dictionary, lets assume that we create memory here - 
             * they should only delete this with our delete dictionary method */

            if (elemList.decode(iter, null) < 0)
                return CodecReturnCodes.FAILURE;

            while ((ret = elemEntry.decode(iter)) != CodecReturnCodes.END_OF_CONTAINER)
            {
                if (ret < 0)
                    return ret;

                if (decodeDictionaryTag(iter, elemEntry, Dictionary.Types.FIELD_DEFINITIONS, error) != CodecReturnCodes.SUCCESS)
                    return CodecReturnCodes.FAILURE;
            }
        }

        if (series.checkHasSetDefs())
        {
            setDb.clear();
            if ((ret = setDb.decode(iter)) < 0)
            {
                setError(error, "DecodeLocalElementSetDefDb failed - " + ret);
                return ret;
            }
        }

        while ((ret = seriesEntry.decode(iter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            /* reinitialize fid */
            fid = MAX_FID + 1;

            if (ret < 0)
                return ret;

            /* decode element list here */
            if ((ret = elemList.decode(iter, setDb)) < 0)
                return ret;

            newDictEntry = new DictionaryEntryImpl();

            while ((ret = elemEntry.decode(iter)) != CodecReturnCodes.END_OF_CONTAINER)
            {
                if (ret < 0)
                    return CodecReturnCodes.FAILURE;

                if (elemEntry._name.equals(ElementNames.FIELD_NAME))
                {
                    if (elemEntry._dataType != DataTypes.ASCII_STRING)
                    {
                        setError(error, "Cannot decode '" + ElementNames.FIELD_NAME.toString() + "' element.");
                        return CodecReturnCodes.FAILURE;
                    }

                    ((BufferImpl)newDictEntry._acronym).data(elemEntry._encodedData.toString());
                }
                else if (elemEntry._name.equals(ElementNames.FIELD_ID))
                {
                    if (elemEntry._dataType != DataTypes.INT || Decoders.decodeInt(iter, tempInt) < 0)
                    {
                        setError(error, "Cannot decode '" + ElementNames.FIELD_ID.toString() + "' element.");
                        return CodecReturnCodes.FAILURE;
                    }

                    /* now populate fid */
                    newDictEntry._fid = (int)tempInt.toLong();

                    /* do max and min fid stuff */
                    if (newDictEntry._fid > _maxFid)
                        _maxFid = newDictEntry._fid;
                    if (newDictEntry._fid < _minFid)
                        _minFid = newDictEntry._fid;
                }
                else if (elemEntry._name.equals(ElementNames.FIELD_RIPPLETO))
                {
                    if (elemEntry._dataType != DataTypes.INT || Decoders.decodeInt(iter, tempInt) < 0)
                    {
                        setError(error, "Cannot decode '" + ElementNames.FIELD_RIPPLETO.toString() + "' element.");
                        return CodecReturnCodes.FAILURE;
                    }
                    newDictEntry._rippleToField = (int)tempInt.toLong();
                }
                else if (elemEntry._name.equals(ElementNames.FIELD_TYPE))
                {
                    if (elemEntry._dataType != DataTypes.INT || Decoders.decodeInt(iter, tempInt) < 0)
                    {
                        setError(error, "Cannot decode '" + ElementNames.FIELD_TYPE.toString() + "' element.");
                        return CodecReturnCodes.FAILURE;
                    }
                    newDictEntry._fieldType = (int)tempInt.toLong();
                }
                else if (elemEntry._name.equals(ElementNames.FIELD_LENGTH))
                {
                    if (elemEntry._dataType != DataTypes.UINT || Decoders.decodeUInt(iter, tempUInt) < 0)
                    {
                        setError(error, "Cannot decode '" + ElementNames.FIELD_LENGTH.toString() + "' element.");
                        return CodecReturnCodes.FAILURE;
                    }
                    newDictEntry._length = (int)tempUInt.toLong();
                }
                else if (elemEntry._name.equals(ElementNames.FIELD_RWFTYPE))
                {
                    if (elemEntry._dataType != DataTypes.UINT || Decoders.decodeUInt(iter, tempUInt) < 0)
                    {
                        setError(error, "Cannot decode '" + ElementNames.FIELD_RWFTYPE.toString() + "' element.");
                        return CodecReturnCodes.FAILURE;
                    }

                    /* Need to do table lookup so legacy types (e.g. INT32/REAL32) are converted. */
                    newDictEntry._rwfType = Decoders.convertToPrimitiveType((int)tempUInt.toLong());
                }
                else if (elemEntry._name.equals(ElementNames.FIELD_RWFLEN))
                {
                    if (elemEntry._dataType != DataTypes.UINT || Decoders.decodeUInt(iter, tempUInt) < 0)
                    {
                        setError(error, "Cannot decode '" + ElementNames.FIELD_RWFLEN.toString() + "' element.");
                        return CodecReturnCodes.FAILURE;
                    }
                    newDictEntry._rwfLength = (int)tempUInt.toLong();
                }
                else if (verbosity >= Dictionary.VerbosityValues.NORMAL)/* optional elements depending on verbosity */
                {
                    if (elemEntry._name.equals(ElementNames.FIELD_ENUMLENGTH))
                    {
                        if (elemEntry._dataType != DataTypes.UINT || Decoders.decodeUInt(iter, tempUInt) < 0)
                        {
                            setError(error, "Cannot decode '" + ElementNames.FIELD_ENUMLENGTH.toString() + "' element.");
                            return CodecReturnCodes.FAILURE;
                        }
                        newDictEntry._enumLength = (int)tempUInt.toLong();
                    }
                    else if (elemEntry._name.equals(ElementNames.FIELD_LONGNAME))
                    {
                        if (elemEntry._dataType != DataTypes.ASCII_STRING)
                        {
                            setError(error, "Cannot decode '" + ElementNames.FIELD_LONGNAME.toString() + "' element.");
                            return CodecReturnCodes.FAILURE;
                        }
                        ((BufferImpl)newDictEntry._ddeAcronym).data(elemEntry._encodedData.toString());
                    }
                }
            }

            if (addFieldToDictionary(newDictEntry, error, -1) != CodecReturnCodes.SUCCESS)
                return CodecReturnCodes.FAILURE;
            newDictEntry = null;
        }

        return CodecReturnCodes.SUCCESS;
    }
    
    @Override
    public int encodeEnumTypeDictionary(EncodeIterator iter, int verbosity, Error error)
    {
        int ret;

        if (!_isInitialized)
        {
            setError(error, "Dictionary not initialized");
            return CodecReturnCodes.FAILURE;
        }

        series.clear();
        seriesEntry.clear();
        elemList.clear();
        elemEntry.clear();
        arr.clear();
        arrEntry.clear();
        tempInt.clear();
        tempEnum.clear();
        setDb.clear();
        setDb.definitions()[0].count(enumSetDef0_Normal.count());
        setDb.definitions()[0].entries(enumSetDef0_Normal.entries());
        setDb.definitions()[0].setId(enumSetDef0_Normal.setId());

        /* Set the data format */
        series.containerType(DataTypes.ELEMENT_LIST);
        series.flags(SeriesFlags.HAS_SUMMARY_DATA);

        /* Don't encode set definitions for info */
        if (verbosity > Dictionary.VerbosityValues.INFO)
            series.applyHasSetDefs();

        /* If first packet, then send hint and summary data */
        if ((ret = series.encodeInit(iter, 0, 0)) < 0)
        {
            setError(error, "encodeSeriesInit failed " + ret);
            return CodecReturnCodes.FAILURE;
        }

        if (verbosity > Dictionary.VerbosityValues.INFO)
        {
            /* Encode set definition */
            if ((ret = setDb.encode(iter)) < 0)
            {
                setError(error, "encodeLocalElementSetDefDb failed " + ret);
                return CodecReturnCodes.FAILURE;
            }

            if ((ret = series.encodeSetDefsComplete(iter, true)) < 0)
            {
                setError(error, "encodeSeriesSetDefsComplete failed " + ret);
                return CodecReturnCodes.FAILURE;
            }
        }

        /* Summary data */
        if ((ret = encodeDataDictSummaryData(iter, Dictionary.Types.ENUM_TABLES, series, error)) < 0)
            return CodecReturnCodes.FAILURE;

        /* Don't encode actual entries for info */
        if (verbosity > Dictionary.VerbosityValues.INFO)
        {
            for (int i = 0; i < _enumTableCount; ++i)
            {
                /* Encode each table */
                EnumTypeTable table;

                seriesEntry.clear();

                if ((ret = seriesEntry.encodeInit(iter, 0)) < 0)
                {
                    setError(error, "encodeSeriesEntryInit failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }

                elemList.clear();
                elemList.flags(ElementListFlags.HAS_SET_DATA | ElementListFlags.HAS_SET_ID);
                elemList.setId(0);

                if ((ret = elemList.encodeInit(iter, setDb, 0)) < 0)
                {
                    setError(error, "encodeElementListInit failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }

                table = _enumTables[i];

                /* Fids */
                elemEntry.clear();
                elemEntry.dataType(DataTypes.ARRAY);
                elemEntry.name(ElementNames.ENUM_FIDS);
                if ((ret = elemEntry.encodeInit(iter, 0)) < 0)
                {
                    setError(error, "encodeElementEntryInit failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }

                arr.clear();
                arr.itemLength(2);
                arr.primitiveType(DataTypes.INT);
                if ((ret = arr.encodeInit(iter)) < 0)
                {
                    setError(error, "encodeArrayInit failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }

                for (int j = 0; j < table.fidReferenceCount(); ++j)
                {
                    tempInt.value(table.fidReferences()[j]);
                    arrEntry.clear();
                    if ((ret = arrEntry.encode(iter, tempInt)) < 0)
                    {
                        setError(error, "encodeArrayEntry failed " + ret);
                        return CodecReturnCodes.FAILURE;
                    }
                }

                if ((ret = arr.encodeComplete(iter, true)) < 0)
                {
                    setError(error, "encodeArrayComplete failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }

                if ((ret = elemEntry.encodeComplete(iter, true)) < 0)
                {
                    setError(error, "encodeElementEntryComplete failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }

                /* Values */
                elemEntry.clear();
                elemEntry.dataType(DataTypes.ARRAY);
                elemEntry.name(ElementNames.ENUM_VALUE);
                if ((ret = elemEntry.encodeInit(iter, 0)) < 0)
                {
                    setError(error, "encodeElementEntryInit failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }

                arr.clear();
                arr.itemLength(0);
                arr.primitiveType(DataTypes.ENUM);
                if ((ret = arr.encodeInit(iter)) < 0)
                {
                    setError(error, "encodeArrayInit failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }

                for (int j = 0; j <= table.maxValue(); ++j)
                {
                    arrEntry.clear();
                    if (table.enumTypes()[j] != null)
                    {
                        tempEnum.value(table.enumTypes()[j].value());
                        if ((ret = arrEntry.encode(iter, tempEnum)) < 0)
                        {
                            setError(error, "encodeArrayEntry failed " + ret);
                            return CodecReturnCodes.FAILURE;
                        }
                    }
                }

                if ((ret = arr.encodeComplete(iter, true)) < 0)
                {
                    setError(error, "encodeArrayComplete failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }

                if ((ret = elemEntry.encodeComplete(iter, true)) < 0)
                {
                    setError(error, "encodeElementEntryComplete failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }

                /* Display */
                elemEntry.clear();
                elemEntry.dataType(DataTypes.ARRAY);
                elemEntry.name(ElementNames.ENUM_DISPLAY);
                if ((ret = elemEntry.encodeInit(iter, 0)) < 0)
                {
                    setError(error, "encodeElementEntryInit failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }

                arr.clear();
                arr.itemLength(0);
                arr.primitiveType(DataTypes.ASCII_STRING);
                if ((ret = arr.encodeInit(iter)) < 0)
                {
                    setError(error, "encodeArrayInit failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }

                for (int j = 0; j <= table.maxValue(); ++j)
                {
                    arrEntry.clear();
                    if (table.enumTypes()[j] != null && (ret = arrEntry.encode(iter, table.enumTypes()[j].display())) < 0)
                    {
                        setError(error, "encodeArrayEntry failed " + ret);
                        return CodecReturnCodes.FAILURE;
                    }
                }

                if ((ret = arr.encodeComplete(iter, true)) < 0)
                {
                    setError(error, "encodeArrayComplete failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }

                if ((ret = elemEntry.encodeComplete(iter, true)) < 0)
                {
                    setError(error, "encodeElementEntryComplete failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }

                if ((ret = elemList.encodeComplete(iter, true)) < 0)
                {
                    setError(error, "encodeElementListComplete failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }

                if ((ret = seriesEntry.encodeComplete(iter, true)) < 0)
                {
                    setError(error, "encodeSeriesEntryComplete failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }
            }
        }

        if ((ret = series.encodeComplete(iter, true)) < 0)
        {
            setError(error, "encodeSeriesComplete failed " + ret);
            return CodecReturnCodes.FAILURE;
        }

        return CodecReturnCodes.SUCCESS;
    }
    

    
    private int rollbackEnumDictionaryElementList(EncodeIterator iter)
    {
    	int ret;
    	if ((ret = elemList.encodeComplete(iter, false)) < 0 )
    		return ret;
    	if ((ret = seriesEntry.encodeComplete(iter, false)) < 0)
    		return ret;
    	return series.encodeComplete(iter, true);
    }    
    
    private int rollbackEnumDictionaryElementEntry(EncodeIterator iter)
    {
    	int ret;
    	if ((ret = elemEntry.encodeComplete(iter, false)) < 0 )
    		return ret;
    	return rollbackEnumDictionaryElementList(iter);
    }      
    
    private int rollbackEnumDictionaryArray(EncodeIterator iter)
    {
    	int ret;
    	if ((ret = arr.encodeComplete(iter, false)) < 0 )
    		return ret;
    	return rollbackEnumDictionaryElementEntry(iter);
    }      
    
    private int rollbackEnumDictionarySeriesEntry(EncodeIterator iter)
    {
    	int ret;
    	if ((ret = seriesEntry.encodeComplete(iter, false)) < 0 )
    		return ret;
    	return series.encodeComplete(iter, true);
    }    
    
    @Override
    public int encodeEnumTypeDictionaryAsMultiPart(EncodeIterator iter, Int currentEnumTableEntry, int verbosity, Error error)
    {
        int ret;
        int curEnumTableEntry = (int) currentEnumTableEntry.toLong();        

        if (!_isInitialized)
        {
            setError(error, "Dictionary not initialized");
            return CodecReturnCodes.FAILURE;
        }

        series.clear();
        seriesEntry.clear();
        elemList.clear();
        elemEntry.clear();
        arr.clear();
        arrEntry.clear();
        tempInt.clear();
        tempEnum.clear();
        setDb.clear();
        setDb.definitions()[0].count(enumSetDef0_Normal.count());
        setDb.definitions()[0].entries(enumSetDef0_Normal.entries());
        setDb.definitions()[0].setId(enumSetDef0_Normal.setId());

        /* Set the data format */
        series.containerType(DataTypes.ELEMENT_LIST);
        series.flags(SeriesFlags.HAS_SUMMARY_DATA);

        /* Don't encode set definitions for info */
        if (verbosity > Dictionary.VerbosityValues.INFO)
            series.applyHasSetDefs();

        /* If first packet, then send hint and summary data */
        if ((ret = series.encodeInit(iter, 0, 0)) < 0)
        {
            setError(error, "encodeSeriesInit failed " + ret);
            return ret == CodecReturnCodes.BUFFER_TOO_SMALL ? ret : CodecReturnCodes.FAILURE;
        }

        if (verbosity > Dictionary.VerbosityValues.INFO)
        {
            /* Encode set definition */
            if ((ret = setDb.encode(iter)) < 0)
            {
                setError(error, "encodeLocalElementSetDefDb failed " + ret);
                return ret == CodecReturnCodes.BUFFER_TOO_SMALL ? ret : CodecReturnCodes.FAILURE;
            }

            if ((ret = series.encodeSetDefsComplete(iter, true)) < 0)
            {
                setError(error, "encodeSeriesSetDefsComplete failed " + ret);
                return ret == CodecReturnCodes.BUFFER_TOO_SMALL ? ret : CodecReturnCodes.FAILURE;
            }
        }

        /* Summary data */
        if ((ret = encodeDataDictSummaryData(iter, Dictionary.Types.ENUM_TABLES, series, error)) < 0)
            return ret == CodecReturnCodes.BUFFER_TOO_SMALL ? ret : CodecReturnCodes.FAILURE;

        /* Don't encode actual entries for info */
        if (verbosity > Dictionary.VerbosityValues.INFO)
        {
        	
    		/* Need to keep track of the number of the series entry we are encoding, if it is the first series entry
			in the message and we get the RSSL_RET_BUFFER_TOO_SMALL we can not encode partial series entry and 
			we need to fail */
        	int startCount = curEnumTableEntry;        	

            for (; curEnumTableEntry < _enumTableCount; ++curEnumTableEntry)
            {
                /* Encode each table */
                EnumTypeTable table;

                seriesEntry.clear();

                if ((ret = seriesEntry.encodeInit(iter, 0)) == CodecReturnCodes.BUFFER_TOO_SMALL &&
                		curEnumTableEntry > startCount)
                {
                	if ((ret = rollbackEnumDictionarySeriesEntry(iter)) < 0)
                	{
                		setError(error, "encodeSeriesEntryInit failed " + ret);
                    	return CodecReturnCodes.FAILURE;
                	}
                	else
                	{
                		currentEnumTableEntry.value(curEnumTableEntry);
                		return CodecReturnCodes.DICT_PART_ENCODED;
                	}
                }
                else if (ret < 0)
                {
            		setError(error, "encodeSeriesEntryInit failed " + ret);
            		return ret == CodecReturnCodes.BUFFER_TOO_SMALL ? ret : CodecReturnCodes.FAILURE;
                }

                elemList.clear();
                elemList.flags(ElementListFlags.HAS_SET_DATA | ElementListFlags.HAS_SET_ID);
                elemList.setId(0);

                if ((ret = elemList.encodeInit(iter, setDb, 0)) == CodecReturnCodes.BUFFER_TOO_SMALL &&
                		curEnumTableEntry > startCount)
                {
                	if ((ret = rollbackEnumDictionaryElementList(iter)) < 0)
                	{
                		setError(error, "rollbackEnumDictionaryElementList failed " + ret);
                		return CodecReturnCodes.FAILURE;
                	}
                	else
                	{
                		currentEnumTableEntry.value(curEnumTableEntry);
                		return CodecReturnCodes.DICT_PART_ENCODED;
                	}
                }
                else if (ret < 0)
                {
                	setError(error, "encodeElementListInit failed " + ret);
                	return ret == CodecReturnCodes.BUFFER_TOO_SMALL ? ret : CodecReturnCodes.FAILURE;
                }

                table = _enumTables[curEnumTableEntry];

                /* Fids */
                elemEntry.clear();
                elemEntry.dataType(DataTypes.ARRAY);
                elemEntry.name(ElementNames.ENUM_FIDS);
                if ((ret = elemEntry.encodeInit(iter, 0)) == CodecReturnCodes.BUFFER_TOO_SMALL &&
                		curEnumTableEntry > startCount)
                {
                	if ((ret = rollbackEnumDictionaryElementEntry(iter)) < 0)
                	{
                		setError(error, "rollbackEnumDictionaryElementEntry failed " + ret);
                		return CodecReturnCodes.FAILURE;                		
                	}
                	else
                	{
                		currentEnumTableEntry.value(curEnumTableEntry);
                		return CodecReturnCodes.DICT_PART_ENCODED;
                	}
                }
                else if (ret < 0)
                {
                    setError(error, "encodeElementEntryInit failed " + ret);
                    return ret == CodecReturnCodes.BUFFER_TOO_SMALL ? ret : CodecReturnCodes.FAILURE;
                }

                arr.clear();
                arr.itemLength(2);
                arr.primitiveType(DataTypes.INT);
                if ((ret = arr.encodeInit(iter)) == CodecReturnCodes.BUFFER_TOO_SMALL &&
                		curEnumTableEntry > startCount)
                {
                	if ((ret = rollbackEnumDictionaryArray(iter)) < 0)
                	{
                		setError(error, "rollbackEnumDictionaryArray failed " + ret);
                		return CodecReturnCodes.FAILURE;                     		
                	}
                	else
                	{
                		currentEnumTableEntry.value(curEnumTableEntry);
                		return CodecReturnCodes.DICT_PART_ENCODED;
                	}
                }
                else if (ret < 0)
                {
                    setError(error, "encodeArrayInit failed " + ret);
                    return ret == CodecReturnCodes.BUFFER_TOO_SMALL ? ret : CodecReturnCodes.FAILURE;
                }

                for (int j = 0; j < table.fidReferenceCount(); ++j)
                {
                    tempInt.value(table.fidReferences()[j]);
                    arrEntry.clear();
                    if ((ret = arrEntry.encode(iter, tempInt)) == CodecReturnCodes.BUFFER_TOO_SMALL &&
                    		curEnumTableEntry > startCount)
                    {
                    	if ((ret = rollbackEnumDictionaryArray(iter)) < 0)
                    	{
                    		setError(error, "rollbackEnumDictionaryArray failed " + ret);
                    		return CodecReturnCodes.FAILURE;                     		
                    	}
                    	else
                    	{
                    		currentEnumTableEntry.value(curEnumTableEntry);
                    		return CodecReturnCodes.DICT_PART_ENCODED;
                    	}                   	
                    }
                    else if (ret < 0)
                    {
                        setError(error, "encodeArrayEntry failed " + ret);
                        return ret == CodecReturnCodes.BUFFER_TOO_SMALL ? ret : CodecReturnCodes.FAILURE;
                    }
                }

                if ((ret = arr.encodeComplete(iter, true)) < 0)
                {
                    setError(error, "encodeArrayComplete failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }

                if ((ret = elemEntry.encodeComplete(iter, true)) < 0)
                {
                    setError(error, "encodeElementEntryComplete failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }

                /* Values */
                elemEntry.clear();
                elemEntry.dataType(DataTypes.ARRAY);
                elemEntry.name(ElementNames.ENUM_VALUE);
                if ((ret = elemEntry.encodeInit(iter, 0)) == CodecReturnCodes.BUFFER_TOO_SMALL &&
                		curEnumTableEntry > startCount)
                {
                	if ((ret = rollbackEnumDictionaryElementEntry(iter)) < 0)
                	{
                		setError(error, "rollbackEnumDictionaryElementEntry failed " + ret);
                		return CodecReturnCodes.FAILURE;                   		
                	}
                	else
                	{
                		currentEnumTableEntry.value(curEnumTableEntry);
                		return CodecReturnCodes.DICT_PART_ENCODED;
                	}   
                }
                else if (ret < 0)
                {
                    setError(error, "encodeElementEntryInit failed " + ret);
                    return ret == CodecReturnCodes.BUFFER_TOO_SMALL ? ret : CodecReturnCodes.FAILURE;
                }

                arr.clear();
                arr.itemLength(0);
                arr.primitiveType(DataTypes.ENUM);
                if ((ret = arr.encodeInit(iter)) == CodecReturnCodes.BUFFER_TOO_SMALL &&
                		curEnumTableEntry > startCount)
                {
                	if ((ret = rollbackEnumDictionaryArray(iter)) < 0)
                	{
                		setError(error, "rollbackEnumDictionaryArray failed " + ret);
                		return CodecReturnCodes.FAILURE;                 		
                	}
                	else
                	{
                		currentEnumTableEntry.value(curEnumTableEntry);
                		return CodecReturnCodes.DICT_PART_ENCODED;
                	}
                }
                else if (ret < 0)
                {
                    setError(error, "encodeArrayInit failed " + ret);
                    return ret == CodecReturnCodes.BUFFER_TOO_SMALL ? ret : CodecReturnCodes.FAILURE;           	
                }

                for (int j = 0; j <= table.maxValue(); ++j)
                {
                    arrEntry.clear();
                    if (table.enumTypes()[j] != null)
                    {
                        tempEnum.value(table.enumTypes()[j].value());
                        if ((ret = arrEntry.encode(iter, tempEnum)) == CodecReturnCodes.BUFFER_TOO_SMALL &&
                        		curEnumTableEntry > startCount)
                        {
                        	if ((ret = rollbackEnumDictionaryArray(iter)) < 0)
                        	{
                        		setError(error, "rollbackEnumDictionaryArray failed " + ret);
                        		return CodecReturnCodes.FAILURE;                   		
                        	}
                        	else
                        	{
                        		currentEnumTableEntry.value(curEnumTableEntry);
                        		return CodecReturnCodes.DICT_PART_ENCODED;
                        	}
                        }
                        else if (ret < 0)
                        {
                            setError(error, "encodeArrayEntry failed " + ret);
                            return ret == CodecReturnCodes.BUFFER_TOO_SMALL ? ret : CodecReturnCodes.FAILURE;      
                        }
                    }
                }

                if ((ret = arr.encodeComplete(iter, true)) < 0)
                {
                    setError(error, "encodeArrayComplete failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }

                if ((ret = elemEntry.encodeComplete(iter, true)) < 0)
                {
                    setError(error, "encodeElementEntryComplete failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }

                /* Display */
                elemEntry.clear();
                elemEntry.dataType(DataTypes.ARRAY);
                elemEntry.name(ElementNames.ENUM_DISPLAY);
                if ((ret = elemEntry.encodeInit(iter, 0)) == CodecReturnCodes.BUFFER_TOO_SMALL &&
                		curEnumTableEntry > startCount)
                {
                	if ((ret = rollbackEnumDictionaryElementEntry(iter)) < 0)
                	{
                		setError(error, "rollbackEnumDictionaryElementEntry failed " + ret);
                		return CodecReturnCodes.FAILURE;                   		
                	}
                	else
                	{
                		currentEnumTableEntry.value(curEnumTableEntry);
                		return CodecReturnCodes.DICT_PART_ENCODED;
                	}
                }
                else if (ret < 0)
                {
                    setError(error, "encodeElementEntryInit failed " + ret);
                    return ret == CodecReturnCodes.BUFFER_TOO_SMALL ? ret : CodecReturnCodes.FAILURE;     
                }

                arr.clear();
                arr.itemLength(0);
                arr.primitiveType(DataTypes.ASCII_STRING);
                if ((ret = arr.encodeInit(iter)) == CodecReturnCodes.BUFFER_TOO_SMALL &&
                		curEnumTableEntry > startCount)
                {
                	if ((ret = rollbackEnumDictionaryArray(iter)) < 0)
                	{
                		setError(error, "rollbackEnumDictionaryArray failed " + ret);
                		return CodecReturnCodes.FAILURE;                   		
                	}
                	else
                	{
                		currentEnumTableEntry.value(curEnumTableEntry);
                		return CodecReturnCodes.DICT_PART_ENCODED;
                	}                		
                }
            	else if (ret < 0)
            	{
            		setError(error, "encodeArrayInit failed " + ret);
            		return ret == CodecReturnCodes.BUFFER_TOO_SMALL ? ret : CodecReturnCodes.FAILURE;     
            	}

                for (int j = 0; j <= table.maxValue(); ++j)
                {
                    arrEntry.clear();
                    if (table.enumTypes()[j] != null && (ret = arrEntry.encode(iter, table.enumTypes()[j].display()))== CodecReturnCodes.BUFFER_TOO_SMALL &&
                    		curEnumTableEntry > startCount)
                    {
                    	if ((ret = rollbackEnumDictionaryArray(iter)) < 0)
                    	{
                    		setError(error, "rollbackEnumDictionaryArray failed " + ret);
                    		return CodecReturnCodes.FAILURE;                   		
                    	}
                    	else
                    	{
                    		currentEnumTableEntry.value(curEnumTableEntry);
                    		return CodecReturnCodes.DICT_PART_ENCODED;
                    	}                      	
                    }
                    else if (ret < 0)
                    {
                        setError(error, "encodeArrayEntry failed " + ret);
                		return ret == CodecReturnCodes.BUFFER_TOO_SMALL ? ret : CodecReturnCodes.FAILURE;                     	
                    }
                }

                if ((ret = arr.encodeComplete(iter, true)) < 0)
                {
                    setError(error, "encodeArrayComplete failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }

                if ((ret = elemEntry.encodeComplete(iter, true)) < 0)
                {
                    setError(error, "encodeElementEntryComplete failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }

                if ((ret = elemList.encodeComplete(iter, true)) < 0)
                {
                    setError(error, "encodeElementListComplete failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }

                if ((ret = seriesEntry.encodeComplete(iter, true)) < 0)
                {
                    setError(error, "encodeSeriesEntryComplete failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }
            }
        }

        if ((ret = series.encodeComplete(iter, true)) < 0)
        {
            setError(error, "encodeSeriesComplete failed " + ret);
            return CodecReturnCodes.FAILURE;
        }

        return CodecReturnCodes.SUCCESS;
    }
    
    @Override
    public int decodeEnumTypeDictionary(DecodeIterator iter, int verbosity, Error error)
    {
        int ret;
        int fidsCount = 0;
        int maxValue = 0;

        if (!_isInitialized && initDictionary(error) != CodecReturnCodes.SUCCESS)
            return CodecReturnCodes.FAILURE;

        series.clear();
        elemList.clear();
        elemEntry.clear();
        seriesEntry.clear();
        arr.clear();
        arrEntry.clear();

        if (series.decode(iter) < 0)
            return CodecReturnCodes.FAILURE;

        /* if this is not an element list, we should fail for now */
        if (series._containerType != DataTypes.ELEMENT_LIST)
            return CodecReturnCodes.FAILURE;

        /* decode summary data */
        if (series.checkHasSummaryData())
        {
            if (elemList.decode(iter, null) < 0)
                return CodecReturnCodes.FAILURE;

            while ((ret = elemEntry.decode(iter)) != CodecReturnCodes.END_OF_CONTAINER)
            {
                if (ret < 0)
                {
                    setError(error, "DecodeElementEntry failed - " + ret);
                    return ret;
                }

                if (decodeDictionaryTag(iter, elemEntry, Dictionary.Types.ENUM_TABLES, error) != CodecReturnCodes.SUCCESS)
                    return CodecReturnCodes.FAILURE;
            }
        }

        if (series.checkHasSetDefs())
        {
            setDb.clear();
            if ((ret = setDb.decode(iter)) < 0)
            {
                setError(error, "DecodeLocalElementSetDefDb failed - " + ret);
                return ret;
            }
        }

        while ((ret = seriesEntry.decode(iter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            boolean haveEnumValues = false, haveEnumDisplays = false;
            int enumValueCount = -1, enumDisplayCount = -1;

            if (ret < 0)
            {
                setError(error, "DecodeSeriesEntry failed - " + ret);
                return ret;
            }

            /* decode element list here */
            if ((ret = elemList.decode(iter, setDb)) < 0)
            {
                setError(error, "DecodeElementList failed - " + ret);
                return ret;
            }

            while ((ret = elemEntry.decode(iter)) != CodecReturnCodes.END_OF_CONTAINER)
            {
                if (ret < 0)
                {
                    setError(error, "DecodeElementEntry failed - " + ret);
                    return ret;
                }

                if ((elemEntry._name.equals(ElementNames.ENUM_FIDS)) || (elemEntry._name.equals(ENUM_FID)))
                {
                    if (elemEntry._dataType != DataTypes.ARRAY)
                    {
                        setError(error, "'" + ElementNames.ENUM_FIDS.toString() + "' element has wrong data type.");
                        return CodecReturnCodes.FAILURE;
                    }

                    if ((ret = arr.decode(iter)) < 0)
                    {
                        setError(error, "DecodeArray failed - " + ret);
                        return ret;
                    }

                    if (arr._primitiveType != DataTypes.INT)
                    {
                        setError(error, "'" + ElementNames.ENUM_FIDS.toString() + "' array has wrong primitive type.");
                        return CodecReturnCodes.FAILURE;
                    }

                    while ((ret = arrEntry.decode(iter)) != CodecReturnCodes.END_OF_CONTAINER)
                    {
                        tempInt.clear();
                        if (ret < 0 || (ret = Decoders.decodeInt(iter, tempInt)) < 0)
                        {
                            setError(error, "Error while decoding '" + ElementNames.ENUM_FIDS.toString() + "' array - " + ret);
                            return ret;
                        }

                        _referenceFidArray[fidsCount] = (int)tempInt.toLong();
                        ++fidsCount;
                    }

                }
                else if ((elemEntry._name.equals(ElementNames.ENUM_VALUE)) || (elemEntry._name.equals(VALUES)))
                {
                    if (haveEnumValues)
                    {
                        setError(error, "Duplicate '" + ElementNames.ENUM_VALUE.toString() + "' element.");
                        return CodecReturnCodes.FAILURE;
                    }

                    if (elemEntry._dataType != DataTypes.ARRAY)
                    {
                        setError(error, "Cannot decode '" + ElementNames.ENUM_VALUE.toString() + "' element.");
                        return CodecReturnCodes.FAILURE;
                    }

                    if (arr.decode(iter) < 0)
                    {
                        setError(error, "Cannot decode '" + ElementNames.ENUM_VALUE.toString() + "' array.");
                        return CodecReturnCodes.FAILURE;
                    }

                    if (arr._primitiveType != DataTypes.ENUM)
                    {
                        setError(error, "'" + ElementNames.ENUM_VALUE.toString() + "' array has wrong primtive type.");
                        return CodecReturnCodes.FAILURE;
                    }

                    enumValueCount = -1;
                    while ((ret = arrEntry.decode(iter)) != CodecReturnCodes.END_OF_CONTAINER)
                    {
                        tempEnum.clear();
                        if (ret < 0 || (ret = Decoders.decodeEnum(iter, tempEnum)) < 0)
                        {
                            setError(error, "Error while decoding '" + ElementNames.ENUM_VALUE.toString() + "' array - " + ret);
                            return ret;
                        }

                        enumValueCount++;
                        if (haveEnumDisplays)
                        {
                            /* Found the display values first, so go down the list filling up the entries */
                            if (_enumTypeArray[enumDisplayCount] == null)
                            {
                                setError(error, "Different number of display and value elements.");
                                return CodecReturnCodes.FAILURE;
                            }

                            _enumTypeArray[enumValueCount].value(tempEnum.toInt());
                        }
                        else
                        {
                            _enumTypeArray[enumValueCount] = new EnumTypeImpl();

                            _enumTypeArray[enumValueCount].value(tempEnum.toInt());
                        }

                        if (tempEnum.toInt() > maxValue)
                            maxValue = tempEnum.toInt();
                    }

                    /* Make sure we didn't have more display elements than values */
                    if (haveEnumDisplays && enumValueCount != enumDisplayCount)
                    {
                        setError(error, "Different number of display and value elements.");
                        return CodecReturnCodes.FAILURE;
                    }

                    haveEnumValues = true;
                }
                else if ((elemEntry._name.equals(ElementNames.ENUM_DISPLAY)) || (elemEntry._name.equals(DISPLAYS)))
                {

                    if (elemEntry._dataType != DataTypes.ARRAY)
                    {
                        setError(error, "Cannot decode '" + ElementNames.ENUM_DISPLAY.toString() + "' element.");
                        return CodecReturnCodes.FAILURE;
                    }

                    if (arr.decode(iter) < 0)
                    {
                        setError(error, "Cannot decode '" + ElementNames.ENUM_DISPLAY.toString() + "' array.");
                        return CodecReturnCodes.FAILURE;
                    }

                    if ((arr._primitiveType != DataTypes.ASCII_STRING) && (arr._primitiveType != DataTypes.RMTES_STRING)
                            && (arr._primitiveType != DataTypes.UTF8_STRING))
                    {
                        setError(error, "'" + ElementNames.ENUM_DISPLAY.toString() + "' array has wrong primtive type.");
                        return CodecReturnCodes.FAILURE;
                    }

                    enumDisplayCount = -1;
                    while ((ret = arrEntry.decode(iter)) != CodecReturnCodes.END_OF_CONTAINER)
                    {
                        if (ret < 0)
                        {
                            setError(error, "Error while decoding '" + ElementNames.ENUM_DISPLAY.toString() + "' array - " + ret);
                            return ret;
                        }

                        enumDisplayCount++;
                        if (haveEnumValues)
                        {
                            /* Found the enum values first, so go down the list filling up the entries */
                            if (_enumTypeArray[enumValueCount] == null)
                            {
                                setError(error, "Different number of display and value elements.");
                                return CodecReturnCodes.FAILURE;
                            }

                            ((BufferImpl)_enumTypeArray[enumDisplayCount].display()).data(arrEntry.encodedData().toString());
                        }
                        else
                        {
                            _enumTypeArray[enumDisplayCount] = new EnumTypeImpl();

                            ((BufferImpl)_enumTypeArray[enumDisplayCount].display()).data(arrEntry.encodedData().toString());
                        }
                    }

                    /* Make sure we didn't have more value elements than displays */
                    if (haveEnumValues && enumDisplayCount != enumValueCount)
                    {
                        setError(error, "Different number of display and value elements.");
                        return CodecReturnCodes.FAILURE;
                    }

                    haveEnumDisplays = true;
                }
            }

            if (!haveEnumValues)
            {
                setError(error, "\"" + ElementNames.ENUM_VALUE.toString() + "\" element not found");
                return CodecReturnCodes.FAILURE;
            }

            if (!haveEnumDisplays)
            {
                setError(error, "\"" + ElementNames.ENUM_DISPLAY.toString() + "\" element not found");
                return CodecReturnCodes.FAILURE;
            }

            _enumTypeArrayCount = enumValueCount;
            if (addTableToDictionary(fidsCount, _referenceFidArray, _referenceFidAcronymArray, maxValue,
                                     _enumTypeArray, _enumTypeArrayCount, error, -1) != CodecReturnCodes.SUCCESS)
            {
                return CodecReturnCodes.FAILURE;
            }

            maxValue = 0;
            fidsCount = 0;
            _enumTypeArrayCount = -1;
        }

        return CodecReturnCodes.SUCCESS;
    }
    
    @Override
    public FieldSetDefDb fieldSetDef()
    {
        return fieldSetDef;
    }
    
    /* gets the start of data from a line of data */
    private void findLineStart(char[] fileData)
    {
        if (_lastPosition != 0)
        {
            while (fileData[_lastPosition] != '\r' && fileData[_lastPosition] != '\n')
            {
                _lastPosition++;
            }
        }

        for (_lineStartPosition = _lastPosition; _lineStartPosition < fileData.length; _lineStartPosition++)
        {
            if (fileData[_lineStartPosition] != ' ' &&
                    fileData[_lineStartPosition] != '\t' &&
                    fileData[_lineStartPosition] != '\r' &&
                    fileData[_lineStartPosition] != '\n')
            {
                _lastPosition = _lineStartPosition;
                break;
            }
        }
    }

    /* go to next line */
    private void nextLine(char[] fileData)
    {
        while (fileData[_lastPosition] != '\r' && fileData[_lastPosition] != '\n')
        {
            _lastPosition++;
        }
    }

    /* gets the tag value */
    private String tagName(char[] fileData)
    {
        boolean startFound = false;

        for (_lastPosition = 4 + _lineStartPosition; _lastPosition < fileData.length; _lastPosition++)
        {
            if (!startFound)
            {
                if (fileData[_lastPosition] != ' ' && fileData[_lastPosition] != '\t')
                {
                    _startPosition = _lastPosition;
                    startFound = true;
                }
            }
            else
            {
                if (fileData[_lastPosition] == ' ' || fileData[_lastPosition] == '\t')
                {
                    break;
                }
            }
        }

        return new String(fileData, _startPosition, _lastPosition - _startPosition);
    }
    
    /* depends on tagName() being call beforehand */
    private String tagValue(char[] fileData)
    {
        boolean startFound = false;

        for (_lastPosition++; _lastPosition < fileData.length; _lastPosition++)
        {
            if (!startFound)
            {
                if (fileData[_lastPosition] != ' ' && fileData[_lastPosition] != '\t')
                {
                    _startPosition = _lastPosition;
                    startFound = true;
                }
            }
            else
            {
                if (fileData[_lastPosition] == '\r' || fileData[_lastPosition] == '\n')
                {
                    break;
                }
            }
        }

        return new String(fileData, _startPosition, _lastPosition - _startPosition);
    }

    private String acronym(char[] fileData)
    {
        boolean startFound = false;

        for (_lastPosition = _lineStartPosition; _lastPosition < fileData.length; _lastPosition++)
        {
            if (!startFound)
            {
                if (fileData[_lastPosition] != ' ' && fileData[_lastPosition] != '\t')
                {
                    _startPosition = _lastPosition;
                    startFound = true;
                }
            }
            else
            {
                if (fileData[_lastPosition] == ' ' || fileData[_lastPosition] == '\t')
                {
                    break;
                }
            }
        }

        return new String(fileData, _startPosition, _lastPosition - _startPosition);
    }
    
    /* depends on acronym() being call beforehand */
    private String ddeAcronym(char[] fileData)
    {
        boolean startFound = false;

        for (_lastPosition++; _lastPosition < fileData.length; _lastPosition++)
        {
            if (!startFound)
            {
                if (fileData[_lastPosition] != ' ' &&
                        fileData[_lastPosition] != '\t' &&
                        fileData[_lastPosition] != '"')
                {
                    _startPosition = _lastPosition;
                    startFound = true;
                }
            }
            else
            {
                if (fileData[_lastPosition] == '"')
                {
                    break;
                }
            }
        }

        return new String(fileData, _startPosition, _lastPosition - _startPosition);
    }
    
    /* depends on acronym() being call beforehand */
    private boolean isDisplayHex(char[] fileData)
    {
        boolean isHex = false;

        for (int i = _lastPosition; i < fileData.length; i++)
        {
            if (fileData[i] == '#')
            {
                isHex = true;
                _lastPosition = i;
                break;
            }

            if (fileData[i] == '"' || fileData[i] == '\r' || fileData[i] == '\n')
            {
                break;
            }
        }

        return isHex;
    }

    /* depends on isDisplayHex() being call beforehand */
    private int hexLength(char[] fileData)
    {
        int hexLen = 0, i = _lastPosition + 1;

        while (fileData[i] != '#')
        {
            hexLen++;
            i++;
        }

        return hexLen;
    }
    
    /* depends on hexLength() being call beforehand */
    private boolean setDisplayToHex(char[] fileData, Buffer displayBuf, int hexLen)
    {
        boolean retVal = true;
        int firstHexDigit = 0, secondHexDigit = 0;
        int hexMaxPosition = _lastPosition + hexLen + 1;

        ((BufferImpl)displayBuf).data_internal(ByteBuffer.allocate(hexLen / 2));

        for (_lastPosition++; _lastPosition < hexMaxPosition; _lastPosition += 2)
        {
            // get first hex digit
            if (fileData[_lastPosition] >= '0' && fileData[_lastPosition] <= '9')
            {
                firstHexDigit = fileData[_lastPosition] - 0x30;
            }
            else if (fileData[_lastPosition] >= 'A' && fileData[_lastPosition] <= 'F')
            {
                firstHexDigit = fileData[_lastPosition] - 0x41 + 10;
            }
            else if (fileData[_lastPosition] >= 'a' && fileData[_lastPosition] <= 'f')
            {
                firstHexDigit = fileData[_lastPosition] - 0x61 + 10;
            }
            else
            {
                retVal = false;
                break;
            }

            // get second hex digit
            if (fileData[_lastPosition + 1] >= '0' && fileData[_lastPosition + 1] <= '9')
            {
                secondHexDigit = fileData[_lastPosition + 1] - 0x30;
            }
            else if (fileData[_lastPosition + 1] >= 'A' && fileData[_lastPosition + 1] <= 'F')
            {
                secondHexDigit = fileData[_lastPosition + 1] - 0x41 + 10;
            }
            else if (fileData[_lastPosition + 1] >= 'a' && fileData[_lastPosition + 1] <= 'f')
            {
                secondHexDigit = fileData[_lastPosition + 1] - 0x61 + 10;
            }
            else
            {
                retVal = false;
                break;
            }

            // Translate two digits into a byte.
            // Append first and second hex digits to display buffer.
            ((BufferImpl)displayBuf).appendByte((byte)(firstHexDigit * 16 + secondHexDigit));
        }

        return retVal;
    }

    /* depends on () being call beforehand */
    private String display(char[] fileData)
    {
        boolean startFound = false;
        char delim = 0;

        for (_lastPosition++; _lastPosition < fileData.length; _lastPosition++)
        {
            if (!startFound)
            {
                if (fileData[_lastPosition] == '"' || fileData[_lastPosition] == '#')
                {
                    delim = fileData[_lastPosition++];
                    _startPosition = _lastPosition;
                    startFound = true;
                }
            }
            else
            {
                if (fileData[_lastPosition] == delim )
                {
                    break;
                }
            }
        }

        return new String(fileData, _startPosition, _lastPosition - _startPosition);
    }

    /* depends on ddeAcronym() being call beforehand */
    private int fid(char[] fileData)
    {
        return intField(fileData);
    }
    
    /* depends on fid() being call beforehand */
    private int ripplesToPosition(char[] fileData)
    {
        boolean startFound = false;

        for (_lastPosition++; _lastPosition < fileData.length; _lastPosition++)
        {
            if (!startFound)
            {
                if (fileData[_lastPosition] != ' ' && fileData[_lastPosition] != '\t')
                {
                    _startPosition = _lastPosition;
                    startFound = true;
                }
            }
            else
            {
                if (fileData[_lastPosition] == ' ' || fileData[_lastPosition] == '\t')
                {
                    break;
                }
            }
        }

        return _startPosition;
    }
    
    /* depends on ripplesToPosition() being call beforehand */
    private void findFieldTypeStr(char[] fileData)
    {
        boolean startFound = false;

        for (_lastPosition++; _lastPosition < fileData.length; _lastPosition++)
        {
            if (!startFound)
            {
                if (fileData[_lastPosition] != ' ' && fileData[_lastPosition] != '\t')
                {
                    _startPosition = _lastPosition;
                    startFound = true;
                }
            }
            else
            {
                if (fileData[_lastPosition] == ' ' || fileData[_lastPosition] == '\t')
                {
                    break;
                }
            }
        }
    }
    
    /* depends on fieldTypeStr() being call beforehand */
    private int length(char[] fileData)
    {
        return intField(fileData);
    }
    
    /* depends on length() being call beforehand */
    private int enumLength(char[] fileData)
    {
        int intValue = 0, position = 0;
        boolean openParenFound = false, closeParenFound = false;

        for (position = _lastPosition; position < fileData.length; position++)
        {
            if (fileData[position] == '\r' || fileData[position] == '\n')
            {
                break;
            }

            if (fileData[position] == '(')
            {
                openParenFound = true;
                _lastPosition = position;
                intValue = intField(fileData);
                position = _lastPosition - 1;
            }
            else if (fileData[position] == ')')
            {
                closeParenFound = true;
                _lastPosition = position;
                break;
            }
        }

        if ((openParenFound && !closeParenFound) || (openParenFound && intValue == 0))
        {
            intValue = -1;
        }

        return intValue;
    }

    /* depends on enumLength() being call beforehand */
    private void findRwfTypeStr(char[] fileData)
    {
        boolean startFound = false;

        for (_lastPosition++; _lastPosition < fileData.length; _lastPosition++)
        {
            if (!startFound)
            {
                if (fileData[_lastPosition] != ' ' && fileData[_lastPosition] != '\t')
                {
                    _startPosition = _lastPosition;
                    startFound = true;
                }
            }
            else
            {
                if (fileData[_lastPosition] == ' ' || fileData[_lastPosition] == '\t')
                {
                    break;
                }
            }
        }
    }
    
    /* depends on rwfTypeStr() being call beforehand */
    private int rwfLength(char[] fileData)
    {
        return intField(fileData);
    }
    
    /* depends on display() being call beforehand */
    private String meaning(char[] fileData)
    {
        boolean startFound = false;
        String retStr = null;

        for (_lastPosition++; _lastPosition < fileData.length; _lastPosition++)
        {
            if (!startFound)
            {
                if (fileData[_lastPosition] != ' ' && fileData[_lastPosition] != '\t')
                {
                    if (fileData[_lastPosition] == '\n' || fileData[_lastPosition] == '\r')
                        break;

                    _startPosition = _lastPosition;
                    startFound = true;
                }
            }
            else
            {
                if (fileData[_lastPosition] == '\r' || fileData[_lastPosition] == '\n')
                {
                    break;
                }
            }
        }

        if (_lastPosition - _startPosition > 0 && startFound)
        {
            retStr = new String(fileData, _startPosition, _lastPosition - _startPosition);
        }
        else
            retStr = new String();

        return retStr;
    }

    /* utility for fid(), length(), enumLength() and rwfLength() */
    private int intField(char[] fileData)
    {
        boolean startFound = false;
        int intCharCount = 0, intValue = MIN_FID - 1, intMultiplier = 0, intDigit = 0;
        boolean isNegative = false;

        for (_lastPosition++; _lastPosition < fileData.length; _lastPosition++)
        {
            if (!startFound)
            {
                if (fileData[_lastPosition] == '-')
                {
                    isNegative = true;
                    continue;
                }
                if (fileData[_lastPosition] >= '0' && fileData[_lastPosition] <= '9')
                {
                    _startPosition = _lastPosition;
                    startFound = true;
                    intValue = 0;
                    intCharCount++;
                }
                else if (fileData[_lastPosition] != ' ' && fileData[_lastPosition] != '\t')
                {
                    break;
                }
            }
            else
            {
                if (fileData[_lastPosition] == ' ' ||
                        fileData[_lastPosition] == '\t' ||
                        fileData[_lastPosition] == ')' ||
                        fileData[_lastPosition] == '\r' ||
                        fileData[_lastPosition] == '\n')
                {
                    break;
                }
                else if (fileData[_lastPosition] >= '0' && fileData[_lastPosition] <= '9')
                {
                    intCharCount++;
                }
                else
                // not an integer after all
                {
                    intValue = MIN_FID - 1;
                    intCharCount = 0;
                    break;
                }
            }
        }

        for (int i = _startPosition; intCharCount > 0 && i < _lastPosition; i++)
        {
            intMultiplier = (int)Math.pow(10, --intCharCount);
            intDigit = fileData[i] - 0x30;
            intValue += (intDigit * intMultiplier);
        }

        return ((isNegative == false) ? intValue : -intValue);
    }
    
    /* utility for fieldType() and rwfFieldType() */
    private boolean compareTo(char[] fileData, String compareStr)
    {
        assert (compareStr != null) : "compareStr must be non-null";
        assert (fileData != null) : "fileData must be non-null";

        boolean retVal = true;

        for (int i = 0; i < compareStr.length(); i++)
        {
            if (fileData[_startPosition + i] != compareStr.charAt(i))
            {
                retVal = false;
                break;
            }
        }

        return retVal;
    }

    int encodeDataDictSummaryData(EncodeIterator iter, int type, Series series, Error error)
    {
        int ret;

        elemList.clear();
        elemEntry.clear();
        tempInt.clear();

        elemList.applyHasStandardData();

        if ((ret = elemList.encodeInit(iter, null, 0)) < 0)
        {
            setError(error, "encodeElementListInit failed " + ret);
            return CodecReturnCodes.FAILURE;
        }

        elemEntry.dataType(DataTypes.INT);
        elemEntry.name(ElementNames.DICT_TYPE);
        tempInt.value(type);
        if (elemEntry.encode(iter, tempInt) < 0)
        {
            setError(error, "encodeElementEntry failed " + ret + " - Type");
            return CodecReturnCodes.FAILURE;
        }

        elemEntry.dataType(DataTypes.INT);
        elemEntry.name(ElementNames.DICTIONARY_ID);
        tempInt.value(_infoDictionaryId);
        if (elemEntry.encode(iter, tempInt) < 0)
        {
            setError(error, "encodeElementEntry failed " + ret + " - DictionaryId");
            return CodecReturnCodes.FAILURE;
        }

        switch (type)
        {
            case Dictionary.Types.FIELD_DEFINITIONS:
                /* Version */
                elemEntry.dataType(DataTypes.ASCII_STRING);
                elemEntry.name(ElementNames.DICT_VERSION);
                if (elemEntry.encode(iter, _infoFieldVersion) < 0)
                {
                    setError(error, "encodeElementEntry failed " + ret + " - Version");
                    return CodecReturnCodes.FAILURE;
                }
                break;
            case Dictionary.Types.ENUM_TABLES:
                /* RT_Version */
                elemEntry.dataType(DataTypes.ASCII_STRING);
                elemEntry.name(ElementNames.ENUM_RT_VERSION);
                if (elemEntry.encode(iter, _infoEnumRTVersion) < 0)
                {
                    setError(error, "encodeElementEntry failed " + ret + " - RT_Version");
                    return CodecReturnCodes.FAILURE;
                }

                /* DT_Version */
                elemEntry.dataType(DataTypes.ASCII_STRING);
                elemEntry.name(ElementNames.ENUM_DT_VERSION);
                if (elemEntry.encode(iter, _infoEnumDTVersion) < 0)
                {
                    setError(error, "encodeElementEntry failed " + ret + " - DT_Version");
                    return CodecReturnCodes.FAILURE;
                }

                /* Version */
                elemEntry.dataType(DataTypes.ASCII_STRING);
                elemEntry.name(ElementNames.DICT_VERSION);
                if (elemEntry.encode(iter, _infoEnumDTVersion) < 0)
                {
                    setError(error, "encodeElementEntry failed " + ret + " - Version");
                    return CodecReturnCodes.FAILURE;
                }
                break;
        }

        if ((ret = elemList.encodeComplete(iter, true)) < 0)
        {
            setError(error, "encodeElementListComplete failed " + ret);
            return CodecReturnCodes.FAILURE;
        }

        if ((ret = series.encodeSummaryDataComplete(iter, true)) < 0)
        {
            setError(error, "encodeSeriesSummaryDataComplete failed " + ret);
            return CodecReturnCodes.FAILURE;
        }

        return 1;
    }
    
    int encodeDataDictEntry(EncodeIterator iter, DictionaryEntryImpl entry, int verbosity, Error error, LocalElementSetDefDbImpl setDb)
    {
        int ret;
        int maxEncSizeNeeded = entry.acronym().length() + entry.ddeAcronym().length() + 14;

        seriesEntry.clear();
        elemEntry.clear();
        elemList.clear();
        tempUInt.clear();
        tempInt.clear();

        if (((EncodeIteratorImpl)iter).isIteratorOverrun(3 + maxEncSizeNeeded))
            return CodecReturnCodes.DICT_PART_ENCODED;

        if ((ret = seriesEntry.encodeInit(iter, maxEncSizeNeeded)) < 0)
        {
            setError(error, "encodeSeriesEntryInit failed " + ret);
            return CodecReturnCodes.FAILURE;
        }

        elemList.applyHasSetData();

        if ((ret = elemList.encodeInit(iter, setDb, 0)) < 0)
        {
            setError(error, "encodeElementListInit failed " + ret);
            return CodecReturnCodes.FAILURE;
        }

        elemEntry.name(ElementNames.FIELD_NAME);
        elemEntry.dataType(DataTypes.ASCII_STRING);
        if ((ret = elemEntry.encode(iter, entry.acronym())) != CodecReturnCodes.SUCCESS)
        {
            setError(error, "encodeElementEntry NAME '" + entry.acronym().toString() + "' failed " + ret);
            return CodecReturnCodes.FAILURE;
        }

        elemEntry.name(ElementNames.FIELD_ID);
        elemEntry.dataType(DataTypes.INT);
        tempInt.value(entry.fid());
        if ((ret = elemEntry.encode(iter, tempInt)) != CodecReturnCodes.SUCCESS)
        {
            setError(error, "encodeElementEntry FID " + entry.fid() + " failed " + ret);
            return CodecReturnCodes.FAILURE;
        }

        elemEntry.name(ElementNames.FIELD_RIPPLETO);
        elemEntry.dataType(DataTypes.INT);
        tempInt.value(entry.rippleToField());
        if ((ret = elemEntry.encode(iter, tempInt)) != CodecReturnCodes.SUCCESS)
        {
            setError(error, "encodeElementEntry RIPPLETO " + entry.rippleToField() + " failed " + ret);
            return CodecReturnCodes.FAILURE;
        }

        elemEntry.name(ElementNames.FIELD_TYPE);
        elemEntry.dataType(DataTypes.INT);
        tempInt.value(entry.fieldType());
        if ((ret = elemEntry.encode(iter, tempInt)) != CodecReturnCodes.SUCCESS)
        {
            setError(error, "encodeElementEntry TYPE " + entry.fieldType() + " failed " + ret);
            return CodecReturnCodes.FAILURE;
        }

        elemEntry.name(ElementNames.FIELD_LENGTH);
        elemEntry.dataType(DataTypes.UINT);
        tempUInt.value(entry.length());
        if ((ret = elemEntry.encode(iter, tempUInt)) != CodecReturnCodes.SUCCESS)
        {
            setError(error, "encodeElementEntry LENGTH " + entry.length() + " failed " + ret);
            return CodecReturnCodes.FAILURE;
        }

        elemEntry.name(ElementNames.FIELD_RWFTYPE);
        elemEntry.dataType(DataTypes.UINT);
        tempUInt.value(entry.rwfType());
        if ((ret = elemEntry.encode(iter, tempUInt)) != CodecReturnCodes.SUCCESS)
        {
            setError(error, "encodeElementEntry RWFTYPE " + entry.rwfType() + " failed " + ret);
            return CodecReturnCodes.FAILURE;
        }

        elemEntry.name(ElementNames.FIELD_RWFLEN);
        elemEntry.dataType(DataTypes.UINT);
        tempUInt.value(entry.rwfLength());
        ret = elemEntry.encode(iter, tempUInt);
        if ((verbosity >= Dictionary.VerbosityValues.NORMAL && ret != CodecReturnCodes.SUCCESS
                || verbosity < Dictionary.VerbosityValues.NORMAL && ret != CodecReturnCodes.SET_COMPLETE))
        {
            setError(error, "encodeElementEntry RWFLEN " + entry.rwfLength() + " failed " + ret);
            return CodecReturnCodes.FAILURE;
        }

        if (verbosity >= Dictionary.VerbosityValues.NORMAL)
        {
            elemEntry.name(ElementNames.FIELD_ENUMLENGTH);
            elemEntry.dataType(DataTypes.UINT);
            tempUInt.value(entry.enumLength());
            if ((ret = elemEntry.encode(iter, tempUInt)) != CodecReturnCodes.SUCCESS)
            {
                setError(error, "encodeElementEntry ENUMLENGTH " + entry.enumLength() + " failed " + ret);
                return CodecReturnCodes.FAILURE;
            }

            elemEntry.name(ElementNames.FIELD_LONGNAME);
            elemEntry.dataType(DataTypes.ASCII_STRING);
            if ((ret = elemEntry.encode(iter, entry.ddeAcronym())) != CodecReturnCodes.SET_COMPLETE)
            {
                setError(error, "encodeElementEntry LONGNAME Acronym '" + entry.ddeAcronym().toString() + "' failed " + ret);
                return CodecReturnCodes.FAILURE;
            }
        }

        if ((ret = elemList.encodeComplete(iter, true)) < 0)
        {
            setError(error, "encodeElementListComplete failed " + ret);
            return CodecReturnCodes.FAILURE;
        }

        if ((ret = seriesEntry.encodeComplete(iter, true)) < 0)
        {
            setError(error, "encodeSeriesEntryComplete failed " + ret);
            return CodecReturnCodes.FAILURE;
        }
        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int minFid()
    {
        return _minFid;
    }

    @Override
    public int maxFid()
    {
        return _maxFid;
    }

    @Override
    public int numberOfEntries()
    {
        return _numberOfEntries;
    }

    @Override
    public EnumTypeTable[] enumTables()
    {
        return _enumTables;
    }

    @Override
    public int enumTableCount()
    {
        return _enumTableCount;
    }

    @Override
    public int infoDictionaryId()
    {
        return _infoDictionaryId;
    }

    @Override
    public Buffer infoFieldVersion()
    {
        return _infoFieldVersion;
    }

    @Override
    public Buffer infoEnumRTVersion()
    {
        return _infoEnumRTVersion;
    }

    @Override
    public Buffer infoEnumDTVersion()
    {
        return _infoEnumDTVersion;
    }

    @Override
    public Buffer infoFieldFilename()
    {
        return _infoFieldFilename;
    }

    @Override
    public Buffer infoFieldDesc()
    {
        return _infoFieldDesc;
    }

    @Override
    public Buffer infoFieldBuild()
    {
        return _infoFieldBuild;
    }

    @Override
    public Buffer infoFieldDate()
    {
        return _infoFieldDate;
    }

    @Override
    public Buffer infoEnumFilename()
    {
        return _infoEnumFilename;
    }

    @Override
    public Buffer infoEnumDesc()
    {
        return _infoEnumDesc;
    }

    @Override
    public Buffer infoEnumDate()
    {
        return _infoEnumDate;
    }
    
    @Override
    public String toString()
    {
        if (!_isInitialized)
            return null;

        if (dictionaryString == null)
        {
            StringBuilder sb = new StringBuilder();

            sb.append("Data Dictionary Dump: MinFid=" + _minFid + " MaxFid=" + _maxFid + " NumEntries " + _numberOfEntries + "\n\n");

            sb.append("Tags:\n  DictionaryId=\"" + _infoDictionaryId + "\"\n\n");

            sb.append("  [Field Dictionary Tags]\n" +
                      "      Filename=\"" + _infoFieldFilename + "\"\n" +
                      "          Desc=\"" + _infoFieldDesc + "\"\n" +
                      "       Version=\"" + _infoFieldVersion + "\"\n" +
                      "         Build=\"" + _infoFieldBuild + "\"\n" +
                      "          Date=\"" + _infoFieldDate + "\"\n\n");

            sb.append("  [Enum Type Dictionary Tags]\n" +
                      "      Filename=\"" + _infoEnumFilename + "\"\n" +
                      "          Desc=\"" + _infoEnumDesc + "\"\n" +
                      "    RT_Version=\"" + _infoEnumRTVersion + "\"\n" +
                      "    DT_Version=\"" + _infoEnumDTVersion + "\"\n" +
                      "          Date=\"" + _infoEnumDate + "\"\n\n");

            sb.append("Field Dictionary:\n");

            for (int i = 0; i <= MAX_FID - MIN_FID; i++)
            {
                if (_entriesArray[i] != null && _entriesArray[i].rwfType() != DataTypes.UNKNOWN)
                {
                    sb.append("  Fid=" + _entriesArray[i].fid() + " '" + _entriesArray[i].acronym() + "' '" + _entriesArray[i].ddeAcronym() +
                              "' Type=" + _entriesArray[i].fieldType() +
                              " RippleTo=" + _entriesArray[i].rippleToField() + " Len=" + _entriesArray[i].length() +
                              " EnumLen=" + _entriesArray[i].enumLength() +
                              " RwfType=" + _entriesArray[i].rwfType() + " RwfLen=" + _entriesArray[i].rwfLength() + "\n");
                }
            }

            /* Enum Tables Dump */

            sb.append("\nEnum Type Tables:\n");

            for (int i = 0; i < _enumTableCount; ++i)
            {
                EnumTypeTable table = _enumTables[i];

                for (int j = 0; j < table.fidReferenceCount(); ++j)
                    sb.append("(Referenced by Fid " + table.fidReferences()[j] + ")\n");

                for (int j = 0; j <= table.maxValue(); ++j)
                {
                    EnumType enumType = table.enumTypes()[j];

                    if (enumType != null)
                    {
                        sb.append("value=" + enumType.value() +
                                  " display=\"" + enumType.display() +
                                  "\" meaning=\"" + enumType.meaning() + "\"\n");
                    }
                }

                sb.append("\n");
            }

            sb.append("\nField Set Defs Tables:\n");

            for (int i = 0; i < _enumTableCount; ++i)
            {
                EnumTypeTable table = _enumTables[i];

                for (int j = 0; j < table.fidReferenceCount(); ++j)
                    sb.append("(Referenced by Fid " + table.fidReferences()[j] + ")\n");

                for (int j = 0; j <= table.maxValue(); ++j)
                {
                    EnumType enumType = table.enumTypes()[j];

                    if (enumType != null)
                    {
                        sb.append("value=" + enumType.value() +
                                  " display=\"" + enumType.display() +
                                  "\" meaning=\"" + enumType.meaning() + "\"\n");
                    }
                }

                sb.append("\n");
            }

            dictionaryString = sb.toString();
        }

        return dictionaryString;
    }
}
