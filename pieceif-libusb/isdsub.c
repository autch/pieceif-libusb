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
//  v1.01 2001.11.10 MIO.H ismRTCSet 追加
//  v1.02 2001.11.16 MIO.H ismLCDCaptureScreen 追加
//  v1.06 2001.12.21 MIO.H ファイル名の不正チェック
//  ????? 2002.01.26 Missing cat(Modify) / Rokuhara(Test) [Privately]
//                   ---v1.06 based 2MB/512KB Flash(SST) support version---

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#pragma hdrstop
#define INDLL
#include "pieceif.h"
#define NOPCESPRINTF
#define __PCEKN__
#include "piece.h"

#define TBUFSIZ (32*1024)



extern unsigned char _piece_version_info[32];
extern int _piece_version;
int ismAppCtrl( long stat );
int ismAppPause( int pausef );
int ismLCDGetInfo( int *pstat, unsigned long *pbuff );


static pffsMASTERBLOCK msblk;
static unsigned long PFFSTOP;
static unsigned long PFFSEND;
static int MAXFAT2;

#define MSBLKADR PFFSTOP


static unsigned long read_mlong( unsigned char *mem )
{
	return (((((mem[0]<<8)+mem[1])<<8)+mem[2])<<8)+mem[3];
}

static unsigned short read_mshort( unsigned char *mem )
{
	return (mem[0]<<8)+mem[1];
}

static int check_fname( const char *p )
{
	int i;

	for ( i = 0; i < 2; i++ ) {
		int c = 0;
		char a;

		while ( 1 ) {
			a = *p++;
			if ( !(		// ↓ファイル名に許される文字セット [0-9a-z_]
				( a >= '0' && a <= '9' ) ||
				( a >= 'a' && a <= 'z' ) ||
				( a == '_' )
				) ) break;
			c++;
		}

//		printf( "[%d]", c );

		if ( i == 0 ) {
			if ( c == 0 || c > 8 ) return 1;
			if ( !a ) return 0;
			if ( a != '.' ) return 2;
		}
		else {
			if ( c == 0 || c > 3 ) return 3;
			if ( !a ) return 0;
		}
	}

	return 4;
}





static int readfile_srf( FILE *fp )
{
	unsigned long last = -1L, ent = -1L;
	unsigned char tmp[64];
	unsigned long next;
	int m = 100;
	unsigned char *tbuff;

	fseek(fp, 0, SEEK_SET);

	if ( fread((char *)tmp, 1, 16, fp) != 16 ) return 2;

	if ( (read_mshort(tmp) | 8)!=0x000e ) return 3;

	printf( "\n" );

	next = read_mlong( tmp+8 );

	tbuff = malloc( TBUFSIZ );

	while ( next ) {
		unsigned long len;

		//printf( "[%x]", next);
		fseek(fp, next, SEEK_SET);

		if ( fread(tmp, 1, 44, fp) != 44 ) break;

		next = read_mlong( tmp );

		len = read_mlong( tmp+38 );

		if ( len ) {
			unsigned long adr = read_mlong( tmp+10 );
			unsigned long pos = read_mlong( tmp+34 );
			if ( pos ) {
				//printf( "  %06x %06x %06x\n", adr, len, pos );
				printf( "  %06x-%06x\n", adr, adr+len-1 );
				fseek(fp, pos, SEEK_SET);
				while ( len ) {
					unsigned long len2 = len;
					if ( len2 > TBUFSIZ ) len2 = TBUFSIZ;
					fread(tbuff,1,len2,fp);
					ismWriteMem(tbuff, adr, len2);
					adr += len2;
					len -= len2;
				}
			}
		}
	}

	free( tbuff );

	//if ( ent >= 0 ) printf( "Entry address = %06lx\n", ent );
	return 0;
}

int DLLAPI ismWriteSrfFile( const char *infname, int run )
{
	FILE *fp = NULL;
	int err = 1;

	if ( fopen_s(&fp, infname, "rb") == 0 ) {
		if ( run ) ismAppStop();
		err = readfile_srf( fp );
		fclose( fp );
		if ( run ) ismAppStart();
	}

	return err;
}



int ismFlashErase( unsigned long adrs, int *ret )
{
	unsigned char tmp[5];

	if ( _piece_version < 31 ) return ERR_PIECEIF_ILL_VER; // BIOSのバージョンが違う

	tmp[0] = 8;
	*(unsigned long *)(tmp+1) = adrs;

	if ( ismCmdR(tmp,5,tmp,2) ) return 1;

	*ret = *(short *)tmp;

	printf( "FlashErase %x %x\n", adrs, *ret );

	return 0;
}


int ismFlashWrite( unsigned long dadrs, unsigned long sadrs, unsigned len, int *ret )
{
	unsigned char tmp[13];

	if ( _piece_version < 31 ) return ERR_PIECEIF_ILL_VER; // BIOSのバージョンが違う

	tmp[0] = 9;
	*(unsigned long *)(tmp+1) = dadrs;
	*(unsigned long *)(tmp+5) = sadrs;
	*(unsigned long *)(tmp+9) = len;

	if ( ismCmdR(tmp,13,tmp,2) ) return 1;

	*ret = *(short *)tmp;

	printf( "FlashWrite %x %x %x %x\n", dadrs, sadrs, len, *ret );

	return 0;
}






static int ffs_loaddir( void )
{
	{
		SYSTEMINFO *sip = (SYSTEMINFO *)_piece_version_info;
		PFFSTOP = (unsigned long)sip->pffs_top;
		PFFSEND = (unsigned long)sip->pffs_end;
		MAXFAT2 = ((PFFSEND - PFFSTOP) >> 12);
		if ( MAXFAT2 > MAXFAT ) MAXFAT2 = MAXFAT;
		//printf( "MAXFAT2=%d\n", MAXFAT2 );
	}
	return ismReadMem( (unsigned char *)&msblk, MSBLKADR, sizeof(msblk) ); 
}


static int flash_write( unsigned long adr, void *ptr, unsigned long len )
{
	int a;

	if ( ismWriteMem( (unsigned char *)ptr, 0x130000, len ) ) return 1;
	if ( ismFlashErase( adr, &a ) ) return 1;
	if ( a ) return a;
	if ( ismFlashWrite( adr, 0x130000, len, &a ) ) return 1;

	return a;
}


static int ffs_savedir( void )
{
	return flash_write( MSBLKADR, &msblk, sizeof(msblk) );
}

static DIRECTORY *ffs_srchdir( const char *name )
{
	DIRECTORY *pdir = msblk.dir;
	int i;

	for ( i = 0; i < MAXDIR; pdir++,i++ ) {
		unsigned char a = pdir->name[0];
		if ( a != 0 && a != 0xff ) {
			if ( !strcmp( name, pdir->name ) ) return pdir;
		}
	}

	return NULL;
}

static DIRECTORY *ffs_freedir( void )
{
	DIRECTORY *pdir = msblk.dir;
	int i;

	for ( i = 0; i < MAXDIR; pdir++,i++ ) {
		unsigned char a = pdir->name[0];
		if ( a == 0 || a == 0xff ) return pdir;
	}

	return NULL;
}


static void ffs_delete( DIRECTORY *pdir )
{
	int n = pdir->chain;
	FAT *pfat = msblk.fat;

	memset( pdir, 0xff, sizeof(*pdir) );

	while ( n < MAXFAT2 ) {
		int n1 = pfat[n].chain;
		pfat[n].chain = FAT_FREE;
		n = n1;
	}
}


static int ffs_read( DIRECTORY *pdir, FILE *fp )
{
	int n = pdir->chain;
	unsigned len = pdir->size;
	FAT *pfat = msblk.fat;
	char *tbuff = malloc( 4096 );

	while ( n < MAXFAT2 ) {
		int n1 = pfat[n].chain;
		unsigned len0 = len;
		if ( len0 > 4096 ) len0 = 4096;
		if ( ismReadMem( tbuff, PFFSTOP+n*0x1000, len0 ) ) break;
		if ( fwrite( tbuff, 1, len0, fp ) != len0 ) break;
		n = n1;
		len -= len0;
		if ( !len ) break;
	}

	free( tbuff );

	return len;
}


static int ffs_save( const char *name, unsigned char *pdata, int len )
{
		int i, n, len2, len3;
		unsigned short tbl[MAXFAT];
		DIRECTORY *pdir;
		FAT *pfat = msblk.fat;

		if ( !len ) return 1;

		len2 = len / 4096;
		len3 = len % 4096;
		if ( len3 ) len2++;

		pdir = ffs_srchdir( name );
		if ( pdir ) {
			ffs_delete( pdir );
		}

		pdir = ffs_freedir();

		if ( !pdir ) {
			printf( "ディレクトリが足りません。\n" );
			return ERR_PIECEIF_PFFS_DIR_EMPTY;
		}

		// 配置のストラテジー
		{
			i = 0;
			for ( n = 0; n < MAXFAT2; n++ ) {
				if ( pfat[n].chain == FAT_FREE ) {
					tbl[i++] = n;
					if ( i == len2 ) break;
				}
			}
			
			if ( i < len2 ) {
				printf( "空きが足りません。\n" );
				return ERR_PIECEIF_PFFS_EMPTY;
			}
		}

		memset( pdir, 0, sizeof(*pdir) );
		strcpy_s(pdir->name, sizeof pdir->name, name);
		pdir->size = len;
		pdir->chain = tbl[0];

		i = 0;
		while ( len ) {
			n = tbl[i];
			len3 = len;
			if ( len3 >= 4096 ) len3 = 4096;
			if ( flash_write( PFFSTOP+n*0x1000, pdata, len3 ) ) break;
			pdata += len3;
			len -= len3;
			pfat[n].chain = ( i < len2-1 ) ? tbl[i+1] : FAT_END;
			i++;
		}

	return len ? 4 : 0;
}


int DLLAPI ismPFFSWrite( const char *fname, const char *infname )
{
	unsigned char *xbuff;
	int len;
	int err = 1;
	FILE *fp;

	if (_piece_version < 44) return ERR_PIECEIF_ILL_VER; // BIOSのバージョンが違う

	if (check_fname(fname)) return ERR_PIECEIF_PFFS_BAD_FAILENAME; // PFFS のファイル名が不正

	fp = NULL;
	if ( fopen_s(&fp, infname, "rb") != 0 ) return err;		// ファイルオープンに失敗

	fseek( fp, 0, SEEK_END );
	len = ftell( fp );
	fseek( fp, 0, SEEK_SET );
	xbuff = malloc( len );
	fread( xbuff, 1, len, fp );
	fclose( fp );

	if ( !ismAppStop() ) {
		if ( !ffs_loaddir() ) {
			err = ffs_save( fname, xbuff, len );
			if ( !err ) {
				err = ffs_savedir();
			}
		}
		ismAppStart();
	}

	free( xbuff );

	return err;
}

int DLLAPI ismPFFSRead( const char *fname,  const char *outfname )
{
		DIRECTORY *pdir;
		int err = 1;

		if ( _piece_version < 44 ) return ERR_PIECEIF_ILL_VER; // BIOSのバージョンが違う

		if ( !ffs_loaddir() ) {
			pdir = ffs_srchdir( fname );
			if ( pdir ) {
				FILE *fp = NULL;
				if ( fopen_s(&fp, outfname, "wb") == 0 ) {
					printf( "reading '%s'", fname );
					err = ffs_read( pdir, fp );
					fclose( fp );
					printf( " done\n" );
				}
			}
		}

	return err;
}

int DLLAPI ismPFFSDelete( const char *fname )
{
	DIRECTORY *pdir;
	int err = 1;

	if ( _piece_version < 44 ) return ERR_PIECEIF_ILL_VER; // BIOSのバージョンが違う

	if ( !ismAppStop() ) {
		ffs_loaddir();
		pdir = ffs_srchdir( fname );
		if ( pdir ) {
			ffs_delete( pdir );
			err = ffs_savedir();
		}
		ismAppStart();
	}

	return err;
}


int DLLAPI ismPFFSInit( void )
{
	//                     012345678901234567890123
	static char sig[24] = "PFFS Master Block";
	int n = ( MSBLKADR - PFFSTOP ) / 4096;
	int err = 1;

	if ( _piece_version < 44 ) return ERR_PIECEIF_ILL_VER; // BIOSのバージョンが違う

	if ( !ffs_loaddir() ) {

		memset( &msblk, 0xff, sizeof(msblk) );
		memcpy( msblk.mark.signature, sig, sizeof(msblk.mark.signature) );
		msblk.mark.ptr = MSBLKADR+4;
		msblk.fat[n].chain = FAT_SYSTEM;
		for ( n = MAXFAT2; n < MAXFAT; n++ ) {
			msblk.fat[n].chain = FAT_INVALID;
		}
	
		if ( !ismAppStop() ) {
			err = ffs_savedir();
			ismAppStart();
		}
	}

	return err;
}


typedef struct tagCHARBUF {
	char *buf;
	char *end;
} CHARBUF;


static void putbuf( CHARBUF *pcb, char *in )
{
	int n = strlen( in );
	int len = pcb->end - pcb->buf;
	if ( n > len ) n = len;
	memcpy( pcb->buf, in, n );
	pcb->buf += n;
}

int DLLAPI ismPFFSDir( char *buff, unsigned len, int flag )
{
	CHARBUF cb = {buff, buff+len-1};

	if ( _piece_version < 44 ) return ERR_PIECEIF_ILL_VER; // BIOSのバージョンが違う

	if ( !ffs_loaddir() ) {
		DIRECTORY *pdir = msblk.dir;
		int i, c = 0;
		char tmp[256];

		for ( i = 0; i < MAXDIR; pdir++,i++ ) {
			unsigned char a = pdir->name[0];
			if ( a != 0 && a != 0xff ) {
				snprintf( tmp, sizeof tmp, "+%3d:%6d %s\n", i, pdir->size, pdir->name );
				putbuf( &cb, tmp );
			}
		}

		for ( i = 0; i < MAXFAT2; i++ ) {
			if ( msblk.fat[i].chain == FAT_FREE ) c++;
		}
		snprintf( tmp, sizeof tmp, "    %d sectors (%d bytes) free\n", c, c<<12 );
		putbuf( &cb, tmp );
	}

	*cb.buf = 0;

	return 0;
}



int DLLAPI ismAppStart( void )
{
	return ismAppCtrl( 0x01 );
}

int DLLAPI ismAppStop( void )
{
	return ismAppCtrl( 0x03 );
}


int DLLAPI ismRTCSet( int year, int mon, int day, int hour, int min, int sec )
{
	unsigned char tmp[10];
	PCETIME *ppt = (PCETIME *)(tmp+2);

	if ( _piece_version < 0x101 ) return ERR_PIECEIF_ILL_VER; // BIOSのバージョンが違う

	tmp[0] = 15;
	tmp[1] = 0;
	ppt->yy = year;
	ppt->mm = mon;
	ppt->dd = day;
	ppt->hh = hour;
	ppt->mi = min;
	ppt->ss = sec;
	ppt->s100 = 0;

	return ismCmdW(tmp,10,0,0);
}


#define ismWait(n) SleepEx(n,TRUE)

int DLLAPI ismLCDCaptureScreen( unsigned char *buff, int len )
{
	int err = 0;
	unsigned long vbuff;
	int i, f;

	if ( (err = ismAppPause( 1 )) != 0 ) return err;

	for ( i = 0; i < 100; i++ ) {
		if ( (err = ismLCDGetInfo( &f, &vbuff )) != 0 ) break;
		if ( f ) {
			err = ismReadMem( buff, vbuff, len );
			break;
		}
		ismWait( 20 );
		err = ERR_PIECEIF_TIMEOUT; // タイムアウト
	}

	ismAppPause( 0 );

	return err;
}



