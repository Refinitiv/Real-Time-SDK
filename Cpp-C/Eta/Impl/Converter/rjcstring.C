/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "rtr/rjcstring.h"

#ifndef _RDEV_NO_STL_
#include <sstream>
using namespace std;
#endif

RJCString::RJCString()
	: _area(0), _capacity(defaultCapacity), _count(0)
{
	_area = new char [ (unsigned int) defaultCapacity ];
	_area[_count] = '\0';
}

RJCString::RJCString(unsigned int n)
	: _area(0), _capacity(n + 1), _count(0)
{
	_area = new char [ _capacity ];
	_area[_count] = '\0';
}

RJCString::RJCString(char c, unsigned int n)
	: _area(0), _capacity(n + 1), _count(n)
{
	_area = new char [ _capacity ];
	(void)memset(_area, c, _capacity);
	_area[_count] = '\0';
}

		// Null terminate here for compatibility with old version.
RJCString::RJCString(const char *str)
	: _area(0), _capacity(0), _count(0)
{
	if (str)
		_count = (unsigned int) strlen(str);

	_capacity = _count + 1;
	_area = new char [ _capacity ];
	(void)memcpy(_area, str, _count);
	_area[_count] = '\0';
}

		// Null terminate here for compatibility with old version.
RJCString::RJCString(const char *str, unsigned int n)
	: _area(0), _capacity(n + 1), _count(n)
{
	_area = new char [ _capacity ];
	(void)memcpy(_area, str, _count);
	_area[_count] = '\0';
}

RJCString::RJCString(const RJCString& o)
	: _area(0), _capacity(o._capacity), _count(o._count)
{
	_area = new char [ _capacity ];
	(void)memcpy(_area, o._area, _count);
	_area[_count] = '\0';
}

RJCString::~RJCString()
{
	delete [] _area;
}
unsigned long RJCString::hash() const
{
	unsigned long result = 0;
	unsigned long magic = 8388593;			// the greatest prime lower than 2^23

	char *s = _area;
	int n = _count;
	while (n--)
		result = ((result % magic) << 8) + (unsigned int) *s++;
	return result;
}


RJCString& RJCString::set(const char *str, unsigned int p1, unsigned int p2)
{
	RTPRECONDITION ( p1 <= p2 );

	unsigned int n = p2 - p1 + 1;
	if ( n >= _capacity )
	{
		delete [] _area;
		allocate(n + 1);
	}
	(void)memcpy(_area, str + p1, n);
	_count = n;
	_area[_count] = '\0';

	return *this;
}

RJCString& RJCString::set(RJCString& str, unsigned int p1, unsigned int p2)
{
	RTPRECONDITION ( p1 <= p2 );

	unsigned int n = p2 - p1 + 1;
	if ( n >= _capacity )
	{
		delete [] _area;
		allocate(n + 1);
	}
	(void)memcpy(_area, str._area + p1 - 1, n);
	_count = n;
	_area[_count] = '\0';

	return *this;
}

RJCString& RJCString::set(const char *str, unsigned int n)
{
	if ( n >= _capacity )
	{
		delete [] _area;
		allocate(n + 1);
	}
	(void)memcpy(_area, str, n);
	_count = n;
	_area[_count] = '\0';

	return *this;
}

#ifndef _RDEV_NO_STL_
RJCString& RJCString::readLine(istream& istr, RTRBOOL b)
{
	long svf = istr.flags() & ios::skipws;
	if (b || svf)
		istr.setf(ios::skipws);
	istr.getline(_area, _capacity);
	_count = (unsigned int) strlen(_area);
	_area[_count] = '\0';

	return *this;
}
#endif

RJCString& RJCString::fromNumeric(int l)
{
	grow(11);
	_count = sprintf(_area, "%i", l);
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::fromNumeric(unsigned int l)
{
	grow(11);
	_count = sprintf(_area, "%u", l);
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::fromNumeric(long l)
{
	grow(11);
	_count = sprintf(_area, "%li", l);
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::fromNumeric(RTR_I64 l)
{
	grow(21);
	_count = sprintf(_area, RTR_LLD, l);
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::fromNumeric(unsigned long l)
{
	grow(11);
	_count = sprintf(_area, "%lu", l);
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::fromNumeric(double d)
{
	/*
	if ( defaultCapacity > _capacity )
	{
		delete [] _area;
		allocate(defaultCapacity);
	}
	ostringstream strm(_area, _capacity - 1);
	strm << d <<ends;
	strm.flush();
	*/

	char cstr[80];

	_count = sprintf(cstr, "%.14f", d);
	if(_count > 0)
	{
		grow(_count + 1);
		memcpy(_area, cstr, _count);
	}
	else
		_count = 0;
	
	_area[_count] = '\0';

	return *this;
}

char& RJCString::operator[](int i)
{
	RTPRECONDITION ( i >= lower() );
	RTPRECONDITION ( i <= upper() );

	return _area[ i - 1 ];
}

RJCString& RJCString::prepend(const char * str)
{
	if (str == 0)
		return *this;

	int n = (int) strlen(str);
	grow(_count + n + 1);
	int i = _count - 1;
	int j = i + n;
	for (; i >= 0; i--, j--)
		_area[j] = _area[i];
	(void)memcpy(_area, str,(unsigned int)n);
	_count += n;
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::prepend(char c)
{
	grow(_count + 1 + 1);
	int i = _count - 1;
	int j = i + 1;
	for (; i >= 0; i--, j--)
		_area[j] = _area[i];
	_area[0] = c;
	_count++;
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::prepend(unsigned long i)
{
	char *tmp = _area;
	allocate(_count + defaultCapacity);
	fromNumeric(i);
	append(tmp);
	delete [] tmp;
	return *this;
}

RJCString& RJCString::prepend(long i)
{
	char *tmp = _area;
	allocate(_count + defaultCapacity);
	fromNumeric(i);
	append(tmp);
	delete [] tmp;
	return *this;
}

RJCString& RJCString::prepend(double d)
{
	char *tmp = _area;
	allocate(_count + defaultCapacity);
	fromNumeric(d);
	append(tmp);
	delete [] tmp;
	return *this;
}

RJCString& RJCString::append(const char *str)
{
	if (str == 0)
		return *this;

	int n = (int) strlen(str);
	grow(_count + n + 1);
	(void)memcpy(_area + _count, str, (unsigned int)n);
	_count += n;
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::append(const char *str, int n)
{
	grow(_count + n + 1);
	(void)memcpy(_area + _count, str, (unsigned int)n);
	_count += n;
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::append(const RJCString& other)
{
	int n = (int) other.count();
	grow(_count + n + 1);
	(void)memcpy(_area + _count, other._area, (unsigned int)n);
	_count += n;
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::append(const char c)
{
	grow(_count + 1 + 1);
	_area[_count++] = c;
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::append(const unsigned char c)
{
	grow(_count + 1 + 1);
	_area[_count++] = c;
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::append(const short s)
{
	/*
	grow(_count + defaultCapacity);
	ostringstream strm(_area + _count, _capacity - _count - 1);
	strm << s <<ends;
	strm.flush();
	*/
	grow(_count+11);
	_count += sprintf(_area + _count, "%hi", s);
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::append(const unsigned short s)
{
	grow(_count+11);
	_count += sprintf(_area + _count, "%hu", s);
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::append(const int i)
{
	grow(_count+12);
	_count += sprintf(_area + _count, "%i", i);
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::append(const unsigned int i)
{
	grow(_count+12);
	_count += sprintf(_area + _count, "%u", i);
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::append(const long i)
{
// need 11 characters plus \0 == 12 for signed 32 bit
// need 20 characters plus \0 == 21 for signed 64 bit
#ifdef COMPILE_64BITS
	grow(_count+21);
#else
	grow(_count+12);
#endif
	_count += sprintf(_area + _count, "%li", i);
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::append(const unsigned long i)
{
// change in 32 bit case only needed for symmetry with int case
// need 10 characters plus \0 == 11 for unsigned 32 bit
// need 19 characters plus \0 == 20 for unsigned 64 bit
#ifdef COMPILE_64BITS
	grow(_count+21);
#else
	grow(_count+12);
#endif
	_count += sprintf(_area + _count, "%lu", i);
	_area[_count] = '\0';
	return *this;
}


RJCString& RJCString::append(const RTR_I64 i)
{
	/* need 20 characters plus \0 == 21 for unsigned 64 bit */
	grow(_count+21);
	_count += sprintf(_area + _count, RTR_LLD, i);
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::append(const unsigned RTR_I64 i)
{
	/* need 20 characters plus \0 == 21 for unsigned 64 bit */
	grow(_count+21);
	_count += sprintf(_area + _count, RTR_LLU, i);
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::append(const float f)
{
	grow(_count + defaultCapacity);
	_count += sprintf(_area + _count, "%.6f", f);
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::append(const double d)
{
	grow(_count + defaultCapacity);
	_count += sprintf(_area + _count, "%.14f", d);
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::appendHex(const char h)
{
	grow(_count+6);
	_count += sprintf(_area + _count, "0x%02x", h);
	_area[_count] = '\0';
	return *this;
}
RJCString& RJCString::appendHex(const unsigned char h)
{
	grow(_count+6);
	_count += sprintf(_area + _count, "0x%02x", h);
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::appendHex(const short h)
{
	grow(_count+8);
	_count += sprintf(_area + _count, "0x%04x", h);
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::appendHex(const unsigned short h)
{
	grow(_count+8);
	_count += sprintf(_area + _count, "0x%04x", h);
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::appendHex(const int h)
{
// need 11 characters plus \0 == 12 for signed 32 bit
	grow(_count+12);
	_count += sprintf(_area + _count, "0x%08x", h);
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::appendHex(const unsigned int h)
{
// need 11 characters plus \0 == 12 for signed 32 bit
	grow(_count+12);
	_count += sprintf(_area + _count, "0x%08x", h);
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::appendHex(const long h)
{
// need 11 characters plus \0 == 12 for signed 32 bit
// need 20 characters plus \0 == 21 for signed 64 bit
#if !defined(_WIN32) && defined(COMPILE_64BITS)
	grow(_count+21);
	_count += sprintf(_area + _count, "0x%016lx", h);
#else
	grow(_count+12);
	_count += sprintf(_area + _count, "0x%08lx", h);
#endif
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::appendHex(const unsigned long h)
{
// need 11 characters plus \0 == 12 for signed 32 bit
// need 20 characters plus \0 == 21 for signed 64 bit
#if !defined(_WIN32) && defined(COMPILE_64BITS)
	grow(_count+21);
	_count += sprintf(_area + _count, "0x%016lx", h);
#else
	grow(_count+12);
	_count += sprintf(_area + _count, "0x%08lx", h);
#endif
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::appendHex(const RTR_I64 h)
{
	grow(_count+21);
#ifdef _WIN32
	_count += sprintf(_area + _count, "0x%016I64x", h);
#else
	_count += sprintf(_area + _count, "0x%016llx", h);
#endif
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::appendHex(const unsigned RTR_I64 h)
{
	grow(_count+21);
#ifdef _WIN32
	_count += sprintf(_area + _count, "0x%016I64x", h);
#else
	_count += sprintf(_area + _count, "0x%016llx", h);
#endif
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::toLower()
{
	for (unsigned int i = 0; i < _count; i++)
		_area[i] = tolower(_area[i]);
	return *this;
}

RJCString& RJCString::toUpper()
{
	for (unsigned int i = 0; i < _count; i++)
		_area[i] = toupper(_area[i]);
	return *this;
}

void RJCString::leftAdjust()
{
	unsigned int i;
#ifndef _WIN32
	for (i = 0; i < _count && isspace(_area[i]); i++)
		;
#else
	for (i = 0; i < _count && (_area[i] > 0) && (_area[i] < 0x21) && isspace(_area[i]); i++)
		;
#endif
	unsigned int j;
	for (j = 0; i < _count; i++, j++)
		_area[j] = _area[i];
	_count = j;
	_area[_count] = '\0';
}

void RJCString::rightAdjust()
{
	for (int i = _count - 1; i >= 0; i--)
	{
#ifndef _WIN32
		if (isspace(_area[i]))
#else
		if ((_area[i] > 0) && (_area[i] < 0x21) && isspace(_area[i]))
#endif
			_count--;
		else
			break;
	}
	_area[_count] = '\0';
}

RJCString& RJCString::head(unsigned int n)
{
	if ( _count > n )
	{
		_count = n;
		_area[_count] = '\0';
	}

	RTPOSTCONDITION ( count() <= n );
	return *this;
}

RJCString& RJCString::tail(unsigned int n)
{
	unsigned int j = 0;
	for (unsigned int i = _count - n; i < _count; i++, j++)
		_area[j] = _area[i];
	_count = n;
	_area[_count] = '\0';

	RTPOSTCONDITION ( count() == n );
	return *this;
}

int RJCString::compare(const char *str) const
{
	return strcmp(*this, str);
}

char RJCString::operator[](int i) const
{
	RTPRECONDITION ( i >= lower() );
	RTPRECONDITION ( i <= upper() );

	return _area[ i - 1 ];
}

RJCString::operator const char *() const
{
	if (_area[_count] != '\0')
	{
		((RJCString *)this)->grow(_count + 1);
		_area[_count] = '\0';
	}
	return _area;
}


RJCString RJCString::subString(int p1, int p2)
{
	RTPRECONDITION ( p1 >= lower() );
	RTPRECONDITION ( p2 <= upper() );
	RTPRECONDITION ( p1 <= p2 );

	return RJCString(_area + p1 - 1, p2 - p1 + 1);
}

RTRBOOL RJCString::contains(const char *str) const
{
	if (str == 0)
		return RTRFALSE;

	RTRBOOL result = RTRFALSE;
	size_t n = strlen(str);
	if ( _count < n )
		return result;
	for (unsigned int i = 0; (i <= _count - n) && !result; i++)
		if (!memcmp(_area + i, str, n))
				result = RTRTRUE;
	return result;
}

RTRBOOL RJCString::contains(const char c) const
{
	unsigned int i;
	for (i = 0; i < _count; i++)
		if (_area[i] == c) break;
	return i < _count;
}

int	RJCString::indexOf(char c, int p1)
{
	RTPRECONDITION ( p1 >= lower() );
	RTPRECONDITION ( p1 <= upper() );

	int i;
	for (i = p1 - 1; i < (int)_count; i++)
		if (_area[i] == c) break;

	int result = ( i < (int)_count ) ? i + 1 : -1;

	RTPOSTCONDITION ( (result == -1) || (result >= lower()) == (operator[](result) == c) );
	return result;
}

int RJCString::toInteger() const
{
	return atoi((const char *)(*this));
}

float RJCString::toFloat() const
{
	return (float)atof((const char *)(*this));
}

double RJCString::toDouble() const
{
	return strtod((const char *)(*this), 0);
}

RTRBOOL RJCString::toBoolean() const
{
	RJCString tmp((const char *)(*this));
	tmp.toLower();
	return (tmp == trueString);
}

RJCString& RJCString::operator=(const char *str)
{
	if (str != _area)
	{
		if (str)
		{
			int n = (int) strlen(str);
			set(str, n);
		}
		else
		{
			_count = 0;
			_area[_count] = '\0';
		}
	}
	return *this;
}

RJCString& RJCString::operator=(const RJCString& other)
{
	const char *str = other._area;
	if (other._area != _area)
	{
		if (other._area && other.count())
			set(other._area, other.count());
		else
		{
			_count = 0;
			_area[_count] = '\0';
		}
	}
	return *this;
}

RTRBOOL RJCString::operator==(const char *str) const
{
	return !strcmp(_area, str);
}

RTRBOOL RJCString::operator==(const RJCString& other) const
{
	return !strcmp(_area, other._area);
}

RTRBOOL RJCString::operator!=(const char *str) const
{
	return (strcmp(_area, str) != 0);
}

RTRBOOL RJCString::operator!=(const RJCString& other) const
{
	return (strcmp(_area, other) != 0);
}

RTRBOOL RJCString::operator>(const char *str) const
{
	return (strcmp(_area, str) > 0);
}

RTRBOOL RJCString::operator>(const RJCString& other) const
{
	return (strcmp(*this, other) > 0);
}

RTRBOOL RJCString::operator>=(const char *str) const
{
	return !operator<(str);
}

RTRBOOL RJCString::operator>=(const RJCString& other) const
{
	return !operator<(other);
}

RTRBOOL RJCString::operator<(const char *str) const
{
	return (strcmp(*this, str) < 0);
}

RTRBOOL RJCString::operator<(const RJCString& other) const
{
	return (strcmp(*this, other) < 0);
}

RTRBOOL RJCString::operator<=(const char *str) const
{
	return !operator>(str);
}

RTRBOOL RJCString::operator<=(const RJCString& other) const
{
	return !operator>(other);
}

RJCString& RJCString::operator+=(const char *str)
{
	if (str == 0)
		return *this;

	int n = (int) strlen(str);
	grow(_count + n + 1);
	(void)memcpy(_area + _count, str, (unsigned int)n);
	_count += n;
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::operator+=(const RJCString& other)
{
	int n = other.count();
	grow(_count + n + 1);
	(void)memcpy(_area + _count, other._area, (unsigned int)n);
	_count += n;
	_area[_count] = '\0';
	return *this;
}

RJCString& RJCString::operator+=(const char c)
{
	grow(_count + 1 + 1);
	_area[_count] = c;
	_count += 1;
	_area[_count] = '\0';
	return *this;
}

void RJCString::grow(unsigned int n)
{
	if (_capacity <= n) 
		reallocate(n);
	RTPOSTCONDITION ( capacity() >= n );
}

void RJCString::trim(unsigned int n)
{
	int c = ( _count <= n ) ? _count : n;
	char *old = _area;
	allocate(n + 1);
	(void)memcpy(_area, old, (unsigned int)c);
	delete [] old;
	_count = c;
	_area[_count] = '\0';

	RTPOSTCONDITION ( capacity() <= n );
	RTPOSTCONDITION ( count() <= n );
}

void RJCString::setCount(unsigned int i)
{
	RTPRECONDITION ( i <= capacity() );
	_count = i;
	_area[_count] = '\0';
}

RJCString::RJCString(const char *str, int n)
	: _area(0), _capacity(n + 1), _count(n)
{
	_area = new char [ _capacity ];
	(void)memcpy(_area, str, _count);
	_area[_count] = '\0';
}

RTRBOOL RJCString::isEqual(const char *str) const
{
	return compare(str) == 0;
}

int	RJCString::index(char c, int start)
{
	return indexOf(c, start);
}

RJCString& RJCString::fromInteger(int i)
{
	fromNumeric(i);
	return *this;
}

void RJCString::allocate(unsigned int n)
{
	_capacity = n;
	_area = new char [ _capacity ];
}

void RJCString::reallocate(unsigned int n)
{
	char *old = _area;
	allocate(n + 1);
	(void)memcpy(_area, old, _count);
	_area[_count] = '\0';
	delete [] old;
}

#ifndef _RDEV_NO_STL_
ostream& operator<<(ostream& os, const RJCString& s)
{
	os << (const char*)s;
	return os;
}
#else
void RJCString::print(FILE* os)
{
	fprintf(os, to_c());
}
#endif

int RJCString::defaultCapacity = 80;
char *RJCString::trueString = (char *)"true";
