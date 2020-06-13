// Author:	supadupaplex
// License:	cheack readme.txt and license.txt

#ifndef FOPS_H
#define FOPS_H

#ifdef _WIN32
	#include <windows.h>
	#define PATH_LEN MAX_PATH
	#define DIR_DELIM		"\\"
	#define DIR_DELIM_CH	'\\'
	#define DIR_NOT_DELIM		"/"
	#define DIR_NOT_DELIM_CH	'/'
#else
	#include <limits.h>
	#define PATH_LEN PATH_MAX
	#define DIR_DELIM		"/"
	#define DIR_DELIM_CH	'/'
	#define DIR_NOT_DELIM		"\\"
	#define DIR_NOT_DELIM_CH	'\\'
#endif

size_t FileSize(FILE **ptrFile); // Reads file size
void FileReadBlock(FILE **ptrSrcFile, void * DstBuff, size_t Addr, size_t Size); // Reads chunk from file
void FileWriteBlock(FILE **ptrDstFile, const void * SrcBuff, size_t Addr, size_t Size); // Writes chunk to file
void FileWriteBlock(FILE **ptrDstFile, const void * SrcBuff, size_t Size); // Writes chunk to file from prev. pos
void SafeFileOpen(FILE **ptrFile, const char * FileName, const char * Mode); // Opens file, checks that everything is alright
void FileGetExtension(const char * Path, char * OutputBuffer, int OutputBufferSize); // Fetches extension from file name
void FileGetName(const char * Path, char * OutputBuffer, int OutputBufferSize, bool WithExtension); // Fetches short name from full file name
void FileGetFullName(const char * Path, char * OutputBuffer, int OutputBufferSize); // Cuts extension from full file name
void FileGetPath(const char * Path, char * OutputBuffer, int OutputBufferSize); // Fetches path from full file name
bool CheckFile(char * FileName); // Checks that file exists
bool CheckDir(const char * Path); // Check if Path is directory
void NewDir(const char * DirName); // Makes dir
void GenerateFolders(char * cPath); // Makes all dirs from the path if they don't exist
void PatchSlashes(char * cPathBuff, int BuffSize, bool PakToFs); // Fixes slashes in path
void ProgGetPath(char * OutputBuffer, int OutputBufferSize); // Gets path to the executable file
void FileSafeRename(char * OldName, char * NewName); // Safe file rename
void DirIterInit(const char * Dir); // Init/reset dir iterator
void DirIterClose(); // Deinit dir iterator
const char * DirIterGet(); // Dir iterator, returns NULL on end

#endif