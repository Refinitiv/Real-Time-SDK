package com.rtsdk.eta.valueadd.common;

/* Double-linked List class for maintaining objects that might have multiple links, so that
 * those objects can be in multiple lists simultaneously. The object does this
 * by implementing instances of VaDoubleLinkList.Link. When using list
 * functions, pass the appropriate instance of VaDoubleLinkList.Link as a parameter.
 * This method also allows those objects to have such links without inheriting. */
public class VaDoubleLinkList<T>
{
	private T _head, _tail;
	private T _iter;
	private int _count;
	private static final boolean DEBUG_LIST = false;

	/* Used by objects to implement one or more link accessors. When
	 * calling list functions, provide an instance of the appropriate
	 * Link implementation, which tells it which links to use
	 * for browsing and editing the list. */
	public interface Link<T>
	{
		/* Returns the given elem's previous link */
		T getPrev(T thisPrev);

		/* Sets the given elem's previous link */
		void setPrev(T thisPrev, T thatPrev);

		/* Returns the given elem's next link */
		T getNext(T thisNext);

		/* Sets the given elem's next link */
		void setNext(T thisNext, T thatNext);
	}

	public VaDoubleLinkList()
	{
		clear();
	}

	public void clear()
	{
		_head = null;
		_tail = null;
		_count = 0;
	}

	/* Returns the number of elems in the list. */
	public int count() { return _count; }

	/* Adds a elem to the front of the list. */
	public void push(T elem, Link<T> queueLink)
	{
		if (DEBUG_LIST) verifyList(queueLink);
		if (_tail != null)
		{
			queueLink.setPrev(elem, _tail);
			queueLink.setNext(_tail, elem);
			_tail = elem;
		}
		else
		{
			queueLink.setPrev(elem, null);
			_head = _tail = elem;
		}

		queueLink.setNext(elem, null);
		++_count;
		if (DEBUG_LIST) verifyList(queueLink);
	}

	/* Adds an elem to the back of the list */
	public void pushBack(T elem, Link<T> queueLink)
	{
		if (DEBUG_LIST) verifyList(queueLink);
		if (_head != null)
		{
			queueLink.setNext(elem, _head);
			queueLink.setPrev(_head, elem);
			_head = elem;
		}
		else
		{
			queueLink.setNext(elem, null);
			_head = _tail = elem;
		}

		queueLink.setPrev(elem, null);
		++_count;
		if (DEBUG_LIST) verifyList(queueLink);
	}

	public void insertBefore(T thisBuffer, T newBuffer, Link<T> queueLink)
	{
		if (DEBUG_LIST) verifyList(queueLink);
		if (queueLink.getPrev(thisBuffer) != null)
			queueLink.setNext(queueLink.getPrev(thisBuffer), newBuffer);
		queueLink.setPrev(newBuffer, queueLink.getPrev(thisBuffer));
		queueLink.setNext(newBuffer, thisBuffer);
		queueLink.setPrev(thisBuffer, newBuffer);

		if (thisBuffer == _head)
			_head = newBuffer;

		++_count;
		if (DEBUG_LIST) verifyList(queueLink);
	}

	public void insertAfter(T thisBuffer, T newBuffer, Link<T> queueLink)
	{
		if (DEBUG_LIST) verifyList(queueLink);
		if (queueLink.getNext(thisBuffer) != null)
			queueLink.setPrev(queueLink.getNext(thisBuffer), newBuffer);
		queueLink.setNext(newBuffer, queueLink.getNext(thisBuffer));
		queueLink.setPrev(newBuffer, thisBuffer);
		queueLink.setNext(thisBuffer, newBuffer);

		if (thisBuffer == _tail)
			_tail = newBuffer;

		++_count;
		if (DEBUG_LIST) verifyList(queueLink);
	}

	/* Returns the first elem in the list. */
	public T peek()
	{
		return _head;
	}

	/* Returns the last elem in the list. */
	public T peekTail()
	{
		return _tail;
	}

	/* Removes and returns the first elem in the list. */
	public T pop(Link<T> queueLink)
	{
		T head = _head;
		if (head != null)
			remove(head, queueLink);
		return head;
	}

	public T start(Link<T> queueLink)
	{
		if (_head != null)
			_iter = queueLink.getNext(_head);
		else
			_iter = null;
		
		return _head;
	}

	public T forth(Link<T> queueLink)
	{
		T iter;

		iter = _iter;
		if (_iter != null)
			_iter = queueLink.getNext(_iter);
		return iter;
	}

	/* Removes a elem from the list. */
	public void remove(T elem, Link<T> queueLink)
	{
		if (_iter != null && elem == _iter)
			_iter = queueLink.getNext(elem);

		if (DEBUG_LIST) verifyList(queueLink);
		if (queueLink.getPrev(elem) != null)
			queueLink.setNext(queueLink.getPrev(elem), queueLink.getNext(elem));
		else
		{
			assert _head == elem;
			_head = queueLink.getNext(elem);
		}

		if (queueLink.getNext(elem) != null)
			queueLink.setPrev(queueLink.getNext(elem), queueLink.getPrev(elem));
		else
		{
			assert _tail == elem;
			_tail = queueLink.getPrev(elem);
		}

		queueLink.setNext(elem, null);
		queueLink.setPrev(elem, null);
		--_count;

		if (DEBUG_LIST) verifyList(queueLink);
	}

	public void append(VaDoubleLinkList<T> otherList, Link<T> queueLink)
	{
		if (DEBUG_LIST) verifyList(queueLink);

		if (otherList._head == null)
			return;

		/* Link the lists. */
		if (_head != null)
		{
			queueLink.setNext(_tail, otherList._head);
			queueLink.setPrev(otherList._head, _tail);
			_tail = otherList._tail;
		}
		else
		{
			_head = otherList._head;
			_tail = otherList._tail;
		}
			
		
		
		/* Update count and clear old list. */
		_count += otherList._count;
		otherList._head = null;
		otherList._tail = null;
		otherList._count = 0;

		
		if (DEBUG_LIST) verifyList(queueLink);
	}

	void verifyList(Link<T> queueLink)
	{
		int i = 0;
		assert (_count >= 0);
		for(T link = _head; link != null; link = queueLink.getNext(link))
		{
			++i;
			if (queueLink.getNext(link) == null)
				assert(link == _tail);
		}

		assert(i == _count);
	}
	
	/* Returns true if an element is contained in the list */
	public boolean contains(T elem, Link<T> queueLink)
	{
        for(T link = _head; link != null; link = queueLink.getNext(link))
        {
            if (link == elem)
                return true;
        }
        
        return false;	    
	}
}

