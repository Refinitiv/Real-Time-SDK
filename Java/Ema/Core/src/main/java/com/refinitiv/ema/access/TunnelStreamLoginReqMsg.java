package com.refinitiv.ema.access;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.MsgClasses;

public class TunnelStreamLoginReqMsg 
{
	private OmmInvalidUsageExceptionImpl _ommIUExcept;
	private ReqMsg _loginReqMsg;
	private Buffer _buffer;
	private com.refinitiv.eta.codec.Msg _msg;
	private DecodeIterator _decIter;

	public TunnelStreamLoginReqMsg() 
	{
		_decIter = CodecFactory.createDecodeIterator();
		_buffer = CodecFactory.createBuffer();
	}

	public TunnelStreamLoginReqMsg loginReqMsg(ReqMsg loginReqMsg)
	{
		Utilities.copy(((ReqMsgImpl) loginReqMsg).encodedData(), _buffer);

		return this;
	}

	public ReqMsg loginReqMsg() 
	{
		((ReqMsgImpl) _loginReqMsg).decode(_buffer, Codec.majorVersion(), Codec.minorVersion(), null, null);

		return _loginReqMsg;
	}

	public Buffer buffer() 
	{
		return _buffer;
	}

	public com.refinitiv.eta.codec.Msg rsslMsg()
	{
		_decIter.clear();

		int retCode = _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
		if (CodecReturnCodes.SUCCESS != retCode)
			throw ommIUExcept()
					.message("Internal Error. Failed to set decode iterator version in TunnelStreamLoginReqMsg.msg().", retCode);

		if (_msg != null)
			_msg.clear();
		else
			_msg = CodecFactory.createMsg();
		
		_msg.msgClass(MsgClasses.REQUEST);

		retCode = _msg.decode(_decIter);
		if (CodecReturnCodes.SUCCESS != retCode)
			throw ommIUExcept().message("Internal Error. Failed to decode message in TunnelStreamLoginReqMsg.msg().", retCode);

		return _msg;

	}

	OmmInvalidUsageExceptionImpl ommIUExcept()
	{
		if (_ommIUExcept == null)
			_ommIUExcept = new OmmInvalidUsageExceptionImpl();

		return _ommIUExcept;
	}
}
