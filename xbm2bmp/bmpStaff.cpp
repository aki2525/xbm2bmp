#include "stdafx.h"
#include "shlwapi.h"

BOOL ExportBmpFile( HWND hwnd, PTSTR ptszFile, PRECT prcClient )
{
HDC hdc;
HANDLE hFile;
HDC hdcbmp = NULL;
BOOL bResult = FALSE;
TCHAR tsz[ MAX_PATH ];
LONG lSizeHead, lSizeWidth, lSizeImage;
DWORD dwSize;
PVOID pPixel = NULL;
HBITMAP hBitmap = NULL, hOldBitmap = NULL;
BITMAPINFO bi;
BITMAPFILEHEADER bfh;

	if ( !hwnd )
		return bResult;
	hdc = GetDC( hwnd );
	if ( !hdc )
		return bResult;

	_tcscpy( tsz, ptszFile );
	PathRenameExtension( tsz, _T( ".bmp" ) );
	hFile = CreateFile( tsz, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE ) {
		lSizeHead  = ( sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFO ) );
		lSizeWidth = ( ( prcClient->right - prcClient->left ) * sizeof( DWORD ) );
		lSizeImage = ( lSizeWidth * ( prcClient->bottom - prcClient->top ) );

		ZeroMemory( &bfh, sizeof( bfh ) );
		bfh.bfType = 0x4d42;	// 'BM'
		bfh.bfSize = lSizeHead + lSizeImage;
		bfh.bfOffBits = lSizeHead;
        
		ZeroMemory( &bi, sizeof( bi ) );
		bi.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
		bi.bmiHeader.biWidth = prcClient->right - prcClient->left;
		bi.bmiHeader.biHeight = prcClient->bottom - prcClient->top;
		bi.bmiHeader.biPlanes = 1;
		bi.bmiHeader.biBitCount = 32;
		bi.bmiHeader.biCompression = BI_RGB;
		bi.bmiHeader.biSizeImage = 0;
		bi.bmiHeader.biXPelsPerMeter = 0;
		bi.bmiHeader.biYPelsPerMeter = 0;
		bi.bmiHeader.biClrUsed = 0;
		bi.bmiHeader.biClrImportant = 0;
        
		hBitmap = CreateDIBSection( NULL, &bi, DIB_RGB_COLORS, &pPixel, NULL, 0 );
		if ( hBitmap )
			hdcbmp = CreateCompatibleDC( hdc );
		if ( hdcbmp )
			hOldBitmap = (HBITMAP)SelectObject( hdcbmp, hBitmap );

		if ( hdcbmp && pPixel ) {
			BitBlt( hdcbmp, 0, 0, prcClient->right - prcClient->left, prcClient->bottom - prcClient->top, hdc, prcClient->left, prcClient->top, SRCCOPY );
			WriteFile( hFile, &bfh, sizeof( BITMAPFILEHEADER ), &dwSize, NULL );
			if ( dwSize == sizeof( BITMAPFILEHEADER ) ) {
				WriteFile( hFile, &bi, sizeof( BITMAPINFO ), &dwSize, NULL );
				if ( dwSize == sizeof( BITMAPINFO ) ) {
					WriteFile( hFile, pPixel,  lSizeImage, &dwSize, NULL );
					if ( dwSize == lSizeImage ) {
						bResult = TRUE;
					}
				}
			}
		}

		if ( hdcbmp ) {
			SelectObject( hdcbmp, hOldBitmap );
			DeleteDC( hdcbmp );
		}
		if ( hBitmap )
			DeleteObject( hBitmap );
		ReleaseDC( hwnd, hdc );
		CloseHandle( hFile );
	}

	return bResult;
}
