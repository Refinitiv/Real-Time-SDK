package com.rtsdk.eta.codec;

import com.rtsdk.eta.codec.Array;
import com.rtsdk.eta.codec.Buffer;
import com.rtsdk.eta.codec.BufferImpl;
import com.rtsdk.eta.codec.CloseMsg;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;
import com.rtsdk.eta.codec.DataTypes;
import com.rtsdk.eta.codec.Date;
import com.rtsdk.eta.codec.DateTime;
import com.rtsdk.eta.codec.ElementEntry;
import com.rtsdk.eta.codec.ElementList;
import com.rtsdk.eta.codec.EncodeIterator;
import com.rtsdk.eta.codec.Enum;
import com.rtsdk.eta.codec.FieldEntry;
import com.rtsdk.eta.codec.FieldList;
import com.rtsdk.eta.codec.FieldSetDef;
import com.rtsdk.eta.codec.FieldSetDefEntry;
import com.rtsdk.eta.codec.FilterEntry;
import com.rtsdk.eta.codec.FilterEntryActions;
import com.rtsdk.eta.codec.FilterEntryFlags;
import com.rtsdk.eta.codec.FilterList;
import com.rtsdk.eta.codec.FilterListFlags;
import com.rtsdk.eta.codec.Int;
import com.rtsdk.eta.codec.LocalElementSetDefDb;
import com.rtsdk.eta.codec.LocalFieldSetDefDb;
import com.rtsdk.eta.codec.Map;
import com.rtsdk.eta.codec.MapEntry;
import com.rtsdk.eta.codec.MapEntryActions;
import com.rtsdk.eta.codec.MapFlags;
import com.rtsdk.eta.codec.Msg;
import com.rtsdk.eta.codec.MsgClasses;
import com.rtsdk.eta.codec.MsgImpl;
import com.rtsdk.eta.codec.MsgKeyFlags;
import com.rtsdk.eta.codec.MsgKeyImpl;
import com.rtsdk.eta.codec.PostMsgFlags;
import com.rtsdk.eta.codec.PostMsg;
import com.rtsdk.eta.codec.Qos;
import com.rtsdk.eta.codec.QosRates;
import com.rtsdk.eta.codec.QosTimeliness;
import com.rtsdk.eta.codec.Real;
import com.rtsdk.eta.codec.RealHints;
import com.rtsdk.eta.codec.RefreshMsgFlags;
import com.rtsdk.eta.codec.RefreshMsg;
import com.rtsdk.eta.codec.RequestMsg;
import com.rtsdk.eta.codec.Series;
import com.rtsdk.eta.codec.SeriesEntry;
import com.rtsdk.eta.codec.State;
import com.rtsdk.eta.codec.StatusMsgFlags;
import com.rtsdk.eta.codec.StatusMsg;
import com.rtsdk.eta.codec.StreamStates;
import com.rtsdk.eta.codec.Time;
import com.rtsdk.eta.codec.UInt;
import com.rtsdk.eta.codec.UpdateMsg;
import com.rtsdk.eta.codec.Vector;
import com.rtsdk.eta.codec.VectorEntry;
import com.rtsdk.eta.codec.VectorEntryActions;
import com.rtsdk.eta.codec.VectorFlags;

class Encoders
{
    // This class field will be used to bypass asserts when running junits.
    static boolean _runningInJunits = false;
	
    static final int __RSZUI8 = 1;
    static final int __RSZFLT = 4;
    static final int __RSZDBL = 8;
		
    static final int RWF_MAX_16 = 0xFFFF;
    static final int RWF_MAX_U15 = 0x7FFF;
    static final int RWF_MAX_U22 = 0x3FFFFF;
    static final int RWF_MAX_U30 = 0x3FFFFFFF;
    static final int RWF_MAX_U31 = 0x7FFFFFFF;
    static final int HAS_MSG_KEY = 1;
    static final int HAS_EXT_HEADER = 2;
	
    static class PrimitiveEncoder
    {
        interface EncodeAction
        {
            int encode(EncodeIteratorImpl iter, Object data);
        }
		
        static EncodeAction[] _setEncodeActions = new EncodeAction[]
        {
            null, null, null,
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeIntWithLength(iter, (Int)data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeUIntWithLength(iter, (UInt)data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeFloatWithLength(iter, (Float)data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeDoubleWithLength(iter, (Double)data);}},
            null,
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeRealWithLength(iter, (Real)data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeDateWithLength(iter, (Date)data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeTimeWithLength(iter, (Time)data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeDateTimeWithLength(iter, (DateTime)data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeQosWithLength(iter, (Qos)data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeStateWithLength(iter, (State)data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeEnumWithLength(iter, (Enum)data);}},
            null, 
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeBufferWithLength(iter, (Buffer)data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeBufferWithLength(iter, (Buffer)data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeBufferWithLength(iter, (Buffer)data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeBufferWithLength(iter, (Buffer)data);}},
            null,
            null, null, null, null, null, null, null, null, null, null,
            null, null, null, null, null, null, null, null, null, null,
            null, null, null, null, null, null, null, null, null, null,
            null, null, null, null, null, null, null, null, null, null,
            null, null, null,
            // the index has to be offset by SET_PRIMITIVE_MIN (64)
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeInt1(iter, (Int) data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeUInt1(iter, (UInt) data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeInt2(iter, (Int) data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeUInt2(iter, (UInt) data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeInt4(iter, (Int) data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeUInt4(iter, (UInt) data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeInt8(iter, (Int) data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeUInt8(iter, (UInt) data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeFloat4(iter, (Float)data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeDouble8(iter, (Double)data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeReal4(iter, (Real) data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeReal8(iter, (Real) data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeDate4(iter, (Date) data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeTime3(iter, (Time) data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeTime5(iter, (Time) data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeDateTime7(iter, (DateTime) data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeDateTime9(iter, (DateTime) data);}},			
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeDateTime11(iter, (DateTime) data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeDateTime12(iter, (DateTime) data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeTime7(iter, (Time) data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeTime8(iter, (Time) data);}},
        };
		
        static EncodeAction[][] _arrayEncodeActions = new EncodeAction[][]
        {
            {null, null, null, null, null, null, null, null, null, null},
            {null, null, null, null, null, null, null, null, null, null},
            {null, null, null, null, null, null, null, null, null, null},
            {new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeIntWithLength(iter, (Int)data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeInt1(iter, (Int)data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeInt2(iter, (Int)data);}},
            null,
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeInt4(iter, (Int)data);}},
            null, null, null,
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeInt8(iter, (Int)data);}},
            null},
            {new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeUIntWithLength(iter, (UInt)data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeUInt1(iter, (UInt)data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeUInt2(iter, (UInt)data);}},
            null,
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeUInt4(iter, (UInt)data);}},
            null, null, null,
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeUInt8(iter, (UInt)data);}},
            null},
            {new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeFloatWithLength(iter, (Float)data);}},
            null, null, null,
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeFloat4(iter, (Float)data);}},
            null, null, null, null, null},
            {new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeDoubleWithLength(iter, (Double)data);}},
            null, null, null, null, null, null, null,
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeDouble8(iter, (Double)data);}},
            null},
            {null, null, null, null, null, null, null, null, null, null},
            {new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeRealWithLength(iter, (Real)data);}},
            null, null, null, null, null, null, null, null, null},
            {new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeDateWithLength(iter, (Date)data);}},
            null, null, null, 
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeDate4(iter, (Date)data);}},
            null, null, null, null, null},
            {new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeTimeWithLength(iter, (Time)data);}},
            null, null, 
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeTime3(iter, (Time)data);}},
            null, 
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeTime5(iter, (Time)data);}},
            null, null, null, null},
            {new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeDateTimeWithLength(iter, (DateTime)data);}},
            null, null, null, null, null, null, 
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeDateTime7(iter, (DateTime)data);}},
            null, 
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeDateTime9(iter, (DateTime)data);}}},
            {new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeQosWithLength(iter, (Qos)data);}},
            null, null, null, null, null, null, null, null, null},
            {new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeStateWithLength(iter, (State)data);}},
            null, null, null, null, null, null, null, null, null},
            {new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeEnumWithLength(iter, (Enum)data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeEnum1(iter, (Enum)data);}},
            new EncodeAction() {public int encode(EncodeIteratorImpl iter, Object data) {return encodeEnum2(iter, (Enum)data);}},
            null, null, null, null, null, null, null}			
        };
		
        static int encodeArrayData(EncodeIteratorImpl iter, Object data, int type, int length)
        {
            return _arrayEncodeActions[type][length].encode(iter, data);
        }
		
        static int encodeSetData(EncodeIteratorImpl iter, Object data, int type)
        {
            return _setEncodeActions[type].encode(iter, data);
        }

        // the encode methods
        static int encodeBuffer(EncodeIteratorImpl iter, Buffer data)
        {
            assert (iter != null);
            assert (data != null);

            if (iter.isIteratorOverrun(data.length()))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.write((BufferImpl)data);
            iter._curBufPos = iter._writer.position();
            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeBufferWithLength(EncodeIteratorImpl iter, Buffer data)
        {
            assert (iter != null);
            assert (data != null);

            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            /* Check to see if what type of length encoding. */
            if (_levelInfo._encodingState == EncodeIteratorStates.PRIMITIVE_U15)
            {
                int len = data.length();
                if (len > RWF_MAX_U15)
                    return CodecReturnCodes.ENCODING_UNAVAILABLE;

                // len is written on wire as uShort15rb
                // If the value is smaller than 0x80, it is written on the wire as one byte,
                // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
                // The code below checks if the buffer is sufficient for each case.
                if (len < 0x80)
                {
                    if (iter.isIteratorOverrun(1 + len)) // 1 byte len + len
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    iter._writer.writeByte(len);
                    iter._writer.write((BufferImpl)data);
                }
                else
                {
                    if (iter.isIteratorOverrun(2 + len)) // 2 bytes len + len
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    iter._writer.writeUShort15rbLong((short)len);
                    iter._writer.write((BufferImpl)data);
                }
                iter._curBufPos = iter._writer.position();
            }
            else
            {
                int len = data.length();

                // len value is written on wire as uShort16ob
                // If the value is smaller than 0xFE, it is written on the wire as one byte,
                // otherwise, it is written as three bytes by calling writeUShort16obLong method.
                // The code below checks if the buffer is sufficient for each case.
                if (len < 0xFE)
                {
                    if (iter.isIteratorOverrun(1 + len))
                        return CodecReturnCodes.BUFFER_TOO_SMALL;

                    iter._writer.writeByte(len);
                    iter._writer.write((BufferImpl)data);
                }
                else if (len <= 0xFFFF)
                {
                    if (iter.isIteratorOverrun(3 + len))
                        return CodecReturnCodes.BUFFER_TOO_SMALL;

                    iter._writer.writeUShort16obLong(len);
                    iter._writer.write((BufferImpl)data);

                }
                else
                    return CodecReturnCodes.INVALID_DATA;

                iter._curBufPos = iter._writer.position();
            }
            return CodecReturnCodes.SUCCESS;
        }

        static int encodeInt(EncodeIteratorImpl iter, Int data)
        {
            assert (iter != null);
            assert (data != null);

            long val = data.toLong();

            int ret = iter._writer.writeLong64ls(val);
            if (ret != CodecReturnCodes.SUCCESS) // checks for buffer too small
                return ret;
            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeIntWithLength(EncodeIteratorImpl iter, Int data)
        {
            assert (iter != null);
            assert (data != null);

            long val = data.toLong();

            int ret = iter._writer.writeLong64lsWithLength(val);
            if (ret != CodecReturnCodes.SUCCESS) // checks for buffer too small
                return ret;
            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeInt1(EncodeIteratorImpl iter, Int data)
        {
            assert (iter != null);
            assert (data != null);

            long val = data.toLong();

            if ((val < -0x80) || (val >= 0x80))
                return CodecReturnCodes.VALUE_OUT_OF_RANGE;

            if (iter.isIteratorOverrun(1))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeByte((int)val);
            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeInt2(EncodeIteratorImpl iter, Int data)
        {
            assert (iter != null);
            assert (data != null);

            long val = data.toLong();

            if ((val < -0x8000) || (val >= 0x8000))
                return CodecReturnCodes.VALUE_OUT_OF_RANGE;

            if (iter.isIteratorOverrun(2))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeShort((int)val);
            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeInt4(EncodeIteratorImpl iter, Int data)
        {
            assert (iter != null);
            assert (data != null);

            long val = data.toLong();

            if ((val < -0x80000000L) || (val >= 0x80000000L))
                return CodecReturnCodes.VALUE_OUT_OF_RANGE;

            if (iter.isIteratorOverrun(4))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeInt((int)val);
            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeInt8(EncodeIteratorImpl iter, Int data)
        {
            assert (iter != null);
            assert (data != null);

            long val = data.toLong();

            if (iter.isIteratorOverrun(8))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeLong(val);
            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeUInt(EncodeIteratorImpl iter, UInt data)
        {
            assert (iter != null);
            assert (data != null);

            long val = data.toLong();

            int ret = iter._writer.writeULong64ls(val);
            if (ret != CodecReturnCodes.SUCCESS) // checks for buffer too small
                return ret;
            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeUIntWithLength(EncodeIteratorImpl iter, UInt data)
        {
            assert (iter != null);
            assert (data != null);

            long val = data.toLong();

            int ret = iter._writer.writeULong64lsWithLength(val);
            if (ret != CodecReturnCodes.SUCCESS) // checks for buffer too small
                return ret;
            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeUInt1(EncodeIteratorImpl iter, UInt data)
        {
            assert (iter != null);
            assert (data != null);

            long val = data.toLong();

            if ((val < 0) || (val > 0xFFL))
                return CodecReturnCodes.VALUE_OUT_OF_RANGE;

            if (iter.isIteratorOverrun(1))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeByte((int)val);
            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeUInt2(EncodeIteratorImpl iter, UInt data)
        {
            assert (iter != null);
            assert (data != null);

            long val = data.toLong();

            if ((val < 0) || (val > 0xFFFFL))
                return CodecReturnCodes.VALUE_OUT_OF_RANGE;

            if (iter.isIteratorOverrun(2))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeShort((int)val);
            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeUInt4(EncodeIteratorImpl iter, UInt data)
        {
            assert (iter != null);
            assert (data != null);

            long val = data.toLong();

            if ((val < 0) || (val > 0xFFFFFFFFL))
                return CodecReturnCodes.VALUE_OUT_OF_RANGE;

            if (iter.isIteratorOverrun(4))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeInt((int)val);
            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeUInt8(EncodeIteratorImpl iter, UInt data)
        {
            assert (iter != null);
            assert (data != null);

            long val = data.toLong();

            if (iter.isIteratorOverrun(8))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeLong(val);
            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeFloat4(EncodeIteratorImpl iter, Float data)
        {
            assert (iter != null);
            assert (data != null);
            assert (iter._levelInfo[iter._encodingLevel]._encodingState == EncodeIteratorStates.SET_DATA) : "Unexpected encoding attempted";

            return encodeFloat(iter, data);
        }
		
        static int encodeFloat(EncodeIteratorImpl iter, Float data)
        {
            assert (iter != null);
            assert (data != null);

            if (iter.isIteratorOverrun(__RSZFLT))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeFloat(data.toFloat());
            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
        
        static int encodeDouble8(EncodeIteratorImpl iter, Double data)
        {
            assert (iter != null);
            assert (data != null);

            assert (iter._levelInfo[iter._encodingLevel]._encodingState == EncodeIteratorStates.SET_DATA) : "Unexpected encoding attempted";

            return encodeDouble(iter, data);
        }

        static int encodeDouble(EncodeIteratorImpl iter, Double data)
        {
            assert (iter != null);
            assert (data != null);

            if (iter.isIteratorOverrun(__RSZDBL))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeDouble(data.toDouble());
            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
        
        static int encodeFloatWithLength(EncodeIteratorImpl iter, Float data)
        {
            assert (iter != null);
            assert (data != null);

            if (iter.isIteratorOverrun(5))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeByte(4);
            iter._writer.writeFloat(data.toFloat());
            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeDoubleWithLength(EncodeIteratorImpl iter, Double data)
        {
            assert (iter != null);
            assert (data != null);

            if (iter.isIteratorOverrun(9))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeByte(8);
            iter._writer.writeDouble(data.toDouble());
            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeReal(EncodeIteratorImpl iter, Real data)
        {
            assert (iter != null);
            assert (data != null);

            if (iter.isIteratorOverrun(1))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            if (data.isBlank())
            {
                putLenSpecBlank(iter);
                return CodecReturnCodes.SUCCESS;
            }

            if (data.hint() > RealHints.NOT_A_NUMBER || data.hint() == 31 || data.hint() == 32)
                return CodecReturnCodes.INVALID_DATA;

            iter._writer.writeUByte(data.hint());

            switch (data.hint())
            { // for these three, only write the hint
                case RealHints.INFINITY:
                case RealHints.NEG_INFINITY:
                case RealHints.NOT_A_NUMBER:
                    // do nothing, just skip out
                    break;
                default: // all other hints
                {
                    int ret = iter._writer.writeLong64ls(data.toLong()); // checks if buffer too small
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
            }

            iter._curBufPos = iter._writer.position();
            return CodecReturnCodes.SUCCESS;
        }

        static int encodeRealWithLength(EncodeIteratorImpl iter, Real data)
        {
            assert (iter != null);
            assert (data != null);

            if (iter.isIteratorOverrun(2)) // check if byte for length and byte for hint would fit
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            if (data.isBlank())
            {
                putLenSpecBlank(iter);
                return CodecReturnCodes.SUCCESS;
            }

            if (data.hint() > RealHints.NOT_A_NUMBER || data.hint() == 31 || data.hint() == 32)
                return CodecReturnCodes.INVALID_DATA;

            switch (data.hint())
            { // for these three, only write the length and hint
                case RealHints.INFINITY:
                case RealHints.NEG_INFINITY:
                case RealHints.NOT_A_NUMBER:
                    iter._writer.writeUByte(1);
                    iter._writer.writeUByte(data.hint());
                    break;
                default: // all other hints
                {
                    int lenPos = iter._writer.position();
                    iter._writer.skipBytes(2); // byte for length and byte for hint
                    int ret;
                    if ((ret = iter._writer.writeLong64ls(data.toLong())) != CodecReturnCodes.SUCCESS)
                        return ret;
                    int endPosition = iter._writer.position();
                    iter._writer.position(lenPos); // length including hint
                    iter._writer.writeUByte(endPosition - lenPos - 1);
                    iter._writer.writeUByte(data.hint());
                    iter._writer.position(endPosition);
                }
            }

            iter._curBufPos = iter._writer.position();
            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeReal4(EncodeIteratorImpl iter, Real data)
        {
            assert (iter != null);
            assert (data != null);

            if (data.toLong() <= Integer.MAX_VALUE)
            {
                if (iter.isIteratorOverrun(5))
                    return CodecReturnCodes.BUFFER_TOO_SMALL;

                byte rwfHint;
                if (data.isBlank())
                {
                    rwfHint = 0x20;
                }
                else
                {
                    if (data.hint() > RealHints.MAX_DIVISOR) // we don't want to write +/-Infinity, NaN yet
                        return CodecReturnCodes.INVALID_DATA;

                    int value = (int)data.toLong();
                    byte length = 0;
                    while (value != 0)
                    {
                        value = value >> 8;
                        length++;
                    }
                    rwfHint = (byte)(((length - 1) << 6) | (byte)data.hint());
                }
                iter._writer.writeByte(rwfHint);
                iter._writer.writeLong64ls(data.toLong());

                iter._curBufPos = iter._writer.position();

                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.VALUE_OUT_OF_RANGE;
        }

        static int encodeReal8(EncodeIteratorImpl iter, Real data)
        {
            assert (iter != null);
            assert (data != null);

            if (iter.isIteratorOverrun(9))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            byte rwfHint;
            if (data.isBlank())
            {
                rwfHint = 0x20;
            }
            else
            {
                if (data.hint() > RealHints.MAX_DIVISOR) // we don't want to write +/-Infinity, NaN yet
                    return CodecReturnCodes.INVALID_DATA;

                long value = data.toLong();
                byte length = 0;
                while (value != 0)
                {
                    value = value >> 8;
                    length++;
                }
                rwfHint = (byte)((((length - 1) / 2) << 6) | (byte)data.hint());
            }
            iter._writer.writeByte(rwfHint);
            iter._writer.writeLong64ls(data.toLong());

            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeDate(EncodeIteratorImpl iter, Date data)
        {
            assert (iter != null);
            assert (data != null);

            if (iter.isIteratorOverrun(4))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeByte(data.day());
            iter._writer.writeByte(data.month());
            iter._writer.writeShort(data.year());
            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeDateWithLength(EncodeIteratorImpl iter, Date data)
        {
            assert (iter != null);
            assert (data != null);

            if (iter.isIteratorOverrun(5))
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            iter._writer.writeByte(4);

            iter._writer.writeByte(data.day());
            iter._writer.writeByte(data.month());
            iter._writer.writeShort(data.year());
            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeDate4(EncodeIteratorImpl iter, Date data)
        {
            return encodeDate(iter, data);
        }
		
        static int encodeTime(EncodeIteratorImpl iter, Time data)
        {
            assert (iter != null);
            assert (data != null);

            int len;

            if (data.nanosecond() != 0)
                len = 8;
            else if (data.microsecond() != 0)
                len = 7;
            else if (data.millisecond() != 0)
                len = 5;
            else if (data.second() != 0)
                len = 3;
            else
                len = 2;

            if (iter.isIteratorOverrun(len))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeByte(data.hour());
            iter._writer.writeByte(data.minute());

            switch (len)
            {
                case 2: // already done
                    break;
                case 3:
                    iter._writer.writeByte(data.second());
                    break;
                case 5:
                    iter._writer.writeByte(data.second());
                    iter._writer.writeShort(data.millisecond());
                    break;
                case 7:
                    iter._writer.writeByte(data.second());
                    iter._writer.writeShort(data.millisecond());
                    iter._writer.writeShort(data.microsecond());
                    break;
                case 8:
                {
                    byte tempNano = (byte)data.nanosecond();
                    short tempMicro = (short)(((data.nanosecond() & 0xFF00) << 3) | data.microsecond());
                    iter._writer.writeByte(data.second());
                    iter._writer.writeShort(data.millisecond());
                    iter._writer.writeShort(tempMicro);
                    iter._writer.writeByte(tempNano);
                    break;
                }
                default:
                    return CodecReturnCodes.INVALID_DATA;
            }

            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }

        static int encodeTimeWithLength(EncodeIteratorImpl iter, Time data)
        {
            assert (iter != null);
            assert (data != null);

            int len;

            if (data.nanosecond() != 0)
                len = 8;
            else if (data.microsecond() != 0)
                len = 7;
            else if (data.millisecond() != 0)
                len = 5;
            else if (data.second() != 0)
                len = 3;
            else
                len = 2;

            if (iter.isIteratorOverrun(len + 1))
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            iter._writer.writeByte(len);

            iter._writer.writeByte(data.hour());
            iter._writer.writeByte(data.minute());

            switch (len)
            {
                case 2: // already done
                    break;
                case 3:
                    iter._writer.writeByte(data.second());
                    break;
                case 5:
                    iter._writer.writeByte(data.second());
                    iter._writer.writeShort(data.millisecond());
                    break;
                case 7:
                    iter._writer.writeByte(data.second());
                    iter._writer.writeShort(data.millisecond());
                    iter._writer.writeShort(data.microsecond());
                    break;
                case 8:
                {
                    byte tempNano = (byte)data.nanosecond();
                    short tempMicro = (short)(((data.nanosecond() & 0xFF00) << 3) | data.microsecond());
                    iter._writer.writeByte(data.second());
                    iter._writer.writeShort(data.millisecond());
                    iter._writer.writeShort(tempMicro);
                    iter._writer.writeByte(tempNano);
                    break;
                }
                default:
                    return CodecReturnCodes.INVALID_DATA;
            }

            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeTime3(EncodeIteratorImpl iter, Time data)
        {
            assert (iter != null);
            assert (data != null);

            if (iter.isIteratorOverrun(3))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeByte(data.hour());
            iter._writer.writeByte(data.minute());
            iter._writer.writeByte(data.second());

            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeTime5(EncodeIteratorImpl iter, Time data)
        {
            assert (iter != null);
            assert (data != null);

            if (iter.isIteratorOverrun(5))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeByte(data.hour());
            iter._writer.writeByte(data.minute());
            iter._writer.writeByte(data.second());
            iter._writer.writeShort(data.millisecond());

            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeTime7(EncodeIteratorImpl iter, Time data)
        {
            assert (iter != null);
            assert (data != null);

            if (iter.isIteratorOverrun(7))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeByte(data.hour());
            iter._writer.writeByte(data.minute());
            iter._writer.writeByte(data.second());
            iter._writer.writeShort(data.millisecond());
            iter._writer.writeShort(data.microsecond());

            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeTime8(EncodeIteratorImpl iter, Time data)
        {
            assert (iter != null);
            assert (data != null);

            if (iter.isIteratorOverrun(8))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            byte tempNano = (byte)data.nanosecond();
            short tempMicro = (short)(((data.nanosecond() & 0xFF00) << 3) | data.microsecond());
            iter._writer.writeByte(data.hour());
            iter._writer.writeByte(data.minute());
            iter._writer.writeByte(data.second());
            iter._writer.writeShort(data.millisecond());
            iter._writer.writeShort(tempMicro);
            iter._writer.writeByte(tempNano);

            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeDateTime(EncodeIteratorImpl iter, DateTime data)
        {
            assert (iter != null);
            assert (data != null);

            int len;

            if (data.nanosecond() != 0)
                len = 12;
            else if (data.microsecond() != 0)
                len = 11;
            else if (data.millisecond() != 0)
                len = 9;
            else if (data.second() != 0)
                len = 7;
            else
                len = 6;

            if (iter.isIteratorOverrun(len))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeByte(data.day());
            iter._writer.writeByte(data.month());
            iter._writer.writeShort(data.year());
            iter._writer.writeByte(data.hour());
            iter._writer.writeByte(data.minute());

            switch (len)
            {
                case 6: // already done
                    break;
                case 7:
                    iter._writer.writeByte(data.second());
                    break;
                case 9:
                    iter._writer.writeByte(data.second());
                    iter._writer.writeShort(data.millisecond());
                    break;
                case 11:
                    iter._writer.writeByte(data.second());
                    iter._writer.writeShort(data.millisecond());
                    iter._writer.writeShort(data.microsecond());
                    break;
                case 12:
                {
                    byte tempNano = (byte)data.nanosecond();
                    short tempMicro = (short)(((data.nanosecond() & 0xFF00) << 3) | data.microsecond());
                    iter._writer.writeByte(data.second());
                    iter._writer.writeShort(data.millisecond());
                    iter._writer.writeShort(tempMicro);
                    iter._writer.writeByte(tempNano);

                }
                    break;
                default:
                    return CodecReturnCodes.INVALID_DATA;
            }

            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }

        static int encodeDateTimeWithLength(EncodeIteratorImpl iter, DateTime data)
        {
            assert (iter != null);
            assert (data != null);

            int len;

            if (data.nanosecond() != 0)
                len = 12;
            else if (data.microsecond() != 0)
                len = 11;
            else if (data.millisecond() != 0)
                len = 9;
            else if (data.second() != 0)
                len = 7;
            else
                len = 6;

            if (iter.isIteratorOverrun(len + 1))
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            iter._writer.writeByte(len);

            iter._writer.writeByte(data.day());
            iter._writer.writeByte(data.month());
            iter._writer.writeShort(data.year());
            iter._writer.writeByte(data.hour());
            iter._writer.writeByte(data.minute());

            switch (len)
            {
                case 6: // already done
                    break;
                case 7:
                    iter._writer.writeByte(data.second());
                    break;
                case 9:
                    iter._writer.writeByte(data.second());
                    iter._writer.writeShort(data.millisecond());
                    break;
                case 11:
                    iter._writer.writeByte(data.second());
                    iter._writer.writeShort(data.millisecond());
                    iter._writer.writeShort(data.microsecond());
                    break;
                case 12:
                {
                    byte tempNano = (byte)data.nanosecond();
                    short tempMicro = (short)(((data.nanosecond() & 0xFF00) << 3) | data.microsecond());
                    iter._writer.writeByte(data.second());
                    iter._writer.writeShort(data.millisecond());
                    iter._writer.writeShort(tempMicro);
                    iter._writer.writeByte(tempNano);
                    break;
                }
                default:
                    return CodecReturnCodes.INVALID_DATA;
            }

            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeDateTime7(EncodeIteratorImpl iter, DateTime data)
        {
            assert (iter != null);
            assert (data != null);

            if (iter.isIteratorOverrun(7))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeByte(data.day());
            iter._writer.writeByte(data.month());
            iter._writer.writeShort(data.year());
            iter._writer.writeByte(data.hour());
            iter._writer.writeByte(data.minute());
            iter._writer.writeByte(data.second());

            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeDateTime9(EncodeIteratorImpl iter, DateTime data)
        {
            assert (iter != null);
            assert (data != null);

            if (iter.isIteratorOverrun(9))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeByte(data.day());
            iter._writer.writeByte(data.month());
            iter._writer.writeShort(data.year());
            iter._writer.writeByte(data.hour());
            iter._writer.writeByte(data.minute());
            iter._writer.writeByte(data.second());
            iter._writer.writeShort(data.millisecond());
            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeDateTime11(EncodeIteratorImpl iter, DateTime data)
        {
            assert (iter != null);
            assert (data != null);

            if (iter.isIteratorOverrun(11))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeByte(data.day());
            iter._writer.writeByte(data.month());
            iter._writer.writeShort(data.year());
            iter._writer.writeByte(data.hour());
            iter._writer.writeByte(data.minute());
            iter._writer.writeByte(data.second());
            iter._writer.writeShort(data.millisecond());
            iter._writer.writeShort(data.microsecond());
            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeDateTime12(EncodeIteratorImpl iter, DateTime data)
        {
            assert (iter != null);
            assert (data != null);
            byte tempNano = (byte)data.nanosecond();
            short tempMicro = (short)(((data.nanosecond() & 0xFF00) << 3) | data.microsecond());

            if (iter.isIteratorOverrun(12))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeByte(data.day());
            iter._writer.writeByte(data.month());
            iter._writer.writeShort(data.year());
            iter._writer.writeByte(data.hour());
            iter._writer.writeByte(data.minute());
            iter._writer.writeByte(data.second());
            iter._writer.writeShort(data.millisecond());
            iter._writer.writeShort(tempMicro);
            iter._writer.writeByte(tempNano);

            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeQos(EncodeIteratorImpl iter, Qos data)
        {
            assert (iter != null);
            assert (data != null);

            int dataLength = 1;
            int Qos;

            if (data.timeliness() == QosTimeliness.UNSPECIFIED || data.rate() == QosRates.UNSPECIFIED)
                return CodecReturnCodes.INVALID_DATA;

            dataLength += (data.timeliness() > QosTimeliness.DELAYED_UNKNOWN) ? 2 : 0;
            dataLength += (data.rate() > QosRates.JIT_CONFLATED) ? 2 : 0;

            if (iter.isIteratorOverrun(dataLength))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            Qos = (data.timeliness() << 5);
            Qos |= (data.rate() << 1);
            Qos |= (data.isDynamic() ? 1 : 0);

            iter._writer.writeByte(Qos);
            if (data.timeliness() > QosTimeliness.DELAYED_UNKNOWN)
            {
                iter._writer.writeShort(data.timeInfo());
            }
            if (data.rate() > QosRates.JIT_CONFLATED)
            {
                iter._writer.writeShort(data.rateInfo());
            }
            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }

        static int encodeQosWithLength(EncodeIteratorImpl iter, Qos data)
        {
            assert (iter != null);
            assert (data != null);

            int dataLength = 1;
            int Qos;

            if (data.timeliness() == QosTimeliness.UNSPECIFIED || data.rate() == QosRates.UNSPECIFIED)
                return CodecReturnCodes.INVALID_DATA;

            dataLength += (data.timeliness() > QosTimeliness.DELAYED_UNKNOWN) ? 2 : 0;
            dataLength += (data.rate() > QosRates.JIT_CONFLATED) ? 2 : 0;

            if (iter.isIteratorOverrun(dataLength + 1))
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            iter._writer.writeByte(dataLength);

            Qos = (data.timeliness() << 5);
            Qos |= (data.rate() << 1);
            Qos |= (data.isDynamic() ? 1 : 0);

            iter._writer.writeByte(Qos);
            if (data.timeliness() > QosTimeliness.DELAYED_UNKNOWN)
            {
                iter._writer.writeShort(data.timeInfo());
            }
            if (data.rate() > QosRates.JIT_CONFLATED)
            {
                iter._writer.writeShort(data.rateInfo());
            }
            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeState(EncodeIteratorImpl iter, State data)
        {
            assert (iter != null);
            assert (data != null);

            if (data.streamState() == StreamStates.UNSPECIFIED)
                return CodecReturnCodes.INVALID_DATA;

            int State = (data.streamState() << 3);
            State |= data.dataState();

            int len = data.text().length();

            // len value is written on wire as uShort15rb
            // If the value is smaller than 0x80, it is written on the wire as one byte,
            // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
            // The code below checks if the buffer is sufficient for each case.
            if (len < 0x80)
            {
                if (iter.isIteratorOverrun(3 + len)) // 1 byte state + 1 byte code + 1 byte len + len
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                iter._writer.writeByte(State);
                iter._writer.writeByte(data.code());
                iter._writer.writeByte(len);
                iter._writer.write((BufferImpl)data.text());
            }
            else
            {
                if (iter.isIteratorOverrun(4 + len)) // 2 bytes state + 1 byte code + 1 byte len + len
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                iter._writer.writeByte(State);
                iter._writer.writeByte(data.code());
                iter._writer.writeUShort15rbLong((short)len);
                iter._writer.write((BufferImpl)data.text());
            }

            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }

        static int encodeStateWithLength(EncodeIteratorImpl iter, State data)
        {
            assert (iter != null);
            assert (data != null);

            if (data.streamState() == StreamStates.UNSPECIFIED)
                return CodecReturnCodes.INVALID_DATA;

            int State = (data.streamState() << 3);
            State |= data.dataState();

            int len = data.text().length();

            // len value is written on wire as uShort15rb
            // If the value is smaller than 0x80, it is written on the wire as one byte,
            // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
            // The code below checks if the buffer is sufficient for each case.
            if (len < 0x80)
            {
                if ((len + 1) < 0x80)
                {
                    if (iter.isIteratorOverrun(4 + len)) // 1 byte state len + 1 byte state + 1 byte code + 1 byte len + len
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    iter._writer.writeByte(len + 3);
                }
                else
                {
                    if (iter.isIteratorOverrun(5 + len)) // 2 bytes state len + 1 byte state + 1 byte code + 1 byte len + len
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    iter._writer.writeUShort15rbLong((short)(len + 3));
                }
                iter._writer.writeByte(State);
                iter._writer.writeByte(data.code());
                iter._writer.writeByte(len);
                iter._writer.write((BufferImpl)data.text());
            }
            else
            {
                if (iter.isIteratorOverrun(6 + len)) // 2 byte state len + 1 byte state + 1 byte code + 2 bytes len + len
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                iter._writer.writeUShort15rbLong((short)(len + 4));
                iter._writer.writeByte(State);
                iter._writer.writeByte(data.code());
                iter._writer.writeUShort15rbLong((short)len);
                iter._writer.write((BufferImpl)data.text());
            }

            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeEnum(EncodeIteratorImpl iter, Enum data)
        {
            assert (iter != null);
            assert (data != null);

            int ret = iter._writer.writeUInt16ls(data.toInt());
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeEnumWithLength(EncodeIteratorImpl iter, Enum data)
        {
            assert (iter != null);
            assert (data != null);

            int ret = iter._writer.writeUInt16lsWithLength(data.toInt());
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeEnum1(EncodeIteratorImpl iter, Enum data)
        {
            assert (iter != null);
            assert (data != null);

            if (data.toInt() > 0xFF)
                return CodecReturnCodes.VALUE_OUT_OF_RANGE;

            if (iter.isIteratorOverrun(1))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeByte(data.toInt());
            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
		
        static int encodeEnum2(EncodeIteratorImpl iter, Enum data)
        {
            assert (iter != null);
            assert (data != null);

            if (iter.isIteratorOverrun(2))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeShort(data.toInt());
            iter._curBufPos = iter._writer.position();

            return CodecReturnCodes.SUCCESS;
        }
    }
	
    static int encodeMsg(EncodeIterator iterInt, Msg msg)
    {
        int            ret;
        int            headerSize;
        EncodingLevel  _levelInfo;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;

        assert null != msg : "Invalid parameters or parameters passed in as NULL";
        assert null != iter : "Invalid parameters or parameters passed in as NULL";

        _levelInfo = iter._levelInfo[++iter._encodingLevel];
        if (iter._encodingLevel >= EncodeIteratorImpl.ENC_ITER_MAX_LEVELS)
            return CodecReturnCodes.ITERATOR_OVERRUN;

        /* _initElemStartPos and encoding state should be the only two members used at a msg encoding level. */
        _levelInfo._initElemStartPos = iter._curBufPos;

        _levelInfo._listType = msg;

        /* header length */
        _levelInfo._countWritePos = iter._curBufPos;

        /* reset key and extended header reserved flags */
        ((MsgImpl)msg)._extendedHeaderReserved = false;
        ((MsgImpl)msg)._keyReserved = false;

        if ((ret = encodeMsgInternal(iter, msg)) < 0)
        {
            iter._curBufPos = _levelInfo._initElemStartPos;
            iter._writer.position(iter._curBufPos);
            return ret;
        }

        /* Encode end of the header */
        if ((ret = finishMsgHeader(iter, msg)) < 0)
        {
            iter._curBufPos = _levelInfo._initElemStartPos;
            iter._writer.position(iter._curBufPos);
            return ret;
        }

        /* Keys and or extendedHeaders have to be encoded */
        if (((MsgImpl)msg)._keyReserved || ((MsgImpl)msg)._extendedHeaderReserved)
        {
            iter._curBufPos = _levelInfo._initElemStartPos;
            iter._writer.position(iter._curBufPos);
            return CodecReturnCodes.INCOMPLETE_DATA;
        }

        /* Make sure NoData is handled properly */
        if ((msg.containerType() == DataTypes.NO_DATA) && (msg.encodedDataBody().length() > 0))
        {
            iter._curBufPos = _levelInfo._initElemStartPos;
            iter._writer.position(iter._curBufPos);
            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
            return CodecReturnCodes.UNEXPECTED_ENCODER_CALL;
        }

        /* write header size */
        headerSize = iter._curBufPos - _levelInfo._countWritePos - 2;
        iter._writer.position(_levelInfo._countWritePos);
        iter._writer.writeShort(headerSize);
        iter._writer.position(iter._curBufPos);

        if (msg.encodedDataBody().length() > 0)
        {
            if (iter.isIteratorOverrun(msg.encodedDataBody().length()))
            {
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.position(iter._curBufPos);
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            }

            encodeCopyData(iter, (BufferImpl)msg.encodedDataBody());
        }

        iter._curBufPos = iter._writer.position();

        --iter._encodingLevel;

        return CodecReturnCodes.SUCCESS;
    }
	
    static int encodeMsgInit(EncodeIterator iterInt, Msg msg, int dataMaxSize)
    {
        int             retVal;
        int             headerSize;
        EncodingLevel   _levelInfo;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;

        assert null != msg : "Invalid parameters or parameters passed in as NULL";
        assert null != iter : "Invalid parameters or parameters passed in as NULL";

        _levelInfo = iter._levelInfo[++iter._encodingLevel];
        if (iter._encodingLevel >= EncodeIteratorImpl.ENC_ITER_MAX_LEVELS)
            return CodecReturnCodes.ITERATOR_OVERRUN;

        _levelInfo._internalMark._sizePos = 0;
        _levelInfo._internalMark._sizeBytes = 0;
        _levelInfo._internalMark2._sizePos = 0;
        _levelInfo._internalMark2._sizeBytes = 0;

        /* _initElemStartPos and encoding state should be the only two members used at a msg encoding level. */
        _levelInfo._initElemStartPos = iter._curBufPos;

        /* Set the message onto the current _levelInfo */
        _levelInfo._listType = msg;
        _levelInfo._containerType = DataTypes.MSG;

        /* header length */
        _levelInfo._countWritePos = iter._curBufPos;

        if ((retVal = encodeMsgInternal(iter, msg)) < 0)
        {
            iter._curBufPos = _levelInfo._initElemStartPos;
            iter._writer.position(iter._curBufPos);
            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
            return retVal;
        }

        if (((MsgImpl)msg)._keyReserved && ((MsgImpl)msg)._extendedHeaderReserved)
        {
            _levelInfo._encodingState = EncodeIteratorStates.OPAQUE_AND_EXTENDED_HEADER;
            return CodecReturnCodes.ENCODE_MSG_KEY_ATTRIB;
        }
        else if (((MsgImpl)msg)._keyReserved)
        {
            _levelInfo._encodingState = EncodeIteratorStates.OPAQUE;
            return CodecReturnCodes.ENCODE_MSG_KEY_ATTRIB;
        }
        else if (((MsgImpl)msg)._extendedHeaderReserved)
        {
            _levelInfo._encodingState = EncodeIteratorStates.EXTENDED_HEADER;
            return CodecReturnCodes.ENCODE_EXTENDED_HEADER;
        }
        else
        {
            /* Header is finished, encode the end of the header */
            if ((retVal = finishMsgHeader(iter, (Msg)_levelInfo._listType)) != CodecReturnCodes.SUCCESS)
            {
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.position(iter._curBufPos);
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return retVal;
            }

            /* write header size */
            headerSize = iter._curBufPos - _levelInfo._countWritePos - 2;
            iter._writer.position(_levelInfo._countWritePos);
            iter._writer.writeShort(headerSize);
            _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;
            iter._writer.position(iter._curBufPos);

            /* now store current location so we can check it to ensure user did not put data
             * when they shouldnt */
            /* count has been filled in already */
            _levelInfo._countWritePos = iter._curBufPos;

            if (msg.containerType() != DataTypes.NO_DATA)
            	return CodecReturnCodes.ENCODE_CONTAINER;
            
            return CodecReturnCodes.SUCCESS;
        }
    }

    static int encodeMsgComplete(EncodeIterator iterInt, boolean success)
    {
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        /* Validations */
        assert null != iter : "Invalid parameters or parameters passed in as NULL";
        assert (_levelInfo._encodingState == EncodeIteratorStates.ENTRIES) ||
               (_levelInfo._encodingState == EncodeIteratorStates.WAIT_COMPLETE) : "Unexpected encoding attempted";
        assert null != iter._buffer : "Invalid iterator use - check buffer";
        assert iter._curBufPos <= iter._endBufPos : "Data exceeds iterators buffer length";

        if (!success)
        {
            /* _initElemStartPos is at the start of the msg at this level.
             * Typically this should be the same as iter.buffer.data. */
            iter._curBufPos = _levelInfo._initElemStartPos;
            iter._writer.position(iter._curBufPos);
        }
        else
        {
            Msg msg = (Msg)_levelInfo._listType;
            if (msg.containerType() == DataTypes.NO_DATA && _levelInfo._countWritePos != iter._curBufPos)
            {
                /* user encoded payload when they should not have */
                /* roll back */
                iter._curBufPos = _levelInfo._initElemStartPos;
                _levelInfo._initElemStartPos = 0;
                iter._writer.position(iter._curBufPos);
                return CodecReturnCodes.INVALID_DATA;
            }
        }

        --iter._encodingLevel;

        return CodecReturnCodes.SUCCESS;
    }

    private static int encodeMsgInternal(EncodeIteratorImpl iter, Msg msg)
    {
        int retVal = CodecReturnCodes.SUCCESS;

        /* ensure container type is valid */
        if (!(validAggregateDataType(msg.containerType())))
            return CodecReturnCodes.UNSUPPORTED_DATA_TYPE;

        /* make sure required elements can be encoded */
        /* msgClass (1) domainType(1) stream id (4) */
        if (iter.isIteratorOverrun(8))
            return CodecReturnCodes.BUFFER_TOO_SMALL;

        iter._curBufPos += 2;
        iter._writer.skipBytes(2);

        /* Store msgClass as UInt8 */
        iter._writer.writeUByte(msg.msgClass());
        iter._curBufPos = iter._writer.position();

        /* Store domainType as UInt8 */
        iter._writer.writeUByte(msg.domainType());
        iter._curBufPos = iter._writer.position();

        /* Store streamId as Int32 */
        iter._writer.writeInt(msg.streamId());
        iter._curBufPos = iter._writer.position();

        /* IMPORTANT: When new message classes are added, CopyMsg and ValidateMsg have to modified as well */

        switch (msg.msgClass())
        {
            case MsgClasses.UPDATE:
                UpdateMsg updateMsg = (UpdateMsg)msg;
                retVal = encodeUpdateMsg(iter, updateMsg);
                break;
            case MsgClasses.REFRESH:
                RefreshMsg refreshMsg = (RefreshMsg)msg;
                retVal = encodeRefreshMsg(iter, refreshMsg);
                break;
            case MsgClasses.POST:
                PostMsg postMsg = (PostMsg)msg;
                retVal = encodePostMsg(iter, postMsg);
                break;
            case MsgClasses.REQUEST:
                RequestMsg requestMsg = (RequestMsg)msg;
                retVal = encodeRequestMsg(iter, requestMsg);
                break;
            case MsgClasses.CLOSE:
                CloseMsg closeMsg = (CloseMsg)msg;
                retVal = encodeCloseMsg(iter, closeMsg);
                break;
            case MsgClasses.STATUS:
                StatusMsg statusMsg = (StatusMsg)msg;
                retVal = encodeStatusMsg(iter, statusMsg);
                break;
            case MsgClasses.GENERIC:
                GenericMsg genericMsg = (GenericMsg)msg;
                retVal = encodeGenericMsg(iter, genericMsg);
                break;
            case MsgClasses.ACK:
                AckMsg ackMsg = (AckMsg)msg;
                retVal = encodeAckMsg(iter, ackMsg);
                break;
            default:
                retVal = CodecReturnCodes.INVALID_ARGUMENT;
        }

        return retVal;
    }

    private static int encodeUpdateMsg(EncodeIteratorImpl iter, UpdateMsg msg)
    {

        /* Store update flags as UInt15 */
        int flags = msg.flags();

        if (msg.checkHasPermData() && msg.permData().length() == 0)
        {
            flags &= ~UpdateMsgFlags.HAS_PERM_DATA;
        }
        if (msg.checkHasConfInfo() && (msg.conflationCount() == 0 && msg.conflationTime() == 0))
        {
            flags &= ~UpdateMsgFlags.HAS_CONF_INFO;
        }

        // flags value is written on wire as uShort15rb
        // If the value is smaller than 0x80, it is written on the wire as one byte,
        // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
        // The code below checks if the buffer is sufficient for each case.
        if (flags < 0x80)
        {
            if (iter.isIteratorOverrun(3)) // 1 byte flags + 1 byte container type + 1 byte update type
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            iter._writer.writeByte(flags);
        }
        else
        {
            if (iter.isIteratorOverrun(4)) // 2 bytes flags + 1 byte container type + 1 byte update type
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            iter._writer.writeUShort15rbLong((short)flags);
        }

        iter._curBufPos = iter._writer.position();

        /* Store containerType as UInt8 */
        /* container type needs to be scaled before encoding */
        iter._writer.writeUByte(msg.containerType() - DataTypes.CONTAINER_TYPE_MIN);
        iter._curBufPos = iter._writer.position();

        /* Store update type as UInt8 */
        iter._writer.writeByte(msg.updateType());
        iter._curBufPos = iter._writer.position();

        if ((flags & UpdateMsgFlags.HAS_SEQ_NUM) > 0)
        {
            if (iter.isIteratorOverrun(4))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeUInt(msg.seqNum());
            iter._curBufPos = iter._writer.position();
        }

        if ((flags & UpdateMsgFlags.HAS_CONF_INFO) > 0)
        {
            if (msg.conflationCount() < 0x80)
            {
                if (iter.isIteratorOverrun(3))
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                iter._writer.writeByte(msg.conflationCount());
            }
            else
            {
                if (iter.isIteratorOverrun(4))
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                iter._writer.writeUShort15rbLong((short)msg.conflationCount());
            }
            iter._writer.writeShort(msg.conflationTime());
            iter._curBufPos = iter._writer.position();
        }

        /* Store Perm info */
        if ((flags & UpdateMsgFlags.HAS_PERM_DATA) > 0)
        {
            int len = msg.permData().length();
            if (len < 0x80)
            {
                if (iter.isIteratorOverrun(1 + len))
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                iter._writer.writeByte(len);
                iter._writer.write((BufferImpl)msg.permData());
            }
            else
            {
                if (iter.isIteratorOverrun(2 + len))
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                iter._writer.writeUShort15rbLong((short)len);
                iter._writer.write((BufferImpl)msg.permData());
            }
            iter._curBufPos = iter._writer.position();
        }

        int keyAndExtHeader = 0;
        if ((flags & UpdateMsgFlags.HAS_MSG_KEY) > 0)
            keyAndExtHeader = HAS_MSG_KEY;
        if ((flags & UpdateMsgFlags.HAS_EXTENDED_HEADER) > 0)
            keyAndExtHeader = keyAndExtHeader + HAS_EXT_HEADER;

        return encodeKeyAndExtHeader(iter, msg, keyAndExtHeader);
    }

    private static int encodeRefreshMsg(EncodeIteratorImpl iter, RefreshMsg msg)
    {
        int retVal = CodecReturnCodes.SUCCESS;
        /* Store refresh flags as UInt16, cleaning perm flag if necessary */
        int flags = msg.flags();

        if (msg.checkHasPermData() && msg.permData().length() == 0)
        {
            flags &= ~RefreshMsgFlags.HAS_PERM_DATA;
        }

        // flags value is written on wire as uShort15rb
        // If the value is smaller than 0x80, it is written on the wire as one byte,
        // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
        // The code below checks if the buffer is sufficient for each case.
        if (flags < 0x80)
        {
            if (iter.isIteratorOverrun(2)) // 1 byte flags + 1 byte container type
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            iter._writer.writeByte(flags);
        }
        else
        {
            if (iter.isIteratorOverrun(3)) // 2 bytes flags + 1 byte container type
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            iter._writer.writeUShort15rbLong((short)flags);
        }

        /* Store containerType as UInt8 */
        /* container type needs to be scaled */
        iter._writer.writeUByte(msg.containerType() - DataTypes.CONTAINER_TYPE_MIN);
        iter._curBufPos = iter._writer.position();

        if ((flags & RefreshMsgFlags.HAS_SEQ_NUM) > 0)
        {
            if (iter.isIteratorOverrun(4))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeUInt(msg.seqNum());
            iter._curBufPos = iter._writer.position();
        }

        if ((retVal = PrimitiveEncoder.encodeState(iter, msg.state())) < 0)
            return retVal;

        /* Store groupId as small buffer */
        if (msg.groupId().length() > 0)
        {
            if (iter.isIteratorOverrun(msg.groupId().length() + 1))
            {
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            }

            encodeBuffer8(iter, (BufferImpl)msg.groupId());
            iter._curBufPos = iter._writer.position();
        }
        else
        {
            if (iter.isIteratorOverrun(3))
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            /* No group Id - store 0 in the format used by group mechanism */
            /* length is 2 */
            /* write length */
            iter._writer.writeByte(2);
            /* now write value */
            int groupId = 0;
            iter._writer.writeShort(groupId);
            iter._curBufPos = iter._writer.position();
        }

        if ((flags & RefreshMsgFlags.HAS_PERM_DATA) > 0)
        {
            int len = msg.permData().length();
            if (len < 0x80)
            {
                if (iter.isIteratorOverrun(1 + len))
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                iter._writer.writeByte(len);
                iter._writer.write((BufferImpl)msg.permData());
            }
            else
            {
                if (iter.isIteratorOverrun(2 + len))
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                iter._writer.writeUShort15rbLong((short)len);
                iter._writer.write((BufferImpl)msg.permData());
            }

            iter._curBufPos = iter._writer.position();
        }

        /* Store QoS */
        if ((flags & RefreshMsgFlags.HAS_QOS) > 0)
        {
            if ((retVal = PrimitiveEncoder.encodeQos(iter, msg.qos())) < 0)
                return retVal;
        }

        int keyAndExtHeader = 0;
        if ((flags & RefreshMsgFlags.HAS_MSG_KEY) > 0)
            keyAndExtHeader = HAS_MSG_KEY;
        if ((flags & RefreshMsgFlags.HAS_EXTENDED_HEADER) > 0)
            keyAndExtHeader = keyAndExtHeader + HAS_EXT_HEADER;

        return encodeKeyAndExtHeader(iter, msg, keyAndExtHeader);
    }

    private static int encodePostMsg(EncodeIteratorImpl iter, PostMsg msg)
    {
        int flags = msg.flags();

        if (msg.checkHasPermData() && msg.permData().length() == 0)
        {
            flags &= ~PostMsgFlags.HAS_PERM_DATA;
        }

        // flags value is written on wire as uShort15rb
        // If the value is smaller than 0x80, it is written on the wire as one byte,
        // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
        // The code below checks if the buffer is sufficient for each case.
        if (flags < 0x80)
        {
            if (iter.isIteratorOverrun(10)) // 1 byte flags + 1 byte container type + 4 bytes user address + 4 bytes user Id
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            iter._writer.writeByte(flags);
        }
        else
        {
            if (iter.isIteratorOverrun(11)) // 2 bytes flags + 1 byte container type + 4 bytes user address + 4 bytes user Id
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            iter._writer.writeUShort15rbLong((short)flags);
        }

        /* Store containerType as UInt8 */
        /* container type needs to be scaled */
        iter._writer.writeUByte(msg.containerType() - DataTypes.CONTAINER_TYPE_MIN);
        iter._curBufPos = iter._writer.position();

        /* Put User Address */
        iter._writer.writeUInt(msg.postUserInfo().userAddr());
        iter._curBufPos = iter._writer.position();

        /* Put User ID */
        iter._writer.writeUInt(msg.postUserInfo().userId());
        iter._curBufPos = iter._writer.position();

        if ((flags & PostMsgFlags.HAS_SEQ_NUM) > 0)
        {
            if (iter.isIteratorOverrun(4))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeUInt(msg.seqNum());
            iter._curBufPos = iter._writer.position();
        }

        if ((flags & PostMsgFlags.HAS_POST_ID) > 0)
        {
            if (iter.isIteratorOverrun(4))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeUInt(msg.postId());
            iter._curBufPos = iter._writer.position();
        }

        if ((flags & PostMsgFlags.HAS_PERM_DATA) > 0)
        {
            int len = msg.permData().length();

            // len value is written on wire as uShort15rb
            // If the value is smaller than 0x80, it is written on the wire as one byte,
            // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
            // The code below checks if the buffer is sufficient for each case.
            if (len < 0x80)
            {
                if (iter.isIteratorOverrun(1 + len))
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                iter._writer.writeByte(len);
                iter._writer.write((BufferImpl)msg.permData());
            }
            else
            {
                if (iter.isIteratorOverrun(2 + len))
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                iter._writer.writeUShort15rbLong((short)len);
                iter._writer.write((BufferImpl)msg.permData());
            }

            iter._curBufPos = iter._writer.position();
        }

        int keyAndExtHeader = 0;
        if ((flags & PostMsgFlags.HAS_MSG_KEY) > 0)
            keyAndExtHeader = HAS_MSG_KEY;
        if ((flags & PostMsgFlags.HAS_EXTENDED_HEADER) > 0)
            keyAndExtHeader = keyAndExtHeader + HAS_EXT_HEADER;

        return encodeKeyAndExtHeader(iter, msg, keyAndExtHeader);
    }

    private static int encodeRequestMsg(EncodeIteratorImpl iter, RequestMsg msg)
    {
        int retVal = CodecReturnCodes.SUCCESS;

        if (msg.flags() < 0x80)
        {
            if (iter.isIteratorOverrun(2))
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            iter._writer.writeByte(msg.flags());
        }
        else
        {
            if (iter.isIteratorOverrun(3))
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            iter._writer.writeUShort15rbLong((short)msg.flags());
        }

        /* Store containerType as UInt8 */
        /* container type needs to be scaled */
        iter._writer.writeUByte(msg.containerType() - DataTypes.CONTAINER_TYPE_MIN);
        iter._curBufPos = iter._writer.position();

        if (msg.checkHasPriority())
        {

            // count value is written on wire as uShort16ob
            // If the value is smaller than 0xFE, it is written on the wire as one byte,
            // otherwise, it is written as three bytes by calling writeUShort16obLong method.
            // The code below checks if the buffer is sufficient for each case.
            if (msg.priority().count() < 0xFE)
            {
                if (iter.isIteratorOverrun(2)) // 1 byte priority class + 1 byte count
                    return CodecReturnCodes.BUFFER_TOO_SMALL;

                iter._writer.writeUByte(msg.priority().priorityClass());
                iter._writer.writeByte(msg.priority().count());
            }
            else
            {
                if (iter.isIteratorOverrun(4)) // 1 byte priority class + 3 bytes count
                    return CodecReturnCodes.BUFFER_TOO_SMALL;

                iter._writer.writeUByte(msg.priority().priorityClass());
                iter._writer.writeUShort16obLong(msg.priority().count());

            }
            iter._curBufPos = iter._writer.position();
        }

        /* Store Qos */
        if (msg.checkHasQos())
        {
            if ((retVal = PrimitiveEncoder.encodeQos(iter, msg.qos())) < 0)
                return retVal;
        }

        /* Store WorstQos */
        if (msg.checkHasWorstQos())
        {
            if ((retVal = PrimitiveEncoder.encodeQos(iter, msg.worstQos())) < 0)
                return retVal;
        }

        int keyAndExtHeader = HAS_MSG_KEY;
        if (msg.checkHasExtendedHdr())
            keyAndExtHeader = keyAndExtHeader + HAS_EXT_HEADER;

        return encodeKeyAndExtHeader(iter, msg, keyAndExtHeader);
    }

    private static int encodeCloseMsg(EncodeIteratorImpl iter, CloseMsg msg)
    {
        // flags value is written on wire as uShort15rb
        // If the value is smaller than 0x80, it is written on the wire as one byte,
        // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
        // The code below checks if the buffer is sufficient for each case.
        if (msg.flags() < 0x80)
        {
            if (iter.isIteratorOverrun(2)) // 1 byte flags + 1 byte container type
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            iter._writer.writeByte(msg.flags());
        }
        else
        {
            if (iter.isIteratorOverrun(3)) // 2 bytes flags + 1 byte container type
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            iter._writer.writeUShort15rbLong((short)msg.flags());
        }
        iter._curBufPos = iter._writer.position();

        /* container type needs to be scaled */
        iter._writer.writeUByte(msg.containerType() - DataTypes.CONTAINER_TYPE_MIN);
        iter._curBufPos = iter._writer.position();

        if (msg.checkHasExtendedHdr())
        {
            if (msg.extendedHeader().length() > 0)
            {
                /* now put data header there */
                if (iter.isIteratorOverrun(msg.extendedHeader().length() + 1))
                {
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                }

                encodeBuffer8(iter, (BufferImpl)msg.extendedHeader());
                iter._curBufPos = iter._writer.position();
            }
            else
            {
                if (iter.isIteratorOverrun(1))
                {
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                }

                ((MsgImpl)msg)._extendedHeaderReserved = true;
                /* must reserve space now */
                iter._levelInfo[iter._encodingLevel]._internalMark._sizePos = iter._curBufPos;
                iter._levelInfo[iter._encodingLevel]._internalMark._sizeBytes = 1;
                iter._curBufPos++; // move pointer */
                iter._writer.position(iter._curBufPos);
            }
        }
        return CodecReturnCodes.SUCCESS;
    }

    private static int encodeStatusMsg(EncodeIteratorImpl iter, StatusMsg msg)
    {
        int retVal = CodecReturnCodes.SUCCESS;
        /* Store flags as UInt16, cleaning perm flag if necessary */
        int flags = msg.flags();

        if (msg.checkHasPermData() && msg.permData().length() == 0)
        {
            flags &= ~StatusMsgFlags.HAS_PERM_DATA;
        }

        if (msg.checkHasGroupId() && (msg.groupId().length() == 0))
        {
            flags &= ~StatusMsgFlags.HAS_GROUP_ID;
        }

        // flags value is written on wire as uShort15rb
        // If the value is smaller than 0x80, it is written on the wire as one byte,
        // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
        // The code below checks if the buffer is sufficient for each case.
        if (flags < 0x80)
        {
            if (iter.isIteratorOverrun(2)) // 1 byte flags + 1 byte container type
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            iter._writer.writeByte(flags);
        }
        else
        {
            if (iter.isIteratorOverrun(3)) // 2 bytes flags + 1 byte container type
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            iter._writer.writeUShort15rbLong((short)flags);
        }

        /* Store containerType as UInt8 */
        /* container type needs to be scaled */
        iter._writer.writeUByte(msg.containerType() - DataTypes.CONTAINER_TYPE_MIN);
        iter._curBufPos = iter._writer.position();

        /* Store state */
        if ((flags & StatusMsgFlags.HAS_STATE) > 0)
        {
            if ((retVal = PrimitiveEncoder.encodeState(iter, msg.state())) < 0)
                return retVal;
        }

        /* Store groupId as small buffer */
        if ((flags & StatusMsgFlags.HAS_GROUP_ID) > 0)
        {
            if (iter.isIteratorOverrun(msg.groupId().length() + 1))
            {
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            }

            encodeBuffer8(iter, (BufferImpl)msg.groupId());
            iter._curBufPos = iter._writer.position();
        }

        /* Store Permission info */
        if ((flags & RefreshMsgFlags.HAS_PERM_DATA) > 0)
        {
            int len = msg.permData().length();
            if (len < 0x80)
            {
                if (iter.isIteratorOverrun(1 + len))
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                iter._writer.writeByte(len);
                iter._writer.write((BufferImpl)msg.permData());
            }
            else
            {
                if (iter.isIteratorOverrun(2 + len))
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                iter._writer.writeUShort15rbLong((short)len);
                iter._writer.write((BufferImpl)msg.permData());
            }

            iter._curBufPos = iter._writer.position();
        }

        int keyAndExtHeader = 0;
        if ((flags & StatusMsgFlags.HAS_MSG_KEY) > 0)
            keyAndExtHeader = HAS_MSG_KEY;
        if ((flags & StatusMsgFlags.HAS_EXTENDED_HEADER) > 0)
            keyAndExtHeader = keyAndExtHeader + HAS_EXT_HEADER;

        return encodeKeyAndExtHeader(iter, msg, keyAndExtHeader);
    }
	
    private static int encodeGenericMsg(EncodeIteratorImpl iter, GenericMsg msg)
    {
        /* Store update flags as UInt15 */
        int flags = msg.flags();

        if (msg.checkHasPermData() && msg.permData().length() == 0)
        {
            flags &= ~GenericMsgFlags.HAS_PERM_DATA;
        }

        // flags value is written on wire as uShort15rb
        // If the value is smaller than 0x80, it is written on the wire as one byte,
        // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
        // The code below checks if the buffer is sufficient for each case.
        if (flags < 0x80)
        {
            if (iter.isIteratorOverrun(2)) // 1 byte flags + 1 byte container type
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            iter._writer.writeByte(flags);
        }
        else
        {
            if (iter.isIteratorOverrun(3)) // 2 bytes flags + 1 byte container type
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            iter._writer.writeUShort15rbLong((short)flags);
        }

        /* Store containerType as UInt8 */
        /* container type needs to be scaled before encoding */
        iter._writer.writeUByte(msg.containerType() - DataTypes.CONTAINER_TYPE_MIN);
        iter._curBufPos = iter._writer.position();

        if ((flags & GenericMsgFlags.HAS_SEQ_NUM) > 0)
        {
            if (iter.isIteratorOverrun(4))
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            iter._writer.writeUInt(msg.seqNum());
            iter._curBufPos = iter._writer.position();
        }

        if ((flags & GenericMsgFlags.HAS_SECONDARY_SEQ_NUM) > 0)
        {
            if (iter.isIteratorOverrun(4))
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            iter._writer.writeUInt(msg.secondarySeqNum());
            iter._curBufPos = iter._writer.position();
        }

        /* Store Perm info */
        if ((flags & GenericMsgFlags.HAS_PERM_DATA) > 0)
        {
            int len = msg.permData().length();
            if (len < 0x80)
            {
                if (iter.isIteratorOverrun(1 + len))
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                iter._writer.writeByte(len);
                iter._writer.write((BufferImpl)msg.permData());
            }
            else
            {
                if (iter.isIteratorOverrun(2 + len))
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                iter._writer.writeUShort15rbLong((short)len);
                iter._writer.write((BufferImpl)msg.permData());
            }

            iter._curBufPos = iter._writer.position();
        }

        int keyAndExtHeader = 0;
        if ((flags & GenericMsgFlags.HAS_MSG_KEY) > 0)
            keyAndExtHeader = HAS_MSG_KEY;
        if ((flags & GenericMsgFlags.HAS_EXTENDED_HEADER) > 0)
            keyAndExtHeader = keyAndExtHeader + HAS_EXT_HEADER;

        return encodeKeyAndExtHeader(iter, msg, keyAndExtHeader);
    }
	
    private static int encodeAckMsg(EncodeIteratorImpl iter, AckMsg msg)
    {
        int flags = msg.flags();

        if (msg.checkHasNakCode() && msg.nakCode() == NakCodes.NONE)
            flags &= ~AckMsgFlags.HAS_NAK_CODE;

        if (msg.checkHasText() && msg.text().length() == 0)
            flags &= ~AckMsgFlags.HAS_TEXT;

        /* Store flags flags as UInt15 */
        // flags value is written on wire as uShort15rb
        // If the value is smaller than 0x80, it is written on the wire as one byte,
        // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
        // The code below checks if the buffer is sufficient for each case.
        if (flags < 0x80)
        {
            if (iter.isIteratorOverrun(6)) // 1 byte flags + 1 byte container type + 4 bytes ack id
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            iter._writer.writeByte(flags);
        }
        else
        {
            if (iter.isIteratorOverrun(7)) // 2 bytes flags + 1 byte container type + 4 bytes ack id
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            iter._writer.writeUShort15rbLong((short)flags);
        }

        /* Store containerType as UInt8 */
        /* container type needs to be scaled before encoding */
        iter._writer.writeUByte(msg.containerType() - DataTypes.CONTAINER_TYPE_MIN);
        /* Store ackId as UInt32 */
        iter._writer.writeUInt(msg.ackId());

        if ((flags & AckMsgFlags.HAS_NAK_CODE) > 0)
        {
            if (iter.isIteratorOverrun(1))
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            /* Store nakCode as UInt8 */
            iter._writer.writeUByte(msg.nakCode());
        }

        if ((flags & AckMsgFlags.HAS_TEXT) > 0)
        {
            /* Store text as a string */
            int len = msg.text().length();

            // len value is written on wire as uShort16ob
            // If the value is smaller than 0xFE, it is written on the wire as one byte,
            // otherwise, it is written as three bytes by calling writeUShort16obLong method.
            // The code below checks if the buffer is sufficient for each case.
            if (len < 0xFE)
            {
                if (iter.isIteratorOverrun(1 + len))
                    return CodecReturnCodes.BUFFER_TOO_SMALL;

                iter._writer.writeByte(len);
                iter._writer.write((BufferImpl)msg.text());
            }
            else if (len <= 0xFFFF)
            {
                if (iter.isIteratorOverrun(3 + len))
                    return CodecReturnCodes.BUFFER_TOO_SMALL;

                iter._writer.writeUShort16obLong(len);
                iter._writer.write((BufferImpl)msg.text());
            }
            else
            {
                return CodecReturnCodes.INVALID_DATA;
            }
        }

        if ((flags & AckMsgFlags.HAS_SEQ_NUM) > 0)
        {
            if (iter.isIteratorOverrun(4))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.writeInt((int)msg.seqNum());
        }
        iter._curBufPos = iter._writer.position();

        int keyAndExtHeader = 0;
        if ((flags & AckMsgFlags.HAS_MSG_KEY) > 0)
            keyAndExtHeader = HAS_MSG_KEY;
        if ((flags & AckMsgFlags.HAS_EXTENDED_HEADER) > 0)
            keyAndExtHeader = keyAndExtHeader + HAS_EXT_HEADER;

        return encodeKeyAndExtHeader(iter, msg, keyAndExtHeader);
    }

    private static int encodeKeyAndExtHeader(EncodeIteratorImpl iter, Msg msg, int flags)
    {
        int retVal = CodecReturnCodes.SUCCESS;

        if ((flags & HAS_MSG_KEY) > 0)
        {
            /* save position for storing key size */
            int lenPos = iter._curBufPos;

            if ((retVal = encodeKeyInternal(iter, (MsgKeyImpl)msg.msgKey())) < 0)
                return retVal;

            /* Store opaque as SmallBuffer */
            if (msg.msgKey().checkHasAttrib())
            {
                /* write opaque data format and save length position */
                if (iter.isIteratorOverrun(1))
                {
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                }

                if (!(validAggregateDataType(msg.msgKey().attribContainerType())))
                    return CodecReturnCodes.UNSUPPORTED_DATA_TYPE;

                /* opaque container type needs to be scaled before encoding */
                iter._writer.writeUByte(msg.msgKey().attribContainerType() - DataTypes.CONTAINER_TYPE_MIN);
                iter._curBufPos = iter._writer.position();

                /* if we have a key opaque here, put it on the wire */
                if (msg.msgKey().encodedAttrib().length() > 0)
                {
                    int len = msg.msgKey().encodedAttrib().length();

                    // len value is written on wire as uShort15rb
                    // If the value is smaller than 0x80, it is written on the wire as one byte,
                    // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
                    // The code below checks if the buffer is sufficient for each case.
                    if (len < 0x80)
                    {
                        if (iter.isIteratorOverrun(1 + len))
                            return CodecReturnCodes.BUFFER_TOO_SMALL;
                        iter._writer.writeByte(len);
                        iter._writer.write((BufferImpl)msg.msgKey().encodedAttrib());
                    }
                    else
                    {
                        if (iter.isIteratorOverrun(2 + len))
                            return CodecReturnCodes.BUFFER_TOO_SMALL;
                        iter._writer.writeUShort15rbLong((short)len);
                        iter._writer.write((BufferImpl)msg.msgKey().encodedAttrib());
                    }

                    iter._curBufPos = iter._writer.position();
                }
                else
                {
                    /* opaque needs to be encoded */
                    /* save U15 mark */
                    if (msg.msgKey().attribContainerType() != DataTypes.NO_DATA)
                    {
                        if (iter.isIteratorOverrun(2))
                        {
                            return CodecReturnCodes.BUFFER_TOO_SMALL;
                        }
                        iter._curBufPos = setupU15Mark(iter._levelInfo[iter._encodingLevel]._internalMark2, 0, iter._curBufPos);
                        iter._writer.position(iter._curBufPos);
                    }
                    else
                    {
                        iter._levelInfo[iter._encodingLevel]._internalMark2._sizeBytes = 0;
                        iter._levelInfo[iter._encodingLevel]._internalMark2._sizePos = iter._curBufPos;
                    }
                    ((MsgImpl)msg)._keyReserved = true;
                }
            }

            /* user is done with key and there is no opaque they still need to write */
            if (!((MsgImpl)msg)._keyReserved)
            {
                /* write key length */
                /* now store the size - have to trick it into being an RB15 */
                /* only want the encoded size of the key */
                int keySize = (iter._curBufPos - lenPos - 2);
                /* now set the RB bit */
                keySize |= 0x8000;
                /* store it - don't need to increment iterator because its already at end of key */
                iter._writer.position(lenPos);
                iter._writer.writeShort(keySize);
                iter._writer.position(iter._curBufPos);
            }
            else
            {
                /* user still has to encode key opaque */
                /* store this in the internalMark to fill in later */
                iter._levelInfo[iter._encodingLevel]._internalMark._sizeBytes = 2;
                iter._levelInfo[iter._encodingLevel]._internalMark._sizePos = lenPos;
            }
        }

        if ((flags & HAS_EXT_HEADER) > 0)
        {
            /* user passed it in, lets encode it now */
            if (((MsgImpl)msg)._keyReserved)
            {
                /* User set flag to indicate there is an extended header but they didn't encode their opaque yet. */
                /* Set us up to expect opaque. */
                ((MsgImpl)msg)._extendedHeaderReserved = true;
            }
            else
            {
                /* no key opaque - if the extended header is here, put it on the wire,
                 * otherwise set it up so we expect extended header */
                if (msg.extendedHeader().length() > 0)
                {
                    if (iter.isIteratorOverrun(msg.extendedHeader().length() + 1))
                    {
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }
                    encodeBuffer8(iter, (BufferImpl)msg.extendedHeader());
                    iter._curBufPos = iter._writer.position();
                }
                else
                {
                    if (iter.isIteratorOverrun(1))
                    {
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }

                    ((MsgImpl)msg)._extendedHeaderReserved = true;
                    /* must reserve space now */
                    iter._levelInfo[iter._encodingLevel]._internalMark._sizePos = iter._curBufPos;
                    iter._levelInfo[iter._encodingLevel]._internalMark._sizeBytes = 1;
                    iter._curBufPos++; // move pointer */
                    iter._writer.position(iter._curBufPos);
                }
            }
        }
        return retVal;
    }
	
    private static int encodeKeyInternal(EncodeIteratorImpl iter, MsgKeyImpl key)
    {
        int flags;

        if (iter.isIteratorOverrun(2))
            return CodecReturnCodes.BUFFER_TOO_SMALL;

        /* Store flags as UInt15-rb */
        flags = key.flags();

        if (key.checkHasName() && key.name().length() == 0)
            flags &= ~MsgKeyFlags.HAS_NAME;

        if (!key.checkHasName() && key.checkHasNameType())
            flags &= ~MsgKeyFlags.HAS_NAME_TYPE;

        // flags value is written on wire as uShort15rb
        // If the value is smaller than 0x80, it is written on the wire as one byte,
        // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
        // The code below checks if the buffer is sufficient for each case.
        if (flags < 0x80)
        {
            if (iter.isIteratorOverrun(3)) // 2 bytes skip + 1 byte flags
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            iter._curBufPos += 2;
            iter._writer.skipBytes(2);
            iter._writer.writeByte(flags);
        }
        else
        {
            if (iter.isIteratorOverrun(4)) // 2 bytes skip + 2 bytes flags
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            iter._curBufPos += 2;
            iter._writer.skipBytes(2);
            iter._writer.writeUShort15rbLong((short)flags);
        }
        iter._curBufPos = iter._writer.position();

        /* Store SourceId as UINt16_ob */
        if (key.checkHasServiceId())
        {
            // ID value is written on wire as uShort16ob
            // If the value is smaller than 0xFE, it is written on the wire as one byte,
            // otherwise, it is written as three bytes by calling writeUShort16obLong method.
            // The code below checks if the buffer is sufficient for each case.
            if (key.serviceId() < 0xFE)
            {
                if (iter.isIteratorOverrun(1))
                    return CodecReturnCodes.BUFFER_TOO_SMALL;

                iter._writer.writeByte(key.serviceId());
            }
            else
            {
                if (iter.isIteratorOverrun(3))
                    return CodecReturnCodes.BUFFER_TOO_SMALL;

                iter._writer.writeUShort16obLong(key.serviceId());

            }
            iter._curBufPos = iter._writer.position();
        }

        /* Store name as CharPtr == SmallBuffer */
        if (key.checkHasName())
        {
            /* verify name length is only 1 byte */
            assert (key.name().length() <= 255) : "Invalid key name";
            if (key.name().length() > 255)
            {
                return CodecReturnCodes.INVALID_ARGUMENT;
            }

            if (iter.isIteratorOverrun(key.name().length() + 1))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            encodeBuffer8(iter, (BufferImpl)key.name());
            iter._curBufPos = iter._writer.position();

            /* should only do things with NameType if we have name */
            if (key.checkHasNameType())
            {
                if (iter.isIteratorOverrun(1))
                    return CodecReturnCodes.BUFFER_TOO_SMALL;

                iter._writer.writeByte(key.nameType());
                iter._curBufPos = iter._writer.position();
            }
        }

        /* Store Filter as UInt32 */
        if (key.checkHasFilter())
        {
            if (iter.isIteratorOverrun(4))
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            iter._writer.writeUInt(key.filter());
            iter._curBufPos = iter._writer.position();
        }

        /* Store Identifier as UInt32 */
        if (key.checkHasIdentifier())
        {
            if (iter.isIteratorOverrun(4))
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            iter._writer.writeInt(key.identifier());
            iter._curBufPos = iter._writer.position();
        }

        return CodecReturnCodes.SUCCESS;
    }
	
    static int encodeMsgKeyAttribComplete(EncodeIterator iterInt, boolean success)
    {
        int ret;
        int headerSize;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];
        Msg msg = (Msg)(_levelInfo._listType);

        /* Validations */
        assert null != iter : "Invalid parameters or parameters passed in as NULL";
        assert (_levelInfo._encodingState == EncodeIteratorStates.OPAQUE)
                || (_levelInfo._encodingState == EncodeIteratorStates.OPAQUE_AND_EXTENDED_HEADER)
                || (_levelInfo._encodingState == EncodeIteratorStates.WAIT_COMPLETE) : "Unexpected encoding attempted";
        assert null != iter._buffer : "Invalid iterator use - check buffer";
        assert iter._curBufPos <= iter._endBufPos : "Data exceeds iterators buffer length";

        if (success)
        {
            if (_levelInfo._internalMark2._sizeBytes > 0)
            {
                if ((ret = finishU15Mark(iter, _levelInfo._internalMark2, iter._curBufPos)) < 0)
                {
                    /* rollback */
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    iter._curBufPos = _levelInfo._internalMark2._sizePos;
                    iter._writer.position(iter._curBufPos);
                    return ret;
                }
            }
            else
            {
                /* no opaque was encoded - failure */
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.position(iter._curBufPos);
                return CodecReturnCodes.FAILURE;
            }

            /* write key length into buffer */
            if (_levelInfo._internalMark._sizeBytes > 0)
            {
                if ((ret = finishU15Mark(iter, _levelInfo._internalMark, iter._curBufPos)) < 0)
                {
                    /* roll back */
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    iter._curBufPos = _levelInfo._internalMark._sizePos;
                    iter._writer.position(iter._curBufPos);
                    return ret;
                }
            }
            else
            {
                /* no key was attempted to be encoded - failure */
                /* go to start of buffer */
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.position(iter._curBufPos);
                return CodecReturnCodes.FAILURE;
            }

            /* if they still need to encode extended header do that */
            if (_levelInfo._encodingState == EncodeIteratorStates.OPAQUE_AND_EXTENDED_HEADER)
            {
                Buffer extHdr;
                assert null != msg : "Invalid parameters or parameters passed in as NULL";
                /* see if the extended header was pre-encoded */
                /* if we can, write it, if not reserve space */

                extHdr = msg.extendedHeader();

                if (extHdr != null && extHdr.length() > 0)
                {
                    /* we can encode this here and now */
                    if (iter.isIteratorOverrun(extHdr.length() + 1))
                    {
                        /* rollback */
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        iter._curBufPos = _levelInfo._initElemStartPos;
                        iter._writer.position(iter._curBufPos);
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }
                    encodeBuffer8(iter, (BufferImpl)extHdr);
                    iter._curBufPos = iter._writer.position();

                    /* Header is finished, encode the end of the header */
                    if ((ret = finishMsgHeader(iter, msg)) != CodecReturnCodes.SUCCESS)
                    {
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        iter._curBufPos = _levelInfo._initElemStartPos;
                        iter._writer.position(iter._curBufPos);
                        return ret;
                    }

                    /* write header size */
                    headerSize = iter._curBufPos - _levelInfo._countWritePos - 2;
                    iter._writer.position(_levelInfo._countWritePos);
                    iter._writer.writeShort(headerSize);
                    iter._writer.position(iter._curBufPos);

                    /* now store current location so we can check it to ensure user did not put data
                     * when they shouldnt */
                    /* count has been filled in already */
                    _levelInfo._countWritePos = iter._curBufPos;

                    _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

                    if (msg.containerType() != DataTypes.NO_DATA)
                        ret =  CodecReturnCodes.ENCODE_CONTAINER;
                    else
                        ret = CodecReturnCodes.SUCCESS;
                }
                else
                {
                    /* it will be encoded after this, reserve space for size */

                    if (iter.isIteratorOverrun(1))
                    {
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        iter._curBufPos = _levelInfo._initElemStartPos;
                        iter._writer.position(iter._curBufPos);
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }

                    /* must reserve space now */
                    _levelInfo._internalMark._sizePos = iter._curBufPos;
                    _levelInfo._internalMark._sizeBytes = 1;
                    iter._curBufPos++; // move pointer */
                    iter._writer.position(iter._curBufPos);

                    _levelInfo._encodingState = EncodeIteratorStates.EXTENDED_HEADER;

                    ret = CodecReturnCodes.ENCODE_EXTENDED_HEADER;
                }
            }
            else
            {
                /* Header is finished, encode the end of the header */
                if ((ret = finishMsgHeader(iter, (Msg)(_levelInfo._listType))) != CodecReturnCodes.SUCCESS)
                {
                    /* rollback */
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    iter._curBufPos = _levelInfo._initElemStartPos;
                    iter._writer.position(iter._curBufPos);
                    return ret;
                }

                /* write header size */
                headerSize = iter._curBufPos - _levelInfo._countWritePos - 2;
                iter._writer.position(_levelInfo._countWritePos);
                iter._writer.writeShort(headerSize);
                iter._writer.position(iter._curBufPos);

                _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

                /* now store current location so we can check it to ensure user did not put data
                 * when they shouldnt */
                /* count has been filled in already */
                _levelInfo._countWritePos = iter._curBufPos;

                if (msg.containerType() != DataTypes.NO_DATA)
                    ret = CodecReturnCodes.ENCODE_CONTAINER;
                else
                    ret = CodecReturnCodes.SUCCESS;
            }
        }
        else
        {
            /* roll back and change state */
            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
            iter._curBufPos = _levelInfo._initElemStartPos;
            iter._writer.position(iter._curBufPos);

            ret = CodecReturnCodes.SUCCESS;
        }

        return ret;
    }

    static int encodeExtendedHeaderComplete(EncodeIterator iterInt, boolean success)
    {
        int ret;
        int size;
        int headerSize;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];
        Msg msg = (Msg)(_levelInfo._listType);

        /* Validations */
        assert null != iter : "Invalid parameters or parameters passed in as NULL";
        assert _levelInfo._encodingState == EncodeIteratorStates.EXTENDED_HEADER : "Unexpected encoding attempted";
        assert null != iter._buffer : "Invalid iterator use - check buffer";
        assert iter._curBufPos <= iter._endBufPos : "Data exceeds iterators buffer length";

        if (success)
        {
            if (_levelInfo._internalMark._sizeBytes > 0)
            {
                /* write one byte length onto wire */
                size = (iter._curBufPos - _levelInfo._internalMark._sizePos - _levelInfo._internalMark._sizeBytes);
                iter._writer.position(_levelInfo._internalMark._sizePos);
                iter._writer.writeUByte(size);
                iter._writer.position(iter._curBufPos);
            }
            else
            {
                /* extended header shouldn't have been encoded */
                /* roll back and change state */
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.position(iter._curBufPos);
            }

            /* Extended Header is finished, encode the end of the header */
            if ((ret = finishMsgHeader(iter, msg)) != CodecReturnCodes.SUCCESS)
            {
                /* roll back and change state */
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.position(iter._curBufPos);
                return ret;
            }

            /* write header size */
            headerSize = iter._curBufPos - _levelInfo._countWritePos - 2;
            iter._writer.position(_levelInfo._countWritePos);
            iter._writer.writeShort(headerSize);
            iter._writer.position(iter._curBufPos);

            /* now store current location so we can check it to ensure user did not put data
             * when they shouldnt */
            /* count has been filled in already */
            _levelInfo._countWritePos = iter._curBufPos;


            /* should be ready to encode entries - don't save length for data */
            _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

            if (msg.containerType() != DataTypes.NO_DATA)
            {
                return CodecReturnCodes.ENCODE_CONTAINER;
            }
            else
            {
                return CodecReturnCodes.SUCCESS;
            }
        }
        else
        {
            /* roll back and change state */
            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
            iter._curBufPos = _levelInfo._initElemStartPos;
            iter._writer.position(iter._curBufPos);
        }

        return CodecReturnCodes.SUCCESS;
    }

    private static void encodeBuffer8(EncodeIteratorImpl iter, BufferImpl buffer)
    {
        int tlen = buffer.length();
        iter._writer.writeUByte(tlen);
        iter._writer.write(buffer);
    }
	
    private static void encodeBuffer15(EncodeIteratorImpl iter, BufferImpl buffer)
    {
        int tlen = buffer.length();
        if (tlen < 0x80)
        {
            iter._writer.writeByte(tlen);
        }
        else
        {
            iter._writer.writeUShort15rbLong((short)tlen);
        }
        iter._writer.write(buffer);
    }

    @SuppressWarnings("unused")
    private static void encodeBuffer32(EncodeIteratorImpl iter, BufferImpl buffer)
    {
        int tlen = buffer.length();
        iter._writer.writeUInt32ob(tlen);
        iter._writer.write(buffer);
    }

    private static int setupU15Mark(EncodeSizeMark mark, int maxSize, int position)
    {
        assert 0 != position : "Invalid encoding attempted";
        assert null != mark && mark._sizePos == 0 : "Invalid encoding attempted";

        mark._sizePos = position;

        if ((maxSize == 0) || (maxSize >= 0x80))
        {
            mark._sizeBytes = 2;
            position += 2;
        }
        else
        {
            mark._sizeBytes = 1;
            position += 1;
        }

        return position;
    }
	
    private static int setupU16Mark(EncodeSizeMark mark, int maxSize, int position)
    {
        assert position != 0 : "Invalid encoding attempted";
        assert null != mark && mark._sizePos == 0 : "Invalid encoding attempted";

        mark._sizePos = position;

        if ((maxSize == 0) || (maxSize >= 0xFE))
        {
            mark._sizeBytes = 3;
            position += 3;
        }
        else
        {
            mark._sizeBytes = 1;
            position += 1;
        }

        return position;
    }

    private static int finishMsgHeader(EncodeIteratorImpl iter, Msg msg)
    {
        switch (msg.msgClass())
        {
            case MsgClasses.UPDATE:
                UpdateMsg updateMsg = (UpdateMsg)msg;
                if (updateMsg.checkHasPostUserInfo())
                {
                    if (iter.isIteratorOverrun(8))
                        return CodecReturnCodes.BUFFER_TOO_SMALL;

                    iter._writer.writeUInt(updateMsg.postUserInfo().userAddr());
                    iter._writer.writeUInt(updateMsg.postUserInfo().userId());
                    iter._curBufPos = iter._writer.position();
                }
                break;
            case MsgClasses.REFRESH:
                RefreshMsg refreshMsg = (RefreshMsg)msg;
                if (refreshMsg.checkHasPostUserInfo())
                {
                    if (iter.isIteratorOverrun(8))
                        return CodecReturnCodes.BUFFER_TOO_SMALL;

                    iter._writer.writeUInt(refreshMsg.postUserInfo().userAddr());
                    iter._writer.writeUInt(refreshMsg.postUserInfo().userId());
                    iter._curBufPos = iter._writer.position();
                }
                if (refreshMsg.checkHasPartNum())
                {
                    // flags value is written on wire as uShort15rb
                    // If the value is smaller than 0x80, it is written on the wire as one byte,
                    // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
                    // The code below checks if the buffer is sufficient for each case.
                    if (refreshMsg.partNum() < 0x80)
                    {
                        if (iter.isIteratorOverrun(1))
                            return CodecReturnCodes.BUFFER_TOO_SMALL;
                        iter._writer.writeByte(refreshMsg.partNum());
                    }
                    else
                    {
                        if (iter.isIteratorOverrun(2))
                            return CodecReturnCodes.BUFFER_TOO_SMALL;
                        iter._writer.writeUShort15rbLong((short)refreshMsg.partNum());
                    }
                }
                iter._curBufPos = iter._writer.position();
                break;
            case MsgClasses.STATUS:
                StatusMsg statusMsg = (StatusMsg)msg;
                if (statusMsg.checkHasPostUserInfo())
                {
                    if (iter.isIteratorOverrun(8))
                    {
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }
                    iter._writer.writeUInt(statusMsg.postUserInfo().userAddr());
                    iter._writer.writeUInt(statusMsg.postUserInfo().userId());
                    iter._curBufPos = iter._writer.position();
                }
                break;
            case MsgClasses.GENERIC:
                GenericMsg genericMsg = (GenericMsg)msg;
                if (genericMsg.checkHasPartNum())
                {
                    if (genericMsg.partNum() < 0x80)
                    {
                        if (iter.isIteratorOverrun(1))
                            return CodecReturnCodes.BUFFER_TOO_SMALL;
                        iter._writer.writeByte(genericMsg.partNum());
                    }
                    else
                    {
                        if (iter.isIteratorOverrun(2))
                            return CodecReturnCodes.BUFFER_TOO_SMALL;
                        iter._writer.writeUShort15rbLong((short)genericMsg.partNum());
                    }
                }
                iter._curBufPos = iter._writer.position();
                break;
            case MsgClasses.POST:
                PostMsg postMsg = (PostMsg)msg;
                if (postMsg.checkHasPartNum())
                {
                    if (postMsg.partNum() < 0x80)
                    {
                        if (iter.isIteratorOverrun(1))
                            return CodecReturnCodes.BUFFER_TOO_SMALL;
                        iter._writer.writeByte(postMsg.partNum());
                    }
                    else
                    {
                        if (iter.isIteratorOverrun(2))
                            return CodecReturnCodes.BUFFER_TOO_SMALL;
                        iter._writer.writeUShort15rbLong((short)postMsg.partNum());
                    }
                }
                if (postMsg.checkHasPostUserRights())
                {
                    if (postMsg.postUserRights() < 0x80)
                    {
                        if (iter.isIteratorOverrun(1))
                            return CodecReturnCodes.BUFFER_TOO_SMALL;
                        iter._writer.writeByte(postMsg.postUserRights());
                    }
                    else
                    {
                        if (iter.isIteratorOverrun(2))
                            return CodecReturnCodes.BUFFER_TOO_SMALL;
                        iter._writer.writeUShort15rbLong((short)postMsg.postUserRights());
                    }
                }
                iter._curBufPos = iter._writer.position();
                break;
            case MsgClasses.REQUEST:
            case MsgClasses.CLOSE:
            case MsgClasses.ACK:
                break;
            default:
                break;
        }

        return CodecReturnCodes.SUCCESS;
    }

    static int encodeElementListInit(EncodeIterator iterInt, ElementList elementListInt,
            LocalElementSetDefDb setDbInt, int setEncodingMaxSize)
    {
        assert null != iterInt : "Invalid IterInt in as NULL";
        assert null != elementListInt : "Invalid elementListInt in as NULL";

        EncodingLevel levelInfo;
        int setId;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        LocalElementSetDefDbImpl setDb = (LocalElementSetDefDbImpl)setDbInt;
        ElementListImpl elementList = (ElementListImpl)elementListInt;

        if (++iter._encodingLevel >= EncodeIteratorImpl.ENC_ITER_MAX_LEVELS)
            return CodecReturnCodes.ITERATOR_OVERRUN;

        levelInfo = iter._levelInfo[iter._encodingLevel];
        levelInfo.init(DataTypes.ELEMENT_LIST, EncodeIteratorStates.NONE, elementList, iter._curBufPos);

        /* make sure required elements can be encoded */
        if (iter.isIteratorOverrun(1))
        {
            levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        /* store flags as UInt8 */
        iter._writer.writeUByte(elementList._flags);
        iter._curBufPos = iter._writer.position();

        /* check for List Info */
        if (elementList.checkHasInfo())
        {
            int infoLen;
            int startPos;

            /* make sure that required elements can be encoded */
            if (iter.isIteratorOverrun(3))
            {
                levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            }

            /* save info length position */
            startPos = iter._curBufPos;

            /* move past it */
            iter._curBufPos += 1;
            iter._writer.skipBytes(1);

            /* put element list number into element list */
            iter._writer.writeShort(elementList._elementListNum);
            iter._curBufPos = iter._writer.position();

            /* encode the length into the element list */
            infoLen = iter._curBufPos - startPos - 1;
            iter._writer.position(startPos);
            iter._writer.writeUByte(infoLen);
            iter._writer.position(iter._curBufPos);
        }

        /* check for set data */
        if (elementList.checkHasSetData())
        {
            /* we have element list set data */
            /* make sure set id and set length can be encoded */
            /* setID (2), setData (2) */
            if (iter.isIteratorOverrun(4))
            {
                levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            }

            /* check for/encode optional element list set id */
            if (elementList.checkHasSetId())
            {

                // ID value is written on wire as uShort15rb
                // If the value is smaller than 0x80, it is written on the wire as one byte,
                // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
                // The code below checks if the buffer is sufficient for each case.
                if (elementList._setId < 0x80)
                {
                    if (iter.isIteratorOverrun(1))
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    iter._writer.writeByte(elementList._setId);
                }
                else
                {
                    if (iter.isIteratorOverrun(2))
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    iter._writer.writeUShort15rbLong((short)elementList._setId);
                }

                iter._curBufPos = iter._writer.position();
                setId = elementList._setId;
            }
            else
                setId = 0;

            if (setDb != null && setId <= LocalElementSetDefDbImpl.MAX_LOCAL_ID && setDb._definitions[setId]._setId != LocalElementSetDefDbImpl.BLANK_ID)
                levelInfo._elemListSetDef = setDb._definitions[setId];
            else if (iter._elementSetDefDb != null && setId <= iter._elementSetDefDb.maxSetId() && iter._elementSetDefDb.definitions()[setId] != null)
                levelInfo._elemListSetDef = (ElementSetDefImpl)iter._elementSetDefDb.definitions()[setId];

            /* check for element list data after the set data */
            if (elementList.checkHasStandardData())
            {
                /* if have one set data and field list data, need length in front of the set */
                if (elementList._encodedSetData.length() > 0)
                {
                    /* the set data is already encoded */
                    /* make sure that the set data can be encoded */
                    int len = elementList._encodedSetData.length();
                    if (len < 0x80)
                    {
                        if (iter.isIteratorOverrun(3 + len))
                        {
                            levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                            return CodecReturnCodes.BUFFER_TOO_SMALL;
                        }
                        iter._writer.writeByte(len);
                        iter._writer.write((BufferImpl)elementList._encodedSetData);
                    }
                    else
                    {
                        if (iter.isIteratorOverrun(4 + len))
                        {
                            levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                            return CodecReturnCodes.BUFFER_TOO_SMALL;
                        }
                        iter._writer.writeUShort15rbLong((short)len);
                        iter._writer.write((BufferImpl)elementList._encodedSetData);
                    }

                    iter._curBufPos = iter._writer.position();

                    /* save bytes for field list data count */
                    levelInfo._countWritePos = iter._curBufPos;
                    levelInfo._encodingState = EncodeIteratorStates.ENTRIES;
                    iter._curBufPos += 2;
                    iter._writer.skipBytes(2);
                    return CodecReturnCodes.SUCCESS;
                }
                else
                {
                    final int reservedBytes = 2;

                    /* the set data needs to be encoded */
                    /* save state and return */
                    if (levelInfo._elemListSetDef != null)
                    {
                        if (iter.isIteratorOverrun(((setEncodingMaxSize >= 0x80 || setEncodingMaxSize == 0) ? 2 : 1) + reservedBytes))
                        {
                            levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                            return CodecReturnCodes.BUFFER_TOO_SMALL;
                        }
                        iter._curBufPos = setupU15Mark(levelInfo._internalMark, setEncodingMaxSize, iter._curBufPos);
                        iter._writer.position(iter._curBufPos);

                        /* Back up endBufPos to account for reserved bytes. */
                        levelInfo._reservedBytes = reservedBytes;
                        iter._endBufPos -= reservedBytes;
                        iter._writer.reserveBytes(reservedBytes);

                        /* If the set actually has no entries, just complete the set. */
                        if (levelInfo._elemListSetDef._count > 0)
                            levelInfo._encodingState = EncodeIteratorStates.SET_DATA;
                        else
                            return completeElementSet(iter, levelInfo, elementList);
                    }
                    else
                    {
                        levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        return CodecReturnCodes.SET_DEF_NOT_PROVIDED;
                    }
                }
            }
            else
            {
                /* don't need length in front of set data */
                /* encode set data if it exists */
                if (elementList._encodedSetData.length() > 0)
                {
                    /* make sure that set data can be encoded */
                    if (iter.isIteratorOverrun(elementList._encodedSetData.length()))
                    {
                        levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }

                    /* don't need a length in front of set data */
                    encodeCopyData(iter, (BufferImpl)elementList._encodedSetData);
                    iter._curBufPos = iter._writer.position();

                    levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return CodecReturnCodes.SUCCESS;
                }
                else
                {
                    /* don't need length in front of set data */
                    /* save size pointer in case of rewind */

                    if (levelInfo._elemListSetDef != null)
                    {
                        levelInfo._internalMark._sizePos = iter._curBufPos;
                        levelInfo._internalMark._sizeBytes = 0;

                        /* If the set actually has no entries, turn around and complete the set. */
                        if (levelInfo._elemListSetDef._count > 0)
                            levelInfo._encodingState = EncodeIteratorStates.SET_DATA;
                        else
                            return completeElementSet(iter, levelInfo, elementList);
                    }
                    else
                    {
                        levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        return CodecReturnCodes.SET_DEF_NOT_PROVIDED;
                    }
                }
            }
        }
        else if (elementList.checkHasStandardData())
        {
            /* we only have data */
            if (iter.isIteratorOverrun(2))
            {
                levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            }

            levelInfo._countWritePos = iter._curBufPos;
            levelInfo._encodingState = EncodeIteratorStates.ENTRIES;
            iter._curBufPos += 2;
            iter._writer.skipBytes(2);
        }

        if (levelInfo._encodingState == EncodeIteratorStates.NONE)
            levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;

        return CodecReturnCodes.SUCCESS;
    }

    static int encodeElementListComplete(EncodeIterator iterInt, boolean success)
    {
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        /* Validations */
        assert null != iter && null != _levelInfo._listType : "Invalid parameters or parameters passed in as NULL";
        assert _levelInfo._containerType == DataTypes.ELEMENT_LIST : "Invalid encoding attempted - wrong type";
        assert (_levelInfo._encodingState == EncodeIteratorStates.ENTRIES) ||
               (_levelInfo._encodingState == EncodeIteratorStates.SET_DATA) ||
               (_levelInfo._encodingState == EncodeIteratorStates.WAIT_COMPLETE) : "Unexpected encoding attempted";
        assert null != iter._buffer : "Invalid iterator use - check buffer";
        assert iter._curBufPos <= iter._endBufPos : "Data exceeds iterators buffer length";

        if (success)
        {
            if (((ElementList)_levelInfo._listType).checkHasStandardData())
            {
                assert _levelInfo._countWritePos != 0 : "Invalid encoding attempted";
                iter._writer.position(_levelInfo._countWritePos);
                iter._writer.writeShort(_levelInfo._currentCount);
                iter._writer.position(iter._curBufPos);
            }
        }
        else
        {
            iter._curBufPos = _levelInfo._containerStartPos;
            iter._writer.position(iter._curBufPos);
        }

        // _levelInfo._encodingState = EncodeIteratorStates.EIS_COMPLETE;
        --iter._encodingLevel;

        return CodecReturnCodes.SUCCESS;
    }
	
    static int encodeElementEntry(EncodeIterator iterInt, ElementEntry elementInt, Object data)
    {
        int ret = 0;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];
        ElementEntryImpl element = (ElementEntryImpl)elementInt;

        /* Validations */
        assert null != iter && null != _levelInfo._listType : "Invalid parameters or parameters passed in as NULL";
        assert null != element : "Invalid parameters or parameters passed in as NULL";
        assert _levelInfo._containerType == DataTypes.ELEMENT_LIST || _runningInJunits : "Invalid encoding attempted - wrong type";
        assert _levelInfo._encodingState == EncodeIteratorStates.ENTRIES ||
                _levelInfo._encodingState == EncodeIteratorStates.SET_DATA : "Unexpected encoding attempted";
        assert null != iter._buffer : "Invalid iterator use - check buffer";
        assert (_levelInfo._currentCount + 1) != 0 : "Invalid encoding attempted";
        assert iter._curBufPos <= iter._endBufPos : "Data exceeds iterators buffer length";

        _levelInfo._initElemStartPos = iter._curBufPos;

        if (_levelInfo._encodingState == EncodeIteratorStates.SET_DATA)
        {
            ElementList elementList = (ElementList)_levelInfo._listType;
            ElementSetDefImpl def = _levelInfo._elemListSetDef;
            assert null != def : "Invalid parameters or parameters passed in as NULL";

            /* Use currentCount as a temporary set iterator. It should be reset before doing standard entries. */
            ElementSetDefEntryImpl encoding = (ElementSetDefEntryImpl)def._entries[_levelInfo._currentCount];

            /* Validate name (if present) */
            if (element._name != null && element._name.equals(encoding.name()) == false)
                return CodecReturnCodes.INVALID_DATA;

            /* Validate type */
            if (element._dataType != Decoders.convertToPrimitiveType(encoding._dataType))
                return CodecReturnCodes.INVALID_DATA;

            /* Encode item according to set type */
            if (data != null)
            {
                if ((ret = PrimitiveEncoder.encodeSetData(iter, data, encoding._dataType)) < 0)
                {
                    /* rollback */
                    iter._curBufPos = _levelInfo._initElemStartPos;
                    iter._writer.position(iter._curBufPos);
                    return ret;
                }
            }
            else if (element._encodedData.length() > 0)/* Encoding pre-encoded data from the field entry */
            {
                /* if the dataType is primitive we need to make sure the data is length specified */
                if ((element._dataType < DataTypes.SET_PRIMITIVE_MIN) || (element._dataType > DataTypes.CONTAINER_TYPE_MIN))
                {
                    int len = (element._encodedData).length();

                    // len value is written on wire as uShort16ob
                    // If the value is smaller than 0xFE, it is written on the wire as one byte,
                    // otherwise, it is written as three bytes by calling writeUShort16obLong method.
                    // The code below checks if the buffer is sufficient for each case.
                    if (len < 0xFE)
                    {
                        if (iter.isIteratorOverrun(1 + len))
                            return CodecReturnCodes.BUFFER_TOO_SMALL;

                        iter._writer.writeByte(len);
                        iter._writer.write((BufferImpl)element._encodedData);
                    }
                    else if (len <= 0xFFFF)
                    {
                        if (iter.isIteratorOverrun(3 + len))
                            return CodecReturnCodes.BUFFER_TOO_SMALL;

                        iter._writer.writeUShort16obLong(len);
                        iter._writer.write((BufferImpl)element._encodedData);
                    }
                    else
                        return CodecReturnCodes.INVALID_DATA;

                    iter._curBufPos = iter._writer.position();
                }
                else
                {
                    if (iter.isIteratorOverrun((element._encodedData).length()))
                        return CodecReturnCodes.BUFFER_TOO_SMALL;

                    iter._writer.write((BufferImpl)element._encodedData);
                    iter._curBufPos = iter._writer.position();
                }
            }
            else
            /* blank */
            {
                if ((ret = encodeBlank(iter, encoding._dataType)) != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (++_levelInfo._currentCount < def._count)
                return CodecReturnCodes.SUCCESS;

            /* Set is complete. */
            if ((ret = completeElementSet(iter, _levelInfo, (ElementListImpl)elementList)) < CodecReturnCodes.SUCCESS)
                return ret;

            return CodecReturnCodes.SET_COMPLETE;
        }

        /* Encoding standard entries. */
        assert _levelInfo._encodingState == EncodeIteratorStates.ENTRIES : "Unexpected encoding attempted";

        if (data != null)
        {
            assert null != element._name.data() : "Missing element name";

            /* store element name as buffer 15 */
            int len = element._name.length();

            // flags value is written on wire as uShort15rb
            // If the value is smaller than 0x80, it is written on the wire as one byte,
            // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
            // The code below checks if the buffer is sufficient for each case.
            if (len < 0x80)
            {
                if (iter.isIteratorOverrun(2 + len)) // 1 byte len + 1 byte datatype + len
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                iter._writer.writeByte(len);
                iter._writer.write((BufferImpl)element._name);
            }
            else
            {
                if (iter.isIteratorOverrun(3 + len)) // 2 bytes len + 1 byte datatype + len
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                iter._writer.writeUShort15rbLong((short)len);
                iter._writer.write((BufferImpl)element._name);
            }

            /* store data type */
            iter._writer.writeUByte(element._dataType);
            iter._curBufPos = iter._writer.position();

            if (element._dataType != DataTypes.NO_DATA)
            {
                _levelInfo._encodingState = EncodeIteratorStates.PRIMITIVE;
                if ((element._dataType >= DataTypes.DATETIME_9) || (PrimitiveEncoder._setEncodeActions[element._dataType] == null))
                    ret = CodecReturnCodes.UNSUPPORTED_DATA_TYPE;
                else
                    ret = PrimitiveEncoder.encodeSetData(iter, data, element._dataType);
            }
            _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

            if (ret < 0)
            {
                /* rollback */
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.position(iter._curBufPos);
                return ret;
            }
            _levelInfo._currentCount++;
            return CodecReturnCodes.SUCCESS;
        }
        else if (element._encodedData.length() > 0)
        {
            int len = element._name.length();
            if (len < 0x80)
            {
                if (iter.isIteratorOverrun(2 + len))
                {
                    /* rollback */
                    iter._curBufPos = _levelInfo._initElemStartPos;
                    iter._writer.position(iter._curBufPos);
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                }
                iter._writer.writeByte(len);
                iter._writer.write((BufferImpl)element._name);
            }
            else
            {
                if (iter.isIteratorOverrun(3 + len))
                {
                    /* rollback */
                    iter._curBufPos = _levelInfo._initElemStartPos;
                    iter._writer.position(iter._curBufPos);
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                }
                iter._writer.writeUShort15rbLong((short)len);
                iter._writer.write((BufferImpl)element._name);
            }

            /* store datatype */
            iter._writer.writeUByte(element._dataType);
            iter._curBufPos = iter._writer.position();

            /* copy encoded data */
            if (element._dataType != DataTypes.NO_DATA)
            {
                len = element._encodedData.length();
                if (len < 0xFE)
                {
                    if (iter.isIteratorOverrun(1 + len))
                    {
                        /* rollback */
                        iter._curBufPos = _levelInfo._initElemStartPos;
                        iter._writer.position(iter._curBufPos);
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }

                    iter._writer.writeByte(len);
                    iter._writer.write((BufferImpl)element._encodedData);
                }
                else if (len <= 0xFFFF)
                {
                    if (iter.isIteratorOverrun(3 + len))
                    {
                        /* rollback */
                        iter._curBufPos = _levelInfo._initElemStartPos;
                        iter._writer.position(iter._curBufPos);
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }

                    iter._writer.writeUShort16obLong(len);
                    iter._writer.write((BufferImpl)element._encodedData);

                }
                else
                    return CodecReturnCodes.INVALID_DATA;

                iter._curBufPos = iter._writer.position();
            }

            _levelInfo._currentCount++;
            return CodecReturnCodes.SUCCESS;
        }
        else
        {
            int len = element._name.length();
            if (len < 0x80)
            {
                if (iter.isIteratorOverrun(2 + len))
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                iter._writer.writeByte(len);
                iter._writer.write((BufferImpl)element._name);
            }
            else
            {
                if (iter.isIteratorOverrun(3 + len))
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                iter._writer.writeUShort15rbLong((short)len);
                iter._writer.write((BufferImpl)element._name);
            }

            /* store datatype */
            iter._writer.writeUByte(element._dataType);
            iter._curBufPos = iter._writer.position();

            /* copy encoded data */
            if (element._dataType != DataTypes.NO_DATA)
            {
                if (iter.isIteratorOverrun(1))
                {
                    /* rollback */
                    iter._curBufPos = _levelInfo._initElemStartPos;
                    iter._writer.position(iter._curBufPos);
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                }

                int zero = 0;
                iter._writer.writeUByte(zero);
                iter._curBufPos = iter._writer.position();
            }

            _levelInfo._currentCount++;
            return CodecReturnCodes.SUCCESS;
        }
    }

    static int encodeElementEntryInit(EncodeIterator iterInt, ElementEntry elementInt, int encodingMaxSize)
    {
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        ElementEntryImpl element = (ElementEntryImpl)elementInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        /* Validations */
        assert null != iter && null != _levelInfo._listType : "Invalid parameters or parameters passed in as NULL";
        assert _levelInfo._containerType == DataTypes.ELEMENT_LIST : "Invalid encoding attempted";
        assert _levelInfo._encodingState == EncodeIteratorStates.ENTRIES ||
                _levelInfo._encodingState == EncodeIteratorStates.SET_DATA : "Unexpected encoding attempted";
        assert null != iter._buffer : "Invalid iterator use - check buffer";
        assert null != element && null != element._name.data() : "Invalid parameters or parameters passed in as NULL";
        assert (_levelInfo._currentCount + 1) != 0 : "Invalid encoding attempted";
        assert iter._curBufPos <= iter._endBufPos : " Data exceeds iterators buffer length";

        _levelInfo._initElemStartPos = iter._curBufPos;

        /* Set data. */
        if (_levelInfo._encodingState == EncodeIteratorStates.SET_DATA)
        {
            ElementSetDefImpl def = _levelInfo._elemListSetDef;

            /* Use currentCount as a temporary set iterator. It should be reset before doing standard entries. */
            ElementSetDefEntryImpl encoding = (ElementSetDefEntryImpl)def._entries[_levelInfo._currentCount];

            assert null != def : "Invalid parameters or parameters passed in as NULL";

            if (!validAggregateDataType(_levelInfo._containerType))
            {
                _levelInfo._encodingState = EncodeIteratorStates.SET_ENTRY_WAIT_COMPLETE;
                return CodecReturnCodes.UNSUPPORTED_DATA_TYPE;
            }

            /* Validate name (if present) */
            if (element._name != null && element._name.equals(encoding.name()) == false)
            {
                _levelInfo._encodingState = EncodeIteratorStates.SET_ENTRY_WAIT_COMPLETE;
                return CodecReturnCodes.INVALID_DATA;
            }

            /* Validate type */
            if (element._dataType != Decoders.convertToPrimitiveType(encoding._dataType))
            {
                _levelInfo._encodingState = EncodeIteratorStates.SET_ENTRY_WAIT_COMPLETE;
                return CodecReturnCodes.INVALID_DATA;
            }

            if (iter.isIteratorOverrun(((encodingMaxSize == 0 || encodingMaxSize >= 0xFE) ? 3 : 1)))
            {
                _levelInfo._encodingState = EncodeIteratorStates.SET_ENTRY_WAIT_COMPLETE;
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            }

            _levelInfo._encodingState = EncodeIteratorStates.SET_ENTRY_INIT;

            /* need to use mark 2 here because mark 1 is used to length specify all the set data */
            iter._curBufPos = setupU16Mark(_levelInfo._internalMark2, encodingMaxSize, iter._curBufPos);
            iter._writer.position(iter._curBufPos);

            return CodecReturnCodes.SUCCESS;
        }

        /* Standard data. */
        int len = element._name.length();

        // len value is written on wire as uShort15rb
        // If the value is smaller than 0x80, it is written on the wire as one byte,
        // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
        // The code below checks if the buffer is sufficient for each case.
        if (len < 0x80)
        {
            if (iter.isIteratorOverrun(2 + len)) // 1 byte len + len + 1 byte dataType
            {
                _levelInfo._encodingState = EncodeIteratorStates.ENTRY_WAIT_COMPLETE;
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            }
            iter._writer.writeByte(len);
            iter._writer.write((BufferImpl)element._name);
        }
        else
        {
            if (iter.isIteratorOverrun(3 + len)) // 2 bytes len + len + 1 byte dataType
            {
                _levelInfo._encodingState = EncodeIteratorStates.ENTRY_WAIT_COMPLETE;
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            }
            iter._writer.writeUShort15rbLong((short)len);
            iter._writer.write((BufferImpl)element._name);
        }

        iter._curBufPos = iter._writer.position();
        /* store data type */
        iter._writer.writeUByte(element._dataType);
        iter._curBufPos = iter._writer.position();

        _levelInfo._encodingState = EncodeIteratorStates.ENTRY_INIT;

        if (element._dataType != DataTypes.NO_DATA)
        {
            if (iter.isIteratorOverrun(((encodingMaxSize == 0 || encodingMaxSize >= 0xFE) ? 3 : 1)))
            {
                _levelInfo._encodingState = EncodeIteratorStates.ENTRY_WAIT_COMPLETE;
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            }

            iter._curBufPos = setupU16Mark(_levelInfo._internalMark, encodingMaxSize, iter._curBufPos);
            iter._writer.position(iter._curBufPos);
        }
        else
        {
            _levelInfo._internalMark._sizeBytes = 0;
            _levelInfo._internalMark._sizePos = iter._curBufPos;
        }

        return CodecReturnCodes.SUCCESS;
    }
	
    static boolean validAggregateDataType(int dataType)
    {
        boolean retVal = false;

        switch (dataType)
        {
            case DataTypes.NO_DATA:
            case DataTypes.OPAQUE:
            case DataTypes.XML:
            case DataTypes.FIELD_LIST:
            case DataTypes.ELEMENT_LIST:
            case DataTypes.ANSI_PAGE:
            case DataTypes.FILTER_LIST:
            case DataTypes.VECTOR:
            case DataTypes.MAP:
            case DataTypes.SERIES:
            case 139:
            case 140:
            case DataTypes.MSG:
            case DataTypes.JSON:
            case 223:
                retVal = true;
                break;
            default:
                if (dataType > DataTypes.MAX_RESERVED && dataType <= DataTypes.LAST)
                {
                    retVal = true;
                }
                break;
        }

        return retVal;
    }
	
    private static boolean validPrimitiveDataType(int dataType)
    {
        boolean retVal = false;

        switch (dataType)
        {
            case DataTypes.INT:
            case DataTypes.UINT:
            case DataTypes.FLOAT:
            case DataTypes.DOUBLE:
            case DataTypes.REAL:
            case DataTypes.DATE:
            case DataTypes.TIME:
            case DataTypes.DATETIME:
            case DataTypes.QOS:
            case DataTypes.STATE:
            case DataTypes.ENUM:
            case DataTypes.ARRAY:
            case DataTypes.BUFFER:
            case DataTypes.ASCII_STRING:
            case DataTypes.UTF8_STRING:
            case DataTypes.RMTES_STRING:
                retVal = true;
                break;
            default:
                break;
        }

        return retVal;
    }

    static int encodeElementEntryComplete(EncodeIterator iterInt, boolean success)
    {
        int ret;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        /* Validations */
        assert null != iter && null != _levelInfo._listType : "Invalid parameters or parameters passed in as NULL";
        assert _levelInfo._containerType == DataTypes.ELEMENT_LIST : "Invalid encoding attempted";
        assert _levelInfo._encodingState == EncodeIteratorStates.ENTRY_INIT ||
                _levelInfo._encodingState == EncodeIteratorStates.SET_ENTRY_INIT ||
                _levelInfo._encodingState == EncodeIteratorStates.SET_ENTRY_WAIT_COMPLETE ||
                _levelInfo._encodingState == EncodeIteratorStates.ENTRY_WAIT_COMPLETE ||
                _levelInfo._encodingState == EncodeIteratorStates.ENTRIES : "Unexpected encoding attempted";
        assert null != iter._buffer : "Invalid iterator use - check buffer";
        assert (_levelInfo._currentCount + 1) != 0 : "Invalid encoding attempted";
        assert iter._curBufPos <= iter._endBufPos : "Data exceeds iterators buffer length";

        /* Set data. */
        if (_levelInfo._encodingState == EncodeIteratorStates.SET_ENTRY_INIT
                || _levelInfo._encodingState == EncodeIteratorStates.SET_ENTRY_WAIT_COMPLETE)
        {
            ElementList elementList = (ElementList)_levelInfo._listType;
            ElementSetDefImpl def = _levelInfo._elemListSetDef;
            if (success)
            {
                if (_levelInfo._internalMark2._sizeBytes > 0)
                {
                    /* need to use mark 2 here because mark 1 is used to length specify all the set data */
                    if ((ret = finishU16Mark(iter, _levelInfo._internalMark2, iter._curBufPos)) < 0)
                    {
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        _levelInfo._initElemStartPos = 0;
                        return ret;
                    }
                }
                _levelInfo._initElemStartPos = 0;

                if (++_levelInfo._currentCount < def._count)
                {
                    _levelInfo._encodingState = EncodeIteratorStates.SET_DATA;
                    return CodecReturnCodes.SUCCESS;
                }

                /* Set is complete. */
                if ((ret = completeElementSet(iter, _levelInfo, (ElementListImpl)elementList)) < 0)
                    return ret;

                return CodecReturnCodes.SET_COMPLETE;
            }
            else
            {
                /* reset the pointer */
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.position(iter._curBufPos);
                _levelInfo._initElemStartPos = 0;
                _levelInfo._encodingState = EncodeIteratorStates.SET_DATA;
                return CodecReturnCodes.SUCCESS;
            }
        }

        /* Standard data. */
        if (success)
        {
            if (_levelInfo._internalMark._sizeBytes > 0)
            {
                if ((ret = finishU16Mark(iter, _levelInfo._internalMark, iter._curBufPos)) < 0)
                {
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    _levelInfo._initElemStartPos = 0;
                    return ret;
                }
            }
            _levelInfo._currentCount++;
        }
        else
        {
            /* reset the pointer */
            iter._curBufPos = _levelInfo._initElemStartPos;
            iter._writer.position(iter._curBufPos);
        }

        _levelInfo._initElemStartPos = 0;
        _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

        return CodecReturnCodes.SUCCESS;
    }
	
    private static int completeElementSet(EncodeIteratorImpl iter, EncodingLevel _levelInfo, ElementListImpl elementList)
    {
        int ret;

        /* Set definition completed. Write the length, and move on to standard data if any. */
        /* length may not be encoded in certain cases */
        if (_levelInfo._internalMark._sizeBytes != 0)
        {
            if ((ret = finishU15Mark(iter, _levelInfo._internalMark, iter._curBufPos)) < 0)
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return ret;
            }

            if (((ElementList)_levelInfo._listType).checkHasStandardData())
            {
                /* Move endBufPos back to original position if bytes were reserved. */
                iter._endBufPos += _levelInfo._reservedBytes;
                iter._writer.unreserveBytes(_levelInfo._reservedBytes);
                _levelInfo._reservedBytes = 0;

                /* Reset entry count. Only Standard Entries actually count towards it. */
                _levelInfo._currentCount = 0;
                _levelInfo._countWritePos = iter._curBufPos;
                iter._curBufPos += 2;
                iter._writer.skipBytes(2);
                _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;
            }
            else
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
        }
        else
            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;

        return CodecReturnCodes.SUCCESS;
    }
	
    static int encodeFieldListInit(EncodeIterator iterInt, FieldList fieldListInt, LocalFieldSetDefDb setDbInt, int setEncodingMaxSize)
    {
        int ret;
        EncodingLevel levelInfo;
        int setId;
        FieldListImpl fieldList = (FieldListImpl)fieldListInt;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        LocalFieldSetDefDbImpl setDb = (LocalFieldSetDefDbImpl)setDbInt;

        /* Assertions */
        assert null != fieldList && null != iter : "Invalid parameters or parameters passed in as NULL";

        levelInfo = iter._levelInfo[++iter._encodingLevel];
        if (iter._encodingLevel >= EncodeIteratorImpl.ENC_ITER_MAX_LEVELS)
            return CodecReturnCodes.ITERATOR_OVERRUN;
        levelInfo.init(DataTypes.FIELD_LIST, EncodeIteratorStates.NONE, fieldList, iter._curBufPos);

        /* Make sure that required elements can be encoded */
        if (iter.isIteratorOverrun(1))
        {
            levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        /* Store flags as UInt8 */
        iter._writer.writeByte(fieldList._flags);
        iter._curBufPos = iter._writer.position();

        /* Check for field list info */
        if (fieldList.checkHasInfo())
        {
            int infoLen;
            int startPos;

            // ID value is written on wire as uShort15rb
            // If the value is smaller than 0x80, it is written on the wire as one byte,
            // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
            // The code below checks if the buffer is sufficient for each case.
            if (fieldList._dictionaryId < 0x80)
            {
                if (iter.isIteratorOverrun(4)) // 1 byte skip + 1 byte id + 2 bytes fieldListNum
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                /* Save info length position */
                startPos = iter._curBufPos;

                /* Skip info length so it can be encoded later */
                iter._writer.skipBytes(1);
                iter._curBufPos += 1;

                iter._writer.writeByte(fieldList._dictionaryId);
            }
            else
            {
                if (iter.isIteratorOverrun(5)) // 1 byte skip + 2 bytes id + 2 bytes fieldListNum
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                /* Save info length position */
                startPos = iter._curBufPos;

                /* Skip info length so it can be encoded later */
                iter._writer.skipBytes(1);
                iter._curBufPos += 1;

                iter._writer.writeUShort15rbLong((short)fieldList._dictionaryId);
            }

            /* Put field list number */
            iter._writer.writeShort(fieldList._fieldListNum);
            iter._curBufPos = iter._writer.position();

            /* encode the field list info length */
            infoLen = iter._curBufPos - startPos - 1;
            iter._writer.position(startPos);
            iter._writer.writeByte(infoLen);
            iter._writer.position(iter._curBufPos);
        }

        /* Check for set data */
        if (fieldList.checkHasSetData())
        {
            /* We have Field List Set Data */
            /* Check for encode optional Field List Set Id */
            if (fieldList.checkHasSetId())
            {
                if (fieldList._setId < 0x80)
                {
                    if (iter.isIteratorOverrun(1))
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    iter._writer.writeByte(fieldList._setId);
                }
                else
                {
                    if (iter.isIteratorOverrun(2))
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    iter._writer.writeUShort15rbLong((short)fieldList._setId);
                }
                iter._curBufPos = iter._writer.position();
                setId = fieldList._setId;
            }
            else
                setId = 0;

            if (setDb != null && setId <= LocalFieldSetDefDbImpl.MAX_LOCAL_ID && setDb._definitions[setId]._setId != LocalFieldSetDefDbImpl.BLANK_ID)
                levelInfo._fieldListSetDef = setDb._definitions[setId];
            else if (iter._fieldSetDefDb != null && setId <= iter._fieldSetDefDb.maxSetId() && iter._fieldSetDefDb.definitions()[setId] != null)
                levelInfo._fieldListSetDef = (FieldSetDefImpl)iter._fieldSetDefDb.definitions()[setId];

            /* Check for Field List Data after the Set Data */
            if (fieldList.checkHasStandardData())
            {
                /* If have set data and field list data, need length in front of the set */
                if (fieldList._encodedSetData.length() > 0)
                {
                    /* The set data is already encoded. */
                    /* Make sure that the set data can be encoded. The length was included in the previous check. */
                    int len = fieldList._encodedSetData.length();
                    if (len < 0x80)
                    {
                        if (iter.isIteratorOverrun(3 + len))
                            return CodecReturnCodes.BUFFER_TOO_SMALL;
                        iter._writer.writeByte(len);
                        iter._writer.write((BufferImpl)fieldList._encodedSetData);
                    }
                    else
                    {
                        if (iter.isIteratorOverrun(4 + len))
                            return CodecReturnCodes.BUFFER_TOO_SMALL;
                        iter._writer.writeUShort15rbLong((short)len);
                        iter._writer.write((BufferImpl)fieldList._encodedSetData);
                    }

                    iter._curBufPos = iter._writer.position();
                    /* Save bytes for field list data count */
                    levelInfo._countWritePos = iter._curBufPos;
                    levelInfo._encodingState = EncodeIteratorStates.ENTRIES;
                    iter._writer.skipBytes(2);
                    iter._curBufPos += 2;
                    return CodecReturnCodes.SUCCESS;
                }
                else
                {
                    final int reservedBytes = 2;

                    /* The set data need to be encoded. */
                    /* Save state and return */
                    if (levelInfo._fieldListSetDef != null)
                    {
                        if (iter.isIteratorOverrun(((setEncodingMaxSize >= 0x80 || setEncodingMaxSize == 0) ? 2 : 1) + reservedBytes))
                        {
                            levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                            return CodecReturnCodes.BUFFER_TOO_SMALL;
                        }
                        iter._curBufPos = setupU15Mark(levelInfo._internalMark, setEncodingMaxSize, iter._curBufPos);
                        iter._writer.position(iter._curBufPos);

                        /* Back up endBufPos to account for reserved bytes. */
                        levelInfo._reservedBytes = reservedBytes;
                        iter._endBufPos -= reservedBytes;
                        iter._writer.reserveBytes(reservedBytes);

                        /* If the set actually has no entries, just complete the set. */
                        if (levelInfo._fieldListSetDef._count > 0)
                        {
                            levelInfo._encodingState = EncodeIteratorStates.SET_DATA;
                            return CodecReturnCodes.SUCCESS;
                        }
                        else
                        {
                            if ((ret = completeFieldSet(iter, levelInfo, fieldList)) != CodecReturnCodes.SUCCESS)
                            {
                                levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                                return ret;
                            }

                            return CodecReturnCodes.SUCCESS;
                        }
                    }
                    else
                    {
                        levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        return CodecReturnCodes.SET_DEF_NOT_PROVIDED;
                    }
                }
            }
            else
            {
                /* Don't need a length in front of set data. */
                /* Encode set data if it exists */
                if (fieldList._encodedSetData.length() > 0)
                {
                    /* Make sure that the set data can be encoded. */
                    if (iter.isIteratorOverrun(fieldList._encodedSetData.length()))
                    {
                        levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }

                    /* Don't need a length in front of set data. */
                    encodeCopyData(iter, (BufferImpl)fieldList._encodedSetData);
                    iter._curBufPos = iter._writer.position();

                    levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return CodecReturnCodes.SUCCESS;
                }
                else
                {
                    /* Don't need a length in front of set data. */
                    /* Save the size pointer in case we need to rewind */
                    if (levelInfo._fieldListSetDef != null)
                    {
                        levelInfo._internalMark._sizePos = iter._curBufPos;
                        levelInfo._internalMark._sizeBytes = 0;

                        /* If the set actually has no entries, just complete the set. */
                        if (levelInfo._fieldListSetDef._count > 0)
                            levelInfo._encodingState = EncodeIteratorStates.SET_DATA;
                        else
                        {
                            if ((ret = completeFieldSet(iter, levelInfo, fieldList)) != CodecReturnCodes.SUCCESS)
                            {
                                levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                                return ret;
                            }
                        }
                        return CodecReturnCodes.SUCCESS;
                    }
                    else
                    {
                        levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        return CodecReturnCodes.SET_DEF_NOT_PROVIDED;
                    }
                }
            }
        }
        else if (fieldList.checkHasStandardData())
        {
            /* We only have field list data */
            if (iter.isIteratorOverrun(2))
            {
                levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            }

            levelInfo._countWritePos = iter._curBufPos;
            levelInfo._encodingState = EncodeIteratorStates.ENTRIES;
            iter._writer.skipBytes(2);
            iter._curBufPos += 2;
        }

        if (levelInfo._encodingState == EncodeIteratorStates.NONE)
            levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;

        return CodecReturnCodes.SUCCESS;
    }

    private static int completeFieldSet(EncodeIteratorImpl iter, EncodingLevel _levelInfo, FieldListImpl fieldList)
    {
        int ret;

        /* Set definition completed. Write the length, and move on to standard data if any. */
        /* length may not be encoded in certain cases */
        if (_levelInfo._internalMark._sizeBytes != 0)
        {
            if ((ret = finishU15Mark(iter, _levelInfo._internalMark, iter._curBufPos)) < 0)
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return ret;
            }

            if (fieldList.checkHasStandardData())
            {
                /* Move endBufPos back to original position if bytes were reserved. */
                iter._endBufPos += _levelInfo._reservedBytes;
                iter._writer.unreserveBytes(_levelInfo._reservedBytes);
                _levelInfo._reservedBytes = 0;

                /* Reset entry count. Only Standard Entries actually count towards it. */
                _levelInfo._currentCount = 0;
                _levelInfo._countWritePos = iter._curBufPos;
                iter._curBufPos += 2;
                iter._writer.skipBytes(2);
                _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;
            }
            else
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
        }
        else
            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;

        return CodecReturnCodes.SUCCESS;
    }
	
    static int encodeFieldListComplete(EncodeIterator iterInt, boolean success)
    {
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        /* Validations */
        assert null != iter && null != _levelInfo._listType : "Invalid parameters or parameters passed in as NULL";
        assert _levelInfo._containerType == DataTypes.FIELD_LIST : "Invalid encoding attempted - wrong type";
        assert ((_levelInfo._encodingState == EncodeIteratorStates.ENTRIES) ||
                (_levelInfo._encodingState == EncodeIteratorStates.SET_DATA) ||
                (_levelInfo._encodingState == EncodeIteratorStates.WAIT_COMPLETE)) : "Unexpected encoding attempted";
        assert null != iter._buffer : "Invalid iterator use - check buffer";
        assert iter._curBufPos <= iter._endBufPos : "Data exceeds iterators buffer length";

        if (success)
        {
            if (_levelInfo._encodingState == EncodeIteratorStates.ENTRIES)
            {
                assert _levelInfo._countWritePos != 0 : "Invalid encoding attempted";
                iter._writer.position(_levelInfo._countWritePos);
                iter._writer.writeShort(_levelInfo._currentCount);
                iter._writer.position(iter._curBufPos);
            }
        }
        else
        {
            iter._curBufPos = _levelInfo._containerStartPos;
            iter._writer.position(iter._curBufPos);
        }
        --iter._encodingLevel;

        return CodecReturnCodes.SUCCESS;
    }

    static int encodeFieldEntry(EncodeIterator iterInt, FieldEntry fieldInt, Object data)
    {
        int ret;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        FieldEntryImpl field = (FieldEntryImpl)fieldInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        /* Validations */
        assert null != iter && null != _levelInfo._listType : "Invalid parameters or parameters passed in as NULL";
        // assert field, Invalid parameters or parameters passed in as NULL";
        assert _levelInfo._containerType == DataTypes.FIELD_LIST : "Invalid encoding attempted - wrong container";
        assert _levelInfo._encodingState == EncodeIteratorStates.ENTRIES ||
                _levelInfo._encodingState == EncodeIteratorStates.SET_DATA : "Unexpected encoding attempted";
        assert null != iter._buffer : "Invalid iterator use - check buffer";
        assert (_levelInfo._currentCount + 1) != 0 : "Invalid encoding attempted";
        assert iter._curBufPos <= iter._endBufPos : "Data exceeds iterators buffer length";

        _levelInfo._initElemStartPos = iter._curBufPos;

        if (_levelInfo._encodingState == EncodeIteratorStates.SET_DATA)
        {
            FieldList fieldList = (FieldList)_levelInfo._listType;
            FieldSetDefImpl def = _levelInfo._fieldListSetDef;

            assert null != def : "Invalid parameters or parameters passed in as NULL";
            /* Use currentCount as a temporary set iterator. It should be reset before doing standard entries. */
            FieldSetDefEntryImpl encoding = (FieldSetDefEntryImpl)def._entries[_levelInfo._currentCount];

            /* Validate fid */
            if (field._fieldId != encoding._fieldId)
                return CodecReturnCodes.INVALID_DATA;

            /* Validate type */
            if (field._dataType != Decoders.convertToPrimitiveType(encoding._dataType))
                return CodecReturnCodes.INVALID_DATA;

            /* Encode item according to set type */
            if (data != null)
            {
                if ((ret = PrimitiveEncoder.encodeSetData(iter, data, encoding._dataType)) < 0)
                {
                    /* rollback */
                    iter._curBufPos = _levelInfo._initElemStartPos;
                    iter._writer.position(iter._curBufPos);
                    return ret;
                }
            }
            else if (field._encodedData.length() > 0)/* Encoding pre-encoded data from the field entry */
            {
                /* if the dataType is primitive we need to make sure the data is length specified */
                if ((field._dataType < DataTypes.SET_PRIMITIVE_MIN) || (field._dataType > DataTypes.CONTAINER_TYPE_MIN))
                {
                    int len = field._encodedData.length();

                    // len value is written on wire as uShort16ob
                    // If the value is smaller than 0xFE, it is written on the wire as one byte,
                    // otherwise, it is written as three bytes by calling writeUShort16obLong method.
                    // The code below checks if the buffer is sufficient for each case.
                    if (len < 0xFE)
                    {
                        if (iter.isIteratorOverrun(1 + len))
                            return CodecReturnCodes.BUFFER_TOO_SMALL;

                        iter._writer.writeByte(len);
                        iter._writer.write((BufferImpl)field._encodedData);
                    }
                    else if (len <= 0xFFFF)
                    {
                        if (iter.isIteratorOverrun(3 + len))
                            return CodecReturnCodes.BUFFER_TOO_SMALL;

                        iter._writer.writeUShort16obLong(len);
                        iter._writer.write((BufferImpl)field._encodedData);

                    }
                    else
                        return CodecReturnCodes.INVALID_DATA;

                    iter._curBufPos = iter._writer.position();
                }
                else
                {
                    if (iter.isIteratorOverrun(field._encodedData.length()))
                        return CodecReturnCodes.BUFFER_TOO_SMALL;

                    iter._writer.write((BufferImpl)field._encodedData);
                    iter._curBufPos = iter._writer.position();
                }
            }
            else
            /* blank */
            {
                if ((ret = encodeBlank(iter, encoding._dataType)) != CodecReturnCodes.SUCCESS)
                    return ret;
            }

            if (++_levelInfo._currentCount < def._count)
                return CodecReturnCodes.SUCCESS;

            if ((ret = completeFieldSet(iter, _levelInfo, (FieldListImpl)fieldList)) < 0)
                return ret;

            return CodecReturnCodes.SET_COMPLETE;
        }

        if (data != null)
        {
            if (iter.isIteratorOverrun(2))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            /* Store FieldId as Uint16 */
            iter._writer.writeShort(field._fieldId);
            iter._curBufPos = iter._writer.position();

            /* Encode the data type */
            _levelInfo._encodingState = EncodeIteratorStates.PRIMITIVE;
            if ((field._dataType >= DataTypes.DATETIME_9) || (PrimitiveEncoder._setEncodeActions[field._dataType] == null))
                ret = CodecReturnCodes.UNSUPPORTED_DATA_TYPE;
            else
                ret = PrimitiveEncoder.encodeSetData(iter, data, field._dataType);

            _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

            if (ret < 0)
            {
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.position(iter._curBufPos);
                return ret;
            }

            _levelInfo._currentCount++;
            return CodecReturnCodes.SUCCESS;
        }
        else if (field._encodedData.length() > 0) /* Encoding pre-encoded data from the field entry */
        {
            if (iter.isIteratorOverrun(2))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            /* Store FieldId as Uint16 */
            iter._writer.writeShort(field._fieldId);
            iter._curBufPos = iter._writer.position();

            int len = field._encodedData.length();
            if (len < 0xFE)
            {
                if (iter.isIteratorOverrun(1 + len))
                {
                    /* rollback */
                    iter._curBufPos = _levelInfo._initElemStartPos;
                    iter._writer.position(iter._curBufPos);
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                }

                iter._writer.writeByte(len);
                iter._writer.write((BufferImpl)field._encodedData);
            }
            else if (len <= 0xFFFF)
            {
                if (iter.isIteratorOverrun(3 + len))
                {
                    /* rollback */
                    iter._curBufPos = _levelInfo._initElemStartPos;
                    iter._writer.position(iter._curBufPos);
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                }

                iter._writer.writeUShort16obLong(len);
                iter._writer.write((BufferImpl)field._encodedData);
            }
            else
            {
                /* rollback */
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.position(iter._curBufPos);
                return CodecReturnCodes.INVALID_DATA;
            }

            iter._curBufPos = iter._writer.position();

            _levelInfo._currentCount++;
            return CodecReturnCodes.SUCCESS;
        }
        else
        /* Encoding as blank */
        {
            int zero = 0;
            if (iter.isIteratorOverrun(3))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            /* Store FieldId as Uint16 */
            iter._writer.writeShort(field._fieldId);
            iter._curBufPos = iter._writer.position();

            iter._writer.writeByte(zero);
            iter._curBufPos = iter._writer.position();

            _levelInfo._currentCount++;
            return CodecReturnCodes.SUCCESS;
        }
    }
	
    static int encodeFieldEntryInit(EncodeIterator iterInt, FieldEntry fieldInt, int encodingMaxSize)
    {
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        FieldEntryImpl field = (FieldEntryImpl)fieldInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        /* Validations */
        assert null != iter && null != _levelInfo._listType : "Invalid parameters or parameters passed in as NULL";
        assert null != field : "Invalid parameters or parameters passed in as NULL";

        assert _levelInfo._containerType == DataTypes.FIELD_LIST : "Invalid encoding attempted - wrong type";
        assert _levelInfo._encodingState == EncodeIteratorStates.ENTRIES ||
                _levelInfo._encodingState == EncodeIteratorStates.SET_DATA : "Unexpected encoding attempted";
        assert null != iter._buffer : "Invalid iterator use - check buffer";
        assert (_levelInfo._currentCount + 1) != 0 : "Invalid encoding attempted";
        assert iter._curBufPos <= iter._endBufPos : "Data exceeds iterators buffer length";

        _levelInfo._initElemStartPos = iter._curBufPos;

        /* Set data */
        if (_levelInfo._encodingState == EncodeIteratorStates.SET_DATA)
        {
            FieldSetDef def = _levelInfo._fieldListSetDef;
            assert null != def : "Invalid parameters or parameters passed in as NULL";
            /* Use currentCount as a temporary set iterator. It should be reset before doing standard entries. */
            FieldSetDefEntry encoding = def.entries()[_levelInfo._currentCount];

            if (!validAggregateDataType(_levelInfo._containerType))
            {
                _levelInfo._encodingState = EncodeIteratorStates.SET_ENTRY_WAIT_COMPLETE;
                return CodecReturnCodes.UNSUPPORTED_DATA_TYPE;
            }

            /* Validate fid */
            if (field.fieldId() != encoding.fieldId())
            {
                _levelInfo._encodingState = EncodeIteratorStates.SET_ENTRY_WAIT_COMPLETE;
                return CodecReturnCodes.INVALID_DATA;
            }

            /* Validate type */
            if (field.dataType() != Decoders.convertToPrimitiveType(encoding.dataType()))
            {
                _levelInfo._encodingState = EncodeIteratorStates.SET_ENTRY_WAIT_COMPLETE;
                return CodecReturnCodes.INVALID_DATA;
            }

            if (iter.isIteratorOverrun(((encodingMaxSize == 0 || encodingMaxSize >= 0xFE) ? 3 : 1)))
            {
                _levelInfo._encodingState = EncodeIteratorStates.SET_ENTRY_WAIT_COMPLETE;
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            }

            _levelInfo._encodingState = EncodeIteratorStates.SET_ENTRY_INIT;

            /* need to use internal mark 2 here because mark 1 is used to length specify set data */
            iter._curBufPos = setupU16Mark(_levelInfo._internalMark2, encodingMaxSize, iter._curBufPos);
            iter._writer.position(iter._curBufPos);

            return CodecReturnCodes.SUCCESS;

        }

        /* Standard data */
        if (iter.isIteratorOverrun((3 + ((encodingMaxSize == 0 || encodingMaxSize >= 0xFE) ? 2 : 0))))
        {
            _levelInfo._encodingState = EncodeIteratorStates.ENTRY_WAIT_COMPLETE;
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        /* Store FieldId as Uint16 */
        iter._writer.writeShort((short)field.fieldId());
        iter._curBufPos = iter._writer.position();
        _levelInfo._encodingState = EncodeIteratorStates.ENTRY_INIT;

        iter._curBufPos = setupU16Mark(_levelInfo._internalMark, encodingMaxSize, iter._curBufPos);
        iter._writer.position(iter._curBufPos);

        return CodecReturnCodes.SUCCESS;
    }
	
    static int encodeFieldEntryComplete(EncodeIterator iterInt, boolean success)
    {
        int ret;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        /* Validations */
        assert null != iter && null != _levelInfo._listType : "Invalid parameters or parameters passed in as NULL";
        assert _levelInfo._containerType == DataTypes.FIELD_LIST : "Invalid encoding attempted - wrong type";
        assert _levelInfo._encodingState == EncodeIteratorStates.ENTRY_INIT ||
                _levelInfo._encodingState == EncodeIteratorStates.SET_ENTRY_INIT ||
                _levelInfo._encodingState == EncodeIteratorStates.ENTRY_WAIT_COMPLETE ||
                _levelInfo._encodingState == EncodeIteratorStates.SET_ENTRY_WAIT_COMPLETE ||
                _levelInfo._encodingState == EncodeIteratorStates.ENTRIES : "Unexpected encoding attempted";
        assert null != iter._buffer : "Invalid iterator use - check buffer";
        assert (_levelInfo._currentCount + 1) != 0 : "Invalid encoding attempted";
        assert iter._curBufPos <= iter._endBufPos : "Data exceeds iterators buffer length";

        /* Set data (user was encoding a container in the set) */
        if (_levelInfo._encodingState == EncodeIteratorStates.SET_ENTRY_INIT
                || _levelInfo._encodingState == EncodeIteratorStates.SET_ENTRY_WAIT_COMPLETE)
        {
            FieldList fieldList = (FieldList)_levelInfo._listType;
            FieldSetDef def = _levelInfo._fieldListSetDef;
            if (success)
            {
                /* need to use internal mark 2 here because mark 1 is used to length specify set data */
                if ((ret = finishU16Mark(iter, _levelInfo._internalMark2, iter._curBufPos)) < 0)
                {
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    _levelInfo._initElemStartPos = 0;
                    return ret;
                }
                _levelInfo._initElemStartPos = 0;

                if (++_levelInfo._currentCount < def.count())
                {
                    _levelInfo._encodingState = EncodeIteratorStates.SET_DATA;
                    return CodecReturnCodes.SUCCESS;
                }

                /* Set is complete. */
                if ((ret = completeFieldSet(iter, _levelInfo, (FieldListImpl)fieldList)) < 0)
                    return ret;

                return CodecReturnCodes.SET_COMPLETE;
            }
            else
            {
                /* Reset the pointer */
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.position(iter._curBufPos);
                _levelInfo._initElemStartPos = 0;
                _levelInfo._encodingState = EncodeIteratorStates.SET_DATA;
                return CodecReturnCodes.SUCCESS;
            }
        }

        /* Standard data. */
        if (success)
        {
            if ((ret = finishU16Mark(iter, _levelInfo._internalMark, iter._curBufPos)) < 0)
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                _levelInfo._initElemStartPos = 0;
                return ret;
            }
            _levelInfo._currentCount++;
        }
        else
        {
            /* Reset the pointer */
            iter._curBufPos = _levelInfo._initElemStartPos;
            iter._writer.position(iter._curBufPos);
        }
        _levelInfo._initElemStartPos = 0;
        _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

        return CodecReturnCodes.SUCCESS;
    }

    private static int finishU15Mark(EncodeIteratorImpl iter, EncodeSizeMark mark, int position)
    {
        int dataLength;

        assert null != mark : "Invalid parameters or parameters passed in as NULL";
        assert mark._sizePos != 0 : "Invalid encoding attempted";
        assert mark._sizeBytes != 0 : "Invalid encoding attempted";
        assert position != 0 : "Invalid encoding attempted";

        dataLength = position - mark._sizePos - mark._sizeBytes;

        if (dataLength > RWF_MAX_U15)
            return CodecReturnCodes.BUFFER_TOO_SMALL;

        if (mark._sizeBytes == 1)
        {
            if (dataLength >= 0x80)
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.position(mark._sizePos);
            iter._writer.writeByte(dataLength);
            iter._writer.position(iter._curBufPos);
        }
        else
        {
            assert mark._sizeBytes == 2 : "Invalid encoding attempted";
            dataLength |= 0x8000;
            iter._writer.position(mark._sizePos);
            iter._writer.writeShort(dataLength);
            iter._writer.position(iter._curBufPos);
        }
        mark._sizePos = 0;
        return CodecReturnCodes.SUCCESS;
    }
	
    private static int finishU16Mark(EncodeIteratorImpl iter, EncodeSizeMark mark, int position)
    {
        int dataLength;

        assert null != mark : "Invalid parameters or parameters passed in as NULL";
        assert mark._sizePos != 0 : "Invalid encoding attempted";
        assert mark._sizeBytes != 0 : "Invalid encoding attempted";
        assert position != 0 : "Invalid encoding attempted";

        dataLength = position - mark._sizePos - mark._sizeBytes;

        if (mark._sizeBytes == 1)
        {
            if (dataLength >= 0xFE)
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.position(mark._sizePos);
            iter._writer.writeByte(dataLength);
            iter._writer.position(iter._curBufPos);
        }
        else
        {
            int dl = dataLength;
            assert mark._sizeBytes == 3 : "Invalid encoding attempted";

            if (dataLength > RWF_MAX_16)
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            iter._writer.position(mark._sizePos);
            iter._writer.writeByte(0xFE);
            iter._writer.writeShort(dl);
            iter._writer.position(iter._curBufPos);
        }
        mark._sizePos = 0;
        return CodecReturnCodes.SUCCESS;
    }
	
    private static void encodeCopyData(EncodeIteratorImpl iter, BufferImpl buf)
    {
        iter._writer.write(buf);
    }
	
    static int encodeArrayInit(EncodeIterator iterInt, Array arrayInt)
    {
        assert null != iterInt : "Invalid IterInt in as NULL";
        assert null != arrayInt : "Invalid arrayInt in as NULL";
        ArrayImpl array = (ArrayImpl)arrayInt;
        EncodingLevel levelInfo;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;

        if ((array._primitiveType >= DataTypes.SET_PRIMITIVE_MIN) || (array._primitiveType == DataTypes.ARRAY))
            return CodecReturnCodes.UNSUPPORTED_DATA_TYPE;

        levelInfo = iter._levelInfo[++iter._encodingLevel];
        if (iter._encodingLevel >= EncodeIteratorImpl.ENC_ITER_MAX_LEVELS)
            return CodecReturnCodes.ITERATOR_OVERRUN;
        levelInfo.init(DataTypes.ARRAY, EncodeIteratorStates.NONE, array, iter._curBufPos);

        // length value is written on wire as uShort16ob
        // If the value is smaller than 0xFE, it is written on the wire as one byte,
        // otherwise, it is written as three bytes by calling writeUShort16obLong method.
        // The code below checks if the buffer is sufficient for each case.
        if (array._itemLength < 0xFE)
        {
            if (iter.isIteratorOverrun(4)) // 1 byte primitive type + 1 byte length + 2 bytes skip
            {
                levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            }
            iter._writer.writeUByte(array._primitiveType);
            iter._writer.writeByte(array._itemLength);
        }
        else
        {
            if (iter.isIteratorOverrun(6)) // 1 byte primitive type + 3 bytes length + 2 bytes skip
            {
                levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            }
            iter._writer.writeUByte(array._primitiveType);
            iter._writer.writeUShort16obLong(array._itemLength);
        }

        iter._curBufPos = iter._writer.position();
        levelInfo._countWritePos = iter._curBufPos;
        iter._curBufPos += 2;
        iter._writer.skipBytes(2);
        /* change encoding state */
        levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

        return CodecReturnCodes.SUCCESS;
    }
	
    static int encodeArrayEntry(EncodeIterator iterInt, Object data)
    {
        ArrayImpl array;
        int ret = CodecReturnCodes.SUCCESS;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        /* Validations */
        assert (iterInt != null);
        assert (data != null);
        assert (_levelInfo._containerType == DataTypes.ARRAY);
        assert (_levelInfo._encodingState == EncodeIteratorStates.ENTRIES);

        array = (ArrayImpl)_levelInfo._listType;

        // set start position for this entry - needed for rollback
        _levelInfo._initElemStartPos = iter._curBufPos;

        switch (array._primitiveType)
        {
            case DataTypes.BUFFER:
            case DataTypes.ASCII_STRING:
            case DataTypes.UTF8_STRING:
            case DataTypes.RMTES_STRING:
                if (array._itemLength == 0)
                {
                    int len = ((Buffer)data).length();

                    // len value is written on wire as uShort16ob
                    // If the value is smaller than 0xFE, it is written on the wire as one byte,
                    // otherwise, it is written as three bytes by calling writeUShort16obLong method.
                    // The code below checks if the buffer is sufficient for each case.
                    if (len < 0xFE)
                    {
                        if (iter.isIteratorOverrun(1 + len))
                            return CodecReturnCodes.BUFFER_TOO_SMALL;

                        iter._writer.writeByte(len);
                        iter._writer.write((BufferImpl)data);
                    }
                    else if (len <= 0xFFFF)
                    {
                        if (iter.isIteratorOverrun(3 + len))
                            return CodecReturnCodes.BUFFER_TOO_SMALL;

                        iter._writer.writeUShort16obLong(len);
                        iter._writer.write((BufferImpl)data);

                    }
                    else
                        return CodecReturnCodes.INVALID_DATA;
                }
                else
                {
                    if (iter.isIteratorOverrun(array._itemLength))
                        return CodecReturnCodes.BUFFER_TOO_SMALL;

                    Buffer buffer = (Buffer)data;

                    // Buffer can be shorter than itemLength.
                    if (buffer.length() < array._itemLength)
                    {
                        for (int i = 0; i < buffer.length(); i++)
                        {
                            iter._writer.writeByte(((BufferImpl)buffer).dataByte(buffer.position() + i));
                        }
                        for (int i = buffer.length(); i < array._itemLength; i++)
                        {
                            iter._writer.writeByte(0x00);
                        }
                    }
                    else
                    {
                        // Make sure we don't copy more bytes than necessary, truncate extra bytes
                        for (int i = 0; i < array._itemLength; i++)
                        {
                            iter._writer.writeByte(((BufferImpl)buffer).dataByte(buffer.position() + i));
                        }
                    }
                }
                iter._curBufPos = iter._writer.position();
                ret = CodecReturnCodes.SUCCESS;
                break;

            default:
                /* Use the appropriate primitive encode method, if one exists for this type and itemLength */
                if ((array._primitiveType > DataTypes.ENUM) || (array._itemLength > 9))
                    return CodecReturnCodes.INVALID_ARGUMENT;

                if (PrimitiveEncoder._arrayEncodeActions[array._primitiveType][array._itemLength] == null)
                    return CodecReturnCodes.INVALID_ARGUMENT;

                if (array._itemLength == 0)
                    _levelInfo._encodingState = EncodeIteratorStates.PRIMITIVE;
                else
                    _levelInfo._encodingState = EncodeIteratorStates.SET_DATA;

                ret = PrimitiveEncoder.encodeArrayData(iter, data, array._primitiveType, array._itemLength);
                break;
        }

        if (ret >= CodecReturnCodes.SUCCESS)
            _levelInfo._currentCount++;
        else
        {
            /* failure - roll back */
            iter._curBufPos = _levelInfo._initElemStartPos;
            _levelInfo._initElemStartPos = 0;
            iter._writer.position(iter._curBufPos);
        }

        _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

        return ret;
    }

    static int encodePreencodedArrayEntry(EncodeIterator iterInt, Buffer encodedData)
    {
        int ret = CodecReturnCodes.SUCCESS;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        ArrayImpl array;

        /* Validations */
        assert (iterInt != null);
        assert (encodedData != null);
        assert (_levelInfo._containerType == DataTypes.ARRAY);
        assert (_levelInfo._encodingState == EncodeIteratorStates.ENTRIES);

        array = (ArrayImpl)_levelInfo._listType;

        // set start position for this entry - needed for rollback
        _levelInfo._initElemStartPos = iter._curBufPos;

        if (encodedData.length() > 0)
        {
            if (array._itemLength > 0)
            {
                /* fixed length items */
                /* check that the length given to us is what we expect */
                assert array._itemLength == encodedData.length() : "Invalid encoded data length";

                if (iter.isIteratorOverrun(array._itemLength))
                    return CodecReturnCodes.BUFFER_TOO_SMALL;

                iter._writer.write((BufferImpl)encodedData);
                iter._curBufPos = iter._writer.position();
            }
            else
            {
                int len = encodedData.length();

                // len value is written on wire as uShort16ob
                // If the value is smaller than 0xFE, it is written on the wire as one byte,
                // otherwise, it is written as three bytes by calling writeUShort16obLong method.
                // The code below checks if the buffer is sufficient for each case.
                if (len < 0xFE)
                {
                    if (iter.isIteratorOverrun(1 + len))
                        return CodecReturnCodes.BUFFER_TOO_SMALL;

                    iter._writer.writeByte(len);
                    iter._writer.write((BufferImpl)encodedData);
                }
                else if (len <= 0xFFFF)
                {
                    if (iter.isIteratorOverrun(3 + len))
                        return CodecReturnCodes.BUFFER_TOO_SMALL;

                    iter._writer.writeUShort16obLong(len);
                    iter._writer.write((BufferImpl)encodedData);

                }
                else
                    return CodecReturnCodes.INVALID_DATA;

                iter._curBufPos = iter._writer.position();
            }
        }
        else
        {
            /* Blank */
            if (array._itemLength > 0)
            {
                if (array._primitiveType <= DataTypes.RMTES_STRING && array._itemLength <= 9)
                {
                    _levelInfo._encodingState = EncodeIteratorStates.SET_DATA;
                    if ((ret = encodeBlank(iter, array._primitiveType)) != CodecReturnCodes.SUCCESS)
                        return ret;
                    iter._curBufPos = iter._writer.position();
                }
                else
                    return CodecReturnCodes.INVALID_ARGUMENT;
            }
            else
            {
                _levelInfo._encodingState = EncodeIteratorStates.PRIMITIVE;
                if ((ret = encodeBlank(iter, array._primitiveType)) != CodecReturnCodes.SUCCESS)
                    return ret;
                iter._curBufPos = iter._writer.position();
            }
        }

        _levelInfo._currentCount++;
        _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;
        return CodecReturnCodes.SUCCESS;
    }
	
    static int encodeArrayComplete(EncodeIterator iterInt, boolean success)
    {
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        /* Validations */
        assert null != iter && null != _levelInfo._listType : "Invalid parameters or parameters passed in as NULL";
        assert _levelInfo._containerType == DataTypes.ARRAY : " Invalid encoding attempted - wrong type";
        assert (_levelInfo._encodingState == EncodeIteratorStates.ENTRIES) ||
               (_levelInfo._encodingState == EncodeIteratorStates.WAIT_COMPLETE) : "Unexpected encoding attempted";
        assert null != iter._buffer : "Invalid iterator use - check buffer";
        assert iter._curBufPos <= iter._endBufPos : "Data exceeds iterators buffer length";

        if (success)
        {
            assert _levelInfo._countWritePos != 0 : "Invalid encoding attempted";
            iter._writer.position(_levelInfo._countWritePos);
            iter._writer.writeShort(_levelInfo._currentCount);
            iter._writer.position(iter._curBufPos);
        }
        else
        {
            iter._curBufPos = _levelInfo._containerStartPos;
            iter._writer.position(iter._curBufPos);
        }
        --iter._encodingLevel;

        return CodecReturnCodes.SUCCESS;
    }

    private static int encodeBlank(EncodeIteratorImpl iter, int type)
    {
        switch (type)
        {
            case DataTypes.REAL_4RB:
            {
                Real blankReal = CodecFactory.createReal();
                blankReal.blank();
                return PrimitiveEncoder.encodeReal4(iter, blankReal);
            }
            case DataTypes.REAL_8RB:
            {
                Real blankReal = CodecFactory.createReal();
                blankReal.blank();
                return PrimitiveEncoder.encodeReal8(iter, blankReal);
            }
            case DataTypes.DATE_4:
            {
                Date blankDate = CodecFactory.createDate();
                blankDate.blank();
                return PrimitiveEncoder.encodeDate4(iter, blankDate);
            }
            case DataTypes.TIME_3:
            {
                Time blankTime = CodecFactory.createTime();
                blankTime.blank();
                return PrimitiveEncoder.encodeTime3(iter, blankTime);
            }
            case DataTypes.TIME_5:
            {
                Time blankTime = CodecFactory.createTime();
                blankTime.blank();
                return PrimitiveEncoder.encodeTime5(iter, blankTime);
            }
            case DataTypes.TIME_7:
            {
                Time blankTime = CodecFactory.createTime();
                blankTime.blank();
                return PrimitiveEncoder.encodeTime7(iter, blankTime);
            }
            case DataTypes.TIME_8:
            {
                Time blankTime = CodecFactory.createTime();
                blankTime.blank();
                return PrimitiveEncoder.encodeTime8(iter, blankTime);
            }
            case DataTypes.DATETIME_7:
            {
                DateTime blankDateTime = new DateTimeImpl();
                blankDateTime.blank();
                return PrimitiveEncoder.encodeDateTime7(iter, blankDateTime);
            }
            case DataTypes.DATETIME_9:
            {
                DateTime blankDateTime = new DateTimeImpl();
                blankDateTime.blank();
                return PrimitiveEncoder.encodeDateTime9(iter, blankDateTime);
            }
            case DataTypes.DATETIME_11:
            {
                DateTime blankDateTime = new DateTimeImpl();
                blankDateTime.blank();
                return PrimitiveEncoder.encodeDateTime11(iter, blankDateTime);
            }
            case DataTypes.DATETIME_12:
            {
                DateTime blankDateTime = new DateTimeImpl();
                blankDateTime.blank();
                return PrimitiveEncoder.encodeDateTime12(iter, blankDateTime);
            }
            default:
            {
                int zero = 0;

                /* Except for the above, only length-spec primitives can be blank. */
                if (type > DataTypes.BASE_PRIMITIVE_MAX)
                    return CodecReturnCodes.INVALID_ARGUMENT;

                if (iter.isIteratorOverrun(1))
                    return CodecReturnCodes.BUFFER_TOO_SMALL;

                iter._writer.writeByte(zero);
                iter._curBufPos = iter._writer.position();
                return CodecReturnCodes.SUCCESS;
            }
        }
    }
	
    static int encodeNonRWFInit(EncodeIterator iterInt, Buffer buffer)
    {
        EncodingLevel levelInfo;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;

        assert null != iter : "Invalid parameters or parameters passed in as NULL";
        assert null != buffer : "Invalid parameters or parameters passed in as NULL";

        if (++iter._encodingLevel >= EncodeIteratorImpl.ENC_ITER_MAX_LEVELS)
            return CodecReturnCodes.ITERATOR_OVERRUN;
        levelInfo = iter._levelInfo[iter._encodingLevel];
        levelInfo.init(DataTypes.OPAQUE, EncodeIteratorStates.NON_RWF_DATA, null, iter._curBufPos);

        ((BufferImpl)buffer).data_internal(iter._writer.buffer(), iter._curBufPos, (iter._endBufPos - iter._curBufPos));

        return CodecReturnCodes.SUCCESS;
    }
	
    static int encodeNonRWFComplete(EncodeIterator iterInt, Buffer buffer, boolean success)
    {
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        /* Validations */
        assert null != buffer : "Invalid buffer";
        assert null != iter : "Invalid parameters or parameters passed in as NULL";
        assert _levelInfo._containerType == DataTypes.OPAQUE : "Invalid container type";
        assert (_levelInfo._encodingState == EncodeIteratorStates.NON_RWF_DATA) : "Unexpected encoding attempted";
        assert null != iter._buffer : "Invalid iterator use - check buffer";
        assert iter._curBufPos <= iter._endBufPos : "Data exceeds iterators buffer length";

        if (success)
        {
            /* verify no overrun */
            if (iter._buffer != buffer.data())
                return CodecReturnCodes.INVALID_DATA;

            if (iter.isIteratorOverrun(buffer.length()))
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            }

            iter._curBufPos += buffer.data().position() - ((BufferImpl)buffer).position();
            iter._writer.position(iter._curBufPos);
        }
        else
        {
            iter._curBufPos = _levelInfo._containerStartPos;
            iter._writer.position(iter._curBufPos);
        }

        --iter._encodingLevel;

        return CodecReturnCodes.SUCCESS;
    }

    private static void putLenSpecBlank(EncodeIteratorImpl iter)
    {
        iter._writer.writeByte(0);
        iter._curBufPos = iter._writer.position();
    }
	
    static int encodeFilterListInit(EncodeIterator iterInt, FilterList filterListInt)
    {
        assert null != iterInt : "Invalid IterInt in as NULL";
        assert null != filterListInt : "Invalid filterListInt in as NULL";

        EncodingLevel _levelInfo;
        int flags;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        FilterListImpl filterList = (FilterListImpl)filterListInt;

        if (!(validAggregateDataType(filterList._containerType)))
            return CodecReturnCodes.UNSUPPORTED_DATA_TYPE;

        if (++iter._encodingLevel >= EncodeIteratorImpl.ENC_ITER_MAX_LEVELS)
            return CodecReturnCodes.ITERATOR_OVERRUN;

        _levelInfo = iter._levelInfo[iter._encodingLevel];
        _levelInfo.init(DataTypes.FILTER_LIST, EncodeIteratorStates.NONE, filterList, iter._curBufPos);
        _levelInfo._flags = EncodeIteratorFlags.NONE;

        /* Make sure that required elements can be encoded */
        /* Flags (1), Opt Data Format (1), Total Count Hint (1), Count (1) */
        if (iter.isIteratorOverrun(4))
        {
            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        flags = (filterList._flags & ~FilterListFlags.HAS_PER_ENTRY_PERM_DATA);
        iter._writer.writeUByte(flags);
        /* container type needs to be scaled before we can send it */
        iter._writer.writeUByte(filterList._containerType - DataTypes.CONTAINER_TYPE_MIN);

        if (filterList.checkHasTotalCountHint())
        {
            iter._writer.writeUByte(filterList._totalCountHint);
        }

        iter._curBufPos = iter._writer.position();

        _levelInfo._countWritePos = iter._curBufPos;
        iter._curBufPos += 1;
        iter._writer.skipBytes(1);

        _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

        return CodecReturnCodes.SUCCESS;
    }
    
    static int encodeFilterListComplete(EncodeIterator iterInt, boolean success, FilterList filterListInt)
    {
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        FilterListImpl filterList = (FilterListImpl)filterListInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];
        int count = _levelInfo._currentCount;
        int flags;

        /* Validations */
        assert null != iter && null != _levelInfo._listType : "Invalid parameters or parameters passed in as NULL";
        assert _levelInfo._encodingState == EncodeIteratorStates.ENTRIES ||
                (_levelInfo._encodingState == EncodeIteratorStates.WAIT_COMPLETE) : "Unexpected encoding attempted";
        assert null != iter._buffer : "Invalid iterator use - check buffer";
        assert iter._curBufPos <= iter._endBufPos : "Data exceeds iterators buffer length";

        if (success)
        {
            assert _levelInfo._encodingState == EncodeIteratorStates.ENTRIES : " Unexpected encoding attempted";
            assert _levelInfo._countWritePos != 0 : "Invalid encoding attempted";
            iter._writer.position(_levelInfo._countWritePos);
            iter._writer.writeUByte(count);
            iter._writer.position(iter._curBufPos);

            if ((_levelInfo._flags & EncodeIteratorFlags.HAS_PER_ENTRY_PERM) > 0)
            {
                /* write per_entry_perm bit */
                /* flags are first byte of container */
                flags = (filterList._flags | FilterListFlags.HAS_PER_ENTRY_PERM_DATA);
                iter._writer.position(_levelInfo._containerStartPos);
                iter._writer.writeUByte(flags);
                iter._writer.position(iter._curBufPos);
            }
        }
        else
        {
            iter._curBufPos = _levelInfo._containerStartPos;
            iter._writer.position(iter._curBufPos);
        }
        // levelInfo._encodingState = EncodeIteratorStates.COMPLETE;
        --iter._encodingLevel;

        return CodecReturnCodes.SUCCESS;
    }

    static int encodeFilterEntryInit(EncodeIterator iterInt, FilterEntryImpl entry, int size)
    {
        int ret = CodecReturnCodes.SUCCESS;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        /* Validations */
        assert null != iter && null != _levelInfo._listType : " Invalid parameters or parameters passed in as NULL";
        assert _levelInfo._containerType == DataTypes.FILTER_LIST : "Invalid encoding attempted";
        assert _levelInfo._encodingState == EncodeIteratorStates.ENTRIES : "Unexpected encoding attempted";
        assert null != iter._buffer : "Invalid iterator use - check buffer";
        assert iter._curBufPos <= iter._endBufPos : "Data exceeds iterators buffer length";

        FilterListImpl filterList = (FilterListImpl)_levelInfo._listType;
        _levelInfo._initElemStartPos = iter._curBufPos;

        if ((ret = encodeFilterEntryInternal(iter, filterList, entry)) < 0)
        {
            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
            return ret;
        }

        _levelInfo._encodingState = EncodeIteratorStates.ENTRY_INIT;

        if (((filterList._containerType != DataTypes.NO_DATA)
                && !((entry._flags & FilterEntryFlags.HAS_CONTAINER_TYPE) > 0) || (entry._containerType != DataTypes.NO_DATA)
                && ((entry._flags & FilterEntryFlags.HAS_CONTAINER_TYPE) > 0))
                && (entry._action != FilterEntryActions.CLEAR))
        {
            if (iter.isIteratorOverrun(((size == 0 || size >= 0xFE) ? 3 : 1)))
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            }

            iter._curBufPos = setupU16Mark(_levelInfo._internalMark, size, iter._curBufPos);
            iter._writer.position(iter._curBufPos);
        }
        else
        {
            _levelInfo._internalMark._sizeBytes = 0;
            _levelInfo._internalMark._sizePos = iter._curBufPos;
        }

        return CodecReturnCodes.SUCCESS;
    }

    static int encodeFilterEntryComplete(EncodeIterator iterInt, boolean success)
    {
        assert null != iterInt : "Invalid IterInt in as NULL";
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        int ret;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        /* Validations */
        assert null != _levelInfo._listType : "Invalid _levelInfo._listType in as NULL";
        assert (_levelInfo._encodingState == EncodeIteratorStates.ENTRY_INIT) ||
        (_levelInfo._encodingState == EncodeIteratorStates.WAIT_COMPLETE)
                || _runningInJunits : "Unexpected encoding attempted";
        assert null != iter._buffer : "Invalid iterator use - check buffer";
        assert iter._curBufPos <= iter._endBufPos || _runningInJunits : "Data exceeds iterators buffer length";

        if (success)
        {
            if (_levelInfo._internalMark._sizeBytes > 0)
            {
                if ((ret = finishU16Mark(iter, _levelInfo._internalMark, iter._curBufPos)) < 0)
                {
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    _levelInfo._initElemStartPos = 0;
                    return ret;
                }
            }
            else
            {
                if (_levelInfo._internalMark._sizePos != iter._curBufPos)
                {
                    _levelInfo._encodingState = EncodeIteratorStates.ENTRY_WAIT_COMPLETE;
                    _levelInfo._initElemStartPos = 0;
                    return CodecReturnCodes.INVALID_DATA;
                }
                _levelInfo._internalMark._sizePos = 0;
            }
            _levelInfo._currentCount++;
        }
        else
        {
            iter._curBufPos = _levelInfo._initElemStartPos;
            iter._writer.position(iter._curBufPos);
        }

        _levelInfo._initElemStartPos = 0;
        _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

        return CodecReturnCodes.SUCCESS;
    }
	
    static int encodeFilterEntry(EncodeIterator iterInt, FilterEntry filterEntryInt)
    {
        assert null != iterInt : "Invalid IterInt in as NULL";
        assert null != filterEntryInt : "Invalid filterEntryInt in as NULL";

        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        FilterEntryImpl filterEntry = (FilterEntryImpl)filterEntryInt;
        int ret;
        FilterListImpl filterList;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        assert _levelInfo._encodingState == EncodeIteratorStates.ENTRIES : "Unexpected encoding attempted";

        filterList = (FilterListImpl)_levelInfo._listType;

        _levelInfo._initElemStartPos = iter._curBufPos;

        if ((ret = encodeFilterEntryInternal(iter, filterList, filterEntry)) < 0)
        {
            /* rollback */
            iter._curBufPos = _levelInfo._initElemStartPos;
            iter._writer.position(iter._curBufPos);
            return ret;
        }

        if (((filterList._containerType != DataTypes.NO_DATA)
                && !((filterEntry._flags & FilterEntryFlags.HAS_CONTAINER_TYPE) > 0) || (filterEntry._containerType != DataTypes.NO_DATA)
                && ((filterEntry._flags & FilterEntryFlags.HAS_CONTAINER_TYPE) > 0))
                && (filterEntry._action != FilterEntryActions.CLEAR))
        {

            int len = filterEntry._encodedData.length();

            // len value is written on wire as uShort16ob
            // If the value is smaller than 0xFE, it is written on the wire as one byte,
            // otherwise, it is written as three bytes by calling writeUShort16obLong method.
            // The code below checks if the buffer is sufficient for each case.
            if (len < 0xFE)
            {
                if (iter.isIteratorOverrun(1 + len))
                {
                    /* rollback */
                    iter._curBufPos = _levelInfo._initElemStartPos;
                    iter._writer.position(iter._curBufPos);
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                }

                iter._writer.writeByte(len);
                iter._writer.write((BufferImpl)filterEntry._encodedData);
            }
            else if (len <= 0xFFFF)
            {
                if (iter.isIteratorOverrun(3 + len))
                {
                    /* rollback */
                    iter._curBufPos = _levelInfo._initElemStartPos;
                    iter._writer.position(iter._curBufPos);
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                }

                iter._writer.writeUShort16obLong(len);
                iter._writer.write((BufferImpl)filterEntry._encodedData);

            }
            else
            {
                /* rollback */
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.position(iter._curBufPos);
                return CodecReturnCodes.INVALID_DATA;
            }

            iter._curBufPos = iter._writer.position();
        }

        _levelInfo._currentCount++;

        return CodecReturnCodes.SUCCESS;
    }
    
    private static int encodeFilterEntryInternal(EncodeIteratorImpl iter, FilterListImpl filterList, FilterEntryImpl filterEntry)
    {
        int flags = 0;

        assert (iter._levelInfo[iter._encodingLevel]._currentCount + 1) != 0 : "Invalid encoding attempted";

        if (iter.isIteratorOverrun(2))
            return CodecReturnCodes.BUFFER_TOO_SMALL;

        flags = filterEntry._flags;
        flags <<= 4;
        flags += filterEntry._action;

        /* store flags */
        iter._writer.writeUByte(flags);

        /* Store id as UInt8 */
        iter._writer.writeUByte(filterEntry._id);
        iter._curBufPos = iter._writer.position();

        /* Store _containerType as UInt8 */
        if (filterEntry.checkHasContainerType())
        {
            if (iter.isIteratorOverrun(1))
                return CodecReturnCodes.BUFFER_TOO_SMALL;

            if (!(validAggregateDataType(filterEntry._containerType)))
                return CodecReturnCodes.UNSUPPORTED_DATA_TYPE;

            /* container type needs to be scaled before its encoded */
            iter._writer.writeUByte(filterEntry.containerType() - DataTypes.CONTAINER_TYPE_MIN);
            iter._curBufPos = iter._writer.position();
        }



        /* Store perm lock */
        if (filterEntry.checkHasPermData())
        {
            iter._levelInfo[iter._encodingLevel]._flags |= EncodeIteratorFlags.HAS_PER_ENTRY_PERM;
            /* Encode the permission expression */
            if (filterEntry._permData.length() == 0)
            {
                /* just encode 0 bytes since none exists */
                int zero = 0;
                iter._writer.writeUByte(zero);
                iter._curBufPos = iter._writer.position();
            }
            else
            {
                int len = filterEntry._permData.length();

                // len value is written on wire as uShort15rb
                // If the value is smaller than 0x80, it is written on the wire as one byte,
                // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
                // The code below checks if the buffer is sufficient for each case.
                if (len < 0x80)
                {
                    if (iter.isIteratorOverrun(1 + len))
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    iter._writer.writeByte(len);
                    iter._writer.write((BufferImpl)filterEntry.permData());
                }
                else
                {
                    if (iter.isIteratorOverrun(2 + len))
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    iter._writer.writeUShort15rbLong((short)len);
                    iter._writer.write((BufferImpl)filterEntry.permData());
                }

                iter._curBufPos = iter._writer.position();
            }
        }

        return CodecReturnCodes.SUCCESS;
    }

    static int encodeMapInit(EncodeIterator iterInt, Map mapInt, int summaryMaxSize, int setMaxSize)
    {
        assert null != iterInt : "Invalid IterInt in as NULL";
        assert null != mapInt : "Invalid mapInt in as NULL";

        EncodingLevel _levelInfo;
        int flags;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        MapImpl map = (MapImpl)mapInt;
        int ret;

        if (!(validPrimitiveDataType(map._keyPrimitiveType)))
            return CodecReturnCodes.UNSUPPORTED_DATA_TYPE;

        if (!(validAggregateDataType(map._containerType)))
            return CodecReturnCodes.UNSUPPORTED_DATA_TYPE;

        if (++iter._encodingLevel >= EncodeIteratorImpl.ENC_ITER_MAX_LEVELS)
            return CodecReturnCodes.ITERATOR_OVERRUN;
        _levelInfo = iter._levelInfo[iter._encodingLevel];
        _levelInfo.init(DataTypes.MAP, EncodeIteratorStates.NONE, map, iter._curBufPos);
        _levelInfo._flags = EncodeIteratorFlags.NONE;

        /* Make sure that required elements can be encoded */
        /* Flags (1), keyPrimitiveType (1), Data Format (1), Count (2) */
        if (iter.isIteratorOverrun(5))
        {
            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        /* If summary data and/or set definitions are to be encoded (but not pre-encoded),
         * reserve space for the count & totalCountHint that will be encoded afterwards. */
        int reservedBytes = 2 /* count */ + (map.checkHasTotalCountHint() ? 4 : 0);

        flags = (map._flags & ~MapFlags.HAS_PER_ENTRY_PERM_DATA);

        iter._writer.writeUByte(flags);
        iter._writer.writeUByte(map._keyPrimitiveType);
        /* container type needs to be scaled before its encoded */
        iter._writer.writeUByte(map._containerType - DataTypes.CONTAINER_TYPE_MIN);
        iter._curBufPos = iter._writer.position();

        if (map.checkHasKeyFieldId())
        {
            if (iter.isIteratorOverrun(2))
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            }

            iter._writer.writeShort(map._keyFieldId);
            iter._curBufPos = iter._writer.position();
        }

        /* Check for List Set Definitions */
        if ((ret = encodeMapSetDefsInit(iter, map, summaryMaxSize, setMaxSize, _levelInfo)) <= CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        /* Check for Summary Data */
        if ((ret = encodeMapSummaryDataInit(iter, map, summaryMaxSize, _levelInfo)) <= CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        if (iter.isIteratorOverrun(reservedBytes))
        {
            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        ret = finishMapInit(map, iter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
            return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }

    private static int encodeMapSetDefsInit(EncodeIteratorImpl iter, MapImpl map, int summaryMaxSize, int setMaxSize, EncodingLevel levelInfo)
    {
        if (map.checkHasSetDefs())
        {
            /* We have list set definitions */
            if (map._encodedSetDefs.length() > 0)
            {
                /* The set data is already encoded. */
                /* Make sure the data can be encoded */
                int len = map._encodedSetDefs.length();

                // len value is written on wire as uShort15rb
                // If the value is smaller than 0x80, it is written on the wire as one byte,
                // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
                // The code below checks if the buffer is sufficient for each case.
                if (len < 0x80)
                {
                    if (iter.isIteratorOverrun(1 + len))
                    {
                        levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }
                    iter._writer.writeByte(len);
                    iter._writer.write((BufferImpl)map._encodedSetDefs);
                }
                else
                {
                    if (iter.isIteratorOverrun(2 + len))
                    {
                        levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }
                    iter._writer.writeUShort15rbLong((short)len);
                    iter._writer.write((BufferImpl)map._encodedSetDefs);
                }

                iter._curBufPos = iter._writer.position();
            }
            else
            {
                int reservedBytes = 2 /* count */ + (map.checkHasTotalCountHint() ? 4 : 0);

                if (map.checkHasSummaryData())
                {
                    if (map.encodedSummaryData().data() != null)
                    {
                        /* Reserve space to encode the summaryData and its length */
                        int summaryLength = map.encodedSummaryData().length();
                        reservedBytes += summaryLength + ((summaryLength >= 0x80) ? 2 : 1);
                    }
                    else
                    {
                        /* store # of bytes for summary data so user does not pass it in again */
                        if (summaryMaxSize == 0 || summaryMaxSize >= 0x80)
                            levelInfo._internalMark2._sizeBytes = 2;
                        else
                            levelInfo._internalMark2._sizeBytes = 1;

                        /* Reserve space for the summaryData size mark. */
                        reservedBytes += levelInfo._internalMark2._sizeBytes;
                    }
                }

                if (iter.isIteratorOverrun(reservedBytes + ((setMaxSize >= 0x80 || setMaxSize == 0) ? 2 : 1)))
                {
                    levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                }

                iter._curBufPos = setupU15Mark(levelInfo._internalMark, setMaxSize, iter._curBufPos);
                iter._writer.position(iter._curBufPos);

                /* Back up endBufPos to account for reserved bytes. */
                levelInfo._reservedBytes = reservedBytes;
                iter._endBufPos -= reservedBytes;
                iter._writer.reserveBytes(reservedBytes);

                /* Save state and return */
                levelInfo._encodingState = EncodeIteratorStates.SET_DEFINITIONS;
                return CodecReturnCodes.SUCCESS;
            }
        }

        return 1;
    }

    private static int encodeMapSummaryDataInit(EncodeIteratorImpl iter, MapImpl map, int summaryMaxSize, EncodingLevel levelInfo)
    {
        if (map.checkHasSummaryData())
        {
            /* We have summary data */
            if (map._encodedSummaryData.length() > 0)
            {
                /* The summary data is already encoded. */
                /* Make sure the data can be encoded */
                int len = map._encodedSummaryData.length();

                // len value is written on wire as uShort15rb
                // If the value is smaller than 0x80, it is written on the wire as one byte,
                // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
                // The code below checks if the buffer is sufficient for each case.
                if (len < 0x80)
                {
                    if (iter.isIteratorOverrun(1 + len))
                    {
                        levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }
                    iter._writer.writeByte(len);
                    iter._writer.write((BufferImpl)map._encodedSummaryData);
                }
                else
                {
                    if (iter.isIteratorOverrun(2 + len))
                    {
                        levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }
                    iter._writer.writeUShort15rbLong((short)len);
                    iter._writer.write((BufferImpl)map._encodedSummaryData);
                }

                iter._curBufPos = iter._writer.position();
            }
            else
            {
                int reservedBytes = 2 /* count */ + (map.checkHasTotalCountHint() ? 4 : 0);

                if (iter.isIteratorOverrun(reservedBytes + ((summaryMaxSize >= 0x80 || summaryMaxSize == 0) ? 2 : 1)))
                {
                    levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                }

                iter._curBufPos = setupU15Mark(levelInfo._internalMark2, summaryMaxSize, iter._curBufPos);
                iter._writer.position(iter._curBufPos);

                /* Back up endBufPos to account for the reserved bytes. */
                levelInfo._reservedBytes = reservedBytes;
                iter._endBufPos -= reservedBytes;
                iter._writer.reserveBytes(reservedBytes);

                /* Save state and return */
                levelInfo._encodingState = EncodeIteratorStates.SUMMARY_DATA;
                return CodecReturnCodes.SUCCESS;
            }
        }

        return 1;
    }

    private static int finishMapInit(MapImpl map, EncodeIteratorImpl iter)
    {
        int ret;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        /* Move endBufPos back to original position if bytes were reserved. */
        iter._endBufPos += _levelInfo._reservedBytes;
        iter._writer.unreserveBytes(_levelInfo._reservedBytes);
        _levelInfo._reservedBytes = 0;

        /* Store Total count hint */
        if (map.checkHasTotalCountHint())
        {
            if ((ret = iter._writer.writeUInt30rb(map._totalCountHint)) != CodecReturnCodes.SUCCESS)
                return ret;
            iter._curBufPos = iter._writer.position();
        }

        _levelInfo._countWritePos = iter._curBufPos;
        iter._curBufPos += 2;
        iter._writer.skipBytes(2);
        _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

        return CodecReturnCodes.SUCCESS;
    }
	
    static int encodeMapComplete(EncodeIterator iterInt, Map mapInt, boolean success)
    {
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        MapImpl map = (MapImpl)mapInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];
        int flags;

        /* Validations */
        assert null != iter && null != _levelInfo._listType : "Invalid parameters or parameters passed in as NULL";
        assert _levelInfo._containerType == DataTypes.MAP : "Invalid encoding attempted - wrong container";
        assert (_levelInfo._encodingState == EncodeIteratorStates.ENTRIES)
                || (_levelInfo._encodingState == EncodeIteratorStates.SET_DEFINITIONS)
                || (_levelInfo._encodingState == EncodeIteratorStates.SUMMARY_DATA)
                || (_levelInfo._encodingState == EncodeIteratorStates.WAIT_COMPLETE) : "Unexpected encoding attempted";
        assert null != iter._buffer : "Invalid iterator use - check buffer";
        assert iter._curBufPos <= iter._endBufPos : "Data exceeds iterators buffer length";

        if (success)
        {
            assert _levelInfo._encodingState == EncodeIteratorStates.ENTRIES : "Unexpected encoding attempted";
            assert _levelInfo._countWritePos != 0 : "Invalid encoding attempted";
            iter._writer.position(_levelInfo._countWritePos);
            iter._writer.writeShort(_levelInfo._currentCount);
            iter._writer.position(iter._curBufPos);

            if ((_levelInfo._flags & EncodeIteratorFlags.HAS_PER_ENTRY_PERM) > 0)
            {
                /* write per_entry_perm bit */
                /* flags are first byte of container */
                flags = (map._flags | MapFlags.HAS_PER_ENTRY_PERM_DATA);
                iter._writer.position(_levelInfo._containerStartPos);
                iter._writer.writeUByte(flags);
                iter._writer.position(iter._curBufPos);
            }
        }
        else
        {
            iter._curBufPos = _levelInfo._containerStartPos;
            iter._writer.position(iter._curBufPos);

            /* Move endBufPos back to original position if bytes were reserved. */
            iter._endBufPos += _levelInfo._reservedBytes;
            iter._writer.unreserveBytes(_levelInfo._reservedBytes);
            _levelInfo._reservedBytes = 0;
        }
        --iter._encodingLevel;

        return CodecReturnCodes.SUCCESS;
    }
	
    static int encodeMapSummaryDataComplete(EncodeIterator iterInt, Map mapInt, boolean success)
    {
        int ret;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        /* Validations */
        assert null != iter : "Invalid parameters or parameters passed in as NULL";
        assert _levelInfo._containerType == DataTypes.MAP : "Invalid encoding attempted - wrong container";
        assert (_levelInfo._encodingState == EncodeIteratorStates.SUMMARY_DATA) ||
               (_levelInfo._encodingState == EncodeIteratorStates.WAIT_COMPLETE) : "Unexpected encoding attempted";
        assert iter._curBufPos != 0 : "Invalid iterator use - check buffer";

        if (success)
        {
            if ((ret = finishU15Mark(iter, _levelInfo._internalMark2, iter._curBufPos)) < 0)
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return ret;
            }

            if ((ret = finishMapInit((MapImpl)_levelInfo._listType, iter)) != CodecReturnCodes.SUCCESS)
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return ret;
            }
        }
        else
        {
            assert _levelInfo._internalMark2._sizePos != 0 : "Invalid encoding attempted";
            iter._curBufPos = _levelInfo._internalMark2._sizePos + _levelInfo._internalMark2._sizeBytes;
            iter._writer.position(iter._curBufPos);
            _levelInfo._encodingState = EncodeIteratorStates.SUMMARY_DATA;
        }

        return CodecReturnCodes.SUCCESS;
    }
	
    static int encodeMapEntryInit(EncodeIterator iterInt, MapEntry mapEntryInt, Object keyData, int maxEncodingSize)
    {
        int ret;
        MapImpl map;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        MapEntryImpl mapEntry = (MapEntryImpl)mapEntryInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        /* Validations */
        assert null != iter && null != mapEntry : "Invalid parameters or parameters passed in as NULL";
        assert _levelInfo._containerType == DataTypes.MAP : "Invalid encoding attempted - wrong type";
        assert _levelInfo._encodingState == EncodeIteratorStates.ENTRIES : "Unexpected encoding attempted";
        assert iter._curBufPos != 0 : "Invalid iterator use - check buffer";
        assert (0 != mapEntry._encodedKey.length() && null != mapEntry._encodedKey.data()) || null != keyData : "Entry key missing";

        map = (MapImpl)_levelInfo._listType;

        assert map._keyPrimitiveType != DataTypes.NO_DATA : "Invalid key type specified";

        _levelInfo._initElemStartPos = iter._curBufPos;

        if ((ret = encodeMapEntryInternal(iter, map, mapEntry, keyData)) < 0)
        {
            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
            return ret;
        }

        _levelInfo._encodingState = EncodeIteratorStates.ENTRY_INIT;

        if ((mapEntry._action != MapEntryActions.DELETE) && (map._containerType != DataTypes.NO_DATA))
        {
            if (iter.isIteratorOverrun(((maxEncodingSize == 0 || maxEncodingSize >= 0xFE) ? 3 : 1)))
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            }

            iter._curBufPos = setupU16Mark(_levelInfo._internalMark, maxEncodingSize, iter._curBufPos);
            iter._writer.position(iter._curBufPos);
        }
        else
        {
            /* set up mark so we know not to close it */
            _levelInfo._internalMark._sizeBytes = 0;
            _levelInfo._internalMark._sizePos = iter._curBufPos;
        }

        return CodecReturnCodes.SUCCESS;
    }

    private static int encodeMapEntryInternal(EncodeIteratorImpl iter, MapImpl map, MapEntryImpl mapEntry, Object key)
    {
        int flags = 0;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        assert (_levelInfo._currentCount + 1) != 0 : "Invalid encoding attempted";

        /* Flags byte made up of flags and action */
        flags = mapEntry._flags;
        flags <<= 4;
        flags += mapEntry._action;

        if (iter.isIteratorOverrun(1))
            return CodecReturnCodes.BUFFER_TOO_SMALL;

        /* Encode the flags */
        iter._writer.writeUByte(flags);
        iter._curBufPos = iter._writer.position();

        /* Check for optional permissions expression per entry */
        if (mapEntry.checkHasPermData())
        {
            /* indicate that we want to set per-entry perm since the user encoded perm data */
            _levelInfo._flags |= EncodeIteratorFlags.HAS_PER_ENTRY_PERM;

            /* Encode the permission expression */
            if (mapEntry._permData.length() == 0)
            {
                if (iter.isIteratorOverrun(1))
                    return CodecReturnCodes.BUFFER_TOO_SMALL;

                /* Just encode 0 bytes since none exists */
                int zero = 0;
                iter._writer.writeUByte(zero);
                iter._curBufPos = iter._writer.position();
            }
            else
            {
                int len = mapEntry.permData().length();

                // len value is written on wire as uShort15rb
                // If the value is smaller than 0x80, it is written on the wire as one byte,
                // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
                // The code below checks if the buffer is sufficient for each case.
                if (len < 0x80)
                {
                    if (iter.isIteratorOverrun(1 + len))
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    iter._writer.writeByte(len);
                    iter._writer.write((BufferImpl)mapEntry._permData);
                }
                else
                {
                    if (iter.isIteratorOverrun(2 + len))
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    iter._writer.writeUShort15rbLong((short)len);
                    iter._writer.write((BufferImpl)mapEntry._permData);
                }

                iter._curBufPos = iter._writer.position();
            }
        }

        /* not pre-encoded - encode it */
        if (key != null)
        {
            int ret = 0;

            if ((map._keyPrimitiveType >= DataTypes.DATETIME_9) || (PrimitiveEncoder._setEncodeActions[map._keyPrimitiveType] == null))
                ret = CodecReturnCodes.UNSUPPORTED_DATA_TYPE;
            else
                ret = PrimitiveEncoder.encodeSetData(iter, key, map._keyPrimitiveType);

            if (ret < 0)
                return ret;
        }
        else if (mapEntry._encodedKey.length() > 0)
        {
            /* Check for pre-encoded key */
            /* We probably don't need to check data or length as the ASSERTS should prevent it,
             * however those are currently only on debug mode */
            assert 0 != mapEntry._encodedKey.length() : "Blank key not allowed";

            /* Key is pre-encoded. */
            /* For buffers, etc; size need to be encoded, hence encoding it as small buffer */
            int len = mapEntry._encodedKey.length();
            if (len < 0x80)
            {
                if (iter.isIteratorOverrun(1 + len))
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                iter._writer.writeByte(len);
                iter._writer.write((BufferImpl)mapEntry._encodedKey);
            }
            else
            {
                if (iter.isIteratorOverrun(2 + len))
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                iter._writer.writeUShort15rbLong((short)len);
                iter._writer.write((BufferImpl)mapEntry._encodedKey);
            }

            iter._curBufPos = iter._writer.position();
        }
        else
        {
            return CodecReturnCodes.FAILURE;
        }

        return CodecReturnCodes.SUCCESS;
    }

    static int encodeMapEntryComplete(EncodeIterator iterInt, MapEntry mapEntryInt, boolean success)
    {
        int ret;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        /* Validations */
        assert null != iter && null != _levelInfo._listType : "Invalid parameters or parameters passed in as NULL";
        assert _levelInfo._containerType == DataTypes.MAP : "Invalid encoding attempted - wrong type";
        assert (_levelInfo._encodingState == EncodeIteratorStates.ENTRY_INIT) ||
               (_levelInfo._encodingState == EncodeIteratorStates.WAIT_COMPLETE) || _runningInJunits : "Unexpected encoding attempted";
        assert null != iter._buffer : "Invalid iterator use - check buffer";
        assert (_levelInfo._currentCount + 1) != 0 : "Invalid encoding attempted";

        if (success)
        {
            if (_levelInfo._internalMark._sizeBytes > 0)
            {
                if ((ret = finishU16Mark(iter, _levelInfo._internalMark, iter._curBufPos)) < 0)
                {
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    _levelInfo._initElemStartPos = 0;
                    return ret;
                }
            }
            else
            {
                /* size bytes is 0 - this means that action was clear or df was no data */
                if (_levelInfo._internalMark._sizePos != iter._curBufPos)
                {
                    /* something was written when it shouldnt have been */
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    _levelInfo._initElemStartPos = 0;
                    return CodecReturnCodes.INVALID_DATA;
                }
                _levelInfo._internalMark._sizePos = 0;
            }
            _levelInfo._currentCount++;
        }
        else
        {
            /* Reset the pointer */
            iter._curBufPos = _levelInfo._initElemStartPos;
            iter._writer.position(iter._curBufPos);
        }

        _levelInfo._initElemStartPos = 0;
        _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

        return CodecReturnCodes.SUCCESS;
    }

    static int encodeMapEntry(EncodeIterator iterInt, MapEntry mapEntryInt, Object keyData)
    {
        assert null != iterInt : "Invalid IterInt in as NULL";
        assert null != mapEntryInt : "Invalid mapEntryInt in as NULL";

        int ret;
        MapImpl map;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        MapEntryImpl mapEntry = (MapEntryImpl)mapEntryInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        assert _levelInfo._containerType == DataTypes.MAP : "Invalid encoding attempted - wrong type";
        assert _levelInfo._encodingState == EncodeIteratorStates.ENTRIES : "Unexpected encoding attempted";
        assert (mapEntry._encodedKey.length() != 0 && null != mapEntry._encodedKey.data()) || null != keyData : "Entry key missing";

        map = (MapImpl)_levelInfo._listType;
        _levelInfo._initElemStartPos = iter._curBufPos;

        if ((ret = encodeMapEntryInternal(iter, map, mapEntry, keyData)) < 0)
        {
            /* rollback */
            iter._curBufPos = _levelInfo._initElemStartPos;
            iter._writer.position(iter._curBufPos);
            return ret;
        }

        if ((mapEntry._action != MapEntryActions.DELETE) && (map._containerType != DataTypes.NO_DATA))
        {
            int len = mapEntry._encodedData.length();
            if (len < 0xFE)
            {
                if (iter.isIteratorOverrun(1 + len))
                {
                    /* rollback */
                    iter._curBufPos = _levelInfo._initElemStartPos;
                    iter._writer.position(iter._curBufPos);
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                }

                iter._writer.writeByte(len);
                iter._writer.write((BufferImpl)mapEntry._encodedData);
            }
            else if (len <= 0xFFFF)
            {
                if (iter.isIteratorOverrun(3 + len))
                {
                    /* rollback */
                    iter._curBufPos = _levelInfo._initElemStartPos;
                    iter._writer.position(iter._curBufPos);
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                }

                iter._writer.writeUShort16obLong(len);
                iter._writer.write((BufferImpl)mapEntry._encodedData);

            }
            else
            {
                /* rollback */
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.position(iter._curBufPos);
                return CodecReturnCodes.INVALID_DATA;
            }

            iter._curBufPos = iter._writer.position();
        }

        _levelInfo._currentCount++;

        return CodecReturnCodes.SUCCESS;
    }
	
    static int encodeSeriesInit(EncodeIterator iterInt, Series seriesInt, int summaryMaxSize, int setMaxSize)
    {
        assert null != iterInt : "Invalid IterInt in as NULL";
        assert null != seriesInt : "Invalid seriesInt in as NULL";

        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        SeriesImpl series = (SeriesImpl)seriesInt;
        EncodingLevel _levelInfo;

        if (!(validAggregateDataType(series.containerType())))
            return CodecReturnCodes.UNSUPPORTED_DATA_TYPE;

        if (++iter._encodingLevel >= EncodeIteratorImpl.ENC_ITER_MAX_LEVELS)
            return CodecReturnCodes.ITERATOR_OVERRUN;
        _levelInfo = iter._levelInfo[iter._encodingLevel];
        _levelInfo.init(DataTypes.SERIES, EncodeIteratorStates.NONE, series, iter._curBufPos);

        /* Make sure required elements can be encoded */
        /* Flags (1), _containerType (1), Count (2) */
        if (iter.isIteratorOverrun(2))
        {
            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        iter._writer.writeUByte(series.flags());
        /* container type needs to be scaled before its encoded */
        iter._writer.writeUByte(series._containerType - DataTypes.CONTAINER_TYPE_MIN);
        iter._curBufPos = iter._writer.position();

        /* If summary data and/or set definitions are to be encoded (but not pre-encoded),
         * reserve space for the count & totalCountHint that will be encoded afterwards. */
        int reservedBytes = 2 /* count */ + (series.checkHasTotalCountHint() ? 4 : 0);

        /* check if we have list set definitions */
        if (series.checkHasSetDefs())
        {
            /* we have definitions */
            if (series.encodedSetDefs().data() != null)
            {
                /* the set data is already encoded */
                /* make sure it fits */
                int len = series.encodedSetDefs().length();

                // len value is written on wire as uShort15rb
                // If the value is smaller than 0x80, it is written on the wire as one byte,
                // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
                // The code below checks if the buffer is sufficient for each case.
                if (len < 0x80)
                {
                    if (iter.isIteratorOverrun(1 + len))
                    {
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }
                    iter._writer.writeByte(len);
                    iter._writer.write((BufferImpl)series.encodedSetDefs());
                }
                else
                {
                    if (iter.isIteratorOverrun(2 + len))
                    {
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }
                    iter._writer.writeUShort15rbLong((short)len);
                    iter._writer.write((BufferImpl)series.encodedSetDefs());
                }

                iter._curBufPos = iter._writer.position();
            }
            else
            {
                if (series.checkHasSummaryData())
                {
                    if (series.encodedSummaryData().data() != null)
                    {
                        /* Reserve space to encode the summaryData and its length */
                        int summaryLength = series.encodedSummaryData().length();
                        reservedBytes += summaryLength + ((summaryLength >= 0x80) ? 2 : 1);
                    }
                    else
                    {
                        /* store # of bytes for summary data so user does not pass it in again */
                        if (summaryMaxSize == 0 || summaryMaxSize >= 0x80)
                            _levelInfo._internalMark2._sizeBytes = 2;
                        else
                            _levelInfo._internalMark2._sizeBytes = 1;

                        /* Reserve space for the summaryData size mark. */
                        reservedBytes += _levelInfo._internalMark2._sizeBytes;
                    }
                }

                if (iter.isIteratorOverrun(reservedBytes + ((setMaxSize >= 0x80 || setMaxSize == 0) ? 2 : 1)))
                {
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                }

                iter._curBufPos = setupU15Mark(_levelInfo._internalMark, setMaxSize, iter._curBufPos);
                iter._writer.position(iter._curBufPos);

                /* Back up endBufPos to account for reserved bytes. */
                _levelInfo._reservedBytes = reservedBytes;
                iter._endBufPos -= reservedBytes;
                iter._writer.reserveBytes(reservedBytes);

                /* save state and return */
                _levelInfo._encodingState = EncodeIteratorStates.SET_DEFINITIONS;
                return CodecReturnCodes.SUCCESS;
            }
        }

        /* check for summary data */
        if (series.checkHasSummaryData())
        {
            /* we have summary data */
            if (series.encodedSummaryData().data() != null)
            {
                /* this is already encoded */
                /* make sure it fits */
                int len = series.encodedSummaryData().length();
                if (len < 0x80)
                {
                    if (iter.isIteratorOverrun(1 + len))
                    {
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }
                    iter._writer.writeByte(len);
                    iter._writer.write((BufferImpl)series.encodedSummaryData());
                }
                else
                {
                    if (iter.isIteratorOverrun(2 + len))
                    {
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }
                    iter._writer.writeUShort15rbLong((short)len);
                    iter._writer.write((BufferImpl)series.encodedSummaryData());
                }

                iter._curBufPos = iter._writer.position();
            }
            else
            {
                if (iter.isIteratorOverrun(reservedBytes + ((summaryMaxSize >= 0x80 || summaryMaxSize == 0) ? 2 : 1)))
                {
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                }

                iter._curBufPos = setupU15Mark(_levelInfo._internalMark2, summaryMaxSize, iter._curBufPos);
                iter._writer.position(iter._curBufPos);

                /* Back up endBufPos to account for the reserved bytes. */
                _levelInfo._reservedBytes = reservedBytes;
                iter._endBufPos -= reservedBytes;
                iter._writer.reserveBytes(reservedBytes);

                /* save state and return */
                _levelInfo._encodingState = EncodeIteratorStates.SUMMARY_DATA;
                return CodecReturnCodes.SUCCESS;
            }
        }

        if (iter.isIteratorOverrun(reservedBytes))
        {
            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        int ret = finishSeriesInit(series, iter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        return CodecReturnCodes.SUCCESS;
    }

    private static int finishSeriesInit(SeriesImpl series, EncodeIteratorImpl iter)
    {
        int ret;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        /* Move endBufPos back to original position if bytes were reserved. */
        iter._endBufPos += _levelInfo._reservedBytes;
        iter._writer.unreserveBytes(_levelInfo._reservedBytes);
        _levelInfo._reservedBytes = 0;

        /* store count hint */
        if (series.checkHasTotalCountHint())
        {
            ret = iter._writer.writeUInt30rb(series._totalCountHint);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
            iter._curBufPos = iter._writer.position();
        }

        /* store the count position */
        _levelInfo._countWritePos = iter._curBufPos;
        iter._curBufPos += 2;
        iter._writer.skipBytes(2);
        _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;
        return CodecReturnCodes.SUCCESS;
    }
	
    static int encodeSeriesComplete(EncodeIterator iterInt, boolean success)
    {
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;

        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        /* Validations */
        assert null != iter && null != _levelInfo._listType : "Invalid parameters or parameters passed in as NULL";
        assert _levelInfo._containerType == DataTypes.SERIES : "Invalid encoding attempted - wrong type";
        assert ((_levelInfo._encodingState == EncodeIteratorStates.ENTRIES) ||
                (_levelInfo._encodingState == EncodeIteratorStates.SET_DEFINITIONS) ||
                (_levelInfo._encodingState == EncodeIteratorStates.SUMMARY_DATA) ||
                (_levelInfo._encodingState == EncodeIteratorStates.WAIT_COMPLETE)) : "Unexpected encoding attempted";
        assert null != iter._buffer : "Invalid iterator use - check buffer";
        assert iter._curBufPos <= iter._endBufPos : "Data exceeds iterators buffer length";

        if (success)
        {
            assert _levelInfo._encodingState == EncodeIteratorStates.ENTRIES : "Unexpected encoding attempted";
            assert _levelInfo._countWritePos != 0 : "Invalid encoding attempted";
            iter._writer.position(_levelInfo._countWritePos);
            iter._writer.writeShort(_levelInfo._currentCount);
            iter._writer.position(iter._curBufPos);

        }
        else
        {
            iter._curBufPos = _levelInfo._containerStartPos;
            iter._writer.position(iter._curBufPos);

            /* Move endBufPos back to original position if bytes were reserved. */
            iter._endBufPos += _levelInfo._reservedBytes;
            iter._writer.unreserveBytes(_levelInfo._reservedBytes);
            _levelInfo._reservedBytes = 0;
        }
        --iter._encodingLevel;

        return CodecReturnCodes.SUCCESS;
    }
	
    static int encodeSeriesEntryInit(EncodeIterator iterInt, SeriesEntry seriesEntryInt, int maxEncodingSize)
    {
        Series series;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        /* Validations */
        assert null != iter && null != seriesEntryInt : "Invalid parameters or parameters passed in as NULL";
        assert _levelInfo._containerType == DataTypes.SERIES : "Invalid encoding attempted - wrong type";
        assert _levelInfo._encodingState == EncodeIteratorStates.ENTRIES : "Unexpected encoding attempted";
        assert iter._curBufPos != 0 : "Invalid encoding attempted - check buffer";

        series = (Series)_levelInfo._listType;

        _levelInfo._initElemStartPos = iter._curBufPos;

        _levelInfo._encodingState = EncodeIteratorStates.ENTRY_INIT;

        if (series.containerType() != DataTypes.NO_DATA)
        {
            if (iter.isIteratorOverrun(((maxEncodingSize == 0 || maxEncodingSize >= 0xFE) ? 3 : 1)))
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            }

            iter._curBufPos = setupU16Mark(_levelInfo._internalMark, maxEncodingSize, iter._curBufPos);
            iter._writer.position(iter._curBufPos);
        }
        else
        {
            _levelInfo._internalMark._sizeBytes = 0;
            _levelInfo._internalMark._sizePos = iter._curBufPos;
        }

        return CodecReturnCodes.SUCCESS;
    }
	
    static int encodeSeriesEntryComplete(EncodeIterator iterInt, boolean success)
    {
        int ret;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        /* Validations */
        assert null != iter && null != _levelInfo._listType : "Invalid parameters or parameters passed in as NULL";
        assert _levelInfo._containerType == DataTypes.SERIES : "Invalid encoding attempted - wrong type";
        assert (_levelInfo._encodingState == EncodeIteratorStates.ENTRY_INIT) ||
               (_levelInfo._encodingState == EncodeIteratorStates.WAIT_COMPLETE) : "Unexpected encoding attempted";
        assert null != iter._buffer : "Invalid iterator use - check buffer";
        assert (_levelInfo._currentCount + 1) != 0 : "Invalid encoding attempted";

        if (success)
        {
            if (_levelInfo._internalMark._sizeBytes > 0)
            {
                if ((ret = finishU16Mark(iter, _levelInfo._internalMark, iter._curBufPos)) < 0)
                {
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    _levelInfo._initElemStartPos = 0;
                    return ret;
                }
            }
            else
            {
                /* size bytes is 0 - this means that action was clear or df was no data */
                if (_levelInfo._internalMark._sizePos != iter._curBufPos)
                {
                    /* something was written when it shouldnt have been */
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    _levelInfo._initElemStartPos = 0;
                    return CodecReturnCodes.INVALID_DATA;
                }
                _levelInfo._internalMark._sizePos = 0;
            }

            _levelInfo._currentCount++;
        }
        else
        {
            /* reset the pointer */
            iter._curBufPos = _levelInfo._initElemStartPos;
            iter._writer.position(iter._curBufPos);
        }

        _levelInfo._initElemStartPos = 0;
        _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

        return CodecReturnCodes.SUCCESS;
    }
	
    static int encodeSeriesEntry(EncodeIterator iterInt, SeriesEntry seriesEntryInt)
    {
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        SeriesEntryImpl seriesEntry = (SeriesEntryImpl)seriesEntryInt;
        Series series;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        /* Validations */
        assert null != iter && null != seriesEntry : "Invalid parameters or parameters passed in as NULL";
        assert _levelInfo._containerType == DataTypes.SERIES : "Invalid encoding attempted - wrong type";
        assert _levelInfo._encodingState == EncodeIteratorStates.ENTRIES : "Unexpected encoding attempted";
        assert iter._curBufPos != 0 : "Invalid iterator use - check buffer";

        series = (Series)_levelInfo._listType;

        if (series.containerType() != DataTypes.NO_DATA)
        {
            int len = (seriesEntry._encodedData).length();

            // len value is written on wire as uShort16ob
            // If the value is smaller than 0xFE, it is written on the wire as one byte,
            // otherwise, it is written as three bytes by calling writeUShort16obLong method.
            // The code below checks if the buffer is sufficient for each case.
            if (len < 0xFE)
            {
                if (iter.isIteratorOverrun(1 + len))
                    /* no need for rollback because iterator was never moved */
                    return CodecReturnCodes.BUFFER_TOO_SMALL;

                iter._writer.writeByte(len);
                iter._writer.write((BufferImpl)seriesEntry._encodedData);
            }
            else if (len <= 0xFFFF)
            {
                if (iter.isIteratorOverrun(3 + len))
                    /* no need for rollback because iterator was never moved */
                    return CodecReturnCodes.BUFFER_TOO_SMALL;

                iter._writer.writeUShort16obLong(len);
                iter._writer.write((BufferImpl)seriesEntry._encodedData);
            }
            else
                return CodecReturnCodes.INVALID_DATA;

            iter._curBufPos = iter._writer.position();
        }

        _levelInfo._currentCount++;

        return CodecReturnCodes.SUCCESS;
    }

    static int encodeVectorInit(EncodeIterator iterInt, Vector vectorInt, int summaryMaxSize, int setMaxSize)
    {
        assert null != iterInt : "Invalid IterInt in as NULL";
        assert null != vectorInt : "Invalid vectorInt in as NULL";

        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        VectorImpl vector = (VectorImpl)vectorInt;
        EncodingLevel _levelInfo;
        int flags;

        if (!(validAggregateDataType(vector.containerType())))
            return CodecReturnCodes.UNSUPPORTED_DATA_TYPE;

        if (++iter._encodingLevel >= EncodeIteratorImpl.ENC_ITER_MAX_LEVELS)
            return CodecReturnCodes.ITERATOR_OVERRUN;
        _levelInfo = iter._levelInfo[iter._encodingLevel];
        _levelInfo.init(DataTypes.VECTOR, EncodeIteratorStates.NONE, vector, iter._curBufPos);
        _levelInfo._flags = EncodeIteratorFlags.NONE;

        /* Make sure required elements can be encoded */
        /* Flags (1), _containerType (1), Count (2) */
        if (iter.isIteratorOverrun(2))
        {
            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        flags = (vector.flags() & ~VectorFlags.HAS_PER_ENTRY_PERM_DATA);
        iter._writer.writeUByte(flags);
        /* container type needs to be scaled before its encoded */
        iter._writer.writeUByte(vector.containerType() - DataTypes.CONTAINER_TYPE_MIN);
        iter._curBufPos = iter._writer.position();

        /* If summary data and/or set definitions are to be encoded (but not pre-encoded),
         * reserve space for the count & totalCountHint that will be encoded afterwards. */
        int reservedBytes = 2 /* count */ + (vector.checkHasTotalCountHint() ? 4 : 0);

        /* check for list set definitions */
        if (vector.checkHasSetDefs())
        {
            /* We have list set definitions */
            if (vector.encodedSetDefs().data() != null)
            {
                /* set data is already encoded */
                /* make sure it can be put in buffer */
                int len = vector.encodedSetDefs().length();

                // len value is written on wire as uShort15rb
                // If the value is smaller than 0x80, it is written on the wire as one byte,
                // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
                // The code below checks if the buffer is sufficient for each case.
                if (len < 0x80)
                {
                    if (iter.isIteratorOverrun(1 + len))
                    {
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }

                    iter._writer.writeByte(len);
                    iter._writer.write((BufferImpl)vector.encodedSetDefs());
                }
                else
                {
                    if (iter.isIteratorOverrun(2 + len))
                    {
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }

                    iter._writer.writeUShort15rbLong((short)len);
                    iter._writer.write((BufferImpl)vector.encodedSetDefs());
                }

                iter._curBufPos = iter._writer.position();
            }
            else
            {
                if (vector.checkHasSummaryData())
                {
                    if (vector.encodedSummaryData().data() != null)
                    {
                        /* Reserve space to encode the summaryData and its length */
                        int summaryLength = vector.encodedSummaryData().length();
                        reservedBytes += summaryLength + ((summaryLength >= 0x80) ? 2 : 1);
                    }
                    else
                    {
                        /* store # of bytes for summary data so user does not pass it in again */
                        if (summaryMaxSize == 0 || summaryMaxSize >= 0x80)
                            _levelInfo._internalMark2._sizeBytes = 2;
                        else
                            _levelInfo._internalMark2._sizeBytes = 1;

                        /* Reserve space for the summaryData size mark. */
                        reservedBytes += _levelInfo._internalMark2._sizeBytes;
                    }
                }

                if (iter.isIteratorOverrun(reservedBytes + ((setMaxSize >= 0x80 || setMaxSize == 0) ? 2 : 1)))
                {
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                }

                iter._curBufPos = setupU15Mark(_levelInfo._internalMark, setMaxSize, iter._curBufPos);
                iter._writer.position(iter._curBufPos);

                /* Back up endBufPos to account for reserved bytes. */
                _levelInfo._reservedBytes = reservedBytes;
                iter._endBufPos -= reservedBytes;
                iter._writer.reserveBytes(reservedBytes);

                /* Save state and return */
                _levelInfo._encodingState = EncodeIteratorStates.SET_DEFINITIONS;

                return CodecReturnCodes.SUCCESS;
            }
        }

        /* Check for summary data */
        if (vector.checkHasSummaryData())
        {
            /* we have summary data */
            if (vector.encodedSummaryData().data() != null)
            {
                /* the summary data is already encoded */
                /* Make sure it can be put in buffer */
                int len = vector.encodedSummaryData().length();
                if (len < 0x80)
                {
                    if (iter.isIteratorOverrun(1 + len))
                    {
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }
                    iter._writer.writeByte(len);
                    iter._writer.write((BufferImpl)vector.encodedSummaryData());
                }
                else
                {
                    if (iter.isIteratorOverrun(2 + len))
                    {
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }
                    iter._writer.writeUShort15rbLong((short)len);
                    iter._writer.write((BufferImpl)vector.encodedSummaryData());
                }

                iter._curBufPos = iter._writer.position();
            }
            else
            {
                if (iter.isIteratorOverrun(reservedBytes + ((summaryMaxSize >= 0x80 || summaryMaxSize == 0) ? 2 : 1)))
                {
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                }

                iter._curBufPos = setupU15Mark(_levelInfo._internalMark2, summaryMaxSize, iter._curBufPos);
                iter._writer.position(iter._curBufPos);

                /* Back up endBufPos to account for the reserved bytes. */
                _levelInfo._reservedBytes = reservedBytes;
                iter._endBufPos -= reservedBytes;
                iter._writer.reserveBytes(reservedBytes);

                /* save state and return */
                _levelInfo._encodingState = EncodeIteratorStates.SUMMARY_DATA;
                return CodecReturnCodes.SUCCESS;
            }
        }

        if (iter.isIteratorOverrun(reservedBytes))
        {
            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        int ret;
        if ((ret = finishVectorInit(vector, iter)) != CodecReturnCodes.SUCCESS)
        {
            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
            return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }

    private static int finishVectorInit(VectorImpl vector, EncodeIteratorImpl iter)
    {
        int ret;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        /* Move endBufPos back to original position if bytes were reserved. */
        iter._endBufPos += _levelInfo._reservedBytes;
        iter._writer.unreserveBytes(_levelInfo._reservedBytes);
        _levelInfo._reservedBytes = 0;

        /* store count hint */
        if (vector.checkHasTotalCountHint())
        {
            ret = iter._writer.writeUInt30rb(vector._totalCountHint);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            iter._curBufPos = iter._writer.position();
        }

        /* store the count position */
        _levelInfo._countWritePos = iter._curBufPos;
        iter._curBufPos += 2;
        iter._writer.skipBytes(2);
        _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;
        return CodecReturnCodes.SUCCESS;
    }

    static int encodeVectorComplete(EncodeIterator iterInt, boolean success, Vector vectorInt)
    {
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        VectorImpl vector = (VectorImpl)vectorInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];
        int flags;

        /* Validations */
        assert null != iter && null != _levelInfo._listType : "Invalid parameters or parameters passed in as NULL";
        assert _levelInfo._containerType == DataTypes.VECTOR : "Invalid encoding attempted - wrong type";
        assert (_levelInfo._encodingState == EncodeIteratorStates.ENTRIES) ||
               (_levelInfo._encodingState == EncodeIteratorStates.SET_DEFINITIONS) ||
               (_levelInfo._encodingState == EncodeIteratorStates.SUMMARY_DATA) ||
               (_levelInfo._encodingState == EncodeIteratorStates.WAIT_COMPLETE) : "Unexpected encoding attempted";
        assert null != iter._buffer : "Invalid iterator use - check buffer";
        assert iter._curBufPos <= iter._endBufPos : "Data exceeds iterators buffer length";

        if (success)
        {
            assert _levelInfo._encodingState == EncodeIteratorStates.ENTRIES : "Unexpected encoding attempted";
            assert _levelInfo._countWritePos != 0 : "Invalid encoding attempted";
            iter._writer.position(_levelInfo._countWritePos);
            iter._writer.writeUShort(_levelInfo._currentCount);
            iter._writer.position(iter._curBufPos);

            if ((_levelInfo._flags & EncodeIteratorFlags.HAS_PER_ENTRY_PERM) > 0)
            {
                /* write per_entry_perm bit */
                /* flags are first byte of container */
                flags = (vector._flags | VectorFlags.HAS_PER_ENTRY_PERM_DATA);
                iter._writer.position(_levelInfo._containerStartPos);
                iter._writer.writeUByte(flags);
                iter._writer.position(iter._curBufPos);
            }
        }
        else
        {
            iter._curBufPos = _levelInfo._containerStartPos;
            iter._writer.position(iter._curBufPos);

            /* Move endBufPos back to original position if bytes were reserved. */
            iter._endBufPos += _levelInfo._reservedBytes;
            iter._writer.unreserveBytes(_levelInfo._reservedBytes);
            _levelInfo._reservedBytes = 0;

        }
        --iter._encodingLevel;

        return CodecReturnCodes.SUCCESS;
    }
	
    static int encodeVectorSummaryDataComplete(EncodeIterator iterInt, Vector vectorInt, boolean success)
    {
        int ret;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        VectorImpl vector = (VectorImpl)vectorInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        /* Validations */
        assert null != iter : "Invalid parameters or parameters passed in as NULL";
        assert _levelInfo._containerType == DataTypes.VECTOR : "Invalid encoding attempted - wrong type";
        assert (_levelInfo._encodingState == EncodeIteratorStates.SUMMARY_DATA) ||
               (_levelInfo._encodingState == EncodeIteratorStates.WAIT_COMPLETE) : "Unexpected encoding attempted";
        assert iter._curBufPos != 0 : "Invalid encoding attempted";

        if (success)
        {
            if ((ret = finishU15Mark(iter, _levelInfo._internalMark2, iter._curBufPos)) < 0)
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return ret;
            }

            if ((ret = finishVectorInit(vector, iter)) != CodecReturnCodes.SUCCESS)
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return ret;
            }
        }
        else
        {
            assert _levelInfo._internalMark2._sizePos != 0 : "Invalid encoding attempted";
            iter._curBufPos = _levelInfo._internalMark2._sizePos + _levelInfo._internalMark2._sizeBytes;
            iter._writer.position(iter._curBufPos);
            _levelInfo._encodingState = EncodeIteratorStates.SUMMARY_DATA;
        }
        return CodecReturnCodes.SUCCESS;
    }
	
    static int encodeVectorEntryInit(EncodeIterator iterInt, VectorEntry vectorEntryInt, int maxEncodingSize)
    {
        int ret;
        Vector vector;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        VectorEntryImpl vectorEntry = (VectorEntryImpl)vectorEntryInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        /* Validations */
        assert null != iter && null != vectorEntry : "Invalid parameters or parameters passed in as NULL";
        assert _levelInfo._containerType == DataTypes.VECTOR : "Invalid encoding attempted - wrong type";
        assert _levelInfo._encodingState == EncodeIteratorStates.ENTRIES : "Unexpected encoding attempted";
        assert iter._curBufPos != 0 : "Invalid encoding attempted";

        vector = (VectorImpl)_levelInfo._listType;

        _levelInfo._initElemStartPos = iter._curBufPos;

        if ((ret = encodeVectorEntryInternal(iter, vector, vectorEntry)) < 0)
        {
            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
            return ret;
        }

        _levelInfo._encodingState = EncodeIteratorStates.ENTRY_INIT;

        if ((vectorEntry.action() != VectorEntryActions.CLEAR)
                && (vectorEntry.action() != VectorEntryActions.DELETE)
                && (vector.containerType() != DataTypes.NO_DATA))
        {
            if (iter.isIteratorOverrun(((maxEncodingSize == 0 || maxEncodingSize >= 0xFE) ? 3 : 1)))
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            }

            iter._curBufPos = setupU16Mark(_levelInfo._internalMark, maxEncodingSize, iter._curBufPos);
            iter._writer.position(iter._curBufPos);
        }
        else
        {
            _levelInfo._internalMark._sizeBytes = 0;
            _levelInfo._internalMark._sizePos = iter._curBufPos;
        }

        return CodecReturnCodes.SUCCESS;
    }
	
    static int encodeVectorEntry(EncodeIterator iterInt, VectorEntry vectorEntryInt)
    {
        assert null != iterInt : "Invalid IterInt in as NULL";
        assert null != vectorEntryInt : "Invalid vectorEntryInt in as NULL";

        int ret;
        Vector vector;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        VectorEntryImpl vectorEntry = (VectorEntryImpl)vectorEntryInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        assert _levelInfo._encodingState == EncodeIteratorStates.ENTRIES : "Unexpected encoding attempted";
        assert _levelInfo._containerType == DataTypes.VECTOR : "Invalid encoding attempted - wrong type";

        vector = (VectorImpl)_levelInfo._listType;

        _levelInfo._initElemStartPos = iter._curBufPos;

        if ((ret = encodeVectorEntryInternal(iter, vector, vectorEntry)) < 0)
        {
            /* rollback */
            iter._curBufPos = _levelInfo._initElemStartPos;
            iter._writer.position(iter._curBufPos);
            return ret;
        }

        /* encode data as small buffer */
        /* we should only have data if we are not deleting/clearing a position */
        if ((vectorEntry.action() != VectorEntryActions.CLEAR) && (vectorEntry.action() != VectorEntryActions.DELETE)
                && (vector.containerType() != DataTypes.NO_DATA))
        {
            int len = (vectorEntry._encodedData).length();

            // len value is written on wire as uShort16ob
            // if the value is smaller than 0xFE, it is written on the wire as one byte,
            // otherwise, it is written as three bytes by calling writeUShort16obLong method.
            // the code below checks if the buffer is sufficient for each case.
            if (len < 0xFE)
            {
                if (iter.isIteratorOverrun(1 + len))
                {
                    /* rollback */
                    iter._curBufPos = _levelInfo._initElemStartPos;
                    iter._writer.position(iter._curBufPos);
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                }

                iter._writer.writeByte(len);
                iter._writer.write((BufferImpl)vectorEntry._encodedData);
            }
            else if (len <= 0xFFFF)
            {
                if (iter.isIteratorOverrun(3 + len))
                {
                    /* rollback */
                    iter._curBufPos = _levelInfo._initElemStartPos;
                    iter._writer.position(iter._curBufPos);
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                }

                iter._writer.writeUShort16obLong(len);
                iter._writer.write((BufferImpl)vectorEntry._encodedData);
            }
            else
            {
                /* rollback */
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.position(iter._curBufPos);
                return CodecReturnCodes.INVALID_DATA;
            }

            iter._curBufPos = iter._writer.position();
        }

        _levelInfo._currentCount++;

        return CodecReturnCodes.SUCCESS;
    }

    private static int encodeVectorEntryInternal(EncodeIteratorImpl iter, Vector vector, VectorEntryImpl vectorEntry)
    {
        int ret;
        int flags = 0;

        /* Preliminary validations */
        assert (iter._levelInfo[iter._encodingLevel]._currentCount + 1) != 0 : "Invalid encoding attempted";

        if (iter.isIteratorOverrun(1))
            return CodecReturnCodes.BUFFER_TOO_SMALL;

        if (vectorEntry.index() > vectorEntry.MAX_INDEX)
            return CodecReturnCodes.INVALID_ARGUMENT;

        /* Flags byte made up of flags and action */
        /* set action and flags in same 8 bit value */
        flags = vectorEntry.flags();
        /* shifts flags by 4 bits */
        flags <<= 4;
        /* sets action bits */
        flags += vectorEntry.action();

        /* put flags/action into packet */
        iter._writer.writeUByte(flags);

        /* Store index as UInt30_rb */
        ret = iter._writer.writeUInt30rb((int)vectorEntry.index());

        if (ret != CodecReturnCodes.SUCCESS)
            return ret;

        iter._curBufPos = iter._writer.position();

        if (vectorEntry.checkHasPermData())
        {
            iter._levelInfo[iter._encodingLevel]._flags |= EncodeIteratorFlags.HAS_PER_ENTRY_PERM;
            /* encode perm exp as small buffer */
            if (vectorEntry.permData().data() == null)
            {
                if (iter.isIteratorOverrun(1))
                    return CodecReturnCodes.BUFFER_TOO_SMALL;
                /* just encode 0 bytes since its empty */
                int zero = 0;
                iter._writer.writeUByte(zero);
            }
            else
            {
                int len = vectorEntry.permData().length();

                // len value is written on wire as uShort15rb
                // If the value is smaller than 0x80, it is written on the wire as one byte,
                // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
                // The code below checks if the buffer is sufficient for each case.
                if (len < 0x80)
                {
                    if (iter.isIteratorOverrun(1 + len))
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    iter._writer.writeByte(len);
                    iter._writer.write((BufferImpl)vectorEntry.permData());
                }
                else
                {
                    if (iter.isIteratorOverrun(2 + len))
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    iter._writer.writeUShort15rbLong((short)len);
                    iter._writer.write((BufferImpl)vectorEntry.permData());
                }
            }
        }

        iter._curBufPos = iter._writer.position();
        return CodecReturnCodes.SUCCESS;
    }

    static int encodeVectorEntryComplete(EncodeIterator iterInt, boolean success)
    {
        int ret;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        /* Validations */
        assert null != iter && null != _levelInfo._listType : "Invalid parameters or parameters passed in as NULL";
        assert _levelInfo._containerType == DataTypes.VECTOR : "Invalid encoding attempted - wrong type";
        assert (_levelInfo._encodingState == EncodeIteratorStates.ENTRY_INIT) ||
               (_levelInfo._encodingState == EncodeIteratorStates.WAIT_COMPLETE) : "Unexpected encoding attempted";
        assert null != iter._buffer : "Invalid iterator use - check buffer";
        assert (_levelInfo._currentCount + 1) != 0 : "Invalid encoding attempted";

        if (success)
        {
            if (_levelInfo._internalMark._sizeBytes > 0)
            {
                if ((ret = finishU16Mark(iter, _levelInfo._internalMark, iter._curBufPos)) < 0)
                {
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    _levelInfo._initElemStartPos = 0;
                    return ret;
                }
            }
            else
            {
                /* size bytes is 0 - this means that action was clear or df was no data */
                if (_levelInfo._internalMark._sizePos != iter._curBufPos)
                {
                    /* something was written when it shouldnt have been */
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    _levelInfo._initElemStartPos = 0;
                    return CodecReturnCodes.INVALID_DATA;
                }
                _levelInfo._internalMark._sizePos = 0;
            }
            _levelInfo._currentCount++;
        }
        else
        {
            /* reset the pointer */
            iter._curBufPos = _levelInfo._initElemStartPos;
            iter._writer.position(iter._curBufPos);
        }

        _levelInfo._initElemStartPos = 0;
        _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

        return CodecReturnCodes.SUCCESS;
    }
	
    static int encodeLocalElementSetDefDb(EncodeIterator iterInt, LocalElementSetDefDb setDb)
    {
        assert null != iterInt : "Invalid IterInt in as NULL";
        assert null != setDb : "Invalid setDb in as NULL";

        int flags = 0;
        int defCount;
        int defs;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        EncodingLevel _levelInfo;

        if (iter._encodingLevel + 1 >= EncodeIteratorImpl.ENC_ITER_MAX_LEVELS)
            return CodecReturnCodes.ITERATOR_OVERRUN;
        _levelInfo = iter._levelInfo[iter._encodingLevel + 1];
        _levelInfo.init(DataTypes.ELEMENT_LIST, EncodeIteratorStates.SET_DEFINITIONS, setDb, iter._curBufPos);

        /* make sure that required elements can be encoded */
        if (iter.isIteratorOverrun(2))
            return CodecReturnCodes.BUFFER_TOO_SMALL;

        /* store flags as uint8 */
        iter._writer.writeByte(flags);
        iter._curBufPos = iter._writer.position();

        _levelInfo._countWritePos = iter._curBufPos; /* store count position */
        iter._curBufPos++; /* skip count byte */
        iter._writer.position(iter._curBufPos);
        /* go through defs and encode them */
        defCount = 0;
        for (defs = 0; defs <= LocalElementSetDefDbImpl.MAX_LOCAL_ID; defs++)
        {
            if (setDb.definitions()[defs].setId() != LocalElementSetDefDbImpl.BLANK_ID)
            {
                int i;
                ElementSetDef setDef = setDb.definitions()[defs];

                assert setDef.setId() == defs : "Invalid set definition)";

                if (setDef.setId() < 0x80)
                {
                    /* make sure required elements fit */
                    if (iter.isIteratorOverrun(2))
                    {
                        /* rollback */
                        iter._curBufPos = _levelInfo._containerStartPos;
                        iter._writer.position(iter._curBufPos);
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }
                    iter._writer.writeByte(setDef.setId());
                }
                else
                {
                    /* make sure required elements fit */
                    if (iter.isIteratorOverrun(3))
                    {
                        /* rollback */
                        iter._curBufPos = _levelInfo._containerStartPos;
                        iter._writer.position(iter._curBufPos);
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }
                    iter._writer.writeUShort15rbLong((short)setDef.setId());
                }
                iter._writer.writeByte(setDef.count());
                iter._curBufPos = iter._writer.position();

                for (i = 0; i < setDef.count(); i++)
                {
                    ElementSetDefEntry elementEnc = setDef.entries()[i];
                    assert null != elementEnc : "Invalid parameters or parameters passed in as NULL";

                    if (iter.isIteratorOverrun(3 + elementEnc.name().length()))
                    {
                        /* rollback */
                        iter._curBufPos = _levelInfo._containerStartPos;
                        iter._writer.position(iter._curBufPos);
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }

                    encodeBuffer15(iter, (BufferImpl)elementEnc.name());
                    iter._writer.writeByte(elementEnc.dataType());
                    iter._curBufPos = iter._writer.position();
                }
                ++defCount;
            }

        }

        iter._writer.position(_levelInfo._countWritePos);
        iter._writer.writeByte(defCount);
        iter._writer.position(iter._curBufPos);

        return CodecReturnCodes.SUCCESS;
    }

    static int encodeSeriesSetDefsComplete(EncodeIterator iterInt, boolean success)
    {
        assert null != iterInt : "Invalid IterInt in as NULL";

        int ret;
        SeriesImpl series;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        assert _levelInfo._containerType == DataTypes.SERIES : "Invalid encoding attempted - wrong type";
        assert _levelInfo._encodingState == EncodeIteratorStates.SET_DEFINITIONS : "Unexpected encoding attempted)";

        series = (SeriesImpl)_levelInfo._listType;

        if (success)
        {
            if ((ret = finishU15Mark(iter, _levelInfo._internalMark, iter._curBufPos)) < 0)
                return (ret);

            /* check for summary data */
            if (series.checkHasSummaryData())
            {
                if (series.encodedSummaryData().data() != null)
                {
                    /* Buffer space was reserved to encode the entire summaryData.
                     * summaryData will be encoded now, so move endBufPos forward again. */
                    int summaryLength = series.encodedSummaryData().length();
                    int reservedBytes = summaryLength + ((summaryLength >= 0x80) ? 2 : 1);

                    iter._endBufPos += reservedBytes;
                    iter._writer.unreserveBytes(reservedBytes);
                    _levelInfo._reservedBytes -= reservedBytes;

                    encodeBuffer15(iter, (BufferImpl)series.encodedSummaryData());
                    iter._curBufPos = iter._writer.position();
                }
                else
                {
                    /* we already stored length in the INIT call */
                    assert _levelInfo._internalMark2._sizePos == 0 : "Invalid encoding attempted";

                    /* Buffer space was reserved for the summaryData size bytes.
                     * User will start encoding summaryData next, so move endBufPos forward again. */
                    int reservedBytes = _levelInfo._internalMark2._sizeBytes;
                    iter._endBufPos += reservedBytes;
                    iter._writer.unreserveBytes(reservedBytes);
                    _levelInfo._reservedBytes -= reservedBytes;

                    /* Reserve size bytes for summary data. */
                    _levelInfo._internalMark2._sizePos = iter._curBufPos;
                    iter._curBufPos += _levelInfo._internalMark2._sizeBytes;
                    iter._writer.position(iter._curBufPos);

                    /* save state and return */
                    _levelInfo._encodingState = EncodeIteratorStates.SUMMARY_DATA;
                    return CodecReturnCodes.SUCCESS;
                }
            }

            if ((ret = finishSeriesInit(series, iter)) != CodecReturnCodes.SUCCESS)
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return CodecReturnCodes.BUFFER_TOO_SMALL;
            }
        }
        else
        {
            assert _levelInfo._internalMark._sizePos != 0 : "Invalid encoding attempted";
            iter._curBufPos = _levelInfo._internalMark._sizePos + _levelInfo._internalMark._sizeBytes;
            iter._writer.position(iter._curBufPos);
            /* Leave state as SET_DEFINITIONS */
        }

        return CodecReturnCodes.SUCCESS;
    }

    static int encodeSeriesSummaryDataComplete(EncodeIterator iterInt, boolean success)
    {
        assert null != iterInt : "Invalid IterInt in as NULL";

        int ret;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        assert _levelInfo._containerType == DataTypes.SERIES : "Invalid encoding attempted - wrong type";
        assert (_levelInfo._encodingState == EncodeIteratorStates.SUMMARY_DATA) ||
               (_levelInfo._encodingState == EncodeIteratorStates.WAIT_COMPLETE) : "Unexpected encoding attempted";

        if (success)
        {
            if ((ret = finishU15Mark(iter, _levelInfo._internalMark2, iter._curBufPos)) < 0)
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return (ret);
            }

            if ((ret = finishSeriesInit((SeriesImpl)_levelInfo._listType, iter)) != CodecReturnCodes.SUCCESS)
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return ret;
            }
        }
        else
        {
            assert _levelInfo._internalMark2._sizePos != 0 : "Invalid encoding attempted";
            iter._curBufPos = _levelInfo._internalMark2._sizePos + _levelInfo._internalMark2._sizeBytes;
            iter._writer.position(iter._curBufPos);
            _levelInfo._encodingState = EncodeIteratorStates.SUMMARY_DATA;
        }
        return CodecReturnCodes.SUCCESS;
    }

    static int encodeMapSetDefsComplete(EncodeIterator iterInt, boolean success)
    {
        assert null != iterInt : "Invalid IterInt in as NULL";

        int ret;
        MapImpl map;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        assert _levelInfo._containerType == DataTypes.MAP : "Invalid encoding attempted - wrong type";
        assert _levelInfo._encodingState == EncodeIteratorStates.SET_DEFINITIONS : "Unexpected encoding attempted";

        map = (MapImpl)_levelInfo._listType;

        if (success)
        {
            if ((ret = finishU15Mark(iter, _levelInfo._internalMark, iter._curBufPos)) < 0)
                return (ret);

            /* check for summary data */
            if (map.checkHasSummaryData())
            {
                if (map.encodedSummaryData().data() != null)
                {
                    /* Buffer space was reserved to encode the entire summaryData.
                     * summaryData will be encoded now, so move endBufPos forward again. */
                    int summaryLength = map.encodedSummaryData().length();
                    int reservedBytes = summaryLength + ((summaryLength >= 0x80) ? 2 : 1);

                    iter._endBufPos += reservedBytes;
                    iter._writer.unreserveBytes(reservedBytes);
                    _levelInfo._reservedBytes -= reservedBytes;

                    assert (_levelInfo._reservedBytes >= 0);

                    encodeBuffer15(iter, (BufferImpl)map.encodedSummaryData());
                    iter._curBufPos = iter._writer.position();
                }
                else
                {
                    /* we already stored length in the INIT call */
                    assert _levelInfo._internalMark2._sizePos == 0 : "Invalid encoding attempted";

                    /* Buffer space was reserved for the summaryData size bytes.
                     * User will start encoding summaryData next, so move endBufPos forward again. */
                    int reservedBytes = _levelInfo._internalMark2._sizeBytes;
                    iter._endBufPos += reservedBytes;
                    iter._writer.unreserveBytes(reservedBytes);
                    _levelInfo._reservedBytes -= reservedBytes;

                    _levelInfo._internalMark2._sizePos = iter._curBufPos;
                    iter._curBufPos += _levelInfo._internalMark2._sizeBytes;
                    iter._writer.position(iter._curBufPos);

                    /* save state and return */
                    _levelInfo._encodingState = EncodeIteratorStates.SUMMARY_DATA;
                    return CodecReturnCodes.SUCCESS;
                }
            }

            finishMapInit(map, iter);
        }
        else
        {
            assert _levelInfo._internalMark._sizePos != 0 : "Invalid encoding attempted";
            iter._curBufPos = _levelInfo._internalMark._sizePos + _levelInfo._internalMark._sizeBytes;
            iter._writer.position(iter._curBufPos);
            /* Leave state as SET_DEFINITIONS */
        }
        return CodecReturnCodes.SUCCESS;
    }

    static int encodeVectorSetDefsComplete(EncodeIterator iterInt, boolean success)
    {
        assert null != iterInt : "Invalid IterInt in as NULL";

        int ret;
        VectorImpl vector;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

        assert _levelInfo._containerType == DataTypes.VECTOR : "Invalid encoding attempted - wrong type";
        assert _levelInfo._encodingState == EncodeIteratorStates.SET_DEFINITIONS : "Unexpected encoding attempted";

        vector = (VectorImpl)_levelInfo._listType;

        if (success)
        {
            if ((ret = finishU15Mark(iter, _levelInfo._internalMark, iter._curBufPos)) < 0)
                return (ret);

            /* check for summary data */
            if (vector.checkHasSummaryData())
            {
                if (vector.encodedSummaryData().data() != null)
                {
                    /* Buffer space was reserved to encode the entire summaryData.
                     * summaryData will be encoded now, so move endBufPos forward again. */
                    int summaryLength = vector.encodedSummaryData().length();
                    int reservedBytes = summaryLength + ((summaryLength >= 0x80) ? 2 : 1);

                    iter._endBufPos += reservedBytes;
                    iter._writer.unreserveBytes(reservedBytes);
                    _levelInfo._reservedBytes -= reservedBytes;

                    encodeBuffer15(iter, (BufferImpl)vector.encodedSummaryData());
                    iter._curBufPos = iter._writer.position();
                }
                else
                {
                    /* we already stored length in the INIT call */
                    assert _levelInfo._internalMark2._sizePos == 0 : "Invalid encoding attempted";

                    /* Buffer space was reserved for the summaryData size bytes. */
                    /* User will start encoding summaryData next, so move endBufPos forward again. */
                    int reservedBytes = _levelInfo._internalMark2._sizeBytes;
                    iter._endBufPos += reservedBytes;
                    iter._writer.unreserveBytes(reservedBytes);
                    _levelInfo._reservedBytes -= reservedBytes;

                    /* Reserve size bytes for summary data. */
                    _levelInfo._internalMark2._sizePos = iter._curBufPos;
                    iter._curBufPos += _levelInfo._internalMark2._sizeBytes;
                    iter._writer.position(iter._curBufPos);

                    /* save state and return */
                    _levelInfo._encodingState = EncodeIteratorStates.SUMMARY_DATA;
                    return CodecReturnCodes.SUCCESS;
                }
            }

            return finishVectorInit(vector, iter);
        }
        else
        {
            assert _levelInfo._internalMark._sizePos != 0 : "Invalid encoding attempted";
            iter._curBufPos = _levelInfo._internalMark._sizePos + _levelInfo._internalMark._sizeBytes;
            iter._writer.position(iter._curBufPos);
            /* Leave state as SET_DEFINITIONS */
        }
        return CodecReturnCodes.SUCCESS;
    }

    static int encodeLocalFieldSetDefDb(EncodeIterator iterInt, LocalFieldSetDefDbImpl setDb)
    {
        assert null != iterInt : "Invalid IterInt in as NULL";
        assert null != setDb : "Invalid setDb in as NULL";

        int flags = 0;
        int defCount;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        EncodingLevel _levelInfo;

        if (iter._encodingLevel + 1 >= EncodeIteratorImpl.ENC_ITER_MAX_LEVELS)
            return CodecReturnCodes.ITERATOR_OVERRUN;
        _levelInfo = iter._levelInfo[iter._encodingLevel + 1];
        _levelInfo.init(DataTypes.FIELD_LIST, EncodeIteratorStates.SET_DEFINITIONS, setDb, iter._curBufPos);

        /* make sure that required elements can be encoded */
        if (iter.isIteratorOverrun(2))
            return CodecReturnCodes.BUFFER_TOO_SMALL;

        /* store flags as uint8 */
        iter._writer.writeByte(flags);
        iter._curBufPos = iter._writer.position();

        _levelInfo._countWritePos = iter._curBufPos; /* store count position */
        iter._curBufPos++; /* skip count byte */
        iter._writer.position(iter._curBufPos);
        /* go through defs and encode them */
        defCount = 0;
        for (int defs = 0; defs <= LocalFieldSetDefDbImpl.MAX_LOCAL_ID; defs++)
        {
            if (setDb.definitions()[defs].setId() != LocalFieldSetDefDbImpl.BLANK_ID)
            {
                int i;
                FieldSetDef setDef = setDb.definitions()[defs];

                assert setDef.setId() == defs : "Invalid set definition";

                if (setDef.setId() < 0x80)
                {
                    /* make sure required elements fit */
                    if (iter.isIteratorOverrun(2))
                    {
                        /* rollback */
                        iter._curBufPos = _levelInfo._containerStartPos;
                        iter._writer.position(iter._curBufPos);
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }
                    iter._writer.writeByte((int)setDef.setId());
                }
                else
                {
                    /* make sure required elements fit */
                    if (iter.isIteratorOverrun(3))
                    {
                        /* rollback */
                        iter._curBufPos = _levelInfo._containerStartPos;
                        iter._writer.position(iter._curBufPos);
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }
                    iter._writer.writeUShort15rbLong((short)setDef.setId());
                }
                iter._writer.writeByte(setDef.count());
                iter._curBufPos = iter._writer.position();

                for (i = 0; i < setDef.count(); i++)
                {
                    FieldSetDefEntry fieldEnc = setDef.entries()[i];
                    assert null != fieldEnc : "Invalid parameters or parameters passed in as NULL";

                    if (iter.isIteratorOverrun(3))
                    {
                        /* rollback */
                        iter._curBufPos = _levelInfo._containerStartPos;
                        iter._writer.position(iter._curBufPos);
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    }

                    iter._writer.writeShort(fieldEnc.fieldId());
                    iter._writer.writeByte(fieldEnc.dataType());
                    iter._curBufPos = iter._writer.position();
                }
                ++defCount;
            }
        }

        iter._writer.position(_levelInfo._countWritePos);
        iter._writer.writeByte(defCount);
        iter._writer.position(iter._curBufPos);

        return CodecReturnCodes.SUCCESS;
    }

    static int encodeFieldSetDefDb(EncodeIterator iterInt, FieldSetDefDbImpl setDb)
    {
        assert null != iterInt : "Invalid IterInt in as NULL";
        assert null != setDb : "Invalid setDb in as NULL";

        int flags = 0;
        int defCount;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        EncodingLevel _levelInfo;

        if (iter._encodingLevel + 1 >= EncodeIteratorImpl.ENC_ITER_MAX_LEVELS)
            return CodecReturnCodes.ITERATOR_OVERRUN;
        _levelInfo = iter._levelInfo[iter._encodingLevel + 1];
        _levelInfo.init(DataTypes.FIELD_LIST, EncodeIteratorStates.SET_DEFINITIONS, setDb, iter._curBufPos);

        /* make sure that required elements can be encoded */
        if (iter.isIteratorOverrun(2))
            return CodecReturnCodes.BUFFER_TOO_SMALL;

        /* store flags as uint8 */
        iter._writer.writeByte(flags);
        iter._curBufPos = iter._writer.position();

        _levelInfo._countWritePos = iter._curBufPos; /* store count position */
        iter._curBufPos++; /* skip count byte */
        iter._writer.position(iter._curBufPos);
        /* go through defs and encode them */
        defCount = 0;
        for (int defs = 0; defs <= setDb.maxLocalId; defs++)
        {
            if (setDb.definitions()[defs].setId() != FieldSetDefDbImpl.BLANK_ID)
            {
                int i;
                FieldSetDef setDef = setDb.definitions()[defs];

                assert setDef.setId() == defs : "Invalid set definition";

                if (setDef.setId() < 0x80)
                {
                    /* make sure required elements fit */
                    if (iter.isIteratorOverrun(2))
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    iter._writer.writeByte((int)setDef.setId());
                }
                else
                {
                    /* make sure required elements fit */
                    if (iter.isIteratorOverrun(3))
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    iter._writer.writeUShort15rbLong((short)setDef.setId());
                }
                iter._writer.writeByte(setDef.count());
                iter._curBufPos = iter._writer.position();

                for (i = 0; i < setDef.count(); i++)
                {
                    FieldSetDefEntry fieldEnc = setDef.entries()[i];
                    assert null != fieldEnc : "Invalid parameters or parameters passed in as NULL";

                    if (iter.isIteratorOverrun(3))
                        return CodecReturnCodes.BUFFER_TOO_SMALL;

                    iter._writer.writeShort(fieldEnc.fieldId());
                    iter._writer.writeByte(fieldEnc.dataType());
                    iter._curBufPos = iter._writer.position();
                }
                ++defCount;
            }
        }

        iter._writer.position(_levelInfo._countWritePos);
        iter._writer.writeByte(defCount);
        iter._writer.position(iter._curBufPos);

        return CodecReturnCodes.SUCCESS;
    }

    static int encodeElementSetDefDb(EncodeIterator iterInt, ElementSetDefDbImpl setDb)
    {
        assert null != iterInt : "Invalid IterInt in as NULL";
        assert null != setDb : "Invalid setDb in as NULL";

        int flags = 0;
        int defCount;
        EncodeIteratorImpl iter = (EncodeIteratorImpl)iterInt;
        EncodingLevel _levelInfo;

        if (iter._encodingLevel + 1 >= EncodeIteratorImpl.ENC_ITER_MAX_LEVELS)
            return CodecReturnCodes.ITERATOR_OVERRUN;
        _levelInfo = iter._levelInfo[iter._encodingLevel + 1];
        _levelInfo.init(DataTypes.FIELD_LIST, EncodeIteratorStates.SET_DEFINITIONS, setDb, iter._curBufPos);

        /* make sure that required elements can be encoded */
        if (iter.isIteratorOverrun(2))
            return CodecReturnCodes.BUFFER_TOO_SMALL;

        /* store flags as uint8 */
        iter._writer.writeByte(flags);
        iter._curBufPos = iter._writer.position();

        _levelInfo._countWritePos = iter._curBufPos; /* store count position */
        iter._curBufPos++; /* skip count byte */
        iter._writer.position(iter._curBufPos);
        /* go through defs and encode them */
        defCount = 0;
        for (int defs = 0; defs <= setDb.MAX_LOCAL_ID; defs++)
        {
            if (setDb.definitions()[defs].setId() != FieldSetDefDbImpl.BLANK_ID)
            {
                int i;
                ElementSetDef setDef = setDb.definitions()[defs];

                assert setDef.setId() == defs : "Invalid set definition";

                if (setDef.setId() < 0x80)
                {
                    /* make sure required elements fit */
                    if (iter.isIteratorOverrun(2))
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    iter._writer.writeByte(setDef.setId());
                }
                else
                {
                    /* make sure required elements fit */
                    if (iter.isIteratorOverrun(3))
                        return CodecReturnCodes.BUFFER_TOO_SMALL;
                    iter._writer.writeUShort15rbLong((short)setDef.setId());
                }
                iter._writer.writeByte(setDef.count());
                iter._curBufPos = iter._writer.position();

                for (i = 0; i < setDef.count(); i++)
                {
                    ElementSetDefEntry fieldEnc = setDef.entries()[i];
                    assert null != fieldEnc : "Invalid parameters or parameters passed in as NULL";

                    if (iter.isIteratorOverrun(3))
                        return CodecReturnCodes.BUFFER_TOO_SMALL;

                    iter._writer.write((BufferImpl)fieldEnc.name());
                    iter._writer.writeByte(fieldEnc.dataType());
                    iter._curBufPos = iter._writer.position();
                }
                ++defCount;
            }
        }

        iter._writer.position(_levelInfo._countWritePos);
        iter._writer.writeByte(defCount);
        iter._writer.position(iter._curBufPos);

        return CodecReturnCodes.SUCCESS;
    }
}
