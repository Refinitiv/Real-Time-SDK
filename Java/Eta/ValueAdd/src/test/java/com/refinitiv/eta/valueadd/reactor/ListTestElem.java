/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.valueadd.common.VaDoubleLinkList.Link;

class ListTestElem
{
	private ListTestElem _next, _prev;
	static class ListTestLink implements Link<ListTestElem>
	{
		public ListTestElem getPrev(ListTestElem thisPrev) { return thisPrev._prev; }
		public void setPrev(ListTestElem thisPrev, ListTestElem thatPrev) { thisPrev._prev = thatPrev; }
		public ListTestElem getNext(ListTestElem thisNext) { return thisNext._next; }
		public void setNext(ListTestElem thisNext, ListTestElem thatNext) { thisNext._next = thatNext; }
	}

	static final ListTestLink LIST_TEST_LINK = new ListTestLink();

	int thing1;
	int thing2;
	long thing3;
	String ohJustNothing;
}

