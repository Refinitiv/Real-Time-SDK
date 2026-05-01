/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2020,2024,2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_Pool_h
#define __refinitiv_ema_access_Pool_h

#include "EmaVector.h"
#include "ExceptionTranslator.h"
#include "Mutex.h"

#include "rtr/rwfNet.h"

#include <new>

namespace refinitiv
{

namespace ema
{

namespace access
{

template <class O>
class Factory
{
public:
	static O* create();

	static void destroy(O* o)
	{
		delete o;
	}
};

template <class O>
O* Factory<O>::create()
{
	try
	{
		return new O;
	}
	catch (std::bad_alloc&)
	{
		const char* temp = "Failed to create object in Factory< O >::create(). Out of memory.";
		throwMeeException(temp);
	}
	return 0;
}

template <class I>
struct RecycleNop final
{
	static void recycle(I*) {};
};

/** Encoders need to be "release"d before returning to the pool. */
template <class I>
struct ReleaseEncoder final
{
	static void recycle(I* item)
	{
		item->release();
	};
};

template <class I, class RecyclingPolicy = RecycleNop<I>>
class Pool
{
public:

	constexpr static UInt32 DEFAULT_INIT_SIZE = 5;

	Pool(UInt32 size = DEFAULT_INIT_SIZE);

	virtual ~Pool();

	void clear();

	I* getItem();

	void returnItem(I*);

	UInt32 count() const;

	void setLimit(UInt32);

	Pool(const Pool&) = delete;
	Pool(Pool&&) = delete;
	Pool& operator=(const Pool&) = delete;
	Pool& operator=(Pool&&) = delete;

private:

	mutable Mutex _lock;

	EmaVector<I*> _vector;

	// current number of objects in the pool
	UInt32 _count;

	// number of objects in the pool above which returned objects are deleted, not reused
	UInt32 _limit;
};

template <class I, class R>
Pool<I, R>::Pool(UInt32 size) :
 _lock(),
 _vector(size),
 _count(0),
 _limit(RWF_MAX_32)
{
	for (UInt32 idx = 0; idx < size; ++idx)
	{
		_vector.push_back(nullptr);
	}
}

template <class I, class R>
Pool<I, R>::~Pool()
{
	clear();
}

template <class I, class R>
void Pool<I, R>::clear()
{
	_lock.lock();

	if (!_count)
	{
		_lock.unlock();
		return;
	}

	for (UInt32 idx = _vector.size(); idx != 0; --idx)
	{
		I* temp = _vector[idx - 1];
		if (temp)
		{
			Factory<I>::destroy(temp);
		}
		_vector[idx - 1] = 0;
	}

	_count = 0;

	_lock.unlock();
}

template <class I, class R>
I* Pool<I, R>::getItem()
{
	_lock.lock();

	if (!_count)
	{
		_lock.unlock();

		return Factory<I>::create();
	}

	I*& itemRef = _vector[--_count];

	I* item = itemRef;

	itemRef = nullptr;

	_lock.unlock();

	return item;
}

template <class I, class R>
void Pool<I, R>::returnItem(I* item)
{
	R::recycle(item);

	_lock.lock();

	if (_count < _limit)
	{
		// configured limit is not reached yet, object can be put back into pool
		if (_count == _vector.size())
		{
			_vector.push_back(nullptr);
		}

		_vector[_count++] = item;
	}
	else
	{
		// number of objects in the pool exceeds configured pool limit, discard returned object
		Factory<I>::destroy(item);
	}

	_lock.unlock();
}

template <class I, class R>
UInt32 Pool<I, R>::count() const
{
	const MutexLocker guard{_lock};

	return _count;
}

template <class I, class R>
void Pool<I, R>::setLimit(UInt32 limit)
{
	_lock.lock();

	_limit = limit;

	_lock.unlock();
}

// Encoders need to be "release()"-ed before put into the pool
template <class I>
using EncoderPool = Pool<I, ReleaseEncoder<I>>;

} // namespace access

} // namespace ema

} // namespace refinitiv

#endif // __refinitiv_ema_access_Pool_h
