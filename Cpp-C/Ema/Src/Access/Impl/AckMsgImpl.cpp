/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "AckMsgImpl.h"

#include "ExceptionTranslator.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

AckMsgImpl::AckMsgImpl() :
 MsgImpl(),
 _decoder(this),
 _textData(),
 _extHeaderData()
{
	clearRsslAckMsg();
}

AckMsgImpl& AckMsgImpl::operator=(const AckMsgImpl& other)
{
	if (this == &other)
		return *this;

	resetRsslInt();

	copyMsgBaseFrom(other, sizeof(RsslAckMsg));

	rsslClearBuffer(&(getRsslAckMsg()->text));
	rsslClearBuffer(&(getRsslAckMsg()->extendedHeader));

	// adjust RsslBuffer values of the rssl msg that are specific only to the Ack msg
	RsslAckMsg& dstMsg = *getRsslAckMsg();
	const RsslAckMsg& srcMsg = *other.getRsslAckMsg();

	// text
	if (other.hasText())
	{
		if (! memberCopyInPlace<AckMsgImpl>(&RsslAckMsg::text, dstMsg, srcMsg))
		{
			setText(other.getText());
		}
	}

	// extendedHeader
	if (other.hasExtendedHeader())
	{
		if (! memberCopyInPlace<AckMsgImpl>(&RsslAckMsg::extendedHeader, dstMsg, srcMsg))
		{
			setExtendedHeader(other.getExtendedHeader());
		}
	}

	return *this;
}

AckMsgImpl::~AckMsgImpl()
{
}

void AckMsgImpl::applyHasMsgKey()
{
	rsslAckMsgApplyHasMsgKey( getRsslAckMsg() );
}

bool AckMsgImpl::hasSeqNum() const
{
	return rsslAckMsgCheckHasSeqNum( getRsslAckMsg() ) == RSSL_TRUE
		   ? true : false;
}

bool AckMsgImpl::hasNackCode() const
{
	return rsslAckMsgCheckHasNakCode( getRsslAckMsg() ) == RSSL_TRUE
		   ? true : false;
}

bool AckMsgImpl::hasText() const
{
	return rsslAckMsgCheckHasText( getRsslAckMsg() ) == RSSL_TRUE
		   ? true : false;
}

UInt32 AckMsgImpl::getAckId() const
{
	return getRsslAckMsg()->ackId;
}

UInt32 AckMsgImpl::getSeqNum() const
{
	if ( !hasSeqNum() )
	{
		EmaString temp( "Attempt to getSeqNum() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return getRsslAckMsg()->seqNum;
}

UInt8 AckMsgImpl::getNackCode() const
{
	if ( !hasNackCode() )
	{
		EmaString temp( "Attempt to getNackCode() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return getRsslAckMsg()->nakCode;
}

const EmaString& AckMsgImpl::getText() const
{
	if ( !hasText() )
	{
		EmaString temp( "Attempt to getText() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_text.setInt( getRsslAckMsg()->text.data, getRsslAckMsg()->text.length, false );

	return _text.toString();
}

bool AckMsgImpl::getPrivateStream() const
{
	return rsslAckMsgCheckPrivateStream( getRsslAckMsg() ) == RSSL_TRUE
		   ? true : false;
}

void AckMsgImpl::clearRsslAckMsg()
{
	rsslClearAckMsg( getRsslAckMsg() );
	getRsslMsg()->msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	getRsslMsg()->msgBase.containerType = RSSL_DT_NO_DATA;
}

void AckMsgImpl::clear()
{
	clearBaseMsgImpl();
	clearRsslAckMsg();
}

void AckMsgImpl::releaseMsgBuffers()
{
	_extHeaderData.release();
}

bool AckMsgImpl::hasExtendedHeader() const
{
	return rsslAckMsgCheckHasExtendedHdr( getRsslAckMsg() ) == RSSL_TRUE
		   ? true : false;
}

const EmaBuffer& AckMsgImpl::getExtendedHeader() const
{
	if ( !hasExtendedHeader() )
	{
		EmaString temp( "Attempt to getExtendedHeader() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_extHeader.setFromInt( getRsslAckMsg()->extendedHeader.data, getRsslAckMsg()->extendedHeader.length );

	return _extHeader.toBuffer();
}

void AckMsgImpl::setExtendedHeader(const EmaBuffer& buffer)
{
	if (buffer.length() == 0)
	{
		rsslClearBuffer(&getRsslAckMsg()->extendedHeader);
		applyHasNoFlag(getRsslAckMsg(), RSSL_AKMF_HAS_EXTENDED_HEADER);
	}
	else
	{
		if (! _arena.trySave(getRsslAckMsg()->extendedHeader, buffer))
		{
			_extHeaderData = buffer;

			getRsslAckMsg()->extendedHeader.data = (char*)_extHeaderData.c_buf();
			getRsslAckMsg()->extendedHeader.length = _extHeaderData.length();
		}

		rsslAckMsgApplyHasExtendedHdr( getRsslAckMsg() );
	}
}

void AckMsgImpl::setAckId( UInt32 ackId )
{
	getRsslAckMsg()->ackId = ackId;
}

void AckMsgImpl::setSeqNum( UInt32 seqNum )
{
	getRsslAckMsg()->seqNum = seqNum;

	rsslAckMsgApplyHasSeqNum( getRsslAckMsg() );
}

void AckMsgImpl::setNackCode( UInt8 nackCode )
{
	getRsslAckMsg()->nakCode = nackCode;

	rsslAckMsgApplyHasNakCode(getRsslAckMsg());
}

void AckMsgImpl::setText( const EmaString& text )
{
	if (text.length() == 0)
	{
		rsslClearBuffer(&getRsslAckMsg()->text);
		applyHasNoFlag(getRsslAckMsg(), RSSL_AKMF_HAS_TEXT);
	}
	else
	{
		if (! _arena.trySave(getRsslAckMsg()->text, text))
		{
			_textData = text;

			getRsslAckMsg()->text.data = ( char* )_textData.c_str();
			getRsslAckMsg()->text.length = _textData.length();
		}

		rsslAckMsgApplyHasText( getRsslAckMsg() );
	}
}

void AckMsgImpl::setPrivateStream( bool privateStream )
{
	if ( privateStream )
		getRsslAckMsg()->flags |= RSSL_AKMF_PRIVATE_STREAM;
	else
		applyHasNoFlag(getRsslAckMsg(), RSSL_AKMF_PRIVATE_STREAM);
}
