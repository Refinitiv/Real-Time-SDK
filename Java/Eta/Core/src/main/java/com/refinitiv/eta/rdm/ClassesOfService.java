/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.rdm;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;

/** Qualified stream class of service specific RDM definitions. */
public class ClassesOfService
{
    // ClassesOfService class cannot be instantiated
    private ClassesOfService()
    {
        throw new AssertionError();
    }

    /**
     * Filter IDs for class of service.
     */
    public static class FilterIds
    {
        // FilterIds class cannot be instantiated
        private FilterIds()
        {
            throw new AssertionError();
        }

        /** Common Properties Filter ID */
        public static final int COMMON_PROPERTIES = 1;
        /** Authentication Filter ID */
        public static final int AUTHENTICATION = 2;
        /** Flow Control Filter ID */
        public static final int FLOW_CONTROL = 3;
        /** Data Integrity Filter ID */
        public static final int DATA_INTEGRITY = 4;
        /** Guarantee Filter ID */
        public static final int GUARANTEE = 5;
    }

    /**
     * Filter flags for class of service.
     */
    public static class FilterFlags
    {
        // FilterFlags class cannot be instantiated
        private FilterFlags()
        {
            throw new AssertionError();
        }

        /** Common Properties Filter Mask */
        public static final int COMMON_PROPERTIES = 0x00000001;
        /** Authentication Filter Mask */
        public static final int AUTHENTICATION = 0x00000002;
        /** Flow Control Filter Mask */
        public static final int FLOW_CONTROL = 0x00000004;
        /** Data Integrity Filter Mask */
        public static final int DATA_INTEGRITY = 0x00000008;
        /** Guarantee Filter Mask */
        public static final int GUARANTEE = 0x00000010;
    }

    /**
     * Element names for class of service.
     */
    public static class ElementNames
    {
        // ElementNames class cannot be instantiated
        private ElementNames()
        {
            throw new AssertionError();
        }
        
        /** :StreamVersion */
        public static final Buffer STREAM_VERSION = CodecFactory.createBuffer();        

        /** :MaxMsgSize */
        public static final Buffer MAX_MSG_SIZE = CodecFactory.createBuffer();        

        /** :MaxFragmentSize */
        public static final Buffer MAX_FRAGMENT_SIZE = CodecFactory.createBuffer();        
        
        /** :ProtocolType */
        public static final Buffer PROTOCOL_TYPE = CodecFactory.createBuffer();        

        /** :ProtocolMajorVersion */
        public static final Buffer PROTOCOL_MAJOR_VERSION = CodecFactory.createBuffer();        

        /** :ProtocolMinorVersion */
        public static final Buffer PROTOCOL_MINOR_VERSION = CodecFactory.createBuffer();        

        /** :RecvWindowSize */
        public static final Buffer RECV_WINDOW_SIZE = CodecFactory.createBuffer();        
        
        /** :Type */
        public static final Buffer TYPE = CodecFactory.createBuffer();
        
        /** :SupportsFragmentation */
        public static final Buffer SUPPS_FRAGMENTATION = CodecFactory.createBuffer();      
        
        static
        {
            /** :StreamVersion */
            STREAM_VERSION.data(":StreamVersion");
            
            /** :MaxMsgSize */
            MAX_MSG_SIZE.data(":MaxMsgSize");
            
            /** :MaxFragmentSize */
            MAX_FRAGMENT_SIZE.data(":MaxFragmentSize");            
            
            /** :ProtocolType */
            PROTOCOL_TYPE.data(":ProtocolType");
            
            /** :ProtocolMajorVersion */
            PROTOCOL_MAJOR_VERSION.data(":ProtocolMajorVersion");
            
            /** :ProtocolMinorVersion */
            PROTOCOL_MINOR_VERSION.data(":ProtocolMinorVersion");
            
            /** :RecvWindowSize */
            RECV_WINDOW_SIZE.data(":RecvWindowSize");
            
            /** :Type */
            TYPE.data(":Type");
            
            /** :SupportsFragmentation */
            SUPPS_FRAGMENTATION.data(":SupportsFragmentation");
        }
    }

    /**
     * Authentication type enumerations for class of service.
     */
    public static class AuthenticationTypes
    {
        // AuthenticationTypes class cannot be instantiated
        private AuthenticationTypes()
        {
            throw new AssertionError();
        }

        /** Not Required */
        public static final int NOT_REQUIRED = 0;

        /** OMM Login */
        public static final int OMM_LOGIN = 1;
    }

    /**
     * Flow control type enumerations for class of service.
     */
    public static class FlowControlTypes
    {
        // FlowControlTypes class cannot be instantiated
        private FlowControlTypes()
        {
            throw new AssertionError();
        }

        /** None */
        public static final int NONE = 0;

        /** Bidirectional */
        public static final int BIDIRECTIONAL = 1;
    }

    /**
     * Data integrity type enumerations for class of service.
     */
    public static class DataIntegrityTypes
    {
        // DataIntegrityTypes class cannot be instantiated
        private DataIntegrityTypes()
        {
            throw new AssertionError();
        }

        /** Best Effort */
        public static final int BEST_EFFORT = 0;

        /** Reliable */
        public static final int RELIABLE = 1;
    }

    /**
     * Guarantee type enumerations for class of service.
     */
    public static class GuaranteeTypes
    {
        // Guarantee classTypes cannot be instantiated
        private GuaranteeTypes()
        {
            throw new AssertionError();
        }

        /** None */
        public static final int NONE = 0;

        /** PersistentQueue */
        public static final int PERSISTENT_QUEUE = 1;
    }
}
