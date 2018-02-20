package com.thomsonreuters.upa.codec;

/* special flags for encode iterator use */
class EncodeIteratorFlags
{
    public static final int NONE               = 0x0000;
    public static final int LENU15             = 0x0001; /* Length needs to be encoded as u15 */
    public static final int HAS_PER_ENTRY_PERM = 0x0002; /* map, vector, or filter list needs per-entry perm flag set */
}
