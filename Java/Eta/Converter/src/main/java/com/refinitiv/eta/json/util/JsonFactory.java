package com.refinitiv.eta.json.util;

import com.refinitiv.eta.codec.Double;
import com.refinitiv.eta.codec.Enum;
import com.refinitiv.eta.codec.Float;
import com.refinitiv.eta.codec.*;

import java.nio.ByteBuffer;

public class JsonFactory {
    private static ObjectPool<Int> intPool = new ObjectPool<>(true, CodecFactory::createInt);
    private static ObjectPool<UInt> uintPool = new ObjectPool<>(true, CodecFactory::createUInt);
    private static ObjectPool<ElementList> elementListPool = new ObjectPool<>(true, CodecFactory::createElementList);
    private static ObjectPool<ElementEntry> elementEntryPool = new ObjectPool<>(true, CodecFactory::createElementEntry);
    private static ObjectPool<Buffer> bufferPool = new ObjectPool<>(true, CodecFactory::createBuffer);
    private static ObjectPool<FieldList> fieldListPool = new ObjectPool<>(true, CodecFactory::createFieldList);
    private static ObjectPool<FieldEntry> fieldEntryPool = new ObjectPool<>(true, CodecFactory::createFieldEntry);
    private static ObjectPool<Vector> vectorPool = new ObjectPool<>(true, CodecFactory::createVector);
    private static ObjectPool<VectorEntry> vectorEntryPool = new ObjectPool<>(true, CodecFactory::createVectorEntry);
    private static ObjectPool<Series> seriesPool = new ObjectPool<>(true, CodecFactory::createSeries);
    private static ObjectPool<SeriesEntry> seriesEntryPool = new ObjectPool<>(true, CodecFactory::createSeriesEntry);
    private static ObjectPool<FilterList> filterListPool = new ObjectPool<>(true, CodecFactory::createFilterList);
    private static ObjectPool<FilterEntry> filterEntryPool = new ObjectPool<>(true, CodecFactory::createFilterEntry);
    private static ObjectPool<Real> realPool = new ObjectPool<>(true, CodecFactory::createReal);
    private static ObjectPool<Double> doublePool = new ObjectPool<>(true, CodecFactory::createDouble);
    private static ObjectPool<Float> floatPool = new ObjectPool<>(true, CodecFactory::createFloat);
    private static ObjectPool<State> statePool = new ObjectPool<>(true, CodecFactory::createState);
    private static ObjectPool<Qos> qosPool = new ObjectPool<>(true, CodecFactory::createQos);
    private static ObjectPool<Map> mapPool = new ObjectPool<>(true, CodecFactory::createMap);
    private static ObjectPool<MapEntry> mapEntryPool = new ObjectPool<>(true, CodecFactory::createMapEntry);
    private static ObjectPool<Enum> enumPool = new ObjectPool<>(true, CodecFactory::createEnum);
    private static ObjectPool<Time> timePool = new ObjectPool<>(true, CodecFactory::createTime);
    private static ObjectPool<Date> datePool = new ObjectPool<>(true, CodecFactory::createDate);
    private static ObjectPool<DateTime> dateTimePool = new ObjectPool<>(true, CodecFactory::createDateTime);
    private static ObjectPool<Array> arrayPool = new ObjectPool<>(true, CodecFactory::createArray);
    private static ObjectPool<ArrayEntry> arrayEntryPool = new ObjectPool<>(true, CodecFactory::createArrayEntry);
    private static ObjectPool<Msg> msgPool = new ObjectPool<>(true, CodecFactory::createMsg);

    private static ObjectPool<LocalFieldSetDefDb> fieldSetDefDbPool = new ObjectPool<>(true, CodecFactory::createLocalFieldSetDefDb);
    private static ObjectPool<LocalElementSetDefDb> elementSetDefDbPool = new ObjectPool<>(true, CodecFactory::createLocalElementSetDefDb);
    private static ObjectPool<DecodeIterator> decodeIterPool = new ObjectPool<>(true, CodecFactory::createDecodeIterator);

    private static ObjectPool<EncodeIterator> encodeIteratorPool = new ObjectPool<>(true, CodecFactory::createEncodeIterator);
    public static final int DEFAULT_BYTEBUFFER_SIZE = 4096;
    private static ObjectPool<ByteBuffer> byteBufferPool = new ObjectPool<>(true, () -> ByteBuffer.allocate(DEFAULT_BYTEBUFFER_SIZE));
    private static ByteArrayPool byteArrayPool = new ByteArrayPool();

    private JsonFactory() {
        throw new AssertionError();
    }

    public static Int createInt() {
        return intPool.get();
    }

    public static void releaseInt(Int objInt) {
        intPool.release(objInt);
    }

    public static UInt createUInt() {
        return uintPool.get();
    }

    public static void releaseUInt(UInt objUInt) {
        uintPool.release(objUInt);
    }

    public static ElementList createElementList() {
        return elementListPool.get();
    }
    public static void releaseElementList(ElementList objElementList) {
        elementListPool.release(objElementList);
    }

    public static ElementEntry createElementEntry() {
        return elementEntryPool.get();
    }

    public static void releaseElementEntry(ElementEntry objElementEntry) {
        elementEntryPool.release(objElementEntry);
    }

    public static Buffer createBuffer() {return bufferPool.get();}
    public static void releaseBuffer(Buffer objBuffer) {bufferPool.release(objBuffer);}

    public static FieldList createFieldList() {return fieldListPool.get();}
    public static void releaseFieldList(FieldList objBuffer) {fieldListPool.release(objBuffer);}

    public static FieldEntry createFieldEntry() {return fieldEntryPool.get();}
    public static void releaseFieldEntry(FieldEntry objBuffer) {fieldEntryPool.release(objBuffer);}

    public static Vector createVector() {return vectorPool.get();}
    public static void releaseVector(Vector objBuffer) {vectorPool.release(objBuffer);}

    public static VectorEntry createVectorEntry() {return vectorEntryPool.get();}
    public static void releaseVectorEntry(VectorEntry objBuffer) {vectorEntryPool.release(objBuffer);}

    public static Series createSeries() {return seriesPool.get();}
    public static void releaseSeries(Series objBuffer) {seriesPool.release(objBuffer);}

    public static SeriesEntry createSeriesEntry() {return seriesEntryPool.get();}
    public static void releaseSeriesEntry(SeriesEntry objBuffer) {seriesEntryPool.release(objBuffer);}

    public static FilterList createFilterList() {return filterListPool.get();}
    public static void releaseFilterList(FilterList objBuffer) {filterListPool.release(objBuffer);}

    public static FilterEntry createFilterEntry() {return filterEntryPool.get();}
    public static void releaseFilterEntry(FilterEntry objBuffer) {filterEntryPool.release(objBuffer);}

    public static MapEntry createMapEntry() {return mapEntryPool.get();}
    public static void releaseMapEntry(MapEntry objBuffer) {mapEntryPool.release(objBuffer);}

    public static Map createMap() {return mapPool.get();}
    public static void releaseMap(Map objMap) {mapPool.release(objMap);}

    public static Real createReal() {return realPool.get();}
    public static void releaseReal(Real objReal) {realPool.release(objReal);}

    public static Time createTime() {return timePool.get();}
    public static void releaseTime(Time objTime) {timePool.release(objTime);}

    public static State createState() {return statePool.get();}
    public static void releaseState(State objState) {statePool.release(objState);}

    public static Date createDate() {return datePool.get();}
    public static void releaseDate(Date objDate) {datePool.release(objDate);}

    public static DateTime createDateTime() {return dateTimePool.get();}
    public static void releaseDateTime(DateTime objDateTime) {dateTimePool.release(objDateTime);}

    public static Qos createQos() {return qosPool.get();}
    public static void releaseQos(Qos objQos) {qosPool.release(objQos);}

    public static Float createFloat() {return floatPool.get();}
    public static void releaseFloat(Float objFloat) {floatPool.release(objFloat);}

    public static Double createDouble() {return doublePool.get();}
    public static void releaseDouble(Double objDouble) {doublePool.release(objDouble);}

    public static Enum createEnum() {return enumPool.get();}
    public static void releaseEnum(Enum objEnum) {enumPool.release(objEnum);}

    public static void releaseFieldSetDefDb(LocalFieldSetDefDb objBuffer) {fieldSetDefDbPool.release(objBuffer);}
    public static void releaseElementSetDefDb(LocalElementSetDefDb objBuffer) {elementSetDefDbPool.release(objBuffer);}

    public static LocalElementSetDefDb createLocalElementSetDefDb() {return elementSetDefDbPool.get();}
    public static LocalFieldSetDefDb createLocalFieldSetDefDb() {return fieldSetDefDbPool.get();}

    public static DecodeIterator createDecodeIterator() {return decodeIterPool.get();}
    public static void releaseDecodeIterator(DecodeIterator objBuffer) {decodeIterPool.release(objBuffer);}

    public static Array createArray() {
        return arrayPool.get();
    }

    public static void releaseArray(Array objArray) {
        arrayPool.release(objArray);
    }

    public static ArrayEntry createArrayEntry() {
        return arrayEntryPool.get();
    }

    public static void releaseArrayEntry(ArrayEntry objArrayEntry) {
        arrayEntryPool.release(objArrayEntry);
    }

    public static Msg createMsg() {
        return msgPool.get();
    }

    public static void releaseMsg(Msg objMsg) {
        msgPool.release(objMsg);
    }

    public static EncodeIterator createEncodeIterator() {
        return encodeIteratorPool.get();
    }

    public static void releaseEncodeIterator(EncodeIterator objEncodeIterator) {
        encodeIteratorPool.release(objEncodeIterator);
    }

    public static ByteBuffer createByteBuffer() {
        return byteBufferPool.get();
    }

    public static void releaseByteBuffer(ByteBuffer objByteBuffer) {
        byteBufferPool.release(objByteBuffer);
    }

    public static Object getPrimitiveType(int dataType) {
        switch (dataType) {
            case DataTypes.ARRAY:
                return createArray();
            case DataTypes.UTF8_STRING:
            case DataTypes.ASCII_STRING:
            case DataTypes.RMTES_STRING:
            case DataTypes.BUFFER:
                return createBuffer();
            case DataTypes.DATE:
            case DataTypes.DATE_4:
                return createDate();
            case DataTypes.TIME:
            case DataTypes.TIME_3:
            case DataTypes.TIME_5:
            case DataTypes.TIME_7:
            case DataTypes.TIME_8:
                return createTime();
            case DataTypes.REAL:
            case DataTypes.REAL_4RB:
            case DataTypes.REAL_8RB:
                return createReal();
            case DataTypes.INT:
            case DataTypes.INT_1:
            case DataTypes.INT_2:
            case DataTypes.INT_4:
            case DataTypes.INT_8:
                return createInt();
            case DataTypes.UINT:
            case DataTypes.UINT_1:
            case DataTypes.UINT_2:
            case DataTypes.UINT_4:
            case DataTypes.UINT_8:
                return createUInt();
            case DataTypes.DATETIME:
            case DataTypes.DATETIME_7:
            case DataTypes.DATETIME_9:
            case DataTypes.DATETIME_11:
            case DataTypes.DATETIME_12:
                return createDateTime();
            case DataTypes.QOS:
                return createQos();
            case DataTypes.STATE:
                return createState();
            case DataTypes.DOUBLE:
            case DataTypes.DOUBLE_8:
                return createDouble();
            case DataTypes.FLOAT:
            case DataTypes.FLOAT_4:
                return createFloat();
            case DataTypes.ENUM:
                return createEnum();
            default:
                return null;
        }
    }

    public static void releasePrimitiveType(int dataType, Object primitive) {
        switch (dataType) {
            case DataTypes.ARRAY:
                releaseArray((Array) primitive);
                break;
            case DataTypes.UTF8_STRING:
            case DataTypes.ASCII_STRING:
            case DataTypes.RMTES_STRING:
            case DataTypes.BUFFER:
                releaseBuffer((Buffer) primitive);
                break;
            case DataTypes.DATE:
            case DataTypes.DATE_4:
                releaseDate((Date) primitive);
                break;
            case DataTypes.TIME:
            case DataTypes.TIME_3:
            case DataTypes.TIME_5:
            case DataTypes.TIME_7:
            case DataTypes.TIME_8:
                releaseTime((Time) primitive);
                break;
            case DataTypes.REAL:
            case DataTypes.REAL_4RB:
            case DataTypes.REAL_8RB:
                releaseReal((Real) primitive);
                break;
            case DataTypes.INT:
            case DataTypes.INT_1:
            case DataTypes.INT_2:
            case DataTypes.INT_4:
            case DataTypes.INT_8:
                releaseInt((Int) primitive);
                break;
            case DataTypes.UINT:
            case DataTypes.UINT_1:
            case DataTypes.UINT_2:
            case DataTypes.UINT_4:
            case DataTypes.UINT_8:
                releaseUInt((UInt) primitive);
                break;
            case DataTypes.DATETIME:
            case DataTypes.DATETIME_7:
            case DataTypes.DATETIME_9:
            case DataTypes.DATETIME_11:
            case DataTypes.DATETIME_12:
                releaseDateTime((DateTime) primitive);
                break;
            case DataTypes.QOS:
                releaseQos((Qos) primitive);
                break;
            case DataTypes.STATE:
                releaseState((State) primitive);
                break;
            case DataTypes.DOUBLE:
            case DataTypes.DOUBLE_8:
                releaseDouble((Double) primitive);
                break;
            case DataTypes.FLOAT:
            case DataTypes.FLOAT_4:
                releaseFloat((Float) primitive);
                break;
            case DataTypes.ENUM:
                releaseEnum((Enum) primitive);
                break;
            default:
                break;
        }
    }

    public static byte[] createByteArray(int length) {
        return byteArrayPool.poll(length);
    }

    public static void releaseByteArray(byte[] array) {
        byteArrayPool.putBack(array);
    }


    public static void initPools(int numOfObjects) {

       intPool.growPool(numOfObjects);
       uintPool.growPool(numOfObjects);
       elementListPool.growPool(numOfObjects);
       elementEntryPool.growPool(numOfObjects);
       bufferPool.growPool(numOfObjects);
       fieldListPool.growPool(numOfObjects);
       fieldEntryPool.growPool(numOfObjects);
       vectorPool.growPool(numOfObjects);
       vectorEntryPool.growPool(numOfObjects);
       seriesPool.growPool(numOfObjects);
       seriesEntryPool.growPool(numOfObjects);
       filterListPool.growPool(numOfObjects);
       filterEntryPool.growPool(numOfObjects);
       realPool.growPool(numOfObjects);
       doublePool.growPool(numOfObjects);
       floatPool.growPool(numOfObjects);
       statePool.growPool(numOfObjects);
       qosPool.growPool(numOfObjects);
       mapPool.growPool(numOfObjects);
       mapEntryPool.growPool(numOfObjects);
       enumPool.growPool(numOfObjects);
       timePool.growPool(numOfObjects);
       datePool.growPool(numOfObjects);
       dateTimePool.growPool(numOfObjects);
       arrayPool.growPool(numOfObjects);
       arrayEntryPool.growPool(numOfObjects);
       msgPool.growPool(numOfObjects);
       fieldSetDefDbPool.growPool(numOfObjects);
       elementSetDefDbPool.growPool(numOfObjects);
       decodeIterPool.growPool(numOfObjects);
       encodeIteratorPool.growPool(numOfObjects);
       byteBufferPool.growPool(numOfObjects);
    }
}
