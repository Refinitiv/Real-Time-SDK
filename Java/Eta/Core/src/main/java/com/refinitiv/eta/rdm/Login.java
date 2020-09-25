package com.refinitiv.eta.rdm;

/** Login specific RDM definitions */
public class Login
{
    // Login class cannot be instantiated
    private Login()
    {
        throw new AssertionError();
    }

    /**
     * Indicates the role of the application logging onto the system.
     */
    public static class RoleTypes
    {
        // RoleTypes class cannot be instantiated
        private RoleTypes()
        {
            throw new AssertionError();
        }

        /** Application logs in as a consumer */
        public static final int CONS = 0;

        /** Application logs in as a provider */
        public static final int PROV = 1;
    }

    /**
     * RDM Login Server Types.
     */
    public static class ServerTypes
    {
        // ServerTypes class cannot be instantiated
        private ServerTypes()
        {
            throw new AssertionError();
        }

        /** Active Server */
        public static final int ACTIVE = 0;

        /** Standby Server */
        public static final int STANDBY = 1;
    }
    
    /**
     * Indicates nametype in RDM login messages.
     */
    public static class UserIdTypes
    {
        // UserIdTypes class cannot be instantiated
        private UserIdTypes()
        {
            throw new AssertionError();
        }

        /** DACS user name. This can be used to authenticate and permission a user. */
        public static final int NAME = 1;

        /** Email address */
        public static final int EMAIL_ADDRESS = 2;

        /**
         * The user token is retrieved from a AAAAPI gateway and then passed through
         * the system via the msgKey.name. To validate users, a provider application
         * can pass this user token to an Authentication Manager application.
         */
        public static final int TOKEN = 3;

        /**
         * This indicates user information is specified in a cookie.
         * If msgKey.name is present it contains cookie file information.
         * If not present, cookie information may be well known or externally specified.
         */
        public static final int COOKIE = 4;
        
        /**
         * String defining User Authentication Token is specified as the AUTHN_TOKEN
         * login attribute.
         */
        public static final int AUTHN_TOKEN = 5;
    }
    
    /** Provider batch support flags. Any combination can be set by provider. */
    public static class BatchSupportFlags
    {
        // BatchSupportFlags class cannot be instantiated
        private BatchSupportFlags()
        {
            throw new AssertionError();
        }
        
        /** Provider does not support batching. */
        public static final int NONE = 0x0;
        
        /** Provider supports batch requests. */
        public static final int SUPPORT_REQUESTS = 0x1;
        
        /** Provider supports batch reissue requests. */
        public static final int SUPPORT_REISSUES = 0x2;
        
        /** Provider supports batch closes. */
        public static final int SUPPORT_CLOSES = 0x4;
    }

    /** Enhanced Symbol List behavior support flags. */
    public static class EnhancedSymbolListSupportFlags
    {
        // EnhancedSymbolListSupportFlags class cannot be instantiated
        private EnhancedSymbolListSupportFlags()
        {
            throw new AssertionError();
        }
        
        /** Supports names only, no additional functionality. */
        public static final int NAMES_ONLY = 0x0;
        
        /** Supports symbol list data streams. */
        public static final int DATA_STREAMS = 0x1;
    }
}
