// Author:	supadupaplex
// License:	BSD-3-Clause (check out license.txt)

// 
// This file contains image conversion functions
// 

////////// Includes //////////
#include "util.h"
#include "main.h"

////////// Functions //////////
bool ConvertPNGtoPSI(const char * FileName);
bool ConvertPSItoPNG(const char * FileName);
void PatchRGBAPalette(uchar * RGBAPalette, ulong RGBAPaletteSize, bool MulDiv);

bool ConvertPNGtoPSI(const char * FileName)
{
	FILE *ptrInputF;						// Input file
	FILE *ptrOutputF;						// Output file

	sPNGHeader PNGHeader;
	sPSIHeader PSIHeader;

	sPNGData * PNGPalette;
	sPNGData * PNGBitmap;
	uchar BytesPerPixel;

	char OutFile[PATH_LEN];
	char TexName[64];

	// Open file
	SafeFileOpen(&ptrInputF, FileName, "rb");

	// Read PNG header
	PNGHeader.UpdateFromFile(&ptrInputF);
	PNGHeader.SwapEndian();

	// Check PNG header
	if (PNGHeader.CheckType() == PNG_RGBA || PNGHeader.CheckType() == PNG_RGB)
	{
		if (PNGHeader.CheckType() == PNG_RGBA)
		{
			printf("32-bit PNG \nParameters - Width: %i, Height: %i \n", PNGHeader.Width, PNGHeader.Height);
			BytesPerPixel = 4;
		}
		else
		{
			printf("24-bit PNG \nParameters - Width: %i, Height: %i \n", PNGHeader.Width, PNGHeader.Height);
			BytesPerPixel = 3;
		}


		// Prepare PSI data
		PNGBitmap = PNGReadBitmap(&ptrInputF, PNGHeader.Width, PNGHeader.Height, BytesPerPixel, PNGHeader.BitDepth);
		for (ulong i = 0; i < PNGBitmap->DataSize; i++)			// Divide PNG bitmap bytes by 2 to match PSI
			PNGBitmap->Data[i] /= 2;

		// Create output file
		FileGetFullName(FileName, OutFile, sizeof(OutFile));
		strcat(OutFile, ".psi");
		SafeFileOpen(&ptrOutputF, OutFile, "wb");

		// Write PSI header
		FileGetName(FileName, TexName, sizeof(TexName), false);
		PSIHeader.Update(TexName, PNGHeader.Width, PNGHeader.Height, PSI_RGBA);
		FileWriteBlock(&ptrOutputF, &PSIHeader, sizeof(sPSIHeader));

		// Write PSI data
		FileWriteBlock(&ptrOutputF, PNGBitmap->Data, PNGBitmap->DataSize);

		// Free memory
		free(PNGBitmap->Data);

		// Close files
		fclose(ptrOutputF);

		puts("Done\n\n");
	}
	else if (PNGHeader.CheckType() == PNG_INDEXED)
	{
		printf("8-bit PNG \nParameters - Width: %i, Height: %i \n", PNGHeader.Width, PNGHeader.Height);
		BytesPerPixel = 1;

		// Prepare PSI palette
		PNGPalette = PNGReadPalette(&ptrInputF);
		PatchRGBAPalette(PNGPalette->Data, PNGPalette->DataSize, false);

		// Prepare PSI bitmap
		PNGBitmap = PNGReadBitmap(&ptrInputF, PNGHeader.Width, PNGHeader.Height, BytesPerPixel, PNGHeader.BitDepth);

		// Create output file
		FileGetFullName(FileName, OutFile, sizeof(OutFile));
		strcat(OutFile, ".psi");
		SafeFileOpen(&ptrOutputF, OutFile, "wb");

		// Write PSI header
		FileGetName(FileName, TexName, sizeof(TexName), false);
		PSIHeader.Update(TexName, PNGHeader.Width, PNGHeader.Height, PSI_INDEXED);
		FileWriteBlock(&ptrOutputF, &PSIHeader, sizeof(sPSIHeader));

		// Write PSI data
		FileWriteBlock(&ptrOutputF, PNGPalette->Data, PNGPalette->DataSize);
		FileWriteBlock(&ptrOutputF, PNGBitmap->Data, PNGBitmap->DataSize);

		// Free memory
		free(PNGBitmap->Data);
		free(PNGPalette->Data);

		// Close files
		fclose(ptrOutputF);

		puts("Done\n\n");
	}
	else
	{
		puts("Unsupported PNG ...");
		return false;
	}

	// Close files
	fclose(ptrInputF);

	return true;
}

bool ConvertPSItoPNG(const char * FileName)
{
	FILE *ptrInputF;						// Input file
	FILE *ptrOutputF;						// Output file

	sPNGHeader PNGHeader;
	sPSIHeader PSIHeader;

	sPNGData PNGPalette;
	sPNGData PNGBitmap;
	uchar BytesPerPixel;

	char OutFile[PATH_LEN];

	uchar * RawBitmap;
	ulong RawBitmapSize;

	uchar * RGBAPalette;
	ulong RGBAPaletteSize;

	// Open PSI
	SafeFileOpen(&ptrInputF, FileName, "rb");

	// Read PSI Header
	PSIHeader.UpdateFromFile(&ptrInputF, 0);

	// Check PSI Header
	if (PSIHeader.CheckType() == PSI_RGBA)
	{
		printf("32-bit PSI image \nParameters - Width: %i, Height: %i \n", PSIHeader.Width1, PSIHeader.Height1);
		BytesPerPixel = 4;

		// Prepare PNG data
		RawBitmapSize = PSIHeader.Height1 * PSIHeader.Width1 * BytesPerPixel;
		RawBitmap = (uchar *)malloc(RawBitmapSize);
		if (RawBitmap == NULL)
		{
			puts("Unable to allocate memory! \n");
			exit(EXIT_FAILURE);
		}
		FileReadBlock(&ptrInputF, RawBitmap, sizeof(sPSIHeader), RawBitmapSize);
		for (ulong i = 0; i < RawBitmapSize; i++)		// Multiply bitmap bytes by 2
			RawBitmap[i] *= 2;
		PNGBitmap.Data = RawBitmap;
		PNGBitmap.DataSize = RawBitmapSize;

		// Create output file
		FileGetFullName(FileName, OutFile, sizeof(OutFile));
		strcat(OutFile, ".png");
		SafeFileOpen(&ptrOutputF, OutFile, "wb");

		// Write PNG header
		PNGHeader.Update(PSIHeader.Width1, PSIHeader.Height1, PNG_RGBA);
		PNGHeader.SwapEndian();
		FileWriteBlock(&ptrOutputF, &PNGHeader, sizeof(sPNGHeader));

		// Write PNG Data
		PNGWriteChunk(&ptrOutputF, "iTXt", "Comment\0\0\0\0\0Converted with PS2 Half-life PSI tool", strlen("CommentConverted with PS2 Half-life PSI tool") + 5);
		PNGWriteBitmap(&ptrOutputF, PSIHeader.Width1, PSIHeader.Height1, BytesPerPixel, &PNGBitmap);
		PNGWriteChunk(&ptrOutputF, "IEND", NULL, NULL);

		// Free memory
		free(PNGBitmap.Data);

		// Close file
		fclose(ptrOutputF);

		puts("Done\n\n");
	}
	else if (PSIHeader.CheckType() == PSI_INDEXED)
	{
		printf("8-bit PSI image \nParameters - Width: %i, Height: %i \n", PSIHeader.Width1, PSIHeader.Height1);
		BytesPerPixel = 1;

		// Prepare PNG palette
		RGBAPaletteSize = 0x400;
		RGBAPalette = (uchar *)malloc(RGBAPaletteSize);
		if (RGBAPalette == NULL)
		{
			puts("Unable to allocate memory! \n");
			exit(EXIT_FAILURE);
		}
		FileReadBlock(&ptrInputF, RGBAPalette, sizeof(sPSIHeader), RGBAPaletteSize);
		PatchRGBAPalette(RGBAPalette, RGBAPaletteSize, true);
		PNGPalette.Data = RGBAPalette;
		PNGPalette.DataSize = RGBAPaletteSize;

		// Prepare PNG bitmap
		RawBitmapSize = PSIHeader.Height1 * PSIHeader.Width1 * BytesPerPixel;
		RawBitmap = (uchar *)malloc(RawBitmapSize);
		if (RawBitmap == NULL)
		{
			puts("Unable to allocate memory! \n");
			exit(EXIT_FAILURE);
		}
		FileReadBlock(&ptrInputF, RawBitmap, sizeof(sPSIHeader) + RGBAPaletteSize, RawBitmapSize);
		PNGBitmap.Data = RawBitmap;
		PNGBitmap.DataSize = RawBitmapSize;

		// Create output file
		FileGetFullName(FileName, OutFile, sizeof(OutFile));
		strcat(OutFile, ".png");
		SafeFileOpen(&ptrOutputF, OutFile, "wb");

		// Write PNG header
		PNGHeader.Update(PSIHeader.Width1, PSIHeader.Height1, PNG_INDEXED);
		PNGHeader.SwapEndian();
		FileWriteBlock(&ptrOutputF, &PNGHeader, sizeof(sPNGHeader));

		// Write PNG data
		PNGWriteChunk(&ptrOutputF, "iTXt", "Comment\0\0\0\0\0Converted with PS2 Half-life PSI tool", strlen("CommentConverted with PS2 Half-life PSI tool") + 5);
		PNGWritePalette(&ptrOutputF, &PNGPalette);
		PNGWriteBitmap(&ptrOutputF, PSIHeader.Width1, PSIHeader.Height1, BytesPerPixel, &PNGBitmap);
		PNGWriteChunk(&ptrOutputF, "IEND", NULL, NULL);

		// Free memory
		free(PNGBitmap.Data);
		free(RGBAPalette);

		// Close file
		fclose(ptrOutputF);

		puts("Done\n\n");
	}
	else
	{
		puts("Unknown image type.");
		return false;
	}

	// Close file
	fclose(ptrInputF);

	return true;
}

void PatchRGBAPalette(uchar * RGBAPalette, ulong RGBAPaletteSize, bool MulDiv)			// Patch color table.
{
	uchar Remainder;
	uchar Temp;

	// Reformat color table
	uint PaletteElementSize = 4;
	for (ulong i = 0; i < RGBAPaletteSize; i++)
	{
		Remainder = i % (0x20 * PaletteElementSize);

		if (((0x10 * PaletteElementSize) <= Remainder) && (Remainder < (0x18 * PaletteElementSize)))
		{
			Temp = RGBAPalette[i];
			RGBAPalette[i] = RGBAPalette[i - (0x08 * PaletteElementSize)];
			RGBAPalette[i - (0x08 * PaletteElementSize)] = Temp;
		}
	}

	// Multiply/Divide color table elements
	for (ulong i = 0; i < RGBAPaletteSize; i++)
	{
		if ((i + 1) % PaletteElementSize == 0)	// Alpha
		{
			if (MulDiv == true)
			{
				if (RGBAPalette[i] > 0x7F)
					RGBAPalette[i] = 0x7F;

				RGBAPalette[i] *= 2;
			}
			else
			{
				RGBAPalette[i] /= 2;

				if (RGBAPalette[i] == 0x7F)
					RGBAPalette[i] = 0x80;
			}

			// Black out transparent pixels to avoid possible artifacts
			if (RGBAPalette[i] == 0x00)
			{
				RGBAPalette[i - 1] = 0;
				RGBAPalette[i - 2] = 0;
				RGBAPalette[i - 3] = 0;
			}
		}
		else									// RGB
		{
			if (MulDiv == true)
				RGBAPalette[i] *= 2;
			else
				RGBAPalette[i] /= 2;
		}
	}
}

int main(int argc, char * argv[])
{
	char Extension[5];

	// Output info
	puts(PROG_TITLE);

	// Check arguments
	if (argc == 1)
	{
		// No arguments - show help screen
		puts(PROG_INFO);
		UTIL_WAIT_KEY("Press any key to exit ...");
	}
	else if (argc == 2)
	{
		FileGetExtension(argv[1], Extension, sizeof(Extension));

		printf("Processing file: %s \n", argv[1]);
		if (!strcmp(Extension, ".png"))				// Convert PNG to PSI
		{
			if (ConvertPNGtoPSI(argv[1]) == true)
				return 0;
			else
				puts("Can't convert image ... \n");
		}
		else if (!strcmp(Extension, ".psi"))		// Convert PSI to PNG
		{
			if (ConvertPSItoPNG(argv[1]) == true)
				return 0;
			else
				puts("Can't convert image ... \n");
		}
		else
		{
			puts("Wrong file extension ... \n");
		}
	}
	else
	{
		puts("Too many arguments ... \n");
	}
	
	return 1;
}
