/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;

namespace LSEG.Eta.Rdm
{
    /// <summary>
    /// Dictionary specific RDM definitions </summary>
    sealed public class Dictionary
	{
		// Dictionary class cannot be instantiated
		private Dictionary()
		{
            throw new System.NotImplementedException();
        }

        /// <summary>
        /// Enumerations describing the Type of a particular dictionary. These values are
        /// associated with the "Type" tag of a dictionary, found in the associated file
        /// or summary data. methods for loading or decoding these dictionaries will
        /// look for this information and use it to verify that the correct type of
        /// dictionary is being interpreted.
        /// </summary>
        /// <seealso cref="LSEG.Eta.Codec.DataDictionary"/>
        public class Types
		{
			// Types class cannot be instantiated
			private Types()
			{
                throw new System.NotImplementedException();
            }

			/// <summary>
			/// Field Dictionary type, typically referring to an RDMFieldDictionary </summary>
			public const int FIELD_DEFINITIONS = 1;

			/// <summary>
			/// Enumeration Dictionary type, typically referring to an enumtype.def </summary>
			public const int ENUM_TABLES = 2;

			/// <summary>
			/// Record template type, typically referring to a template to help with
			/// caching of data - can be referred to by fieldListNum or elemListNum
			/// </summary>
			public const int RECORD_TEMPLATES = 3;

			/// <summary>
			/// Display template type, typically provides information about displaying
			/// data (e.g. position on screen, etc)
			/// </summary>
			public const int DISPLAY_TEMPLATES = 4;

			/// <summary>
			/// Set Data Definition type, contains data definitions that would apply
			/// globally to any messages sent or received from the provider of the dictionary
			/// </summary>
			public const int DATA_DEFINITIONS = 5;

			/// <summary>
			/// Style sheet type, can be used to send style information </summary>
			public const int STYLE_SHEET = 6;

			/// <summary>
			/// Dictionary reference type, additional dictionary information </summary>
			public const int REFERENCE = 7;

			/// <summary>
			/// Field Set Definition Dictionary type, typically referring to an EDF_BATS </summary>
			public const int FIELD_SET_DEFINITION = 8;
		}

        /// <summary>
        /// Enumerations describing how much information about a particular dictionary is
        /// desired. These values are typically set in a <see cref="IRequestMsg"/>'s
        /// <see cref="MsgKey"/> filter when the request for the dictionary is made.
        /// See the ETA RDM Usage Guide for details.
        /// </summary>
        /// <seealso cref="LSEG.Eta.Codec.DataDictionary"/>
        /// <seealso cref="LSEG.Eta.Codec.MsgKey"/>
        public class VerbosityValues
		{
			// VerbosityValues class cannot be instantiated
			private VerbosityValues()
			{
                throw new System.NotImplementedException();
            }

			/// <summary>
			/// (0x00) "Dictionary Info" Verbosity, no data - version information only </summary>
			public const int INFO = 0x00;

			/// <summary>
			/// (0x03) "Minimal" Verbosity, e.g. Cache + ShortName </summary>
			public const int MINIMAL = 0x03;

			/// <summary>
			/// (0x07) "Normal" Verbosity, e.g. all but description </summary>
			public const int NORMAL = 0x07;

			/// <summary>
			/// (0x0F) "Verbose" Verbosity, e.g. all with description </summary>
			public const int VERBOSE = 0x0F;
		}
	}

}