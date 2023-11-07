using LSEG.Eta.Codec;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// VectorAction represents vector entry action.
    /// </summary>
    public sealed class VectorAction
    {
        /// <summary>
        /// Indicates a partial change of the entry.
        /// </summary>
        public const int UPDATE = 1;
        /// <summary>
        /// Indicates to replace the entry.
        /// </summary>
        public const int SET = 2;
        /// <summary>
        /// Indicates to empty the entry. Contains no data.
        /// </summary>
        public const int CLEAR = 3;
        /// <summary>
        /// Indicates to place the entry in between other entries.<br/>
        /// Increases any higher-ordered position by one.<br/>
        /// May leave gaps if previous lower-ordered position is not populated.<br/>
        /// </summary>
        public const int INSERT = 4;
        /// <summary>
        /// Indicates to remove the entry.<br/>
        /// Decreases any higher-ordered position by one.<br/>
        /// Contains no data.<br/>
        /// </summary>
        public const int DELETE = 5;

        internal static int EtaVectorActionToInt(VectorEntryActions action)
        {
            switch (action)
            {
                case VectorEntryActions.UPDATE: return UPDATE;
                case VectorEntryActions.SET: return SET;
                case VectorEntryActions.CLEAR: return CLEAR;
                case VectorEntryActions.INSERT: return INSERT;
                case VectorEntryActions.DELETE: return DELETE;
                default: return 0;
            }
        }

        internal static string VectorActionToString(int action)
        {
            switch (action)
            {
                case UPDATE: return "UPDATE";
                case SET: return "SET";
                case CLEAR: return "CLEAR";
                case INSERT: return "INSERT";
                case DELETE: return "DELETE";
                default: return string.Empty;
            }
        }
    }
}
