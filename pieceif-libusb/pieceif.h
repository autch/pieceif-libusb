
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
// USB �]���R�A�� DLL
//
//  v1.00 2001.11.09 MIO.H
//  v1.01 2001.11.10 MIO.H ismRTCSet �ǉ�
//  v1.06 2001.12.21 MIO.H �G���[�R�[�h�ǉ�
//  v1.07 2002.01.06 MIO.H ������ڑ�
//



#ifndef _PIECEIF_H
#define _PIECEIF_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef INDLL
#define DLLAPI __declspec( dllexport ) WINAPI
#define DLLVAR __declspec( dllexport )
#else
#define DLLAPI __declspec( dllimport ) WINAPI
#define DLLVAR extern __declspec( dllimport )
#endif //#ifdef INDLL

typedef struct tagUSBCOMS {
	unsigned char rxstat; // ��M��
	unsigned char txstat; // ���M��
	unsigned char pistat; // PIECE �̃I�[�v���t���O
	unsigned char mystat; // PC �̃I�[�v���t���O
	unsigned long rxlen;  // ��M�v���T�C�Y
	unsigned long txlen;  // ���M�v���T�C�Y
	char signature[16];   // �ƍ�������
} USBCOMS;

#define USBCOM_STAT_RXWAIT 1
#define USBCOM_STAT_TXWAIT 1

#define PIECE_INVALID_VERSION (-1)

#define PIECE_DEF_WAITN (-1)

#define PIECE_MAX_DEVICES 22



// low-level interface functions
//
int DLLAPI ismInit( void );
int DLLAPI ismExit( void );
int DLLAPI ismInitEx( int devno, int waitn );
int DLLAPI ismExitEx( int devno );
int DLLAPI ismSelect( int devno );
int DLLAPI ismCmdW( const void *ptr1, unsigned len1, const void *ptr2, unsigned len2 );
int DLLAPI ismCmdR( const void *ptr1, unsigned len1, void *ptr2, unsigned len2 );

// basic-level interface functions
//
int DLLAPI ismGetVersion( void *ptr, int renew );
int DLLAPI ismWriteMem( const unsigned char *pBuff, unsigned long adrs, unsigned len );
int DLLAPI ismReadMem( unsigned char *pBuff, unsigned long adrs, unsigned len );
int DLLAPI ismExec( unsigned long adrs );

// USBCOM-level interface functions
//
int DLLAPI ismUCOpen( USBCOMS *pucs );
int DLLAPI ismUCClose( void );
int DLLAPI ismUCGetStat( USBCOMS *pucs );
int DLLAPI ismUCWrite( const void *ptr, int len );
int DLLAPI ismUCRead( void *ptr, int len );

// isd-level interface functions -> isdsub.c

int DLLAPI ismAppStop( void );
int DLLAPI ismAppStart( void );
int DLLAPI ismWriteSrfFile( const char *infname, int run );
int DLLAPI ismPFFSDir( char *buff, unsigned len, int flag );
int DLLAPI ismPFFSWrite( const char *fname, const char *infname );
int DLLAPI ismPFFSRead( const char *fname,  const char *outfname );
int DLLAPI ismPFFSDelete( const char *fname );
int DLLAPI ismPFFSInit( void );
int DLLAPI ismRTCSet( int year, int mon, int day, int hour, int min, int sec );
int DLLAPI ismLCDCaptureScreen( unsigned char *buff, int len );
int DLLAPI ismHeapGetAdrs( unsigned long *padr );

#ifdef __cplusplus
};
#endif

// �G���[�ԍ��̒�`

#define ERR_PIECEIF_TIMEOUT            1001  // �^�C���A�E�g
#define ERR_PIECEIF_ILL_VER            1002  // BIOS�̃o�[�W�������Ⴄ
#define ERR_PIECEIF_PFFS_EMPTY         1005  // PFFS �̗e�ʂ���t�Ńt�@�C�����������߂Ȃ�
#define ERR_PIECEIF_PFFS_DIR_EMPTY     1006  // PFFS �̃f�B���N�g������t�Ńt�@�C�����������߂Ȃ�
#define ERR_PIECEIF_PFFS_BAD_FAILENAME 1007  // PFFS �̃t�@�C�������s��

#define ERR_PIECEIF_ALREADY_RUNNING 1010  // pieceif.dll ��d�N��
//#define ERR_PIECEIF_NO_DRIVERS      1011  // PIECE�̃f�o�C�X�h���C�o������
#define ERR_PIECEIF_OVER_DEVICES      1012  // �f�o�C�X������������

#endif //_PIECEIF_H
