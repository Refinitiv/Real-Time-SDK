#include "fileUtil.h"

#include "rtr/rwfNet.h"

#ifdef WIN32
#include <fcntl.h>
#include <windows.h>
#endif

FILE * openBinFile(const char * fileName, const char * mode)
{
	FILE * file = NULL;
#ifdef WIN32
#if _MSC_VER < 1300
	_fmode = _O_BINARY;
#endif
#endif

	if (fileName == 0)
	{
		if (*mode == 'r')
			fileName = "stdin";
		else if (*mode == 'w')
			fileName = "stdout";
	}

	if (fileName)
	{
		if (strcmp(fileName, "stdin") == 0)
			file = stdin;
		else if (strcmp(fileName, "stdout") == 0)
			file = stdout;
		else if (strcmp(fileName, "stderr") == 0)
			file = stderr;
		else
			file = fopen(fileName, mode);
	}

	if (!file)
	{
		printf("Could not open %s with mode %s", fileName, mode);
		exit(1);
	}

	return file;
}

void closeBinFile(FILE * file)
{
	if (file != stdout && file != stderr && file != stdin)
		fclose(file);
}

FILE * openXmlFile(const char * fileName, const char * opentag)
{
	FILE * file;

	if (fileName == 0)
		file = stdout;
	else if (strcmp(fileName, "stdout") == 0)
		file = stdout;
	else if (strcmp(fileName, "stderr") == 0)
		file = stderr;
	else
		file = fopen(fileName, "w");
	fprintf(file, "<?xml version='1.0' encoding=\"utf-8\" ?>\n");
	fprintf(file, "%s\n", opentag);
	return file;
}

void closeXmlFile(FILE * file, const char * closetag)
{
	fprintf(file, "%s\n", closetag);
	if (file != stdout && file != stderr)
		fclose(file);
}


int readMsg(FILE * file, RsslUInt8 *majorVer, RsslUInt8 *minorVer, RsslBuffer * buffer)
{
	int ret;
	char data[sizeof(RsslUInt32)];
	RsslUInt32 msgSize;
	RsslUInt32 bytesRead;

	ret = (int)fread( data, sizeof( RsslUInt32), 1, file );
	if (ferror(file) || (ret == 0)) return -1;

	RWF_MOVE_32(&msgSize, data);

	/* read major version */
	ret = (int)fread( majorVer, sizeof( RsslUInt8), 1, file );
	if (ferror(file) || (ret == 0)) return -1;
	
	/* read major version */
	ret = (int)fread( minorVer, sizeof( RsslUInt8), 1, file );
	if (ferror(file) || (ret == 0)) return -1;


	if (buffer->length > 0)
		free(buffer->data);

	buffer->data = (char*) malloc(msgSize);
	buffer->length = msgSize;

	ret = (int)fread( buffer->data, 1, msgSize, file );
	bytesRead = ret;
	
	return ret;
}

int writeMsg(FILE * out, RsslBuffer * buffer, RsslUInt8 majorVer, RsslUInt8 minorVer, RsslUInt16 len)
{
	RsslUInt32 count;
	RsslRet ret;

	RWF_MOVE_32(&count, &len);
	ret = (RsslRet)fwrite(&count, 2, 1, out);
	ret = (RsslRet)fwrite(&majorVer, 1, 1, out);
	ret = (RsslRet)fwrite(&minorVer, 1, 1, out);
	ret = (RsslRet)fwrite(buffer->data, 1, len, out);
	return ret;
}
