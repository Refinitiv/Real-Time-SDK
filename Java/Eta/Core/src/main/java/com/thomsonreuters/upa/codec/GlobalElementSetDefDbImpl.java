package com.thomsonreuters.upa.codec;

import java.nio.ByteBuffer;

import com.thomsonreuters.upa.rdm.Dictionary;
import com.thomsonreuters.upa.rdm.ElementNames;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportReturnCodes;

class GlobalElementSetDefDbImpl extends ElementSetDefDbImpl implements GlobalElementSetDefDb
{
    int maxSetId = 0; /* Maximum set definition index */
    
    Buffer info_version = CodecFactory.createBuffer(); /* Tag: Dictionary version */
    int info_DictionaryID = 0; /* Tag: DictionaryId. All dictionaries loaded using this object will have this tag matched if found. */
    
    private class encState
    {
        final static int NONE = 0;
        final static int VECTOR = 1;
        final static int VECTOR_ENTRY = 2;
        final static int ELEM_LIST = 3;
        final static int ELEM_ENTRY = 4;
        final static int ARRAY = 5;
    }
    
    // dictionary encoding variables
    private final Vector encVector = CodecFactory.createVector();
    private final VectorEntry encVectorEntry = CodecFactory.createVectorEntry();
    private final Array encArray = CodecFactory.createArray();
    private final ArrayEntry encArrayEntry = CodecFactory.createArrayEntry();
    private final LocalElementSetDefDb encSetDef = CodecFactory.createLocalElementSetDefDb();
    private final ElementSetDefEntry[] setDef0_Entries = new ElementSetDefEntryImpl[3];
    private final ElementList encElemList = CodecFactory.createElementList();
    private final ElementEntry encElement = CodecFactory.createElementEntry();
    
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
    ElementSetDef newSetDef = CodecFactory.createElementSetDef();
    Buffer tempNameArray = CodecFactory.createBuffer();
    Buffer tempTypeArray = CodecFactory.createBuffer();
    
    GlobalElementSetDefDbImpl()
    {
        super(65535);
        info_version = CodecFactory.createBuffer();
        info_DictionaryID = 0;

        maxSetId = -1;

        setDef0_Entries[0] = CodecFactory.createElementSetDefEntry();
        setDef0_Entries[0].name(ElementNames.SET_NUMENTRIES);
        setDef0_Entries[0].dataType(DataTypes.INT);

        setDef0_Entries[1] = CodecFactory.createElementSetDefEntry();
        setDef0_Entries[1].name(ElementNames.SET_NAMES);
        setDef0_Entries[1].dataType(DataTypes.ARRAY);

        setDef0_Entries[2] = CodecFactory.createElementSetDefEntry();
        setDef0_Entries[2].name(ElementNames.SET_TYPES);
        setDef0_Entries[2].dataType(DataTypes.ARRAY);
    }
    
    public void clear()
    {
        _definitions = new ElementSetDefImpl[65535];

        maxSetId = -1;

        info_version = CodecFactory.createBuffer();
        info_DictionaryID = 0;
    }

    @Override
    public int decode(DecodeIterator iter, int verbosity, Error error)
    {
        int ret;
        int row;
        ByteBuffer tmpByteBuffer;

        ElementSetDefEntry[] tempSetDefEntry;

        vector.clear();
        vectorEntry.clear();
        elemList.clear();
        elemEntry.clear();
        arr.clear();
        arrEntry.clear();
        tmpInt.clear();

        if (vector.decode(iter) < 0)
            return CodecReturnCodes.FAILURE;

        /* if this is not an element list, we should fail for now */
        if (vector._containerType != DataTypes.ELEMENT_LIST)
            return CodecReturnCodes.FAILURE;

        /* decode summary data */
        if (vector.checkHasSummaryData())
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

                if (decodeDictionaryTag(iter, elemEntry, Dictionary.Types.FIELD_SET_DEFINITION, error) != CodecReturnCodes.SUCCESS)
                    return CodecReturnCodes.FAILURE;
            }
        }

        if (vector.checkHasSetDefs())
        {
            setDb.clear();
            if ((ret = setDb.decode(iter)) < 0)
            {
                setError(error, "DecodeLocalElementSetDefDb failed - " + ret);
                return ret;
            }
        }

        while ((ret = vectorEntry.decode(iter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret < 0)
            {
                setError(error, "DecodeVectorEntry failed - " + ret);
                return ret;
            }

            newSetDef.clear();
            newSetDef.setId((int)vectorEntry.index());

            /* decode element list here */
            if ((ret = elemList.decode(iter, setDb)) < 0)
            {
                setError(error, "DecodeElementList failed - " + ret);
                return ret;
            }

            tempNameArray.clear();
            tempTypeArray.clear();

            while ((ret = elemEntry.decode(iter)) != CodecReturnCodes.END_OF_CONTAINER)
            {
                if (ret < 0)
                {
                    setError(error, "DecodeElementEntry failed - " + ret);
                    return ret;
                }

                if (elemEntry._name.equals(ElementNames.SET_NUMENTRIES))
                {
                    if (elemEntry._dataType != DataTypes.INT)
                    {
                        setError(error, "'" + ElementNames.SET_NUMENTRIES.toString() + "' element has wrong data type.");
                        return CodecReturnCodes.FAILURE;
                    }
                    ret = tmpInt.decode(iter);
                    if (ret < 0)
                    {
                        setError(error, "DecodeInt failed - " + ret);
                        return ret;
                    }
                    tmpLong = tmpInt.toLong();
                    tmpLong = (byte)tmpLong; // Get first byte

                    newSetDef.count((int)tmpLong);

                    tempSetDefEntry = new ElementSetDefEntry[newSetDef.count()];

                    for (int i = 0; i < newSetDef.count(); i++)
                    {
                        tempSetDefEntry[i] = CodecFactory.createElementSetDefEntry();
                    }

                    newSetDef.entries(tempSetDefEntry);

                    if (tempNameArray.length() != 0)
                    {
                        tempDecIter.clear();
                        tempDecIter.setBufferAndRWFVersion(tempNameArray, Codec.majorVersion(), Codec.minorVersion());

                        if (arr.decode(iter) < 0)
                        {
                            setError(error, "Cannot decode '" + ElementNames.SET_NAMES.toString() + "' array.");
                            return CodecReturnCodes.FAILURE;
                        }

                        if (arr._primitiveType != DataTypes.BUFFER)
                        {
                            setError(error, "'" + ElementNames.SET_NAMES.toString() + "' array has wrong primtive type.");
                            return CodecReturnCodes.FAILURE;
                        }

                        row = 0;

                        while ((ret = arrEntry.decode(iter)) != CodecReturnCodes.END_OF_CONTAINER)
                        {
                            tempBuffer.clear();
                            ret = tempBuffer.decode(iter);
                            if (ret < 0)
                            {
                                setError(error, "DecodeBuffer failed - " + ret);
                                return ret;
                            }

                            tmpByteBuffer = ByteBuffer.allocate(tempBuffer.length());
                            newSetDef.entries()[row].name().data(tmpByteBuffer);
                            tempBuffer.copy(newSetDef.entries()[row].name());
                            row++;
                        }
                    }

                    if (tempTypeArray.length() != 0)
                    {
                        tempDecIter.clear();
                        tempDecIter.setBufferAndRWFVersion(tempTypeArray, Codec.majorVersion(), Codec.minorVersion());

                        if (arr.decode(iter) < 0)
                        {
                            setError(error, "Cannot decode '" + ElementNames.SET_TYPES.toString() + "' array.");
                            return CodecReturnCodes.FAILURE;
                        }

                        if (arr._primitiveType != DataTypes.INT)
                        {
                            setError(error, "'" + ElementNames.SET_TYPES.toString() + "' array has wrong primtive type.");
                            return CodecReturnCodes.FAILURE;
                        }

                        row = 0;

                        while ((ret = arrEntry.decode(iter)) != CodecReturnCodes.END_OF_CONTAINER)
                        {
                            tmpInt.clear();
                            ret = tmpInt.decode(iter);
                            if (ret < 0)
                            {
                                setError(error, "DecodeInt failed - " + ret);
                                return ret;
                            }
                            tmpLong = tmpInt.toLong();
                            tmpLong = (short)tmpLong; // Get first 2 bytes
                            newSetDef.entries()[row].dataType((int)tmpLong);
                            row++;
                        }
                    }

                }
                else if (elemEntry._name.equals(ElementNames.SET_NAMES))
                {
                    if (elemEntry._dataType != DataTypes.ARRAY)
                    {
                        setError(error, "Cannot decode '" + ElementNames.SET_NAMES.toString() + "' element.");
                        return CodecReturnCodes.FAILURE;
                    }

                    if (newSetDef.entries()[0] == null)
                        tempNameArray.data(elemEntry.encodedData().data());
                    else
                    {
                        if (arr.decode(iter) < 0)
                        {
                            setError(error, "Cannot decode '" + ElementNames.SET_NAMES.toString() + "' array.");
                            return CodecReturnCodes.FAILURE;
                        }

                        if (arr._primitiveType != DataTypes.BUFFER)
                        {
                            setError(error, "'" + ElementNames.ENUM_VALUE.toString() + "' array has wrong primtive type.");
                            return CodecReturnCodes.FAILURE;
                        }

                        row = 0;

                        while ((ret = arrEntry.decode(iter)) != CodecReturnCodes.END_OF_CONTAINER)
                        {
                            tempBuffer.clear();
                            ret = tempBuffer.decode(iter);
                            if (ret < 0)
                            {
                                setError(error, "DecodeBuffer failed - " + ret);
                                return ret;
                            }

                            tmpByteBuffer = ByteBuffer.allocate(tempBuffer.length());
                            newSetDef.entries()[row].name().data(tmpByteBuffer);
                            tempBuffer.copy(newSetDef.entries()[row].name());
                            row++;
                        }
                    }
                }
                else if (elemEntry._name.equals(ElementNames.SET_TYPES))
                {
                    if (elemEntry._dataType != DataTypes.ARRAY)
                    {
                        setError(error, "Cannot decode '" + ElementNames.SET_TYPES.toString() + "' element.");
                        return CodecReturnCodes.FAILURE;
                    }
                    if (newSetDef.entries()[0] == null)
                        tempTypeArray.data(elemEntry.encodedData().data());
                    else
                    {
                        if (arr.decode(iter) < 0)
                        {
                            setError(error, "Cannot decode '" + ElementNames.SET_TYPES.toString() + "' array.");
                            return CodecReturnCodes.FAILURE;
                        }

                        if (arr._primitiveType != DataTypes.INT)
                        {
                            setError(error, "'" + ElementNames.SET_TYPES.toString() + "' array has wrong primtive type.");
                            return CodecReturnCodes.FAILURE;
                        }

                        row = 0;

                        while ((ret = arrEntry.decode(iter)) != CodecReturnCodes.END_OF_CONTAINER)
                        {
                            tmpInt.clear();
                            ret = tmpInt.decode(iter);
                            if (ret < 0)
                            {
                                setError(error, "DecodeInt failed - " + ret);
                                return ret;
                            }
                            tmpLong = tmpInt.toLong();
                            tmpLong = (short)tmpLong; // Get first 2 bytes
                            newSetDef.entries()[row].dataType((int)tmpLong);
                            row++;
                        }
                    }
                }
            }

            _definitions[newSetDef.setId()] = (ElementSetDefImpl)CodecFactory.createElementSetDef();
            newSetDef.copy(_definitions[newSetDef.setId()]);

            if (maxSetId < newSetDef.setId())
            {
                maxSetId = newSetDef.setId();
            }

        }

        return CodecReturnCodes.SUCCESS;
    }
    
    /* Handle dictionary tags.
     * The logic is put here so the file-loading and wire-decoding versions can be kept close to each other. */
    private int decodeDictionaryTag(DecodeIterator iter, ElementEntryImpl element, int type, Error error)
    {
        int ret;

        tempUInt.clear();
        tempInt.clear();

        if (element._name.equals(ElementNames.DICT_TYPE))
        {
            if ((ret = Decoders.decodeUInt(iter, tempUInt)) < 0)
            {
                setError(error, "DecodeUInt failed - " + ret);
                return CodecReturnCodes.FAILURE;
            }

            if (tempUInt.toLong() != Dictionary.Types.FIELD_SET_DEFINITION)
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
            if (tempInt.toLong() != 0 && info_DictionaryID != 0 && tempInt.toLong() != info_DictionaryID)
            {
                setError(error, "DictionaryId mismatch('" + tempInt.toLong() + "' vs. previously found '" + info_DictionaryID + "').");
                return CodecReturnCodes.FAILURE;
            }
            info_DictionaryID((int)tempInt.toLong());
        }
        else if (element._name.equals(ElementNames.DICT_VERSION))
        {
            if ((ret = Decoders.decodeBuffer(iter, tempBuffer)) < 0)
            {
                setError(error, "DecodeBuffer failed - " + ret);
                return CodecReturnCodes.FAILURE;
            }
            ByteBuffer tmpByteBuffer = ByteBuffer.allocate(tempBuffer.length());
            info_version.data(tmpByteBuffer);
            tempBuffer.copy(info_version);
        }

        return CodecReturnCodes.SUCCESS;
    }
    
    private void setError(Error error, String errorStr)
    {
        if (error != null)
        {
            error.channel(null);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text(errorStr);
        }
    }
    
    @Override
    public int encode(EncodeIterator iter, Int currentSetDef, int verbosity, Error error)
    {
        int ret;
        int state = encState.NONE;
        long curSetDef = currentSetDef.toLong();
        boolean finishedSetDef = false;

        if (maxSetId == -1)
        {
            setError(error, "Global Field Set Definition does not contain any definitions");
            return CodecReturnCodes.FAILURE;
        }

        encVector.clear();

        if (verbosity > Dictionary.VerbosityValues.INFO)
        {
            encVector.applyHasSetDefs();

            encSetDef.clear();

            encSetDef.definitions()[0].count(3);
            encSetDef.definitions()[0].entries(setDef0_Entries);
            encSetDef.definitions()[0].setId(0);
        }

        if (curSetDef == 0)
        {
            encVector.applyHasSummaryData();
        }

        encVector.containerType(DataTypes.ELEMENT_LIST);

        if ((ret = encVector.encodeInit(iter, 0, 0)) < CodecReturnCodes.SUCCESS)
        {
            setError(error, "Vector.encodeInit failed " + ret);
            return (ret == CodecReturnCodes.BUFFER_TOO_SMALL ? ret : CodecReturnCodes.FAILURE);
        }

        state = encState.VECTOR;

        if (verbosity > Dictionary.VerbosityValues.INFO)
        {
            if ((ret = encSetDef.encode(iter)) < CodecReturnCodes.SUCCESS)
            {
                setError(error, "LocalElementSetDefDb.encode() failed " + ret);
                return (ret == CodecReturnCodes.BUFFER_TOO_SMALL ? ret : CodecReturnCodes.FAILURE);
            }

            if ((ret = encVector.encodeSetDefsComplete(iter, true)) < CodecReturnCodes.SUCCESS)
            {
                setError(error, "vector.encodeSetDefsComplete() failed " + ret);
                return (ret == CodecReturnCodes.BUFFER_TOO_SMALL ? ret : CodecReturnCodes.FAILURE);
            }
        }

        if (curSetDef == 0)
        {
            encElemList.clear();
            encElemList.applyHasStandardData();

            if ((ret = encElemList.encodeInit(iter, null, 0)) < CodecReturnCodes.SUCCESS)
            {
                setError(error, "encElemlist.encodeInit() failed " + ret);
                return (ret == CodecReturnCodes.BUFFER_TOO_SMALL ? ret : CodecReturnCodes.FAILURE);
            }

            encElement.name(ElementNames.DICT_TYPE);
            encElement.dataType(DataTypes.UINT);
            tempUInt.value(Dictionary.Types.FIELD_SET_DEFINITION);

            if ((ret = encElement.encode(iter, tempUInt)) < CodecReturnCodes.SUCCESS)
            {
                setError(error, "encElemEntry.encode() failed " + ret);
                return (ret == CodecReturnCodes.BUFFER_TOO_SMALL ? ret : CodecReturnCodes.FAILURE);
            }

            encElement.name(ElementNames.VERSION);
            encElement.dataType(DataTypes.BUFFER);

            if ((ret = encElement.encode(iter, info_version)) < CodecReturnCodes.SUCCESS)
            {
                setError(error, "encElemEntry.encode() failed " + ret);
                return (ret == CodecReturnCodes.BUFFER_TOO_SMALL ? ret : CodecReturnCodes.FAILURE);
            }

            encElement.name(ElementNames.DICTIONARY_ID);
            encElement.dataType(DataTypes.UINT);
            tempUInt.value(info_DictionaryID);

            if ((ret = encElement.encode(iter, tempUInt)) < CodecReturnCodes.SUCCESS)
            {
                setError(error, "encElemEntry.encode() failed " + ret);
                return (ret == CodecReturnCodes.BUFFER_TOO_SMALL ? ret : CodecReturnCodes.FAILURE);
            }

            if ((ret = elemList.encodeComplete(iter, true)) < CodecReturnCodes.SUCCESS)
            {
                setError(error, "ElementList.encodeComplete() failed " + ret);
                return (ret == CodecReturnCodes.BUFFER_TOO_SMALL ? ret : CodecReturnCodes.FAILURE);
            }

            if ((ret = encVector.encodeSummaryDataComplete(iter, true)) < CodecReturnCodes.SUCCESS)
            {
                setError(error, "Vector.encodeSummaryDataComplete() failed " + ret);
                return (ret == CodecReturnCodes.BUFFER_TOO_SMALL ? ret : CodecReturnCodes.FAILURE);
            }
        }

        if (verbosity > Dictionary.VerbosityValues.INFO)
        {
            while (curSetDef <= maxSetId)
            {
                if (_definitions[(int)curSetDef] != null && _definitions[(int)curSetDef]._setId != BLANK_ID)
                {
                    encVectorEntry.clear();

                    encVectorEntry.action(VectorEntryActions.SET);
                    encVectorEntry.index(curSetDef);

                    if ((ret = encVectorEntry.encodeInit(iter, 0)) < CodecReturnCodes.SUCCESS)
                    {
                        setError(error, "VectorEntry.encodeInit() failed " + ret);
                        return rollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
                    }

                    state = encState.VECTOR_ENTRY;

                    encElemList.clear();
                    encElemList.applyHasSetData();
                    encElemList.applyHasSetId();
                    encElemList.setId(0);

                    if ((ret = encElemList.encodeInit(iter, encSetDef, 0)) < CodecReturnCodes.SUCCESS)
                    {
                        setError(error, "elementList.encodeInit() failed " + ret);
                        return rollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
                    }

                    state = encState.ELEM_LIST;

                    encElement.clear();
                    encElement.dataType(DataTypes.INT);
                    encElement.name(ElementNames.SET_NUMENTRIES);
                    tmpInt.value(_definitions[(int)curSetDef].count());

                    if ((ret = encElement.encode(iter, tmpInt)) < CodecReturnCodes.SUCCESS)
                    {
                        setError(error, "element.encode() failed " + ret);
                        return rollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
                    }

                    encElement.clear();
                    encElement.dataType(DataTypes.ARRAY);
                    encElement.name(ElementNames.SET_NAMES);

                    if ((ret = encElement.encodeInit(iter, 0)) < CodecReturnCodes.SUCCESS)
                    {
                        setError(error, "element.encodeInit() failed " + ret);
                        return rollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
                    }

                    state = encState.ELEM_ENTRY;

                    encArray.clear();
                    encArray.primitiveType(DataTypes.BUFFER);

                    if ((ret = encArray.encodeInit(iter)) < CodecReturnCodes.SUCCESS)
                    {
                        setError(error, "array.encodeInit() failed " + ret);
                        return rollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
                    }

                    state = encState.ARRAY;

                    for (int i = 0; i < _definitions[(int)curSetDef].count(); i++)
                    {
                        encArrayEntry.clear();
                        if ((ret = encArrayEntry.encode(iter, _definitions[(int)curSetDef]._entries[i].name()))
                                < CodecReturnCodes.SUCCESS)
                        {
                            setError(error, "arrayEntry.encode() failed " + ret);
                            return rollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
                        }
                    }

                    if ((ret = encArray.encodeComplete(iter, true)) < CodecReturnCodes.SUCCESS)
                    {
                        setError(error, "array.encodeInit() failed " + ret);
                        return rollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
                    }

                    state = encState.ELEM_ENTRY;

                    if ((ret = encElement.encodeComplete(iter, true)) < CodecReturnCodes.SUCCESS)
                    {
                        setError(error, "element.encodeComplete() failed " + ret);
                        return rollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
                    }

                    state = encState.ELEM_LIST;

                    encElement.clear();
                    encElement.dataType(DataTypes.ARRAY);
                    encElement.name(ElementNames.SET_TYPES);

                    if ((ret = encElement.encodeInit(iter, 0)) < CodecReturnCodes.SUCCESS)
                    {
                        setError(error, "element.encodeInit() failed " + ret);
                        return rollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
                    }

                    state = encState.ELEM_ENTRY;

                    encArray.clear();
                    encArray.primitiveType(DataTypes.INT);

                    if ((ret = encArray.encodeInit(iter)) < CodecReturnCodes.SUCCESS)
                    {
                        setError(error, "array.encodeInit() failed " + ret);
                        return rollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
                    }

                    state = encState.ARRAY;

                    for (int i = 0; i < _definitions[(int)curSetDef].count(); i++)
                    {
                        encArrayEntry.clear();
                        tmpInt.value(_definitions[(int)curSetDef]._entries[i].dataType());
                        if ((ret = encArrayEntry.encode(iter, tmpInt)) < CodecReturnCodes.SUCCESS)
                        {
                            setError(error, "arrayEntry.encode() failed " + ret);
                            return rollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
                        }
                    }

                    if ((ret = encArray.encodeComplete(iter, true)) < CodecReturnCodes.SUCCESS)
                    {
                        setError(error, "array.encodeInit() failed " + ret);
                        return rollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
                    }

                    state = encState.ELEM_ENTRY;

                    if ((ret = encElement.encodeComplete(iter, true)) < CodecReturnCodes.SUCCESS)
                    {
                        setError(error, "element.encodeComplete() failed " + ret);
                        return rollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
                    }

                    state = encState.ELEM_LIST;

                    if ((ret = encElemList.encodeComplete(iter, true)) < CodecReturnCodes.SUCCESS)
                    {
                        setError(error, "element.encodeComplete() failed " + ret);
                        return rollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
                    }

                    state = encState.VECTOR_ENTRY;

                    if ((ret = encVectorEntry.encodeComplete(iter, true)) < CodecReturnCodes.SUCCESS)
                    {
                        setError(error, "VectorEntry.encodeComplete() failed " + ret);
                        return rollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
                    }

                    finishedSetDef = true;
                    state = encState.VECTOR;
                }

                curSetDef++;
            }

            if ((ret = encVector.encodeComplete(iter, true)) < CodecReturnCodes.SUCCESS)
            {
                setError(error, "Vector.encodeComplete() failed " + ret);
                return rollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
            }

            currentSetDef.value(curSetDef);
        }
        else
        {
            currentSetDef.value(maxSetId);
        }

        return CodecReturnCodes.SUCCESS;
    }
    
    @SuppressWarnings("fallthrough")
	private int rollBack(EncodeIterator iter, int state, boolean finishedSet, Int currentSet, long curSet, Error error)
    {
        int ret;
        switch (state)
        {
        	// fall through here to unroll a stateful encode (go from the primitive Array to top level Vector)
            case encState.ARRAY:
                if ((ret = encArray.encodeComplete(iter, false)) < CodecReturnCodes.SUCCESS)
                {
                    setError(error, "array.encodeComplete() failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }
            case encState.ELEM_ENTRY:
                if ((ret = encElement.encodeComplete(iter, false)) < CodecReturnCodes.SUCCESS)
                {
                    setError(error, "ElementEntry.encodeComplete() failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }
            case encState.ELEM_LIST:
                if ((ret = encElemList.encodeComplete(iter, false)) < CodecReturnCodes.SUCCESS)
                {
                    setError(error, "ElementList.encodeComplete() failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }
            case encState.VECTOR_ENTRY:
                if ((ret = encVectorEntry.encodeComplete(iter, false)) < CodecReturnCodes.SUCCESS)
                {
                    setError(error, "VectorEntry.encodeComplete() failed " + ret);
                    return CodecReturnCodes.FAILURE;
                }
            case encState.VECTOR:
                if (finishedSet == true)
                {
                    if ((ret = encVector.encodeComplete(iter, true)) < CodecReturnCodes.SUCCESS)
                    {
                        setError(error, "Vector.encodeComplete() failed " + ret);
                        return CodecReturnCodes.FAILURE;
                    }

                    currentSet.value(curSet);
                    return CodecReturnCodes.DICT_PART_ENCODED;
                }
                return CodecReturnCodes.FAILURE;
            default:
                break;
        }

        return CodecReturnCodes.FAILURE;
    }
    
    public int addSetDef(ElementSetDef setDef, Error error)
    {
        ElementSetDefEntry[] tempEntries;
        if (_definitions[setDef.setId()] != null)
        {
            setError(error, "Set Definition is already present in set def db");
            return CodecReturnCodes.FAILURE;
        }

        ElementSetDefImpl newDef = (ElementSetDefImpl)CodecFactory.createElementSetDef();

        newDef.count(setDef.count());
        newDef.setId(setDef.setId());

        tempEntries = new ElementSetDefEntry[newDef.count()];
        newDef.entries(tempEntries);

        for (int i = 0; i < setDef.count(); i++)
        {
            newDef.entries()[i] = CodecFactory.createElementSetDefEntry();
            newDef.entries()[i].name(setDef.entries()[i].name());
            newDef.entries()[i].dataType(setDef.entries()[i].dataType());
        }

        _definitions[setDef.setId()] = newDef;

        if (maxSetId < setDef.setId())
        {
            maxSetId = setDef.setId();
        }

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public ElementSetDef[] definitions()
    {
        return _definitions;
    }

    @Override
    public int maxSetId()
    {
        return maxSetId;
    }

    @Override
    public void maxSetId(int setMaxSetId)
    {
        maxSetId = setMaxSetId;
    }
        
    @Override
    public Buffer info_version()
    {
        return info_version;
    }

    @Override
    public void info_version(Buffer setInfo_version)
    {
        info_version = setInfo_version;
    }

    @Override
    public int info_DictionaryID()
    {
        return info_DictionaryID;
    }
    
    @Override
    public void info_DictionaryID(int setInfo_DictionaryID)
    {
        info_DictionaryID = setInfo_DictionaryID;
    }
}
