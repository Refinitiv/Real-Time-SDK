package com.refinitiv.eta.codec;

class DecodingLevel
{
    int                 _endBufPos;         /* Parsing internals, current position */
    Object              _listType;          /* Pointer to actual list type */
    int                 _nextEntryPos;      /* End of the current payload (end of a Message payload, or the end of a container Entry) */
    FieldSetDefImpl     _fieldListSetDef;   /* RsslFieldListSetDef, used to decode this level */
    ElementSetDefImpl   _elemListSetDef;    /* RsslElementListSetDef, used to decode this level */
    int                 _itemCount;         /* number of items in the list */
    int                 _nextItemPosition;  /* index of next item. Iterator is off when _nextItemPosition >= itemCount */
    int                 _setCount;          /* number of items in the set */
    int                 _nextSetPosition;   /* index of next item in a set */
    int                 _containerType;     /* Type of container to decode for this level */
	
    void setup(int type, Object container)
    {
        _itemCount = 0;
        _nextItemPosition = 0;
        _nextSetPosition = 0;
        _containerType = type;
        _listType = container;
    }

    int endBufPos()
    {
        return _endBufPos;
    }

    int endBufPos(int _endBufPos)
    {
        this._endBufPos = _endBufPos;
        return _endBufPos;
    }

}
