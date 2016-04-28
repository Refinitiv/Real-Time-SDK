package com.thomsonreuters.upa.rdm;

import com.thomsonreuters.upa.codec.MsgKey;
import com.thomsonreuters.upa.codec.RequestMsg;

/** Dictionary specific RDM definitions */
public class Dictionary
{
    // Dictionary class cannot be instantiated
    private Dictionary()
    {
        throw new AssertionError();
    }

    /**
     * Enumerations describing the Type of a particular dictionary. These values are
     * associated with the "Type" tag of a dictionary, found in the associated file
     * or summary data. methods for loading or decoding these dictionaries will
     * look for this information and use it to verify that the correct type of
     * dictionary is being interpreted.
     * 
     * @see com.thomsonreuters.upa.codec.DataDictionary
     */
    public static class Types
    {
        // Types class cannot be instantiated
        private Types()
        {
            throw new AssertionError();
        }

        /** Field Dictionary type, typically referring to an RDMFieldDictionary */
        public static final int FIELD_DEFINITIONS = 1;

        /** Enumeration Dictionary type, typically referring to an enumtype.def */
        public static final int ENUM_TABLES = 2;

        /**
         * Record template type, typically referring to a template to help with
         * caching of data - can be referred to by fieldListNum or elemListNum
         */
        public static final int RECORD_TEMPLATES = 3;

        /**
         * Display template type, typically provides information about displaying
         * data (e.g. position on screen, etc)
         */
        public static final int DISPLAY_TEMPLATES = 4;

        /**
         * Set Data Definition type, contains data definitions that would apply
         * globally to any messages sent or received from the provider of the
         * dictionary
         */
        public static final int DATA_DEFINITIONS = 5;

        /** Style sheet type, can be used to send style information */
        public static final int STYLE_SHEET = 6;

        /** Dictionary reference type, additional dictionary information */
        public static final int REFERENCE = 7;
        
        /** Field Set Definition Dictionary type, typically referring to an EDF_BATS */
        public static final int FIELD_SET_DEFINITION = 8;
    }

    /**
     * Enumerations describing how much information about a particular dictionary is
     * desired. These values are typically set in a {@link RequestMsg}'s
     * {@link MsgKey} filter when the request for the dictionary is made. See the
     * UPA RDM Usage Guide for details.
     * 
     * @see com.thomsonreuters.upa.codec.DataDictionary
     * @see com.thomsonreuters.upa.codec.MsgKey
     */
    public static class VerbosityValues
    {
        // VerbosityValues class cannot be instantiated
        private VerbosityValues()
        {
            throw new AssertionError();
        }

        /** (0x00) "Dictionary Info" Verbosity, no data - version information only */
        public static final int INFO = 0x00;

        /** (0x03) "Minimal" Verbosity, e.g. Cache + ShortName */
        public static final int MINIMAL = 0x03;

        /** (0x07) "Normal" Verbosity, e.g. all but description */
        public static final int NORMAL = 0x07;

        /** (0x0F) "Verbose" Verbosity, e.g. all with description */
        public static final int VERBOSE = 0x0F;
    }
}
