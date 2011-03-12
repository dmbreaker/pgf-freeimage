// ==========================================================
// JPEG2000 J2K codestream Loader and Writer
//
// Design and implementation by
// - Hervй Drolon (drolon@infonie.fr)
//
// This file is part of FreeImage 3
//
// COVERED CODE IS PROVIDED UNDER THIS LICENSE ON AN "AS IS" BASIS, WITHOUT WARRANTY
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, WITHOUT LIMITATION, WARRANTIES
// THAT THE COVERED CODE IS FREE OF DEFECTS, MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE
// OR NON-INFRINGING. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE COVERED
// CODE IS WITH YOU. SHOULD ANY COVERED CODE PROVE DEFECTIVE IN ANY RESPECT, YOU (NOT
// THE INITIAL DEVELOPER OR ANY OTHER CONTRIBUTOR) ASSUME THE COST OF ANY NECESSARY
// SERVICING, REPAIR OR CORRECTION. THIS DISCLAIMER OF WARRANTY CONSTITUTES AN ESSENTIAL
// PART OF THIS LICENSE. NO USE OF ANY COVERED CODE IS AUTHORIZED HEREUNDER EXCEPT UNDER
// THIS DISCLAIMER.
//
// Use at your own risk!
// ==========================================================

#ifdef _WIN32
#include <windows.h>
#endif

#include "FreeImage.h"
#include "Utilities.h"
#include "../libpgf/include/PGFimage.h"
#include "../libpgf/include/Stream.h"


// ==========================================================
// Plugin Interface
// ==========================================================

static int s_format_id;

// ==========================================================
// Plugin Implementation
// ==========================================================

static const char * DLL_CALLCONV
Format()
{
	return "PGF";
}

static const char * DLL_CALLCONV
Description() {
	return "Progressive Graphics Format";
}

static const char * DLL_CALLCONV
Extension() {
	return "pgf";
}

static const char * DLL_CALLCONV
RegExpr() {
	return NULL;
}

static const char * DLL_CALLCONV
MimeType() {
	return "image/pgf";
}

static BOOL DLL_CALLCONV
Validate(FreeImageIO *io, fi_handle handle)
{
	/*BYTE jpc_signature[] = { 0xFF, 0x4F };
	BYTE signature[2] = { 0, 0 };

	long tell = io->tell_proc(handle);
	io->read_proc(signature, 1, sizeof(jpc_signature), handle);
	io->seek_proc(handle, tell, SEEK_SET);

	return (memcmp(jpc_signature, signature, sizeof(jpc_signature)) == 0);*/

	return true;
}

static BOOL DLL_CALLCONV
SupportsExportDepth(int depth) {
	return (
		(depth == 24) || 
		(depth == 32)
	);
}

static BOOL DLL_CALLCONV 
SupportsExportType(FREE_IMAGE_TYPE type) {
	return (
		(type == FIT_BITMAP)  /*||
		(type == FIT_UINT32)  ||
		(type == FIT_RGB16) || 
		(type == FIT_RGBA16)*/
	);
}

// ----------------------------------------------------------

static void * DLL_CALLCONV
Open(FreeImageIO *io, fi_handle handle, BOOL read)
{
	return NULL;
}

static void DLL_CALLCONV
Close(FreeImageIO *io, fi_handle handle, void *data)
{
}

// ----------------------------------------------------------
// ============================================================
// ============================================================
// ============================================================
class CFreeImageFileStream : public CPGFStream
{
protected:
	FreeImageIO* mIO;
	fi_handle mHandle;

public:
	CFreeImageFileStream()
	{
		mIO = NULL;
		mHandle = NULL;
	}

	CFreeImageFileStream(FreeImageIO *io, fi_handle handle)
	{
		mIO = io;
		mHandle = handle;
	}
	// ============================================================
	virtual void Read( int* count, void* buffer )
	{
		*count = mIO->read_proc(buffer, *count, 1, mHandle);
	}
	// ============================================================
	virtual void Write( int* count, void* buffer )
	{
		*count = mIO->write_proc(buffer, *count, 1, mHandle);
	}
	// ============================================================
	virtual void SetPos(short posMode, INT64 posOff)
	{
		mIO->seek_proc(mHandle, (long)posOff, posMode);
	}
	// ============================================================
	virtual UINT64 GetPos() const
	{
		return (UINT64)mIO->tell_proc(mHandle);
	}
	// ============================================================
	virtual bool IsValid() const
	{
		return ( GetPos() >= 0 );
	}
	// ============================================================
};
// ============================================================
// ============================================================
// ============================================================

static FIBITMAP * DLL_CALLCONV
Load(FreeImageIO *io, fi_handle handle, int page, int flags, void *data)
{
	FIBITMAP *dib = NULL;
	BOOL header_only = (flags & FIF_LOAD_NOPIXELS) == FIF_LOAD_NOPIXELS;
	
	CPGFImage pgf;

	CFreeImageFileStream stream(io, handle);

	try
	{
		// open pgf image
		pgf.Open(&stream);

	}
	catch(IOException& e)
	{
		int err = e.error;
		if (err >= AppError) err -= AppError;
		char* errText = "Error: Opening and reading PGF image failed";
		return NULL;
	}

	try
	{
		int bpp = (pgf.GetHeader()->channels * 8);
		dib = FreeImage_AllocateHeader(header_only, pgf.Width(), pgf.Height(), bpp, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);//@ возможно нужно маски перевернуть
		//else
		//	dib = FreeImage_AllocateHeader(header_only, pgf.Width(), pgf.Height(), 24, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);//@ возможно нужно маски перевернуть
		//dib = FreeImage_AllocateHeader(header_only, pgf.Width(), pgf.Height(), pgf.BPP(), FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);//@ возможно нужно маски перевернуть
		

		if( !header_only )
		{
			pgf.Read();

			// copy bits
			//int bpp = 32;	// pgf.BPP()
			//UINT8* buff = new UINT8[pgf.Width()*pgf.Height()*bpp/8];	//@ !код опасный, но для теста подойдет 	//(UINT8 *)image->GetBits();
			UINT8* buff = (UINT8*)FreeImage_GetBits(dib);

			int pitch = pgf.Width() * (bpp/8);	// 32Bit buffer

			if (pgf.Mode() == ImageModeRGB48)
			{
				int map[] = { 2, 1, 0, 3 };
				pgf.GetBitmap(-pitch, &(buff[(pgf.Height() - 1) * pitch]), bpp, map);
				//pgf.GetBitmap(pitch, &(buff[0]), bpp, map);
			}
			else
			{
		//#ifdef _BIG_ENDIAN__
		//		int map[] = { 2, 1, 0, 3 };
		//		//pgf.GetBitmap(-pitch, &(buff[(pgf.Height() - 1) * pitch]), bpp, map);
		//		pgf.GetBitmap(pitch, &(buff[0]), bpp, map);
		//#else
				int map[] = { 2, 1, 0, 3 };
				pgf.GetBitmap(-pitch, &(buff[(pgf.Height() - 1) * pitch]), bpp);
				//pgf.GetBitmap(pitch, &(buff[0]), bpp, map);
		//#endif
			}

			// update color table if image is indexed or bitmap
			if ((pgf.Mode() == ImageModeIndexedColor))
			{
				// cannot get number of color table entries directly, so use 2^bitdepth
				///image->SetColorTable(0, 1 << pgf.BPP(), pgf.GetColorTable());
				return NULL;
			}
			else if (pgf.GetHeader()->mode == ImageModeBitmap)
			{
				RGBQUAD bw[2];
				bw[0].rgbRed = 255;
				bw[0].rgbGreen = 255;
				bw[0].rgbBlue = 255;
				bw[1].rgbRed = 0;
				bw[1].rgbGreen = 0;
				bw[1].rgbBlue = 0;
				///image->SetColorTable(0, 2, bw);
				return NULL;
			}
		}

		if (dib == NULL)
		{
			throw FI_MSG_ERROR_DIB_MEMORY;
		}

		// set resolution information
		FreeImage_SetDotsPerMeterX(dib, 90.0f * 100.0f / 2.54f);//bih.biXPelsPerMeter);
		FreeImage_SetDotsPerMeterY(dib, 90.0f * 100.0f / 2.54f);//bih.biYPelsPerMeter);

		FreeImage_SetTransparent(dib, (pgf.GetHeader()->channels == 4));

		return dib;

   ////lpImage->setPixels (pgf.Width(), pgf.Height(), (pgf.GetHeader()->channels == 4), reinterpret_cast<unsigned long*>(buff));


	}
	catch (const char *text)
	{
		if(dib) FreeImage_Unload(dib);

		FreeImage_OutputMessageProc(s_format_id, text);
		return NULL;
	}

	return NULL;
}
// ============================================================
static BOOL DLL_CALLCONV
Save(FreeImageIO *io, FIBITMAP *dib, fi_handle handle, int page, int flags, void *data)
{
	return FALSE;
}

// ==========================================================
//   Init
// ==========================================================

void DLL_CALLCONV
InitPGF(Plugin *plugin, int format_id)
 {
	s_format_id = format_id;

	plugin->format_proc = Format;
	plugin->description_proc = Description;
	plugin->extension_proc = Extension;
	plugin->regexpr_proc = RegExpr;
	plugin->open_proc = Open;
	plugin->close_proc = Close;
	plugin->pagecount_proc = NULL;
	plugin->pagecapability_proc = NULL;
	plugin->load_proc = Load;
	plugin->save_proc = NULL;	//Save
	plugin->validate_proc = Validate;
	plugin->mime_proc = MimeType;
	plugin->supports_export_bpp_proc = NULL;//SupportsExportDepth;
	plugin->supports_export_type_proc = NULL;//SupportsExportType;
	plugin->supports_icc_profiles_proc = NULL;
}
