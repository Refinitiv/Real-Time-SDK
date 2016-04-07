///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;


class OmmQosImpl extends DataImpl implements OmmQos
{
	com.thomsonreuters.upa.codec.Qos _rsslQos = com.thomsonreuters.upa.codec.CodecFactory.createQos();

	@Override
	public int dataType()
	{
		return DataType.DataTypes.QOS;
	}

	@Override
	public String rateAsString()
	{
		_toString.setLength(0);
		
		switch(_rsslQos.rate())
		{
		case com.thomsonreuters.upa.codec.QosRates.TICK_BY_TICK:
			_toString.append("TickByTick");
			break;
		case com.thomsonreuters.upa.codec.QosRates.JIT_CONFLATED:
			_toString.append("JustInTimeConflated");
			break;
		default:
			return _toString.append("Rate: ").append(_rsslQos.rateInfo()).toString();
		}
		
		return _toString.toString();
	}

	@Override
	public String timelinessAsString()
	{
		_toString.setLength(0);
		
		switch(_rsslQos.timeliness())
		{
		case com.thomsonreuters.upa.codec.QosTimeliness.REALTIME:
			_toString.append("RealTime");
			break;
		case com.thomsonreuters.upa.codec.QosTimeliness.DELAYED_UNKNOWN:
			_toString.append("InexactDelayed");
			break;
		default:
			return _toString.append("Timeliness: ").append(_rsslQos.timeInfo()).toString();
		}
		
		return _toString.toString();
	}

	@Override
	public int timeliness()
	{
		int timeliness = OmmQos.Timeliness.INEXACT_DELAYED;

		switch( _rsslQos.timeliness() )
		{
		case com.thomsonreuters.upa.codec.QosTimeliness.REALTIME:
			timeliness = OmmQos.Timeliness.REALTIME;
			break;
		case com.thomsonreuters.upa.codec.QosTimeliness.DELAYED_UNKNOWN :
			timeliness = OmmQos.Timeliness.INEXACT_DELAYED;
			break;
		case com.thomsonreuters.upa.codec.QosTimeliness.DELAYED:
			timeliness = _rsslQos.timeInfo();
			break;
		}

		return timeliness;
	}

	@Override
	public int rate()
	{
		int rate = OmmQos.Rate.JUST_IN_TIME_CONFLATED;

		switch( _rsslQos.rate() )
		{
		case com.thomsonreuters.upa.codec.QosRates.TICK_BY_TICK:
			rate = OmmQos.Rate.TICK_BY_TICK;
			break;
		case com.thomsonreuters.upa.codec.QosRates.JIT_CONFLATED:
			rate = OmmQos.Rate.JUST_IN_TIME_CONFLATED;
			break;
		case com.thomsonreuters.upa.codec.QosRates.TIME_CONFLATED:
			rate = _rsslQos.rateInfo();
			break;
		}

		return rate;
	}

	@Override
	public String toString()
	{
		if (DataCode.BLANK == code())
			return BLANK_STRING;
		
		_toString.setLength(0);
		
		switch(_rsslQos.timeliness())
		{
		case com.thomsonreuters.upa.codec.QosTimeliness.REALTIME:
			_toString.append("RealTime");
			break;
		case com.thomsonreuters.upa.codec.QosTimeliness.DELAYED_UNKNOWN:
			_toString.append("InexactDelayed");
			break;
		case com.thomsonreuters.upa.codec.QosTimeliness.DELAYED:
			_toString.append("Timeliness: ").append(_rsslQos.timeInfo());
			break;
		}

		_toString.append("/");

		switch(_rsslQos.rate())
		{
		case com.thomsonreuters.upa.codec.QosRates.TICK_BY_TICK:
			_toString.append("TickByTick");
			break;
		case com.thomsonreuters.upa.codec.QosRates.JIT_CONFLATED:
			_toString.append("JustInTimeConflated");
			break;
		case com.thomsonreuters.upa.codec.QosRates.TIME_CONFLATED:
			_toString.append("Rate: ").append(_rsslQos.rateInfo());
			break;
		}
		
		return _toString.toString();
	}
	
	@Override
	void decode(com.thomsonreuters.upa.codec.Buffer rsslBuffer, com.thomsonreuters.upa.codec.DecodeIterator dIter)
	{
		_rsslBuffer = rsslBuffer;

		if (com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS == _rsslQos.decode(dIter))
			_dataCode = DataCode.NO_CODE;
		else
		{
			_dataCode = DataCode.BLANK;
			_rsslQos.clear();
		}
	}
	
	void decode(com.thomsonreuters.upa.codec.Qos rsslQos)
	{
		if (rsslQos != null)
		{
			_dataCode = DataCode.NO_CODE;
			
			_rsslQos.rate(rsslQos.rate());
			_rsslQos.timeliness(rsslQos.timeliness());
			_rsslQos.dynamic(rsslQos.isDynamic());
			_rsslQos.timeInfo(rsslQos.timeInfo());
			_rsslQos.rateInfo(rsslQos.rateInfo());
		}
		else
		{
			_dataCode = DataCode.BLANK;
			
			_rsslQos.clear();
		}
	}
	
	void clear()
	{
		_dataCode = DataCode.NO_CODE;
		_rsslQos.clear();
	}
}