package com.rtsdk.ema.access;

public class CosFlowControlImpl implements CosFlowControl 
{

	int _type = CosFlowControl.CosFlowControlType.NONE;
	int _recvWindowSize = -1;
	int _sendWindowSize = -1;
	private OmmOutOfRangeExceptionImpl	_ommOORExcept;

	@Override
	public CosFlowControl clear()
	{
		_type = CosFlowControl.CosFlowControlType.NONE;
		_recvWindowSize = -1;
		_sendWindowSize = -1;

		return this;
	}

	@Override
	public CosFlowControl type(int type)
	{
		if (type < CosFlowControl.CosFlowControlType.NONE || type > CosFlowControl.CosFlowControlType.BIDIRECTIONAL)
			throw ommOORExcept().message("Passed in CosFlowControl Type is not supported.");

		_type = type;
		return this;

	}

	@Override
	public CosFlowControl recvWindowSize(int size)
	{
		if (size < -1)
			throw ommOORExcept().message("Passed int received window size is out of valid range.");

		_recvWindowSize = size;

		return this;
	}

	@Override
	public int type() 
	{
		return _type;
	}

	@Override
	public int recvWindowSize() 
	{
		return _recvWindowSize;
	}

	@Override
	public int sendWindowSize() 
	{
		return _sendWindowSize;
	}

	OmmOutOfRangeExceptionImpl ommOORExcept()
	{
		if (_ommOORExcept == null)
			_ommOORExcept = new OmmOutOfRangeExceptionImpl();

		return _ommOORExcept;
	}
}
