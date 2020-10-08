package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Qos;
import com.refinitiv.eta.codec.QosRates;
import com.refinitiv.eta.codec.QosTimeliness;

class QosImpl implements Qos
{
    int     _timeliness;
    int     _rate;
    boolean _dynamic;
    int     _timeInfo;
    int     _rateInfo;
    boolean _isBlank;
    final int SEED = 23;
    final int PRIME = 31;
	
    @Override
    public void clear()
    {
        _timeliness = QosTimeliness.UNSPECIFIED;
        _rate = QosRates.UNSPECIFIED;
        _dynamic = false;
        _timeInfo = 0;
        _rateInfo = 0;
    }
	
    void blank()
    {
        clear();
        _isBlank = true;
    }

    @Override
    public boolean isBlank()
    {
        return _isBlank;
    }
	
    public int copy(Qos destQoS)
    {
        if (null == destQoS)
            return CodecReturnCodes.INVALID_ARGUMENT;

        ((QosImpl)destQoS)._timeliness = _timeliness;
        ((QosImpl)destQoS)._rate = _rate;
        ((QosImpl)destQoS)._dynamic = _dynamic;
        ((QosImpl)destQoS)._timeInfo = _timeInfo;
        ((QosImpl)destQoS)._rateInfo = _rateInfo;
        ((QosImpl)destQoS)._isBlank = _isBlank;

        return CodecReturnCodes.SUCCESS;
    }
 
    @Override
    public int encode(EncodeIterator iter)
    {
        if (!isBlank())
        {
            return Encoders.PrimitiveEncoder.encodeQos((EncodeIteratorImpl)iter, this);
        }
        else
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }
    }

    @Override
    public int decode(DecodeIterator iter)
    {
        return Decoders.decodeQos(iter, this);
    }

    @Override
    public String toString()
    {
        return "Qos: " + QosTimeliness.toString(_timeliness) + "/"
    + QosRates.toString(_rate) + "/"
                + ((isDynamic()) ? "Dynamic" : "Static")
                + " - timeInfo: " + timeInfo() + " - rateInfo: " + rateInfo();
    }

    @Override
    public boolean equals(Qos thatQos)
    {
        return ((thatQos != null) &&
                (_rate == ((QosImpl)thatQos)._rate) &&
                (_timeliness == ((QosImpl)thatQos)._timeliness) &&
                (_rateInfo == ((QosImpl)thatQos)._rateInfo) &&
                (_timeInfo == ((QosImpl)thatQos)._timeInfo));
    }

    @Override
    public boolean isBetter(Qos thatQos)
    {
        assert (thatQos != null) : "thatQos is null";

        // make UNSPECIFIED qos rate/timeliness worse than others
        int unspecifiedValue = 65535 * 2 + 3;

        int newAdjustedRate = adjustRateQos(unspecifiedValue);
        int oldAdjustedRate = ((QosImpl)thatQos).adjustRateQos(unspecifiedValue);

        int newAdjustedTimeliness = adjustTimeQos(unspecifiedValue);
        int oldAdjustedTimeliness = ((QosImpl)thatQos).adjustTimeQos(unspecifiedValue);

        if (newAdjustedTimeliness < oldAdjustedTimeliness)
            return true;
        if (newAdjustedTimeliness > oldAdjustedTimeliness)
            return false;

        if (newAdjustedRate < oldAdjustedRate)
            return true;

        return false;
    }

    @Override
    public boolean isInRange(Qos bestQos, Qos worstQos)
    {
        assert (bestQos != null) : "bestQos is null";
        assert (worstQos != null) : "worstQos is null";

        // make UNSPECIFIED worstQos rate/timeliness worse than others
        int unspecifiedValueWorst = 65535 * 2 + 3;
        // make UNSPECIFIED bestQos rate/timeliness better than others
        int unspecifiedValueBest = -1;

        int bestAdjustedRate = ((QosImpl)bestQos).adjustRateQos(unspecifiedValueBest);
        int worstAdjustedRate = ((QosImpl)worstQos).adjustRateQos(unspecifiedValueWorst);
        int qosAdjustedRate = adjustRateQos(unspecifiedValueBest);
        int bestAdjustedTimeliness = ((QosImpl)bestQos).adjustTimeQos(unspecifiedValueBest);
        int worstAdjustedTimeliness = ((QosImpl)worstQos).adjustTimeQos(unspecifiedValueWorst);
        int qosAdjustedTimeliness = adjustTimeQos(unspecifiedValueBest);

        // in range if best and worst qos rate/timeliness UNSPECIFIED
        if (((QosImpl)bestQos)._rate == QosRates.UNSPECIFIED && ((QosImpl)bestQos)._timeliness == QosTimeliness.UNSPECIFIED
                && ((QosImpl)worstQos)._rate == QosRates.UNSPECIFIED && ((QosImpl)worstQos)._timeliness == QosTimeliness.UNSPECIFIED)
            return true;

        // in range if best and worst qos and this qos are all equal
        if (bestQos.equals(worstQos))
            return bestQos.equals(this);

        // out of range if this qos rate is UNSPECIFIED but best or worst qos rate is not
        if (_rate == QosRates.UNSPECIFIED)
        {
            if ((((QosImpl)bestQos)._rate != QosRates.UNSPECIFIED) || (((QosImpl)worstQos)._rate != QosRates.UNSPECIFIED))
                return false;
        }

        // out of range if this qos timeliness is UNSPECIFIED but best or worst qos timeliness is not
        if (_timeliness == QosTimeliness.UNSPECIFIED)
        {
            if ((((QosImpl)bestQos)._timeliness != QosTimeliness.UNSPECIFIED) || (((QosImpl)worstQos)._timeliness != QosTimeliness.UNSPECIFIED))
                return false;
        }

        return ((bestAdjustedRate <= qosAdjustedRate) && (worstAdjustedRate >= qosAdjustedRate) &&
                (bestAdjustedTimeliness <= qosAdjustedTimeliness) && (worstAdjustedTimeliness >= qosAdjustedTimeliness));
    }

    @Override
    public int timeliness(int timeliness)
    {
        if (!(timeliness >= 0 && timeliness <= 7))
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        _timeliness = timeliness;
        _isBlank = false;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int timeliness()
    {
        return _timeliness;
    }

    @Override
    public int rate(int rate)
    {
        if (!(rate >= 0 && rate <= 15))
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        _rate = rate;
        _isBlank = false;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int rate()
    {
        return _rate;
    }

    @Override
    public void dynamic(boolean dynamic)
    {
        _dynamic = dynamic;
        _isBlank = false;
    }

    @Override
    public boolean isDynamic()
    {
        return _dynamic;
    }

    @Override
    public int timeInfo(int timeInfo)
    {
        if (!(timeInfo >= 0 && timeInfo <= 65535))
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        _timeInfo = timeInfo;
        _isBlank = false;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int timeInfo()
    {
        return _timeInfo;
    }

    @Override
    public int rateInfo(int rateInfo)
    {
        if (!(rateInfo >= 0 && rateInfo <= 65535))
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        _rateInfo = rateInfo;
        _isBlank = false;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int rateInfo()
    {
        return _rateInfo;
    }

    @Override
    public int hashCode()
    {
        int result = SEED;

        result = PRIME * result * (_timeliness + 1);
        result = PRIME * result * (_rate + 2);
        result = PRIME * result * (_timeInfo + 3);
        result = PRIME * result * (_rateInfo + 4);

        return result;
    }

    // Used to determine better/worse qos rates.
    // Smaller returned values mean better qos rates.
    // unspecifiedValue is returned for qos rate of UNSPECIFIED.
    private int adjustRateQos(int unspecifiedValue)
    {
        if (_rate == QosRates.TICK_BY_TICK)
            return 0;
        if (_rate == QosRates.JIT_CONFLATED)
            return 65535 * 2;
        if (_rate == QosRates.TIME_CONFLATED)
        {
            if (_rateInfo == 65535)
                return 65535 * 2 + 1; // max time conflated is worse than JIT conflated
            return _rateInfo * 2 - 1;
        }

        return unspecifiedValue;
    }
	
    // Used to determine better/worse qos timeliness values.
    // Smaller returned values mean better qos timeliness values.
    // unspecifiedValue is returned for qos timeliness value of UNSPECIFIED.
    private int adjustTimeQos(int unspecifiedValue)
    {
        if (_timeliness == QosTimeliness.REALTIME)
            return 0;
        if (_timeliness == QosTimeliness.DELAYED_UNKNOWN)
            return 65536;
        if (_timeliness == QosTimeliness.DELAYED)
            return _timeInfo;

        return unspecifiedValue;
    }
}
