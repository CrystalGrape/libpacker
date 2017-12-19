#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <windows.h>
#include "libpack.h"

static LpkEntry *lpk_alloc_node()
{
	LpkEntry *node
		= (LpkEntry *)malloc(sizeof(LpkEntry));
	node->nextHeader = NULL;
	return node;
}

static void lpk_init_filename(LpkEntry *node, const char *fileName)
{
	strcpy(node->headerNode.fileName, fileName);
}

static void lpk_list_files(const char *dirPath, const char *root
	, LpkEntry **currentNode)
{
	WIN32_FIND_DATA p;
	HANDLE hFind = FindFirstFile(dirPath, &p);
	char fileName[MAX_PATH];
	char subDir[MAX_PATH];
	char subRoot[MAX_PATH];
	do {
		if (p.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) {
			if (strcmp(p.cFileName, ".") == 0
				|| strcmp(p.cFileName, "..") == 0)
				continue;

			strcpy(subDir, root);
			strcat(subDir, p.cFileName);
			strcat(subDir, "/");
			strcat(subDir, "*.*");

			strcpy(subRoot, root);
			strcat(subRoot, p.cFileName);
			strcat(subRoot, "/");
			lpk_list_files(subDir, subRoot, currentNode);

		}
		else {
			strcpy(fileName, root);
			strcat(fileName, p.cFileName);
			lpk_init_filename(*currentNode, fileName);
			(*currentNode)->nextHeader = lpk_alloc_node();
			*currentNode = (*currentNode)->nextHeader;
		}
	} while (FindNextFile(hFind, &p) != 0);
	FindClose(hFind);
}

static void lpk_strcopy(char *dest, char *src, int offset, int len)
{
	int count = 0;
	while (count <= len && *(src + count + offset) != '\0') {
		*(dest + count) = *(src + count + offset);
		count++;
	}
	*(dest + count) = '\0';
}

static void lpk_root_path(char *filePath, int rootLength)
{
	char result[MAX_PATH];
	int length;
	lpk_strcopy(result, filePath, rootLength, MAXINT);
	length = (int)strlen(result);
	filePath[0] = '\0';
	for (int i = length - 1; i >= 0; i--) {
		if (result[i] == '\\' || result[i] == '/') {
			result[i + 1] = '\0';
			strcpy(filePath, result);
			break;
		}
	}
}

LpkArchive *lpk_write_pack(const char *destFile)
{
	LpkArchive *archive = (LpkArchive *)malloc(sizeof(LpkArchive));
	archive->Stream = fopen(destFile, "wb+");
	archive->fileList = NULL;
	return archive;
}

void lpk_packto(LpkArchive *archive, LpkEntry *entry, const char *destDir)
{
	WIN32_FIND_DATA p;
	char fileName[MAX_PATH];
	HANDLE hFind = FindFirstFile(entry->headerNode.fileName, &p);
	strcpy(fileName, destDir);
	strcat(fileName, p.cFileName);

	fseek(archive->Stream, 0, SEEK_END);
	long startPos = ftell(archive->Stream);
	FILE *srcFp = fopen(entry->headerNode.fileName, "rb");
	fseek(srcFp, 0, SEEK_END);
	entry->headerNode.fileSize = ftell(srcFp);
	fseek(srcFp, 0, SEEK_SET);
	entry->headerNode.fileOffset
		= startPos + sizeof(LpkEntity);
	entry->headerNode.nextHeaderOffset
		= entry->headerNode.fileOffset
		+ entry->headerNode.fileSize;

	strcpy(entry->headerNode.fileName, fileName);
	fwrite(&(entry->headerNode)
		, sizeof(LpkEntity), 1, archive->Stream);
	char buffer[1024];
	int len;
	while ((len = (int)fread(buffer, 1, 1024, srcFp)) > 0) {
		fwrite(buffer, 1, len, archive->Stream);
	}
	fclose(srcFp);
}

LpkEntry *lpk_create_entry(const char *filename)
{
	LpkEntry *entry = (LpkEntry *)malloc(sizeof(LpkEntry));
	strcpy(entry->headerNode.fileName, filename);
	return entry;
}

void lpk_free_entry(LpkEntry *entry)
{
	free(entry);
}

void lpk_pack(const char *dirPath, int rootLength, const char *destFile)
{
	assert(dirPath != NULL);
	assert(strlen(dirPath) != 0);
	char match[MAX_PATH];
	strcpy(match, dirPath);
	strcat(match, "*.*");
	LpkEntry *listHead = lpk_alloc_node();
	LpkEntry *cursor = listHead;
	LpkArchive *archive = lpk_write_pack(destFile);
	lpk_list_files(match, dirPath, &cursor);

	cursor = listHead;
	while (cursor->nextHeader != NULL) {
		LpkEntry *entry = lpk_create_entry(cursor->headerNode.fileName);
		char destDir[MAX_PATH];
		strcpy(destDir, entry->headerNode.fileName);
		lpk_root_path(destDir, rootLength);

		lpk_packto(archive, entry, destDir);
		lpk_free_entry(entry);
		LpkEntry *tmp = cursor;
		cursor = cursor->nextHeader;
		free(tmp);
	}
	lpk_release_pack(archive);
}