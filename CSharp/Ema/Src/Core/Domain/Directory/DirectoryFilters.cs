using System;

namespace LSEG.Ema.Domain.Directory
{
    /// <summary>
    /// Possbile value of directory filter.
    /// </summary>
    [Flags]
    public enum DirectoryFilters : long
    {
        /// <summary>
        /// No filter.
        /// </summary>
        NONE = 0x00,
        /// <summary>
        /// Service Info Filter.
        /// </summary>
        SERVICE_INFO_FILTER = 0x01,
        /// <summary>
        /// Service State Filter.
        /// </summary>
        SERVICE_STATE_FILTER = 0x02,
        /// <summary>
        /// Service Group Filter.
        /// </summary>
        SERVICE_GROUP_FILTER = 0x04,
        /// <summary>
        /// Service Load Filter.
        /// </summary>
        SERVICE_LOAD_FILTER = 0x08,
        /// <summary>
        /// Service Data Filter.
        /// </summary>
        SERVICE_DATA_FILTER = 0x10,
        /// <summary>
        /// Service Link Filter.
        /// </summary>
        SERVICE_LINK_FILTER = 0x20,
        /// <summary>
        /// All filters.
        /// </summary>
        ALL =
            SERVICE_INFO_FILTER |
            SERVICE_STATE_FILTER |
            SERVICE_GROUP_FILTER |
            SERVICE_LOAD_FILTER |
            SERVICE_DATA_FILTER |
            SERVICE_LINK_FILTER
    }
}
