#include "stdafx.h"

// Locals...
#define KEYWORD_WIDTH	"_width"
#define KEYWORD_HEIGHT	"_height"
#define KEYWORD_SPOTX	"_x_spot"
#define KEYWORD_SPOTY	"_y_spot"
#define KEYWORD_BITS	"_bits[]"
#define KEYWORD_BITS_BEGIN	"{"
#define KEYWORD_BITS_END	"}"

BOOL DeleteRemarks( HGLOBAL hXBM, DWORD dwSizeFile );
HGLOBAL ReadXBMFile( HWND hwnd, PTSTR ptszFile, PDWORD pdwSizeFile );
BOOL GetXBMParams( HGLOBAL hXBM, PDWORD pdwWidth, PDWORD pdwHeight, PDWORD pdwBits );
BOOL DrawXBM( HWND hwnd, HGLOBAL hXBM, DWORD dwWidth, DWORD dwHeight, DWORD dwBits );

static ULONG g_tblBits[ 8 ] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

BOOL ImportXBMFile( HWND hwnd, PTSTR ptszFile, PRECT prcClient )
{
BOOL bResult = FALSE;
HGLOBAL hFile = NULL;
DWORD dwSizeFile, dwWidth, dwHeight, dwBits;

	if ( !ptszFile )
		return bResult;

	hFile = ReadXBMFile( hwnd, ptszFile, &dwSizeFile );
	if ( hFile ) {
		bResult = DeleteRemarks( hFile, dwSizeFile );
		if ( bResult ) {
			bResult = GetXBMParams( hFile, &dwWidth, &dwHeight, &dwBits );
			if ( prcClient )
				SetRect( prcClient, 0, 0, dwWidth, dwHeight );
		}
		if ( bResult )
			bResult = DrawXBM( hwnd, hFile, dwWidth, dwHeight, dwBits );
		GlobalFree( hFile );
	}

	return( bResult );
}

PCHAR GetVal( PCHAR pText, PLONG plVal )
{
PCHAR p;
LONG lVal;

	p = pText;
	lVal = -1;
	for ( ; ; ) {
		if ( !*p )
			break;
		switch( *p ) {
		case '0':
			if ( ( *( p + 1 ) == 'x' ) || ( *( p + 1 ) == 'X' ) ) {
				p += 2;
				lVal = 0;
				for ( ; ; ) {
					lVal *= 16;
					if ( ( *p >= '0' ) && ( *p <= '9' ) ) {
						lVal += *p - '0';
					} else if ( ( *p >= 'A' ) && ( *p <= 'F' ) ) {
						lVal += ( *p - 'A' ) + 10;
					} else if ( ( *p >= 'a' ) && ( *p <= 'f' ) ) {
						lVal += ( *p - 'a' ) + 10;
					} else {
						lVal /= 16;
						break;
					}
					p++;
				}
				break;
			}
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			lVal = 0;
			for ( ; ; ) {
				lVal *= 10;
				if ( ( *p >= '0' ) && ( *p <= '9' ) ) {
					lVal += *p - '0';
				} else {
					lVal /= 10;
					break;
				}
				p++;
			}
			break;
		case ',':
			lVal = 0;
			p++;
			break;
		}
		if ( lVal >= 0 ) {
			p++;
			if ( plVal )
				*plVal = lVal;
			break;
		}
		if ( !*p )
			break;
		p++;
	}
	return( p );
}

BOOL DrawXBM( HWND hwnd, HGLOBAL hXBM, DWORD dwWidth, DWORD dwHeight, DWORD dwBits )
{
BOOL bResult = FALSE;
HDC hdc;
DWORD x, y, z;
LONG lVal;
HPEN hPen, hOldPen;
HBRUSH hbr;
PCHAR pXBM;
RECT rc;

	if ( !hXBM )
		return( bResult );
	if ( !hwnd )
		return( bResult );
	hbr = CreateSolidBrush( RGB( 255, 255, 255 ) );
	if ( !hbr )
		return( bResult );
	hPen = CreatePen( PS_SOLID, 1, RGB( 0, 0, 0 ) );
	if ( !hPen ) {
		DeleteObject( hbr );
		return( bResult );
	}

	hdc = GetDC( hwnd );
	if ( hdc ) {
		rc.left = 0;
		rc.top = 0;
		rc.right = dwWidth;
		rc.bottom = dwHeight;
		FillRect( hdc, &rc, hbr );
//
		hOldPen = (HPEN)SelectObject( hdc, hPen );
		pXBM = (PCHAR)GlobalLock( hXBM );
		if ( pXBM ) {
			pXBM += dwBits;
			for ( y = 0; y < dwHeight; y++ ) {
				for ( x = 0; x < dwWidth; x += 8 ) {
					pXBM = GetVal( pXBM, &lVal );
					if ( lVal >= 0 ) {
						for ( z = 0; z < 8; z++ ) {
							if ( lVal & g_tblBits[ z ] ) {
								MoveToEx( hdc, x + z, y, NULL );
								LineTo( hdc, x + z + 1, y );
							}
						}
					}
					if ( !*pXBM )
						break;
				}
				if ( !*pXBM )
					break;
			}
			GlobalUnlock( hXBM );
			bResult = TRUE;
		}
		SelectObject( hdc, hOldPen );
		ReleaseDC( hwnd, hdc );
	}
	if ( hbr ) {
		DeleteObject( hbr );
	}
	if ( hPen ) {
		DeleteObject( hPen );
	}

	return( bResult );
}

BOOL GetXBMParams( HGLOBAL hXBM, PDWORD pdwWidth, PDWORD pdwHeight, PDWORD pdwBits )
{ // sorry, nocare "_x_spot", "_y_spot"
BOOL bResult = FALSE;
PCHAR pXBM, p0, p;

	if ( !hXBM )
		return( bResult );

	pXBM = (PCHAR)GlobalLock( hXBM );
	if ( pXBM ) {
		bResult = TRUE;
// width
		p0 = strstr( pXBM, KEYWORD_WIDTH );
		if ( p0 ) {
			p = p0 + strlen( KEYWORD_WIDTH );
			if ( pdwWidth )
				*pdwWidth = atol( p );
		} else {
			bResult = FALSE; // noting Width
		}
// height
		p0 = strstr( pXBM, KEYWORD_HEIGHT );
		if ( p0 ) {
			p = p0 + strlen( KEYWORD_HEIGHT );
			if ( pdwHeight )
				*pdwHeight = atol( p );
		} else {
			bResult = FALSE; // noting Height
		}
// Location of Bits
		p0 = strstr( pXBM, KEYWORD_BITS );
		if ( p0 ) {
			p = p0 + strlen( KEYWORD_BITS );
			p0 = strstr( pXBM, KEYWORD_BITS_BEGIN );
			p = p0 + strlen( KEYWORD_BITS_BEGIN );
			if ( pdwBits )
				*pdwBits = p - pXBM;
		} else {
			bResult = FALSE; // noting Bits...
		}
		GlobalUnlock( hXBM );
	}
	return( bResult );
}


BOOL DeleteRemarks( HGLOBAL hXBM, DWORD dwSizeFile )
{
BOOL bResult = FALSE;
PCHAR pXBM = NULL, pXBM0 = NULL, p0, p1;
DWORD dwSize = dwSizeFile, dw;

	if ( !hXBM )
		return bResult;
	if ( !dwSizeFile )
		return bResult;

	pXBM0 = (PCHAR)GlobalLock( hXBM );
	if ( pXBM0 ) {
// Pass1 ... delete '//'
		pXBM = pXBM0;
		for ( ; ; ) {
			p0 = strstr( pXBM, "//" ) ;
			if ( p0 ) {
				p1 = strstr( pXBM, "\n" ) ;
				if ( p1 ) {
					p1++;
					dw = p1 - p0;
					memcpy( p1, p0, dwSize - dw + 1 );
					pXBM = p0;
					dwSize -= dw;
				} else {
					*p0 = '\0';
					break;
				}
			} else {
				break;
			}
		}
// Pass2 ... delete '/* ... */'
		pXBM = pXBM0;
		for ( ; ; ) {
			p0 = strstr( pXBM, "/*" ) ;
			if ( p0 ) {
				p1 = strstr( pXBM, "*/" ) ;
				if ( p1 ) {
					p1++;
					dw = p1 - p0;
					memcpy( p1, p0, dwSize - dw + 1 );
					pXBM = p0;
					dwSize -= dw;
				} else {
					*p0 = '\0';
					break;
				}
			} else {
				break;
			}
		}
//dwSizeFile = dwSize;
		bResult = TRUE;
		GlobalUnlock( hXBM );
	}
	return( bResult );
}

HGLOBAL ReadXBMFile( HWND hwnd, PTSTR ptszFile, PDWORD pdwSizeFile )
{
HGLOBAL hGlobal = NULL;
HANDLE hFile;
DWORD dwSizeLow, dwSizeHigh, dwRead;
PVOID pXBM = NULL;
BOOL bError = FALSE;

	hFile = CreateFile( ptszFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE ) {
		dwSizeLow = GetFileSize( hFile, &dwSizeHigh );
		// Sorry, only Low Size 
		hGlobal = GlobalAlloc( GMEM_MOVEABLE | GMEM_ZEROINIT, dwSizeLow + 16 );
		if ( hGlobal )
			pXBM = GlobalLock( hGlobal );
		if ( pXBM ) {
			ReadFile( hFile, pXBM, dwSizeLow, &dwRead, NULL );
			if ( dwSizeLow != dwRead ) {
				if ( pdwSizeFile )
					*pdwSizeFile = dwRead;
				// Error
				bError = TRUE;
			}
			GlobalUnlock( hGlobal );
		}
		CloseHandle( hFile );
	}
	if ( hGlobal ) {
		if ( bError ) {
		// error ... free memory
			GlobalFree( hGlobal );
			hGlobal = NULL;
		}
	}
	return( hGlobal );
}
