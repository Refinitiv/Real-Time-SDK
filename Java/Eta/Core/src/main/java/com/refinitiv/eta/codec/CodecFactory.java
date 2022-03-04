/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import java.nio.ByteBuffer;

/**
 * Factory for encoder/decoder package objects.
 */
public class CodecFactory
{
    
    /**
     * This class is not instantiated.
     */
    private CodecFactory()
    {
        throw new AssertionError();
    }

    /**
     * Creates a {@link Msg} that may be cast to any message class defined by
     * {@link MsgClasses} (e.g. {@link AckMsg}, {@link CloseMsg},
     * {@link GenericMsg}, {@link PostMsg}, {@link RefreshMsg},
     * {@link RequestMsg}, {@link StatusMsg}, {@link UpdateMsg})
     * 
     * @return Msg object
     * 
     * @see MsgClasses
     * @see AckMsg
     * @see CloseMsg
     * @see GenericMsg
     * @see PostMsg
     * @see RefreshMsg
     * @see RequestMsg
     * @see StatusMsg
     * @see UpdateMsg
     */
    public static Msg createMsg()
    {
        return new MsgImpl();
    }

    /**
     * Creates {@link MsgKey}.
     * 
     * @return MsgKey object
     * 
     * @see MsgKey
     */
    public static MsgKey createMsgKey()
    {
        return new MsgKeyImpl();
    }

    /**
     * Creates {@link Buffer} with no data.
     * {@link Buffer#data(java.nio.ByteBuffer)} or {@link Buffer#data(String)}
     * must be called later for buffer to be useful.
     * 
     * @return Buffer object
     * 
     * @see Buffer
     */
    public static Buffer createBuffer()
    {
        return new BufferImpl();
    }

    /**
     * Creates {@link Date}.
     * 
     * @return Date object
     * 
     * @see Date
     */
    public static Date createDate()
    {
        return new DateImpl();
    }

    /**
     * Creates {@link DateTime}.
     * 
     * @return {@link DateTime} object
     * 
     * @see DateTime
     */
    public static DateTime createDateTime()
    {
        return new DateTimeImpl();
    }

    /**
     * Creates {@link DataDictionary}.
     * 
     * @return {@link DataDictionary} object
     * 
     * @see DataDictionary
     */
    public static DataDictionary createDataDictionary()
    {
        return new DataDictionaryImpl();
    }
    
    /**
     * Creates {@link GlobalFieldSetDefDb}.
     * 
     * @return {@link GlobalFieldSetDefDb} object
     * 
     * @see DataDictionary
     */
    public static GlobalFieldSetDefDb createGlobalFieldSetDefDb()
    {
        return new GlobalFieldSetDefDbImpl();
    }
    
    /**
     * Creates {@link GlobalFieldSetDefDb}.
     * 
     * @return {@link GlobalFieldSetDefDb} object
     * 
     * @see DataDictionary
     */
    public static GlobalElementSetDefDb createGlobalElementSetDefDb()
    {
        return new GlobalElementSetDefDbImpl();
    }

    /**
     * Creates {@link ElementEntry}.
     * 
     * @return {@link ElementEntry} object
     * 
     * @see ElementEntry
     */
    public static ElementEntry createElementEntry()
    {
        return new ElementEntryImpl();
    }

    /**
     * Creates {@link ElementList}.
     * 
     * @return {@link ElementList} object
     * 
     * @see ElementList
     */
    public static ElementList createElementList()
    {
        return new ElementListImpl();
    }

    /**
     * Creates {@link DecodeIterator}.
     * 
     * @return {@link DecodeIterator} object
     * 
     * @see DecodeIterator
     */
    public static DecodeIterator createDecodeIterator()
    {
        return new DecodeIteratorImpl();
    }

    /**
     * Creates {@link EncodeIterator}.
     * 
     * @return EncodeIterator object
     * 
     * @see EncodeIterator
     */
    public static EncodeIterator createEncodeIterator()
    {
        return new EncodeIteratorImpl();
    }

    /**
     * Creates {@link FieldList}.
     * 
     * @return FieldList object
     * 
     * @see FieldList
     */
    public static FieldList createFieldList()
    {
        return new FieldListImpl();
    }

    /**
     * Creates {@link FieldSetDef}.
     * 
     * @return FieldSetDef object
     * 
     * @see FieldSetDef
     */
    public static FieldSetDef createFieldSetDef()
    {
        return new FieldSetDefImpl();
    }

    /**
     * Creates {@link FieldSetDefEntry}.
     * 
     * @return FieldSetDefEntry object
     * 
     * @see FieldSetDefEntry
     */
    public static FieldSetDefEntry createFieldSetDefEntry()
    {
        return new FieldSetDefEntryImpl();
    }

    /**
     * Creates {@link FilterEntry}.
     * 
     * @return FilterEntry object
     * 
     * @see FilterEntry
     */
    public static FilterEntry createFilterEntry()
    {
        return new FilterEntryImpl();
    }

    /**
     * Creates {@link ElementSetDef}.
     * 
     * @return ElementSetDef object
     * 
     * @see ElementSetDef
     */
    public static ElementSetDef createElementSetDef()
    {
        return new ElementSetDefImpl();
    }

    /**
     * Creates array of {@link ElementSetDef} with specified size.
     * 
     * @param size of the array.
     * 
     * @return ElementSetDef[]
     * 
     * @see ElementSetDef
     */
    public static ElementSetDef[] createElementSetDefArray(int size)
    {
        assert (size > 0) : "size must be greater than zero";

        return new ElementSetDefImpl[size];
    }

    /**
     * Creates {@link ElementSetDefEntry}.
     * 
     * @return ElementSetDefEntry object
     * 
     * @see ElementSetDefEntry
     */
    public static ElementSetDefEntry createElementSetDefEntry()
    {
        return new ElementSetDefEntryImpl();
    }

    /**
     * Creates {@link FieldEntry}.
     * 
     * @return FieldEntry object
     * 
     * @see FieldEntry
     */
    public static FieldEntry createFieldEntry()
    {
        return new FieldEntryImpl();
    }

    /**
     * Creates {@link FilterList}.
     * 
     * @return FilterList object
     * 
     * @see FilterList
     */
    public static FilterList createFilterList()
    {
        return new FilterListImpl();
    }

    /**
     * Creates {@link Int}.
     * 
     * @return Int object
     * 
     * @see Int
     */
    public static Int createInt()
    {
        return new IntImpl();
    }

    /**
     * Creates {@link LocalElementSetDefDb}.
     * 
     * @return LocalElementSetDefDb object
     * 
     * @see LocalElementSetDefDb
     */
    public static LocalElementSetDefDb createLocalElementSetDefDb()
    {
        return new LocalElementSetDefDbImpl();
    }

    /**
     * Creates {@link LocalFieldSetDefDb}.
     * 
     * @return LocalFieldSetDefDb object
     * 
     * @see LocalFieldSetDefDb
     */
    public static LocalFieldSetDefDb createLocalFieldSetDefDb()
    {
        return new LocalFieldSetDefDbImpl();
    }

    /**
     * Creates {@link MapEntry}.
     * 
     * @return MapEntry object
     * 
     * @see MapEntry
     */
    public static MapEntry createMapEntry()
    {
        return new MapEntryImpl();
    }

    /**
     * Creates {@link Array}.
     * 
     * @return Array object
     * 
     * @see Array
     */
    public static Array createArray()
    {
        return new ArrayImpl();
    }

    /**
     * Creates {@link ArrayEntry}.
     * 
     * @return ArrayEntry object
     * 
     * @see ArrayEntry
     */
    public static ArrayEntry createArrayEntry()
    {
        return new ArrayEntryImpl();
    }

    /**
     * Creates {@link Vector}.
     * 
     * @return Vector object
     * 
     * @see Vector
     */
    public static Vector createVector()
    {
        return new VectorImpl();
    }

    /**
     * Creates {@link VectorEntry}.
     * 
     * @return VectorEntry object
     * 
     * @see VectorEntry
     */
    public static VectorEntry createVectorEntry()
    {
        return new VectorEntryImpl();
    }

    /**
     * Creates {@link UInt}.
     * 
     * @return UInt object
     * 
     * @see UInt
     */
    public static UInt createUInt()
    {
        return new UIntImpl();
    }

    /**
     * Creates {@link Qos}.
     * 
     * @return Qos object
     * 
     * @see Qos
     */
    public static Qos createQos()
    {
        return new QosImpl();
    }

    /**
     * Creates {@link Time}.
     * 
     * @return Time object
     * 
     * @see Time
     */
    public static Time createTime()
    {
        return new TimeImpl();
    }

    /**
     * Creates {@link State}.
     * 
     * @return State object
     * 
     * @see State
     */
    public static State createState()
    {
        return new StateImpl();
    }

    /**
     * Creates {@link Series}.
     * 
     * @return Series object
     * 
     * @see Series
     */
    public static Series createSeries()
    {
        return new SeriesImpl();
    }

    /**
     * Creates {@link SeriesEntry}.
     * 
     * @return SeriesEntry object
     * 
     * @see SeriesEntry
     */
    public static SeriesEntry createSeriesEntry()
    {
        return new SeriesEntryImpl();
    }

    /**
     * Creates {@link Real}.
     * 
     * @return Real object
     * 
     * @see Real
     */
    public static Real createReal()
    {
        return new RealImpl();
    }

    /**
     * Creates {@link Enum}.
     * 
     * @return Enum object
     * 
     * @see Enum
     */
    public static Enum createEnum()
    {
        return new EnumImpl();
    }

    /**
     * Creates {@link Map}.
     * 
     * @return Map object
     * 
     * @see Map
     */
    public static Map createMap()
    {
        return new MapImpl();
    }

    /**
     * Creates {@link Float}.
     * 
     * @return Float object
     * 
     * @see Float
     */
    public static Float createFloat()
    {
        return new FloatImpl();
    }

    /**
     * Creates {@link Double}.
     * 
     * @return Double object
     * 
     * @see Double
     */
    public static Double createDouble()
    {
        return new DoubleImpl();
    }

    /**
     * Creates {@link PostUserInfo}.
     * 
     * @return PostUserInfo object
     * 
     * @see PostUserInfo
     */
    public static PostUserInfo createPostUserInfo()
    {
        return new PostUserInfoImpl();
    }
    
    /**
     * Creates {@link RmtesDecoder}.
     * 
     * @return RmtesDecoder object
     * 
     * @see RmtesDecoder
     */
    public static RmtesDecoder createRmtesDecoder()
    {
        return new RmtesDecoderImpl();
    }
    
    /**
     * Creates {@link RmtesBuffer}.
     *
     * @param x - allocated length of RmtesBuffer
     * @return RmtesBuffer object
     * @see RmtesBuffer
     */
    public static RmtesBuffer createRmtesBuffer(int x)
    {
        return new RmtesBufferImpl(x);
    }
    
    /**
     * Creates {@link RmtesBuffer}.
     *
     * @param dataLength - length of data being stored
     * @param byteBuffer - ByteBuffer to store into RmtesBuffer
     * @param allocLength - allocated length set for RmtesBuffer
     * @return RmtesBuffer object
     * @see RmtesBuffer
     */
    public static RmtesBuffer createRmtesBuffer(int dataLength, ByteBuffer byteBuffer, int allocLength)
    {
        return new RmtesBufferImpl(dataLength, byteBuffer, allocLength);
    }
    
    /**
     * Creates {@link RmtesCacheBuffer}.
     *
     * @param x - allocated length of RmtesBuffer
     * @return RmtesCacheBuffer object
     * @see RmtesCacheBuffer
     */
    public static RmtesCacheBuffer createRmtesCacheBuffer(int x)
    {
        return new RmtesCacheBufferImpl(x);
    }
    
    /**
     * Creates {@link RmtesCacheBuffer}.
     *
     * @param dataLength - length of data being stored
     * @param byteBuffer - ByteBuffer to store into RmtesCacheBuffer
     * @param allocLength - allocated length set for RmtesCacheBuffer
     * @return RmtesCacheBuffer object
     * @see RmtesCacheBuffer
     */
    public static RmtesCacheBuffer createRmtesCacheBuffer(int dataLength, ByteBuffer byteBuffer, int allocLength)
    {
        return new RmtesCacheBufferImpl(dataLength, byteBuffer, allocLength);
    }
    
    /**
     * Creates {@link FieldSetDefDb}.
     *
     * @param x the x
     * @return FieldSetDefDb object
     * @see FieldSetDefDb
     */
    public static FieldSetDefDb createFieldSetDefDb(int x)
    {
        return new FieldSetDefDbImpl(x);
    }
    
    /**
     * Creates {@link ElementSetDefDb}.
     *
     * @param x the x
     * @return ElementSetDefDb object
     * @see ElementSetDefDb
     */
    public static ElementSetDefDb createElementSetDefDb(int x)
    {
        return new ElementSetDefDbImpl(x);
    }

    public static XmlTraceDump createXmlTraceDump() {
        return new XmlTraceDumpImpl();
    }
}
