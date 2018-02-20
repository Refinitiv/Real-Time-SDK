#include <stdlib.h>
#include <string.h>
#include "fieldListDefs.h"
#include "rtr/rsslDataPackage.h"
#include "rtr/rwfNet.h"
#include "rtr/retmacros.h"

void freeFieldLists(FieldList * fieldList)
{
	FieldList * nextFieldList = fieldList;
	while (nextFieldList != 0)
	{
		fieldList = nextFieldList;
		nextFieldList = fieldList->next;
		free(fieldList->defs.data);
		free(fieldList);
	}
}

FieldList * addFieldList(FieldList ** fieldList, RsslUInt16 id, const RsslBuffer * defs)
{
	FieldList * add = (FieldList *) malloc(sizeof(FieldList));
	FieldList * fl = *fieldList;
	
	if (add)
	{
		add->defs.data = (char *)malloc(defs->length);

		if (add->defs.data)
		{
			memcpy(add->defs.data, defs->data, defs->length);
			add->defs.length = defs->length;
			add->id = id;
			add->next = 0;

			if (!fl)
			{
				*fieldList = add;
				return add;
			}
			while (fl->next)
				fl = fl->next;
			fl->next = add;
		}
		else
			add->defs.length = 0;
	}
	
	return add;
}

FieldList * getFieldList(FieldList * fieldList, RsslUInt16 id)
{
	while (fieldList && fieldList->id != id)
		fieldList = fieldList->next;
	return fieldList;
}

