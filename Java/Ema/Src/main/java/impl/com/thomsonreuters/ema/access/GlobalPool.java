///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.util.ArrayDeque;
import java.util.concurrent.locks.ReentrantLock;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.Date;
import com.thomsonreuters.upa.codec.Double;
import com.thomsonreuters.upa.codec.DateTime;
import com.thomsonreuters.upa.codec.Enum;
import com.thomsonreuters.upa.codec.Float;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.Real;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.Time;
import com.thomsonreuters.upa.codec.UInt;

class GlobalPool
{
	private final static int DATA_POOL_INITIAL_SIZE = 40;

	static ReentrantLock _globalLock = new java.util.concurrent.locks.ReentrantLock();
	
	private static ArrayDeque<Date> _DatePool = new ArrayDeque<>(DATA_POOL_INITIAL_SIZE); 
	private static ArrayDeque<DateTime> _DateTimePool = new ArrayDeque<>(DATA_POOL_INITIAL_SIZE);
	private static ArrayDeque<Double> _DoublePool = new ArrayDeque<>(DATA_POOL_INITIAL_SIZE);
	private static ArrayDeque<Enum> _EnumPool = new ArrayDeque<>(DATA_POOL_INITIAL_SIZE);
	private static ArrayDeque<Float> _FloatPool = new ArrayDeque<>(DATA_POOL_INITIAL_SIZE);
	private static ArrayDeque<Int> _IntPool = new ArrayDeque<>(DATA_POOL_INITIAL_SIZE);
	private static ArrayDeque<Qos> _QosPool = new ArrayDeque<>(DATA_POOL_INITIAL_SIZE);
	private static ArrayDeque<Real> _RealPool = new ArrayDeque<>(DATA_POOL_INITIAL_SIZE);
	private static ArrayDeque<Buffer> _BufferPool = new ArrayDeque<>(DATA_POOL_INITIAL_SIZE);
	private static ArrayDeque<State> _StatePool = new ArrayDeque<>(DATA_POOL_INITIAL_SIZE);
	private static ArrayDeque<Time> _TimePool = new ArrayDeque<>(DATA_POOL_INITIAL_SIZE);
	private static ArrayDeque<UInt> _UIntPool = new ArrayDeque<>(DATA_POOL_INITIAL_SIZE);
	private static boolean _intialized = false;
	
	static void initialize()
	{
		if ( _intialized )
			return;
		
		
		for(int i = 0; i < DATA_POOL_INITIAL_SIZE; i++)
		{
			_DatePool.push(CodecFactory.createDate());
			_DateTimePool.push(CodecFactory.createDateTime());
			_DoublePool.push(CodecFactory.createDouble());
			_EnumPool.push(CodecFactory.createEnum());
			_FloatPool.push(CodecFactory.createFloat());
			_IntPool.push(CodecFactory.createInt());
			_QosPool.push(CodecFactory.createQos());
			_RealPool.push(CodecFactory.createReal());
			_BufferPool.push(CodecFactory.createBuffer());
			_StatePool.push(CodecFactory.createState());
			_TimePool.push(CodecFactory.createTime());
			_UIntPool.push(CodecFactory.createUInt());
		}
		
		_intialized = true;
	}
	
	static void lock()
	{
		_globalLock.lock();
	}
	
	static void unlock()
	{
		_globalLock.unlock();
	}
	
	static Date getDate()
	{
		if ( _DatePool.size() == 0)
		{
			return CodecFactory.createDate();
		}
		
		return _DatePool.pop();
	}
	
	static void returnDate(Date date)
	{
		_DatePool.push(date);
	}
	
	static DateTime getDateTime()
	{
		if ( _DatePool.size() == 0)
		{
			return CodecFactory.createDateTime();
		}
		
		return _DateTimePool.pop();
	}
	
	static void returnDateTime(DateTime dateTime)
	{
		_DateTimePool.push(dateTime);
	}

	static Double getDouble()
	{
		if ( _DatePool.size() == 0)
		{
			return CodecFactory.createDouble();
		}
		
		return _DoublePool.pop();
	}
	
	static void returnDouble(Double doubleValue)
	{
		_DoublePool.push(doubleValue);
	}
	
	static Enum getEnum()
	{
		if ( _EnumPool.size() == 0)
		{
			return CodecFactory.createEnum();
		}
		
		return _EnumPool.pop();
	}
	
	static void returnEnum(Enum enumValue)
	{
		_EnumPool.push(enumValue);
	}
	
	static Float getFloat()
	{
		if ( _FloatPool.size() == 0)
		{
			return CodecFactory.createFloat();
		}
		
		return _FloatPool.pop();
	}
	
	static void returnFloat(Float floatValue)
	{
		_FloatPool.push(floatValue);
	}
	
	static Int getInt()
	{
		if ( _IntPool.size() == 0)
		{
			return CodecFactory.createInt();
		}
		
		return _IntPool.pop();
	}
	
	static void returnInt(Int intValue)
	{
		_IntPool.push(intValue);
	}
	
	static Qos getQos()
	{
		if ( _QosPool.size() == 0)
		{
			return CodecFactory.createQos();
		}
		
		return _QosPool.pop();
	}
	
	static void returnQos(Qos qosValue)
	{
		_QosPool.push(qosValue);
	}

	static Real getReal()
	{
		if ( _RealPool.size() == 0)
		{
			return CodecFactory.createReal();
		}
		
		return _RealPool.pop();
	}
	
	static void returnReal(Real realValue)
	{
		_RealPool.push(realValue);
	}
	
	static Buffer getBuffer()
	{
		if ( _BufferPool.size() == 0)
		{
			return CodecFactory.createBuffer();
		}
		
		return _BufferPool.pop();
	}
	
	static void returnBuffer(Buffer buffer)
	{
		_BufferPool.push(buffer);
	}
	
	static State getState()
	{
		if ( _StatePool.size() == 0)
		{
			return CodecFactory.createState();
		}
		
		return _StatePool.pop();
	}
	
	static void returnState(State state)
	{
		_StatePool.push(state);
	}
	
	static Time getTime()
	{
		if ( _TimePool.size() == 0)
		{
			return CodecFactory.createTime();
		}
		
		return _TimePool.pop();
	}
	
	static void returnTime(Time time)
	{
		_TimePool.push(time);
	}

	static UInt getUInt()
	{
		if ( _UIntPool.size() == 0)
		{
			return CodecFactory.createUInt();
		}
		
		return _UIntPool.pop();
	}
	
	static void returnUInt(UInt uint)
	{
		_UIntPool.push(uint);
	}

	static void returnPool(int dataType,Object value)
	{
		if ( dataType == com.thomsonreuters.upa.codec.DataTypes.UNKNOWN )
			return;
		
		switch(dataType)
		{
		case com.thomsonreuters.upa.codec.DataTypes.DATE:
			returnDate((Date)value);
			break;
		case com.thomsonreuters.upa.codec.DataTypes.DATETIME:
			returnDateTime((DateTime)value);
			break;
		case com.thomsonreuters.upa.codec.DataTypes.DOUBLE:
			returnDouble((Double)value);
			break;
		case com.thomsonreuters.upa.codec.DataTypes.ENUM:
			returnEnum((Enum)value);
			break;
		case com.thomsonreuters.upa.codec.DataTypes.FLOAT:
			returnFloat((Float)value);
			break;
		case com.thomsonreuters.upa.codec.DataTypes.INT:
			returnInt((Int)value);
			break;
		case com.thomsonreuters.upa.codec.DataTypes.QOS:
			returnQos((Qos)value);
			break;
		case com.thomsonreuters.upa.codec.DataTypes.REAL:
			returnReal((Real)value);
			break;
		case com.thomsonreuters.upa.codec.DataTypes.BUFFER:
			returnBuffer((Buffer)value);
			break;
		case com.thomsonreuters.upa.codec.DataTypes.STATE:
			returnState((State)value);
			break;
		case com.thomsonreuters.upa.codec.DataTypes.TIME:
			returnTime((Time)value);
			break;
		case com.thomsonreuters.upa.codec.DataTypes.UINT:
			returnUInt((UInt)value);
			break;
		default:
			break;
		}
	}
}
