#pragma once

#include <stdio.h>
#include <string.h>
#include <windows.h>
#pragma pack(1)
typedef struct {
	long fileSize;             //文件大小
	long fileOffset;           //文件起始地址
	long nextHeaderOffset;     //下一个文件头地址
	char fileName[MAX_PATH];   //文件名
}LpkEntity;

typedef struct package_entry {
	LpkEntity headerNode;
	struct package_entry *nextHeader;
}LpkEntry;
#pragma pack()

typedef struct {
	FILE *Stream;
	LpkEntry *fileList;
}LpkArchive;

__declspec(dllexport) void lpk_pack(const char *dirPath, int rootLength, const char *destFile);
__declspec(dllexport) LpkArchive *lpk_write_pack(const char *destFile);
__declspec(dllexport) void lpk_packto(LpkArchive *archive, LpkEntry *entry, const char *destDir);
__declspec(dllexport) LpkEntry *lpk_create_entry(const char *filename);
__declspec(dllexport) void lpk_free_entry(LpkEntry *entry);

__declspec(dllexport) LpkArchive *lpk_read_pack(const char *srcFile);
__declspec(dllexport) void lpk_release_pack(LpkArchive *archive);
__declspec(dllexport) void lpk_unpackto(LpkArchive *archive, LpkEntry *entry, const char *destDir);
__declspec(dllexport) void lpk_unpack(const char *srcFile, const char *destDir);
