// xbm2bmp.cpp: アプリケーションのエントリ ポイントを定義します。
//
#include "stdafx.h"
#include <tchar.h>
#include <stdio.h>
#include <math.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shlwapi.h>
#include "xbm2bmp.h"
#include "xbmStaff.h"
#include "bmpStaff.h"

#define MAX_LOADSTRING 100

// Globals...
HINSTANCE g_hInstance;
TCHAR g_tszTitle[ MAX_LOADSTRING ];
TCHAR g_tszWindowClass[ MAX_LOADSTRING ];

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM MyRegisterClass( HINSTANCE hInstance );
BOOL InitInstance( HINSTANCE hInstance, int nCmd );
LRESULT CALLBACK WndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK About( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

BOOL SelectXBMFile( HWND hwnd, PTSTR ptszFile );
BOOL ConvertSub( HWND hwnd, PTSTR ptsz );
BOOL ConvertXBMFile( HWND hwnd, PTSTR ptszFile );

INT APIENTRY wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR pCmdLine, _In_ INT nCmdShow )
{
UNREFERENCED_PARAMETER( hPrevInstance );
UNREFERENCED_PARAMETER( pCmdLine );
MSG msg;
HACCEL hAccelTable;

	LoadStringW( hInstance, IDS_APP_TITLE, g_tszTitle, _countof( g_tszTitle ) );
	LoadStringW( hInstance, IDC_XBM2BMP, g_tszWindowClass, _countof( g_tszWindowClass ) );
	MyRegisterClass( hInstance );

	if ( !InitInstance (hInstance, nCmdShow ) ) {
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_XBM2BMP));

	while( GetMessage( &msg, nullptr, 0, 0 ) ) {
	if ( !TranslateAccelerator( msg.hwnd, hAccelTable, &msg ) ) {
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}

	return (INT)msg.wParam;
}

//
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
ATOM MyRegisterClass( HINSTANCE hInstance )
{
WNDCLASSEXW wcex;

	wcex.cbSize = sizeof( WNDCLASSEX );

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon( hInstance, MAKEINTRESOURCE( IDI_XBM2BMP ) );
	wcex.hCursor = LoadCursor( nullptr, IDC_ARROW );
	wcex.hbrBackground = (HBRUSH)( COLOR_WINDOW + 1 );
	wcex.lpszMenuName = MAKEINTRESOURCEW( IDC_XBM2BMP );
	wcex.lpszClassName = g_tszWindowClass;
	wcex.hIconSm = LoadIcon( wcex.hInstance, MAKEINTRESOURCE( IDI_SMALL ) );

	return RegisterClassExW( &wcex );
}

//
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します。
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance( HINSTANCE hInstance, INT nCmdShow )
{
HWND hWnd;

	g_hInstance = hInstance;

	hWnd = CreateWindowW( g_tszWindowClass, g_tszTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr );

	if ( !hWnd ) {
		return FALSE;
	}

	ShowWindow( hWnd, nCmdShow );
	UpdateWindow( hWnd );

	return TRUE;
}

//
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND  - アプリケーション メニューの処理
//  WM_PAINT    - メイン ウィンドウの描画
//  WM_DESTROY  - 中止メッセージを表示して戻る
//
//
LRESULT CALLBACK WndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg ) {
    case WM_CREATE:
        DragAcceptFiles( hwnd, TRUE );
        break;
    case WM_DROPFILES: {
	UINT i, uiMax;
	TCHAR tsz[ MAX_PATH ];
	HDROP hDrop;

		hDrop = (HDROP)wParam;
		uiMax = DragQueryFile( hDrop, 0xFFFFFFFF, NULL, 0 );
		for( i = 0; i < uiMax; i++ ) {
			DragQueryFile( hDrop, i, tsz, _countof( tsz ) );
			ConvertSub( hwnd, tsz );
		}
		DragFinish( hDrop );
	}
        break;
	case WM_COMMAND: {
	int wmId = LOWORD( wParam );
		switch( wmId ) {
		case IDM_ABOUT:
			DialogBox( g_hInstance, MAKEINTRESOURCE( IDD_ABOUTBOX ), hwnd, About );
			break;
		case IDM_EXIT:
			DestroyWindow( hwnd );
			break;
		case IDM_FILE_CONVERFILE: {
		RECT rc;
		TCHAR tsz[ MAX_PATH ];
				if ( SelectXBMFile( hwnd, tsz ) ) {
					if ( ImportXBMFile( hwnd, tsz, &rc ) )
						ExportBmpFile( hwnd, tsz, &rc );
				}
		}
			break;
		//case IDM_FILE_CONVERTFOLDER:
		//	break;
		default:
			return DefWindowProc( hwnd, uMsg, wParam, lParam );
		}
					}
		break;
	case WM_PAINT: {
	PAINTSTRUCT ps;
	HDC hdc;
		hdc = BeginPaint( hwnd, &ps );
		EndPaint( hwnd, &ps );
					}
		break;
	case WM_DESTROY:
		PostQuitMessage( 0 );
		break;
	default:
		return DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
	return 0;
}

INT_PTR CALLBACK About( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
UNREFERENCED_PARAMETER(lParam);

	switch( uMsg ) {
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if ( ( LOWORD( wParam ) == IDOK ) || ( LOWORD( wParam ) == IDCANCEL ) ) {
			EndDialog( hDlg, LOWORD( wParam ) );
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


BOOL SelectXBMFile( HWND hwnd, PTSTR ptszFile )
{
BOOL bResult = FALSE;
TCHAR tsz[ MAX_PATH ];
OPENFILENAME ofn;

	if ( !ptszFile )
		return bResult;
	if ( !hwnd )
		return bResult;

	ZeroMemory( &tsz, sizeof( tsz ) );
	ZeroMemory( &ofn, sizeof( ofn ) );
	ofn.lStructSize = sizeof( ofn );
	ofn.hwndOwner = hwnd;
	ofn.hInstance = g_hInstance;
	ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;
	ofn.lpstrTitle = _T( "Select XBM File" );
	ofn.lpstrFile = tsz;
	ofn.nMaxFile = _countof( tsz );
	ofn.lpstrFilter = _T( "XBM Files(*.xbm)\0*.xbm\0All Files(*.*)\0*.*\0\0" );
	ofn.lpstrDefExt = _T( "*.xbm" );
	if ( GetOpenFileName( &ofn ) ) {
		_tcscpy( ptszFile, tsz );
		bResult = TRUE;
	}

	return( bResult );
}

BOOL ConvertDirectory( HWND hwnd, PTSTR ptszFolder )
{
BOOL bResult = FALSE;
TCHAR tsz[ _MAX_PATH ];
HANDLE hHandle;
WIN32_FIND_DATA fd;

	PathCombine( tsz, ptszFolder, _T( "*.*" ) );
	hHandle = FindFirstFile( tsz, &fd );

	do {
		if ( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
			if ( !_tcscmp( fd.cFileName, _T( ".") ) )
				continue;
			if ( !_tcscmp( fd.cFileName, _T( "..") ) )
				continue;
			PathCombine( tsz, ptszFolder,fd.cFileName );
			ConvertDirectory( hwnd, tsz );
		}
		if ( !( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) {
			PathCombine( tsz, ptszFolder,fd.cFileName );
			ConvertXBMFile( hwnd, tsz );
		}
	} while( FindNextFile( hHandle,&fd ) );
 
	FindClose( hHandle );
	return( bResult );
}

BOOL ConvertXBMFile( HWND hwnd, PTSTR ptszFile )
{
BOOL bResult = FALSE;
RECT rc;

	if ( !hwnd )
		return bResult;
	if ( !ptszFile )
		return bResult;

	if ( ImportXBMFile( hwnd, ptszFile, &rc ) )
		bResult = ExportBmpFile( hwnd, ptszFile, &rc );

	return( bResult );
}

BOOL ConvertSub( HWND hwnd, PTSTR ptsz )
{
BOOL bResult = FALSE;

	if ( !ptsz )
		return( bResult );
	if ( !hwnd )
		return( bResult );
	if ( PathIsDirectory( ptsz ) ) {
		bResult = ConvertDirectory( hwnd, ptsz );
	} else {
		bResult = ConvertXBMFile( hwnd, ptsz );
	}
	return( bResult );
}

