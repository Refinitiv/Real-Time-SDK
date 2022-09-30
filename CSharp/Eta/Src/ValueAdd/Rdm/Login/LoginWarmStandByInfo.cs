/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Rdm;
using System.Diagnostics;
using System.Text;
using static Refinitiv.Eta.Rdm.Login;

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// Warm Standby Information for the RDM Login Consumer Connection Status.
    /// </summary>
    public class LoginWarmStandByInfo
    {
        #region Private Fields

        private ElementList elementList = new();
        private ElementEntry elementEntry = new();
        private UInt tmpUInt = new();
        private const string eol = "\n";
        private const string tab = "\t";
        private StringBuilder stringBuffer = new();

        #endregion
        #region Public Message Properties

        /// <summary>
        /// Action associated with this information.
        /// </summary>
        public MapEntryActions Action { get; set; }

        /// <summary>
        /// The desired Warm Standby Mode. Populated by <see cref="ServerTypes"/>.
        /// </summary>
        public long WarmStandbyMode { get; set; }

        #endregion

        public LoginWarmStandByInfo()
        {
            Clear();
        }

        public void Clear()
        {
            Action = MapEntryActions.ADD;
            WarmStandbyMode = ServerTypes.ACTIVE;
        }

        public CodecReturnCode Encode(EncodeIterator EncodeIter)
        {
            elementList.Clear();
            elementList.ApplyHasStandardData();
            CodecReturnCode ret = elementList.EncodeInit(EncodeIter, null, 0);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;
            elementEntry.Clear();
            elementEntry.Name = ElementNames.WARMSTANDBY_MODE;
            elementEntry.DataType = DataTypes.UINT;
            tmpUInt.Value(WarmStandbyMode);
            ret = elementEntry.Encode(EncodeIter, tmpUInt);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;
            ret = elementList.EncodeComplete(EncodeIter, true);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;

            return ret;
        }

        public CodecReturnCode Decode(DecodeIterator dIter, Msg msg)
        {
            Clear();

            elementList.Clear();
            CodecReturnCode ret = elementList.Decode(dIter, null);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            bool warmStandbyModeFound = false;
            while ((ret = elementEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

                if (elementEntry.Name.Equals(ElementNames.WARMSTANDBY_MODE))
                {
                    if (elementEntry.DataType != DataTypes.UINT)
                        return ret;
                    ret = tmpUInt.Decode(dIter);
                    if (ret != CodecReturnCode.SUCCESS && ret != CodecReturnCode.BLANK_DATA)
                        return ret;
                    WarmStandbyMode = tmpUInt.ToLong();
                    warmStandbyModeFound = true;
                }
            }

            if (!warmStandbyModeFound)
                return CodecReturnCode.FAILURE;

            return CodecReturnCode.SUCCESS;
        }

        public override string ToString()
        {
            stringBuffer.Clear();
            stringBuffer.Insert(0, "LoginWarmStandbyInfo: \n");
            stringBuffer.Append(tab);

            stringBuffer.Append("warmStandbyMode: ");
            stringBuffer.Append(WarmStandbyMode);
            stringBuffer.Append(eol);
            stringBuffer.Append(tab);
            stringBuffer.Append("action: ");
            stringBuffer.Append(Action);
            stringBuffer.Append(eol);
            return stringBuffer.ToString();
        }

        public CodecReturnCode Copy(LoginWarmStandByInfo destWarmStandbyInfo)
        {
            Debug.Assert(destWarmStandbyInfo != null);

            destWarmStandbyInfo.Action = Action;
            destWarmStandbyInfo.WarmStandbyMode = WarmStandbyMode;

            return CodecReturnCode.SUCCESS;
        }

        public void CopyReferences(LoginWarmStandByInfo srcWarmStandByInfo)
        {
            Debug.Assert(srcWarmStandByInfo != null);

            Action = srcWarmStandByInfo.Action;
            WarmStandbyMode = srcWarmStandByInfo.WarmStandbyMode;
        }
    }
}
