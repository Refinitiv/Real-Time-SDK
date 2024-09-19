/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */


namespace LSEG.Ema.Access
{
    internal class DictionaryConfig
    {
        // Name of this dictionary
        public string Name { get; set; } = string.Empty;

        /// Sets the location of the EnumTypeDef file.
        public string EnumTypeDefFileName { get; set; } = string.Empty;

        public string EnumTypeDefItemName { get; set; } = string.Empty;

        /// Sets the location of the RdmFieldDictionary
        public string RdmFieldDictionaryFileName { get; set; } = string.Empty;

        public string RdmFieldDictionaryItemName { get; set; } = string.Empty;

        public int DictionaryType { get; set; }

        /// When DictionaryType is DictionaryLoadingMode.FILE the Enterprise Message API
        /// loads the dictionaries from the files specified in the parameters
        /// RdmFieldDictionaryFileName and EnumTypeDefFileName.
        ///
        /// Note: this behavior can be overridden by specifying custom dictionary requests.
        public bool IsLocalDictionary { get; set; } = false;

        internal Ema.Rdm.DataDictionary? DataDictionary;

#pragma warning disable CS8618
        public DictionaryConfig()
#pragma warning restore CS8618
        {
            Clear();
        }

#pragma warning disable CS8618
        public DictionaryConfig(DictionaryConfig oldConfig)
#pragma warning restore CS8618
        {
            Name = oldConfig.Name;
            EnumTypeDefFileName = oldConfig.EnumTypeDefFileName;
            EnumTypeDefItemName = oldConfig.EnumTypeDefItemName;
            RdmFieldDictionaryFileName = oldConfig.RdmFieldDictionaryFileName;
            RdmFieldDictionaryItemName = oldConfig.RdmFieldDictionaryItemName;
            DictionaryType = oldConfig.DictionaryType;
        }

        // Clears the Dictionary info and sets the default options.
        public void Clear()
        {
            Name = string.Empty;
            EnumTypeDefFileName = "./enumtype.def";
            EnumTypeDefItemName = "RWFEnum";
            RdmFieldDictionaryFileName = "./RDMFieldDictionary";
            RdmFieldDictionaryItemName = "RWFFld";
            DictionaryType = EmaConfig.DictionaryTypeEnum.CHANNEL;
            IsLocalDictionary = false;
        }

        /// Copy method, produces a deep copy into DestConfig.
        public void Copy(DictionaryConfig DestConfig)
        {
            DestConfig.Name = Name;
            DestConfig.EnumTypeDefFileName = EnumTypeDefFileName;
            DestConfig.EnumTypeDefItemName = EnumTypeDefItemName;
            DestConfig.RdmFieldDictionaryFileName = RdmFieldDictionaryFileName;
            DestConfig.RdmFieldDictionaryItemName = RdmFieldDictionaryItemName;
            DestConfig.DictionaryType = DictionaryType;
            DestConfig.IsLocalDictionary = IsLocalDictionary;
        }

        internal static int StringToDictionaryMode(string dictionaryMode) => dictionaryMode switch
        {
            "FileDictionary"    => EmaConfig.DictionaryTypeEnum.FILE,
            "ChannelDictionary" => EmaConfig.DictionaryTypeEnum.CHANNEL,
            _ => throw new OmmInvalidConfigurationException("Dictionary mode: " + dictionaryMode + " not recognized. Acceptable inputs: \"FileDictionary\" or \"ChannelDictionary\".")
        };
    }
}