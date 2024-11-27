/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using System.Diagnostics;
using System.Text;
using static LSEG.Eta.Rdm.Login;

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// Warm Standby Information for the RDM Login Consumer Connection Status.
    /// </summary>
    sealed public class LoginWarmStandByInfo
    {
        #region Private Fields

        private ElementList elementList = new();
        private ElementEntry elementEntry = new();
        private UInt tmpUInt = new();
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

        /// <summary>
        /// Login Warm Standby Info constructor.
        /// </summary>
        public LoginWarmStandByInfo()
        {
            Clear();
        }

        /// <summary>
        /// Clears the current contents of the login Warm Standby Info object and prepares it for re-use.
        /// </summary>
        public void Clear()
        {
            Action = MapEntryActions.ADD;
            WarmStandbyMode = ServerTypes.ACTIVE;
        }

        /// <summary>
        /// Performs a deep copy of this object into <c>destWarmStandbyInfo</c>.
        /// </summary>
        /// <param name="destWarmStandbyInfo">LoginWarmStandByInfo object that will have this object's information copied into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Copy(LoginWarmStandByInfo destWarmStandbyInfo)
        {
            Debug.Assert(destWarmStandbyInfo != null);

            destWarmStandbyInfo.Action = Action;
            destWarmStandbyInfo.WarmStandbyMode = WarmStandbyMode;

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Shallow copies the information and references contained in <c>srcWarmStandByInfo</c> into this object.
        /// </summary>
        /// <param name="srcWarmStandByInfo">LoginConnectionConfig that will be copied from.</param>
        public void CopyReferences(LoginWarmStandByInfo srcWarmStandByInfo)
        {
            Debug.Assert(srcWarmStandByInfo != null);

            Action = srcWarmStandByInfo.Action;
            WarmStandbyMode = srcWarmStandByInfo.WarmStandbyMode;
        }

        /// <summary>
        /// Encodes this login warm standby info using the provided <c>encodeIter</c>.
        /// </summary>
        /// <param name="encodeIter">Encode iterator that has a buffer set to encode into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Encode(EncodeIterator encodeIter)
        {
            elementList.Clear();
            elementList.ApplyHasStandardData();
            CodecReturnCode ret = elementList.EncodeInit(encodeIter, null, 0);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;
            elementEntry.Clear();
            elementEntry.Name = ElementNames.WARMSTANDBY_MODE;
            elementEntry.DataType = DataTypes.UINT;
            tmpUInt.Value(WarmStandbyMode);
            ret = elementEntry.Encode(encodeIter, tmpUInt);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;
            ret = elementList.EncodeComplete(encodeIter, true);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;

            return ret;
        }

        /// <summary>
        /// Decodes this Login Warm Stand By Info message using the provided <c>decodeIter</c> and the incoming <c>msg</c>.
        /// </summary>
        /// <param name="decodeIter">Decode iterator that has already decoded the initial message.</param>
        /// <param name="msg">Decoded Msg object for this LoginWarmStandByInfo message.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Decode(DecodeIterator decodeIter, Msg msg)
        {
            Clear();

            elementList.Clear();
            CodecReturnCode ret = elementList.Decode(decodeIter, null);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            bool warmStandbyModeFound = false;
            while ((ret = elementEntry.Decode(decodeIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

                if (elementEntry.Name.Equals(ElementNames.WARMSTANDBY_MODE))
                {
                    if (elementEntry.DataType != DataTypes.UINT)
                        return ret;
                    ret = tmpUInt.Decode(decodeIter);
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

        /// <summary>
        /// Returns a human readable string representation of the Login Warm Stand By Info message.
        /// </summary>
        /// <returns>String containing the string representation.</returns>
        public override string ToString()
        {
            stringBuffer.Clear();
            stringBuffer.Insert(0, $"LoginWarmStandbyInfo: {NewLine}");
            stringBuffer.Append(tab);

            stringBuffer.Append("warmStandbyMode: ");
            stringBuffer.Append(WarmStandbyMode);
            stringBuffer.AppendLine();
            stringBuffer.Append(tab);
            stringBuffer.Append("action: ");
            stringBuffer.Append(Action);
            stringBuffer.AppendLine();
            return stringBuffer.ToString();
        }
    }
}
