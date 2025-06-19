/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __RJC_BUFFER
#define __RJC_BUFFER

#ifndef _RDEV_NO_STL_
#include <iostream>
#else
#include <stdio.h>
#endif

#include "rtr/rtrdefs.h"
#include "rtr/rjcstring.h"
#include "rtr/rjchexdmp.h"

// Synopsis:
//
// Description:
//
// See Also:
//

class RJCBuffer :
	public RJCHexDump
{


#ifndef _RDEV_NO_STL_
friend  std::ostream& operator<<(std::ostream& os, RJCBuffer& bbuf)
	// Perform a hex dump of the data stored in the buffer.
{
	RJCString outstring( 5000 );
	bbuf.HexDump( outstring );
	os << outstring.to_c();
	return os;
};
#else
public:
void print(FILE* os)
{
	RJCString outstring( 5000 );
	HexDump( outstring );
	fprintf(os, "%s", outstring.to_c());
};
#endif

protected:

	char	*_area;
	int		_size;
	int		_count;
	RTRBOOL	_allocated;


public:


// Constructor
	RJCBuffer();
		// Create an empty null buffer.
		// _TAG Constructor

	RJCBuffer( int buf_size );
		// _TAG Constructor

	RJCBuffer( const RJCBuffer& );
		// _TAG Constructor

	RJCBuffer( char *memory, int max_size );
		// Construct a buffer using memory
		// for internal storage.
		// _TAG Constructor

// Destructor
	virtual ~RJCBuffer();
		// _TAG Destructor

// Output
	void HexDump(RJCString& aBuffer, int offset=0);
		// _TAG01 Output

// Storage
	inline int capacity();
		// In bytes
		// _TAG02 Storage

	void minimize();
		// Adapt size to accomodate "exactly" count() bytes (or as close as 
		// practical).
		// Copy Data from old buffer to new buffer.
		// REQUIRE : canResize()
        // _TAG02 Storage

	void resize(int newSize);
		// Adapt size to accomodate "exactly" newSize bytes (or as close as 
		// practical). If newSize is < count() truncation will occur.
		// Copy Data from old buffer to new buffer.
		// REQUIRE : canResize()
        // _TAG02 Storage

	void grow(int newSize);
		// Process a hint that newSize bytes may be required.
		// Copy Data from old buffer to new buffer.
		// REQUIRE : canResize()
        // _TAG02 Storage

	void resizeBuffer(int newSize);
		// Adapt size to accomodate "exactly" newSize bytes (or as close as 
		// practical).
		// Data is NOT copied from old buffer.
		// REQUIRE : canResize()
        // _TAG02 Storage


// Comparison
	RTRBOOL operator==(RJCBuffer&);
		// Is other byte-wise identical to Current?
		// _TAG03 Comparison

	RTRBOOL isEqual(char *, int n1, int n2, int position);
		// Are contents of the Current, starting from position, 
		// equal to the contents of other between positions n1 and n2?
		// _TAG03 Comparison


// Transform
	void wipeOut();
		// Re-initialize buffer 
		// This includes zeroing out the buffer memory
		// ENSURE: count() == 0 && readCursor() == 0;
		// _TAG04 Transform

	inline void clear();
		// Clear out the buffer
		// ENSURE: count() == 0 && readCursor() == 0;
        // _TAG04 Transform
	
	void copy(RJCBuffer& other, int n1, int n2);
		// Initialize Current to the contents of other between positions
		// n1 and n2.
		// REQUIRE : n1 >= 1; n1 <= n2; n2 <= other.count()
		// ENSURE : isEqual(other.to_c(1), n1, n2, 1)
        // _TAG04 Transform

// State
	inline int count() const;
		// Number of bytes currently stored.
		// _TAG05 State

	inline RTRBOOL canResize() const;
		// Can the current buffer be resized?
		// _TAG05 State

// Access
	inline char *to_c( int position ) const;
		// Access to character position.
		// TAG06 Access

	inline operator const char *() const;
		// _TAG06 Access

// Setting
	inline void set_count( int new_count );
		// Set the number of bytes currently stored.
		// REQUIRE: new_count <= capacity();
		// _TAG07 Setting

	inline void setMemory(char *mem,int maxSize);
		// Set the memory to be used by this
		// buffer for storage.
		// REQUIRE : !canResize()

};



inline int RJCBuffer::capacity()
{
	return _size;
};

inline void RJCBuffer::clear()
{
	_count = 0;
};

inline int RJCBuffer::count() const
{
	return(_count);
};

inline void RJCBuffer::set_count(int new_size)
{
	RTPRECONDITION( new_size <= capacity() );

	_count = new_size;

	RTPOSTCONDITION( count() == new_size );
};

inline void RJCBuffer::setMemory(char *mem,int maxSize)
{
	RTPRECONDITION(!canResize());
	_area = mem;
	_size = maxSize;
};

inline RTRBOOL RJCBuffer::canResize() const
{
	return(_allocated);
};

inline char *RJCBuffer::to_c( int position ) const
{
	return(_area + position - 1);
};

inline RJCBuffer::operator const char *() const
{
	return _area;
};

#endif
