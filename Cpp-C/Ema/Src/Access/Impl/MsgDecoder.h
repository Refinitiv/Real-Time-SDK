/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_MsgDecoder_h
#define __refinitiv_ema_access_MsgDecoder_h

#include "rtr/rsslMsg.h"

#include "Decoder.h"
#include "StaticDecoder.h"
#include "Utilities.h"

namespace refinitiv {

namespace ema {

namespace access {

class MsgImpl;

/// Decodes provided message buffer or configuring the associated Ema message with the
/// provided RsslMsg
template<class MsgImplT>
class MsgDecoder final : public Decoder
{
public :

	MsgDecoder(MsgImplT* pMsgImpl);

	virtual ~MsgDecoder() { };

	const RsslBuffer& getRsslBuffer() const final;

	/// Initialize associated _pMsgImpl with the provided rsslMsg
	bool setRsslData(UInt8 majVer, UInt8 minVer, RsslMsg* rsslMsg, const RsslDataDictionary* rsslDictionary) final;

	/// Decode provided message buffer rsslBuffer into associated _pMsgImpl
	bool setRsslData(UInt8 majVer, UInt8 minVer, RsslBuffer* rsslBuffer, const RsslDataDictionary* rsslDictionary, void*) final;

	bool setRsslData(RsslDecodeIterator*, RsslBuffer*) final
	{
		_errorCode = OmmError::UnknownErrorEnum;
		return false;
	}

	OmmError::ErrorCode getErrorCode() const final
	{
		return _errorCode;
	};

private :

	MsgImplT* _pMsgImpl;

	OmmError::ErrorCode _errorCode;
};

template<class MsgImplT>
MsgDecoder<MsgImplT>::MsgDecoder(MsgImplT* pMsgImpl) :
  Decoder(),
  _pMsgImpl(pMsgImpl),
  _errorCode( OmmError::NoErrorEnum )
{
};

template<class MsgImplT>
const RsslBuffer& MsgDecoder<MsgImplT>::getRsslBuffer() const
{
	// return getEncodedBuffer();
	return _pMsgImpl->_pRsslMsg->msgBase.encMsgBuffer;
};

template<class MsgImplT>
bool MsgDecoder<MsgImplT>::setRsslData(UInt8 majVer, UInt8 minVer, RsslMsg* rsslMsg, const RsslDataDictionary* rsslDictionary)
{
	EMA_ASSERT(MsgImplT::RSSL_MSG_CLASS == rsslMsg->msgBase.msgClass, "RsslMsg msgClass must match that of the EMA Message");

	_pMsgImpl->_pRsslMsg = rsslMsg;
	_pMsgImpl->_pRsslDictionary = rsslDictionary;

	_pMsgImpl->_rsslMajVer = majVer;
	_pMsgImpl->_rsslMinVer = minVer;

	_pMsgImpl->_serviceNameSet = false;
	_pMsgImpl->_serviceListNameSet = false;

	// clear flags for cached OmmQos and OmmState "views"
	_pMsgImpl->resetRsslInt();

	StaticDecoder::setRsslData( _pMsgImpl->_attrib, &_pMsgImpl->_pRsslMsg->msgBase.msgKey.encAttrib,
								_pMsgImpl->template hasAttrib<const MsgImplT>()
								? rsslMsg->msgBase.msgKey.attribContainerType
								: RSSL_DT_NO_DATA,
								majVer, minVer, rsslDictionary );

	StaticDecoder::setRsslData( _pMsgImpl->_payload, &_pMsgImpl->_pRsslMsg->msgBase.encDataBody,
								rsslMsg->msgBase.containerType,
								majVer, minVer, rsslDictionary );

	_errorCode = OmmError::NoErrorEnum;

	return true;
};

template<class MsgImplT>
bool MsgDecoder<MsgImplT>::setRsslData(UInt8 majVer, UInt8 minVer, RsslBuffer* rsslBuffer, const RsslDataDictionary* rsslDictionary, void*)
{

	_pMsgImpl->_pRsslMsg = &_pMsgImpl->_rsslMsg;
	_pMsgImpl->_pRsslDictionary = rsslDictionary;

	_pMsgImpl->_rsslMajVer = majVer;
	_pMsgImpl->_rsslMinVer = minVer;

	_pMsgImpl->_serviceNameSet = false;
	_pMsgImpl->_serviceListNameSet = false;

	// clear flags for cached OmmQos and OmmState "views"
	_pMsgImpl->resetRsslInt();

	RsslDecodeIterator decodeIter;
	rsslClearDecodeIterator(&decodeIter);

	RsslRet retCode = rsslSetDecodeIteratorBuffer(&decodeIter, rsslBuffer);
	if (RSSL_RET_SUCCESS != retCode)
	{
		_errorCode = OmmError::IteratorSetFailureEnum;
		return false;
	}

	retCode = rsslSetDecodeIteratorRWFVersion(&decodeIter, _pMsgImpl->_rsslMajVer, _pMsgImpl->_rsslMinVer);
	if (RSSL_RET_SUCCESS != retCode)
	{
		_errorCode = OmmError::IteratorSetFailureEnum;
		return false;
	}

	retCode = rsslDecodeMsg(&decodeIter, _pMsgImpl->_pRsslMsg);

	switch (retCode)
	{
	case RSSL_RET_SUCCESS:
		_errorCode = OmmError::NoErrorEnum;

		StaticDecoder::setRsslData( _pMsgImpl->_attrib, &_pMsgImpl->_pRsslMsg->msgBase.msgKey.encAttrib,
									_pMsgImpl->template hasAttrib<const MsgImplT>() ? _pMsgImpl->_pRsslMsg->msgBase.msgKey.attribContainerType : RSSL_DT_NO_DATA,
									majVer, minVer, _pMsgImpl->_pRsslDictionary );

		StaticDecoder::setRsslData( _pMsgImpl->_payload, &_pMsgImpl->_pRsslMsg->msgBase.encDataBody,
									_pMsgImpl->_pRsslMsg->msgBase.containerType,
									majVer, minVer, _pMsgImpl->_pRsslDictionary );

		return true;
	case RSSL_RET_ITERATOR_OVERRUN:
		_errorCode = OmmError::IteratorOverrunEnum;
		return false;
	case RSSL_RET_INCOMPLETE_DATA:
		_errorCode = OmmError::IncompleteDataEnum;
		return false;
	default:
		_errorCode = OmmError::UnknownErrorEnum;
		return false;
	}
};


}

}

}

#endif //__refinitiv_ema_access_MsgDecoder_h
