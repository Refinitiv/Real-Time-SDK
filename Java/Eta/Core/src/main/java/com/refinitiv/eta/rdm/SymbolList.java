/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.rdm;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;

public class SymbolList 
{	
    private SymbolList()
    {
        throw new AssertionError();
    }

    /**
     * SymbolListSupportFlags
     */
    public static class SymbolListSupportFlags
    {
        // SymbolListSupportFlags class cannot be instantiated
        private SymbolListSupportFlags()
        {
            throw new AssertionError();
        }

        /** Only symbol list name */
        public static final int SYMBOL_LIST_SUPPORT_NAMES_ONLY = 0;
        /**  Enhanced symbol list data */
        public static final int SYMBOL_LIST_SUPPORT_DATA_STREAM = 1;
    }

    /**
     * Symbol List request behavior flags.
     */
    public static class SymbolListDataStreamRequestFlags
    {
        // SymbolListDataStreamRequestFlags class cannot be instantiated
        private SymbolListDataStreamRequestFlags()
        {
            throw new AssertionError();
        }

        /*** Only Symbol List Name */
        public static final int SYMBOL_LIST_NAMES_ONLY = 0x00000000;
        /** Enhanced Symbol List Data Stream */
        public static final int SYMBOL_LIST_DATA_STREAMS = 0x00000001;
        /** Enhanced Symbol List Snapshots*/
        public static final int SYMBOL_LIST_DATA_SNAPSHOTS = 0x00000002;
    }

    /**
     * Element names for SymbolList.
     */
    public static class ElementNames
    {
        // ElementNames class cannot be instantiated
        private ElementNames()
        {
            throw new AssertionError();
        }
        
        /** :SymbolListData Behavior */
        public static final Buffer SYMBOL_LIST_BEHAVIORS = CodecFactory.createBuffer();
        /** :SymbolListDataStream */
        public static final Buffer SYMBOL_LIST_DATA_STREAMS = CodecFactory.createBuffer();
        
        static
        {
            /** :SymbolList Behavior */        
            SYMBOL_LIST_BEHAVIORS.data(":SymbolListBehaviors");
            /** :SymbolListDataStream */
            SYMBOL_LIST_DATA_STREAMS.data(":DataStreams");    
        }
    }	
}
