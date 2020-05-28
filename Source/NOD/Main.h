/*
=====================================================================
Copyright (c) 2018, Alexey Leushin
All rights reserved.

Redistribution and use in source and binary forms, with or
without modification, are permitted provided that the following
conditions are met:
- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
- Neither the name of the copyright holders nor the names of its
contributors may be used to endorse or promote products derived
from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
=====================================================================
*/

//
// This file contains all definitions and declarations
//

#pragma once

////////// Includes //////////
#include <stdio.h>		// puts(), printf(), sscanf(), snprintf(), rename(), remove()
#include <conio.h>		// _getch()
#include <direct.h>		// _mkdir()
#include <string.h>		// strcpy(), strcat(), strlen(), strtok(), strncpy(), memset()
#include <malloc.h>		// malloc(), free()
#include <stdlib.h>		// exit()
#include <math.h>		// round(), sqrt(), ceil()
#include <ctype.h>		// tolower()
#include <sys\stat.h>	// stat()
#include <windows.h>	// CreateDitectoryA()

////////// Definitions //////////
#define PROG_VERSION "0.9"

// Node format specificators
enum eNodFormats
{
	NOD_FORMAT_PC,
	NOD_FORMAT_PS2,
	NOD_ERR_VERSION,
	NOD_ERR_UNKNOWN
};

// Proper node graph version
#define NOD_SUPPORTED_VESION		16

// PS2 extra bytes in CNode structure (took those from PS2's c0a0.nod)
#define PS2_CN_EXTRA1	0x00000000
#define PS2_CN_EXTRA2	0x00000807

// Sizes of structures inside node file
#define SZ_CGRAPH		8396
#define SZ_PC_CNODE		88
#define SZ_PS2_CNODE	96
#define SZ_CLINK		24
#define SZ_DIST_INFO	16

////////// Typedefs //////////
typedef unsigned short int ushort;		// 2 byte unsigned variable
typedef unsigned long int ulong;		// 4 byte unsigned variable
typedef unsigned int uint;				// 4 byte ungigned variable
typedef unsigned char uchar;			// 1 byte unsigned variable

////////// Functions //////////
ulong FileSize(FILE **ptrFile);																			// Get size of file
void FileReadBlock(FILE **ptrSrcFile, void * DstBuff, ulong Addr, ulong Size);							// Read block from file to buffer
void FileWriteBlock(FILE **ptrDstFile, void * SrcBuff, ulong Addr, ulong Size);							// Write data from buffer to file
void FileWriteBlock(FILE **ptrDstFile, void * SrcBuff, ulong Size);										// Write data from buffer to file
void SafeFileOpen(FILE **ptrFile, const char * FileName, char * Mode);									// Try to open file, if problem oocur then exit
void FileGetExtension(const char * Path, char * OutputBuffer, uint OutputBufferSize);					// Get file extension
void FileGetName(const char * Path, char * OutputBuffer, uint OutputBufferSize, bool WithExtension);	// Get name of file with or without extension
void FileGetFullName(const char * Path, char * OutputBuffer, uint OutputBufferSize);					// Get full file name (with folders) without extension
void FileGetPath(const char * Path, char * OutputBuffer, uint OutputBufferSize);						// Get file path
bool CheckFile(char * FileName);																		// Check existance of file
void GenerateFolders(char * cPath);																		// Make sure, that all folders in path are existing
void PatchSlashes(char * cPathBuff, ulong BuffSize, bool SlashToBackslash);								// Patch slashes when transitioning between PAK and Windows file names
bool CheckDir(const char * Path);																		// Check if path is directory
void NewDir(const char * DirName);																		// Create directory
void FileSafeRename(char * OldName, char * NewName);													// Safe file reame

////////// Structures //////////
#pragma pack(1)				// Fix unwanted 0x00 bytes in structure
struct sCGraph
{
	uchar Somedata1[12];

	uint pNodes;
	uint pLinkPool;
	uint pRouteInfo;

	int NodeCount;
	int LinkCount;
	int RouteCount;

	uchar SomeData2[8344];

	uint pHashLinks;
	int HashCount;

	uchar SomeData3[8];
};

#pragma pack(1)				// Fix unwanted 0x00 bytes in structure
struct sCNode_PC
{
	uchar SomeData[SZ_PC_CNODE];
};

#pragma pack(1)				// Fix unwanted 0x00 bytes in structure
struct sCNode_PS2
{
	sCNode_PC CNode;
	ulong ExtraField1;
	ulong ExtraField2;
};

#pragma pack(1)				// Fix unwanted 0x00 bytes in structure
struct sCLink
{
	uchar SomeData[SZ_CLINK];
};

#pragma pack(1)				// Fix unwanted 0x00 bytes in structure
struct sDIST_INFO
{
	uchar SomeData[SZ_DIST_INFO];
};

#pragma pack(1)				// Fix unwanted 0x00 bytes in structure
struct sNodeGraph
{
	int Version;			// Should be 16
	sCGraph CGraph;
	sCNode_PS2 * CNodes;
	sCLink * CLinks;
	sDIST_INFO * DistInfo;
	char * Routes;
	short * Hashes;

	void Init()
	{
		// Clear memory
		memset(this, 0x00, sizeof(this));

		// Initialize pointers
		CNodes = NULL;
		CLinks = NULL;
		DistInfo = NULL;
		Routes = NULL;
		Hashes = NULL;
	}

	void Deinit()
	{
		// Free memory
		if (CNodes != NULL)		free(CNodes);
		if (CLinks != NULL)		free(CLinks);
		if (DistInfo != NULL)	free(DistInfo);
		if (Routes != NULL)		free(Routes);
		if (Hashes != NULL)		free(Hashes);
	}

	int LoadAndCheckHeader(FILE **ptrFile)
	{
		// Load header
		FileReadBlock(ptrFile, this, 0, sizeof(Version) + sizeof(sCGraph));

		// Calculate base size
		ulong BaseSize =	sizeof(Version) +
							sizeof(sCGraph) +
							sizeof(sCLink) * CGraph.LinkCount +
							sizeof(sDIST_INFO) * CGraph.NodeCount +
							sizeof(char) * CGraph.RouteCount +
							sizeof(short) * CGraph.HashCount;

		// Check file
		if (Version == NOD_SUPPORTED_VESION)
		{
			if (FileSize(ptrFile) == (BaseSize + sizeof(sCNode_PC) * CGraph.NodeCount))
			{
				return NOD_FORMAT_PC;
			}
			else if (FileSize(ptrFile) == (BaseSize + sizeof(sCNode_PS2) * CGraph.NodeCount))
			{
				return NOD_FORMAT_PS2;
			}
			else
			{
				return NOD_ERR_UNKNOWN;
			}
		}
		else
		{
			return NOD_ERR_VERSION;
		}
	}

	int UpdateFromFile(FILE **ptrFile)
	{
		// Load and check header
		int Result = LoadAndCheckHeader(ptrFile);
		if (Result == NOD_ERR_VERSION || Result == NOD_ERR_UNKNOWN)
			return Result;

		// Allocate memory for structures //

		// Nodes
		if (CNodes != NULL)
			free(CNodes);
		CNodes = (sCNode_PS2 *)		calloc(sizeof(sCNode_PS2) * CGraph.NodeCount, 1);

		// Links
		if (CLinks != NULL)
			free(CLinks);
		CLinks = (sCLink *)			calloc(sizeof(sCLink) * CGraph.LinkCount, 1);

		// Dists
		if (DistInfo != NULL)
			free(DistInfo);
		DistInfo = (sDIST_INFO *)	calloc(sizeof(sDIST_INFO) * CGraph.NodeCount, 1);

		// Routes
		if (Routes != NULL)
			free(Routes);
		Routes = (char *)			calloc(sizeof(char) * CGraph.RouteCount, 1);

		// Hashes
		if (Hashes != NULL)
			free(Hashes);
		Hashes = (short *)			calloc(sizeof(short) * CGraph.HashCount, 1);

		// Check allocation
		if (CNodes == NULL || CLinks == NULL || DistInfo == NULL || Routes == NULL || Hashes == NULL)
		{
			puts("Memory allocation failed!");
			getch();
			exit(EXIT_FAILURE);
		}

		// Load structures //
		ulong Pointer = sizeof(int) + sizeof(sCGraph);

		// Nodes
		if (Result == NOD_FORMAT_PS2)
		{
			FileReadBlock(ptrFile, CNodes, Pointer, sizeof(sCNode_PS2) * CGraph.NodeCount);
			Pointer += sizeof(sCNode_PS2) * CGraph.NodeCount;
		}
		else if (Result == NOD_FORMAT_PC)
		{
			// Convert to PS2 format on the fly
			for (int i = 0; i < CGraph.NodeCount; i++)
			{
				FileReadBlock(ptrFile, &CNodes[i], Pointer, sizeof(sCNode_PC));
				Pointer += sizeof(sCNode_PC);

				CNodes[i].ExtraField1 = PS2_CN_EXTRA1;
				CNodes[i].ExtraField2 = PS2_CN_EXTRA2;
			}
		}

		// Links
		FileReadBlock(ptrFile, CLinks, Pointer, sizeof(sCLink) * CGraph.LinkCount);
		Pointer += sizeof(sCLink) * CGraph.LinkCount;

		// Dists
		FileReadBlock(ptrFile, DistInfo, Pointer, sizeof(sDIST_INFO) * CGraph.NodeCount);
		Pointer += sizeof(sDIST_INFO) * CGraph.NodeCount;

		// Routes
		FileReadBlock(ptrFile, Routes, Pointer, sizeof(char) * CGraph.RouteCount);
		Pointer += sizeof(char) * CGraph.RouteCount;
		
		// Hashes
		FileReadBlock(ptrFile, Hashes, Pointer, sizeof(short) * CGraph.HashCount);

		return Result;
	}

	void SaveToFile(FILE **ptrFile, eNodFormats Format)
	{
		// Save structures to file //
		ulong Pointer = 0;

		// Header
		FileWriteBlock(ptrFile, this, 0, sizeof(Version) + sizeof(sCGraph));
		Pointer += sizeof(int) + sizeof(sCGraph);

		// Nodes
		if (Format == NOD_FORMAT_PC)
		{
			// Convert to PC format on the fly
			for (int i = 0; i < CGraph.NodeCount; i++)
			{
				FileWriteBlock(ptrFile, &CNodes[i], sizeof(sCNode_PC));
				Pointer += sizeof(sCNode_PS2);
			}
		}
		else
		{
			FileWriteBlock(ptrFile, CNodes, sizeof(sCNode_PS2) * CGraph.NodeCount);
			Pointer += sizeof(sCNode_PS2) * CGraph.NodeCount;
		}

		// Links
		FileWriteBlock(ptrFile, CLinks, sizeof(sCLink) * CGraph.LinkCount);
		Pointer += sizeof(sCLink) * CGraph.LinkCount;

		// Dists
		FileWriteBlock(ptrFile, DistInfo, sizeof(sDIST_INFO) * CGraph.NodeCount);
		Pointer += sizeof(sDIST_INFO) * CGraph.NodeCount;

		// Routes
		FileWriteBlock(ptrFile, Routes, sizeof(char) * CGraph.RouteCount);
		Pointer += sizeof(char) * CGraph.RouteCount;

		// Hashes
		FileWriteBlock(ptrFile, Hashes, sizeof(short) * CGraph.HashCount);
	}
};
