#include <assert.h>
#include "libpack.h"

static LpkEntry *lpk_alloc_node()
{
	LpkEntry *node
		= (LpkEntry *)malloc(sizeof(LpkEntry));
	node->nextHeader = NULL;
	return node;
}

LpkArchive *lpk_read_pack(const char *srcFile)
{
	LpkArchive *archive = (LpkArchive *)malloc(sizeof(LpkArchive));
	archive->Stream = fopen(srcFile, "rb");
	archive->fileList = lpk_alloc_node();
	LpkEntry *cursor = archive->fileList;
	while (1) {
		if (fread(&(cursor->headerNode), sizeof(LpkEntity)
			, 1, archive->Stream) <= 0)
			break;
		cursor->nextHeader = lpk_alloc_node();
		if (fseek(archive->Stream, cursor->headerNode.nextHeaderOffset, SEEK_SET) != 0)
			break;
		cursor = cursor->nextHeader;
	}
	return archive;
}

void lpk_release_pack(LpkArchive *archive)
{
	if (archive->fileList != NULL) {
		while (archive->fileList->nextHeader != NULL)
		{
			LpkEntry *tmp = archive->fileList->nextHeader;
			free(archive->fileList);
			archive->fileList = tmp;
		}
		free(archive->fileList);
	}
	fclose(archive->Stream);
	free(archive);
}

static void lpk_create_dir(const char *filePath, const char *root)
{
	int count = 0;
	char tmpSubDir[MAX_PATH];
	char tmpDir[MAX_PATH];
	for (int i = 0; i < (int)strlen(filePath); i++) {
		if (filePath[i] == '/' || filePath[i] == '\\')
		{
			strcpy(tmpSubDir, filePath);
			tmpSubDir[i] = '\0';
			strcpy(tmpDir, root);
			strcat(tmpDir, tmpSubDir);
			CreateDirectory(tmpDir, NULL);
		}
	}
}

void lpk_unpackto(LpkArchive *archive, LpkEntry *entry, const char *destDir)
{
	char destPath[MAX_PATH];
	lpk_create_dir(entry->headerNode.fileName, destDir);
	strcpy(destPath, destDir);
	strcat(destPath, entry->headerNode.fileName);
	FILE *destFile = fopen(destPath, "wb+");
	fseek(archive->Stream, entry->headerNode.fileOffset, SEEK_SET);
	long readCount = 0;
	char buffer[1024];
	int len;
	while (readCount < entry->headerNode.fileSize) {
		if ((len = (int)fread(buffer, 1, 1024, archive->Stream)) <= 0)
			break;
		if (len + readCount > entry->headerNode.fileSize)
			len = entry->headerNode.fileSize - readCount;
		fwrite(buffer, 1, len, destFile);
		readCount += len;
	}
}
void lpk_unpack(const char *srcFile, const char *destDir)
{
	LpkArchive *archive = lpk_read_pack(srcFile);
	LpkEntry *cursor = archive->fileList;
	while (cursor->nextHeader != NULL)
	{
		lpk_unpackto(archive, cursor, destDir);
		cursor = cursor->nextHeader;
	}
	lpk_release_pack(archive);
}