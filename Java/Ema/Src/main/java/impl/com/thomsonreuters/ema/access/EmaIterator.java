///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.util.Iterator;

class EmaIterator<E> implements Iterator<E>
{
	private Iterator<E> _iterator;
	
	EmaIterator(Iterator<E> iterator)
	{
		_iterator = iterator;
	}
	
	@Override
	public boolean hasNext()
	{
		return _iterator.hasNext();
	}

	@Override
	public E next()
	{
		return _iterator.next();
	}

	@Override
	public void remove()
	{
		throw new UnsupportedOperationException("EMA collection iterator doesn't support this operation.");
	}
}