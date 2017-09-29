///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.upa.valueadd.reactor;


import com.thomsonreuters.upa.valueadd.common.VaDoubleLinkList;

import org.junit.Test;
import static org.junit.Assert.*;

/** Runs some list tests and a simple performance comparison of the MsgQueue DoubleLinkList class against the java.util.LinkedList class. */

public class DoubleLinkListJunit
{

	static final double NANO_PER_SEC = 1000000000.0;
	static final int MAX_ELEMS = 5;
	static final int LOOP_INSERT_DELETE = 100000000;
	static final int LOOP_TRAVERSE = 1000000000;

	@Test
	public void DeleteWhileIteratingTest()
	{
		/* Test correctness of elements in list while iterating and sometimes
		 * deleting them. */
		int i;

		ListTestElem[] elemArray = new ListTestElem[5];
		VaDoubleLinkList<ListTestElem> linkList = new VaDoubleLinkList<ListTestElem>();
		ListTestElem tmpElem;

		for(i = 0; i < elemArray.length; ++i)
		{
			elemArray[i] = new ListTestElem();
			elemArray[i].thing1 = i;
		}

		/* No elements */
		assertTrue(linkList.count() == 0);
		tmpElem = linkList.start(ListTestElem.LIST_TEST_LINK);
		assertNull(tmpElem);
		assertNull(linkList.forth(ListTestElem.LIST_TEST_LINK));

		/* One or more elements */
		for(i = 1; i <= elemArray.length; ++i)
		{
			int j;

			assertTrue(linkList.count() == 0);
			for (j = 0; j < i; ++j)
				linkList.push(elemArray[j], ListTestElem.LIST_TEST_LINK);

			assertTrue(linkList.count() == i);
			tmpElem = linkList.start(ListTestElem.LIST_TEST_LINK);
			assertTrue(tmpElem.thing1 == 0);
			
			/* Iterate */
			for(j = 1; j < i/2; ++j)
			{
				assertNotNull((tmpElem = linkList.forth(ListTestElem.LIST_TEST_LINK)));				
				assertTrue(tmpElem.thing1 == j);
			}

			/* Remove this element */
			linkList.remove(tmpElem, ListTestElem.LIST_TEST_LINK);
			
			/* Keep iterating */
			for(; j < i; ++j)
			{
				assertNotNull((tmpElem = linkList.forth(ListTestElem.LIST_TEST_LINK)));
				assertTrue(tmpElem.thing1 == j);
			}

			assertNull(linkList.forth(ListTestElem.LIST_TEST_LINK));
			assertTrue(linkList.count() == i - 1);

			/* Remove all elements in list. */
			tmpElem = linkList.start(ListTestElem.LIST_TEST_LINK);
			for(j = 0; j < i - 1; ++j)
				linkList.pop(ListTestElem.LIST_TEST_LINK);
			
			assertNull(linkList.forth(ListTestElem.LIST_TEST_LINK));
			assertTrue(linkList.count() == 0);
		}
	}

}
