/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


#include <memory.h>

#include "rtr/rjcbuffer.h"


RJCBuffer::RJCBuffer()
	: _area(0), _count(0), _size(0), _allocated(RTRFALSE)
{
};

RJCBuffer::RJCBuffer(int size) :
	_size(size),
	_count(0),
	_allocated(RTRTRUE)
{
	_area = new char [(unsigned int) size];
};

RJCBuffer::RJCBuffer(const RJCBuffer& other) :
	_count(other.count()),
	_size(other.count()),
	_allocated(RTRTRUE)
{
	_area = new char [(unsigned int) other.count()];
	(void)memcpy(_area, other.to_c(1), (unsigned int)_count);
};

RJCBuffer::RJCBuffer( char *memory, int max_size ) :
	_area(memory),
	_count(0),
	_size(max_size),
	_allocated(RTRFALSE)
{
};


RJCBuffer::~RJCBuffer()
{
	if (_allocated)
		delete [] _area;
};

void RJCBuffer::HexDump(RJCString& aBuffer, int offset)
{
	hex_dump( (_area + offset), (_count - offset),
				aBuffer.to_c(), aBuffer.capacity() );
	int size = ((_count - offset) / 16 + 1) * 60;
	int count = aBuffer.capacity() < size ?
							aBuffer.capacity() : size;
	aBuffer.setCount(count);
};

void RJCBuffer::minimize()
{
	RTPRECONDITION(canResize());
	resize(_count);
};

void RJCBuffer::resize(int newSize)
{
	RTPRECONDITION(canResize());
	char *new_area = new char [(unsigned int) newSize];
	_count = (newSize <= _count) ? newSize : _count;
	(void)memcpy(new_area, _area, (unsigned int) _count);
	delete [] _area;
	_size = newSize;
	_area = new_area;
};

void RJCBuffer::grow(int newSize)
{
	RTPRECONDITION(canResize());
	if (newSize > _size)
		resize(newSize);
};

void RJCBuffer::resizeBuffer(int newSize)
{
	RTPRECONDITION(canResize());
	delete [] _area;
	_area = new char [(unsigned int) newSize];
	_size = newSize;
	_count = (newSize <= _count) ? newSize : _count;
};

RTRBOOL RJCBuffer::operator==(RJCBuffer& other)
{
	if (_count == other.count())
		return (memcmp(other.to_c(1),_area,(unsigned int)_count) == 0) ? RTRTRUE : RTRFALSE;
	else
		return RTRFALSE;
};

RTRBOOL RJCBuffer::isEqual(char *p, int n1, int n2, int position)
{
	int d = memcmp(p, (_area + position), (unsigned int)(n2 - n1 + 1));
	return (d == 0) ? RTRTRUE : RTRFALSE;
};

void RJCBuffer::wipeOut()
{
	clear();
	(void)memset( _area, 0, (unsigned int)_size );
};

void RJCBuffer::copy(RJCBuffer& other, int n1, int n2)
{
	_count = n2 - n1 + 1;
	(void)memcpy(_area, (char*)(other.to_c(1) + n1 - 1), (unsigned int)_count);
};
