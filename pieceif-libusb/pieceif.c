#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////
//
//             /
//      -  P  /  E  C  E  -
//           /                 mobile equipment
//
//              System Programs
//
//
// PIECE TOOLS : pieceif.dll : Ver 1.00
//
// Copyright (C)2001 AUQAPLUS Co., Ltd. / OeRSTED, Inc. all rights reserved.
//
// Coded by MIO.H (OeRSTED)
//
// Comments:
//
// USB 転送コアの DLL
//
//  v1.00 2001.11.09 MIO.H
//  v1.04 2001.11.28 MIO.H	ver1.07以降はウエイトをなくす
//  v1.05 2001.11.29 MIO.H	排他制御で Mutex を使う
//  v1.06 2001.12.21 MIO.H	初期化時のエラーコード(二重起動)
//  v1.07 2002.01.06 MIO.H	複数台接続
//  ????? 2002.01.26 Missing cat(Modify) / Rokuhara(Test) [Privately]
//                   ---v1.07 based 2MB/512KB Flash(SST) support version---


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#pragma hdrstop
#define INDLL
#include "pieceif.h"
#include "libusb-1.0/libusb.h"

//                 01234567890123456789
#define MUTEXNAME "_pieceif_dll_mutex_"

#define ismWait(n) SleepEx(n,TRUE)

static volatile DWORD donef;
static volatile DWORD dlen;
static volatile DWORD derr;

#if 0
VOID CALLBACK acc_done(
  DWORD dwErrorCode,                // 完了コード
  DWORD dwNumberOfBytesTransfered,  // 転送バイト数
  LPOVERLAPPED lpOverlapped         // I/O 情報がある
                                    // 構造体へのポインタ
)
{
	donef = 1;
	derr = dwErrorCode;
	dlen = dwNumberOfBytesTransfered;
}
#endif

CRITICAL_SECTION csLibUSBInit;
LONG lLibUSBInitCount = 0;
libusb_context* ctxLibUSB = NULL;

unsigned char _piece_version_info[32];
int _piece_version;

typedef struct tagUSBHANDLES {
	HANDLE hMutex;
	libusb_device_handle* udev;
} USBHANDLES;

static USBHANDLES usbhandles[PIECE_MAX_DEVICES];
static USBHANDLES *curusb = usbhandles;

#define ismCurrentHandle()	(curusb->udev)

#define USB_ID_PRODUCT 0x1000
#define USB_ID_VENDOR 0x0e19

#define PIECE_END_POINT_IN 0x82
#define PIECE_END_POINT_OUT 0x02
#define BULK_WAIT 500
#define BULK_TIMEOUT	500

static int write(const char *buf, size_t len, size_t* written_p)
{
	unsigned char *data = (unsigned char*)buf;
	int actual_length = 0;
	int r = libusb_bulk_transfer(ismCurrentHandle(), PIECE_END_POINT_OUT, data, len, &actual_length, BULK_TIMEOUT);

	if (written_p != NULL)
		*written_p = actual_length;

	return r;
}

static int read(char *buf, size_t len, size_t* bytes_read_p)
{
	unsigned char *data = buf;
	int actual_length = 0;
	int r = libusb_bulk_transfer(ismCurrentHandle(), PIECE_END_POINT_IN, data, len, &actual_length, BULK_TIMEOUT);
	
	if (bytes_read_p != NULL)
		*bytes_read_p = actual_length;

	return r;
}


int DLLAPI ismCmdW( const void *ptr1, unsigned len1, const void *ptr2, unsigned len2 )
{
	int len3;
	write(ptr1, len1, &len3);

	if ( len2 ) {
		write(ptr2, len2, &len3);
 	}
 	else if ( _piece_version < 0x107 ) {
 		ismWait( 15 );
 	}

	return 0;
}

int DLLAPI ismCmdR( const void *ptr1, unsigned len1, void *ptr2, unsigned len2 )
{
	int len3;

	//printf( "*Write %d(%d)\n", ((unsigned char *)ptr1)[0], len1 );
	{
		write(ptr1, len1, &len3);
	}

	//printf( "*Read\n" );
	{
		read(ptr2, len2, &len3);
		dlen = len3;
	}

	return 0;
}



int DLLAPI ismReadMem( unsigned char *pBuff, unsigned long adrs, unsigned len )
{
	unsigned char tmp[10];

	tmp[0] = 2;
	*(long *)(tmp+1) = adrs;
	*(long *)(tmp+5) = len;

	return ismCmdR(tmp,9,pBuff,len);
}


int DLLAPI ismWriteMem( const unsigned char *pBuff, unsigned long adrs, unsigned len )
{
	unsigned char tmp[10];

	tmp[0] = 3;
	*(long *)(tmp+1) = adrs;
	*(long *)(tmp+5) = len;

	return ismCmdW(tmp,9,pBuff,len);
}


int DLLAPI ismExec( unsigned long adrs )
{
	unsigned char tmp[8];

	tmp[0] = 1;
	if ( adrs & 1 ) tmp[0] = 8;
	*(long *)(tmp+1) = adrs;

	return ismCmdW(tmp,5,0,0);
}



int DLLAPI ismGetVersion( void *ptr, int renew )
{
	unsigned char tmp[4];
	int ver = PIECE_INVALID_VERSION;

	if ( renew ) {
		int retryc = 0;

		_piece_version = ver;

		while ( 1 ) {
			tmp[0] = 0;
			if ( ismCmdR(tmp,1,_piece_version_info,8) ) return _piece_version;

			if ( dlen == 8 ) break;

			printf( "donef=%d dlen=%d derr=%d ResetDevice!!\n", donef, dlen, derr );
			libusb_reset_device(ismCurrentHandle());

			if ( ++retryc >= 3 ) return _piece_version;
		} 

		ver = *(unsigned short *)(_piece_version_info+4);

		if ( ver >= 21 ) {
			tmp[0] = 0;
			tmp[1] = ( *(short *)(_piece_version_info+4) >= 25 ? 32 : 24 );
			if ( ismCmdR(tmp,2,_piece_version_info,tmp[1]) ) return _piece_version;
		}

		_piece_version = ver;
	}

	if ( ptr ) {
		memcpy( ptr, _piece_version_info, 32 );
	}

	return _piece_version;
}


static void ismCloseHandles( USBHANDLES *pu )
{
	if (pu->udev != NULL) {
		libusb_release_interface(pu->udev, 0);
		libusb_reset_device(pu->udev);
		libusb_close(pu->udev);
		pu->udev = NULL;
	}
}



//BOOL WINAPI HandlerRoutine( DWORD dwCtrlType )
//{
//	if ( dwCtrlType == CTRL_C_EVENT ) {
//		fprintf( stderr, "HandlerRoutine(%d)\n", dwCtrlType );
//		ismExit();
//	}
//	return FALSE;
//}

int DLLAPI ismInitEx( int devno, int waitn )
{
	USBHANDLES *pu;
	DWORD time0 = GetTickCount();

	if ( devno >= PIECE_MAX_DEVICES ) return ERR_PIECEIF_OVER_DEVICES;

	if ( waitn == PIECE_DEF_WAITN ) waitn = 500;

	EnterCriticalSection(&csLibUSBInit);
	{
		if (lLibUSBInitCount == 0 && ctxLibUSB == NULL)
			libusb_init(&ctxLibUSB);
		InterlockedIncrement(&lLibUSBInitCount);
	}
	LeaveCriticalSection(&csLibUSBInit);

	pu = usbhandles + devno;

	curusb = pu;

	if ( pu->hMutex ) return 0;

	// ----------------------------------------------

	{
		char name[32];

		if(devno)
			snprintf(name, sizeof name, "%s%d", MUTEXNAME, devno);
		else
			snprintf(name, sizeof name, "%s", MUTEXNAME);
		pu->hMutex = CreateMutexA( NULL, FALSE, name );
	}

	if ( !pu->hMutex ) {
		printf( "Mutex作成失敗\n" );
		return 2;
	}

	if ( WaitForSingleObjectEx( pu->hMutex, waitn, FALSE ) != WAIT_OBJECT_0 ) {
		CloseHandle( pu->hMutex );
		printf( "二重起動?\n" );
		pu->hMutex = NULL;
		return ERR_PIECEIF_ALREADY_RUNNING;
	}

	// ----------------------------------------------

	libusb_device** devices;
	while ( 1 ) {
#if 1
		int found_count = -1;
		pu->udev = NULL;
		libusb_get_device_list(ctxLibUSB, &devices);
		for(libusb_device** p = devices; *p; p++)
		{
			struct libusb_device_descriptor dev_desc;
			int r = libusb_get_device_descriptor(*p, &dev_desc);
			if (r < 0)
				continue;

			if(dev_desc.idVendor == USB_ID_VENDOR && dev_desc.idProduct == USB_ID_PRODUCT)
			{
				if(++found_count == devno)
				{
					libusb_open(*p, &pu->udev);
					break;
				}
			}

		}
		libusb_free_device_list(devices, 1);
		if (found_count == -1)
			return 1;
#else
		pu->udev = libusb_open_device_with_vid_pid(ctxLibUSB, USB_ID_VENDOR, USB_ID_PRODUCT);
#endif
		if ( pu->udev != NULL) {
			int ver;

			libusb_set_configuration(pu->udev, 1);
			libusb_claim_interface(pu->udev, 0);

			ver = ismGetVersion( NULL, 1 );
			if ( ver != PIECE_INVALID_VERSION ) break;

			ismCloseHandles( pu );
		}
		//printf( "(%d,%d)", husb, GetLastError() );

		{
			DWORD time = GetTickCount() - time0;
			if ( time > (DWORD)waitn ) {
				return 1;
			}
		}

		Sleep(10);
	}

	return 0;
}


int DLLAPI ismInit( void )
{
	return ismInitEx( 0, PIECE_DEF_WAITN );
}

int DLLAPI ismExitEx( int devno )
{
	USBHANDLES *pu;

	if ( devno >= PIECE_MAX_DEVICES ) return ERR_PIECEIF_OVER_DEVICES;

	pu = usbhandles + devno;

	fprintf( stderr, "Exit USB#%d\n", devno );

	ismCloseHandles( pu );

	if ( pu->hMutex ) {
		ReleaseMutex( pu->hMutex );
		CloseHandle( pu->hMutex );
		pu->hMutex = NULL;
	}

	EnterCriticalSection(&csLibUSBInit);
	{
		if (InterlockedDecrement(&lLibUSBInitCount) == 0 && ctxLibUSB != NULL) {
			libusb_exit(ctxLibUSB);
			ctxLibUSB = NULL;
		}
	}
	LeaveCriticalSection(&csLibUSBInit);

	return 0;
}

int DLLAPI ismExit( void )
{
	int i, err = 0;

	for ( i = 0; i < PIECE_MAX_DEVICES; i++ ) {
		if ( usbhandles[i].hMutex ) err = ismExitEx( i );
	}

	return err;
}

int DLLAPI ismSelect( int devno )
{
	USBHANDLES *pu;

	if ( devno >= PIECE_MAX_DEVICES ) return ERR_PIECEIF_OVER_DEVICES;

	pu = usbhandles + devno;

	if ( !pu->hMutex ) return 1;

	curusb = pu;

	return 0;
}



int DLLAPI ismUCOpen( USBCOMS *pucs )
{
	unsigned char tmp[1];

	if ( _piece_version < 49 ) return ERR_PIECEIF_ILL_VER; // BIOSのバージョンが違う

	tmp[0] = 13;

	return ismCmdR(tmp,1,pucs,12+16);
}

int DLLAPI ismUCClose( void )
{
	unsigned char tmp[1];

	if ( _piece_version < 49 ) return ERR_PIECEIF_ILL_VER; // BIOSのバージョンが違う

	tmp[0] = 14;

	return ismCmdR(tmp,1,tmp,1);
}

int DLLAPI ismUCWrite( void *ptr, int len )
{
	unsigned char tmp[1];

	if ( _piece_version < 49 ) return ERR_PIECEIF_ILL_VER; // BIOSのバージョンが違う

	tmp[0] = 11;

	return ismCmdW(tmp,1,ptr,len);
}

int DLLAPI ismUCRead( void *ptr, int len )
{
	unsigned char tmp[1];

	if ( _piece_version < 49 ) return ERR_PIECEIF_ILL_VER; // BIOSのバージョンが違う

	tmp[0] = 10;

	return ismCmdR(tmp,1,ptr,len);
}

int DLLAPI ismUCGetStat( USBCOMS *pucs )
{
	unsigned char tmp[1];

	if ( _piece_version < 49 ) return ERR_PIECEIF_ILL_VER; // BIOSのバージョンが違う

	tmp[0] = 12;

	return ismCmdR(tmp,1,pucs,12);
}



int ismSetAppStat( int stat )
{
	unsigned char tmp[4];

	tmp[0] = 4;
	tmp[1] = stat;

	return ismCmdW(tmp,2,0,0);
}


int ismGetAppStat( int *ret )
{
	unsigned char tmp[4];

	tmp[0] = 5;

	if ( ismCmdR(tmp,1,tmp,2) ) return -1;

	*ret = *(short *)tmp;

	return 0;
}


int ismAppCtrl( long stat )
{
	if ( _piece_version >= 22 ) {

		if ( stat == 3 ) {
			if ( ismSetAppStat( stat ) ) return 1;
			stat = 0;
		}
		else if ( stat == 1 ) {
			if ( ismSetAppStat( stat ) ) return 1;
			stat = 2;
		}
		else {
			return 1;
		}
		{
			int c = 100;
			while ( c-- ) {
				int a;
				if ( ismGetAppStat( &a ) ) return 1;
				if ( a == stat ) return 0;
				ismWait( 20 );
			}
			printf( "Time Out\n" );
		}
	}
	else {
		if ( ismSetAppStat( stat ) ) return 1;
		ismWait( 20 );
		return 0;
	}

	return 1;
}


int ismAppPause( int pausef )
{
	unsigned char tmp[2];

	if ( _piece_version < 0x102 ) return ERR_PIECEIF_ILL_VER; // BIOSのバージョンが違う

	tmp[0] = 16;
	tmp[1] = pausef;

	//printf( "Pause %d\n", tmp[1] );

	return ismCmdW(tmp,2,0,0);
}


int ismLCDGetInfo( int *pstat, unsigned long *pbuff )
{
	unsigned char tmp[12];
	int err;

	if ( _piece_version < 0x102 ) return ERR_PIECEIF_ILL_VER; // BIOSのバージョンが違う

	tmp[0] = 17;

	//printf( "LCD Info " );

	err = ismCmdR(tmp,1,tmp,12);

	if ( !err ) {
		*pstat = tmp[0];
		*pbuff = *(unsigned long *)(tmp+8);
		//printf( "%d ", *pstat );
		//printf( "0x%x\n", *pbuff );
	}

	return err;
}


int DLLAPI ismHeapGetAdrs( unsigned long *padr )
{
	unsigned char tmp[1];

	if ( _piece_version < 0x103 ) return ERR_PIECEIF_ILL_VER; // BIOSのバージョンが違う

	tmp[0] = 18;

	return ismCmdR(tmp,1,padr,4);
}



