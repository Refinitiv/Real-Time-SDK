/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2017,2019-2020,2022-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "Encoder.h"
#include "GlobalPool.h"

using namespace refinitiv::ema::access;

Encoder::Encoder() :
 _pEncodeIter( 0 ),
 _pEncodeIterCached( 0 ),
 _iteratorOwner( 0 ),
 _containerComplete( false )
{
}

Encoder::~Encoder()
{
	releaseEncIterator();
}

void Encoder::acquireEncIterator( UInt32 allocatedSize )
{
	if ( !_pEncodeIter )
	{
		if ( _pEncodeIterCached )
		{
			_pEncodeIter = _pEncodeIterCached;
			_pEncodeIterCached = 0;
		}
		else
		{
			_pEncodeIter = g_pool.getEncodeIteratorItem();
		}
	
		_iteratorOwner = this;

		_pEncodeIter->clear( allocatedSize );
	}
}

void Encoder::releaseEncIterator()
{
	if ( _pEncodeIter )
	{
		if ( _iteratorOwner == this )
			g_pool.returnItem( _pEncodeIter );
	
		_pEncodeIter = 0;
		_pEncodeIterCached = 0;
		_iteratorOwner = 0;
		_containerComplete = false;
	}
	else if ( _pEncodeIterCached )
	{
		g_pool.returnItem( _pEncodeIterCached );
		_pEncodeIterCached = 0;
	}
}

void Encoder::clearEncIterator()
{
	if (_pEncodeIter)
	{
		if (_iteratorOwner == this)
		{
			_pEncodeIterCached = _pEncodeIter;
		}

		_pEncodeIter = 0;
		_iteratorOwner = 0;
		_containerComplete = false;
	}
	else if ( _pEncodeIterCached )
	{
		g_pool.returnItem( _pEncodeIterCached );
		_pEncodeIterCached = 0;
	}
}

bool Encoder::ownsIterator() const
{
	return _iteratorOwner == this ? true : false;
}

void Encoder::passEncIterator( Encoder& other )
{
	other._pEncodeIter = _pEncodeIter;
	other._iteratorOwner = this;
}

void Encoder::clear()
{
	if ( _pEncodeIter )
		_pEncodeIter->clear();
}

RsslBuffer& Encoder::getRsslBuffer() const
{
	return _pEncodeIter->_rsslEncBuffer1.data ? _pEncodeIter->_rsslEncBuffer1 : _pEncodeIter->_rsslEncBuffer2;
}

RsslUInt8 Encoder::getMajVer() const
{
	return _pEncodeIter->_rsslMajVer;
}

RsslUInt8 Encoder::getMinVer() const
{
	return _pEncodeIter->_rsslMinVer;
}

bool Encoder::isComplete() const
{
	return _containerComplete;
}

bool Encoder::hasEncIterator() const
{
	return _pEncodeIter != 0 ? true : false;
}

UInt8 Encoder::convertDataType( DataType::DataTypeEnum dType ) const
{
	switch ( dType )
	{
	case DataType::ReqMsgEnum :
	case DataType::RefreshMsgEnum :
	case DataType::UpdateMsgEnum :
	case DataType::StatusMsgEnum :
	case DataType::PostMsgEnum :
	case DataType::AckMsgEnum :
	case DataType::GenericMsgEnum :
		return RSSL_DT_MSG;
	default:
		return dType;
	}
}
