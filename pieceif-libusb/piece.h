
/////////////////////////////////////////////////////////////////////////////
//
//             /
//      -  P  /  E  C  E  -
//           /                 mobile equipment
//
//              System Programs
//
//
// PIECE KERNEL : Ver 1.00
//
// Copyright (C)2001 AUQAPLUS Co., Ltd. / OeRSTED, Inc. all rights reserved.
//
// Coded by MIO.H (OeRSTED)
//
// Comments:
//
//  PIECE ��{�w�b�_�[�t�@�C��
//
//  v1.00 2001.11.09 MIO.H
//  v1.03 2001.11.19 MIO.H
//  v1.06 2001.11.24 MIO.H
//  v1.09 2001.11.30 MIO.H
//  v1.10 2001.11.30 MIO.H
//  v1.14 2001.12.21 MIO.H
//  v1.16 2002.01.07 MIO.H
//  v1.17 2002.01.13 MIO.H
//  v1.18 2002.01.16 MIO.H
//  ????? 2002.01.26 Missing cat(Modify) / Rokuhara(Test) [Privately]
//                   -----v1.18 based 2MB Flash(SST) support version----
//  v1.19 2003.02.14 N.SAWA ALM_EVERYHOUR��ALM_EVERYDAY���t�������̂��C��
//



#ifndef _PIECE_H
#define _PIECE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL 0
#endif //ifndef NULL

#define INVALIDVAL (-1)
#define INVALIDPTR ((void *)-1)

#define VERSION(a,b) ((a<<8)+b)

typedef unsigned char BYTE;

// �x�N�^�[����֌W�̃T�[�r�X

typedef void (*PCETPENT)( void );

PCETPENT pceVectorSetTrap(int no, PCETPENT adrs);

typedef void *PCEKSENT;

PCEKSENT pceVectorSetKs(int no, PCEKSENT adrs);

// �{�^�����͊֌W�̃T�[�r�X

#define PP_MODE_SINGLE 0
#define PP_MODE_REPEAT 1

unsigned long pcePadGetDirect( void );
void pcePadGetProc( void );
unsigned long pcePadGet( void );
void pcePadSetTrigMode( int mode );

// USB �֌W�̃T�[�r�X

void pceUSBDisconnect( void );
void pceUSBReconnect( void );
int pceUSBSetupMode( int mode, void *param2, void *param3 );

#define PUM_BASIC   (0) // �ʏ��PC�ڑ����[�h
#define PUM_GAMEPAD (1) // Game Pad ���[�h

// LCD �֌W�̃T�[�r�X

void pceLCDDispStart( void );
void pceLCDDispStop( void );
void pceLCDTrans( void );
void pceLCDTransDirect( const unsigned char *lcd_direct );
void pceLCDTransRange( int xs, int ys, int xe, int ye );
unsigned char *pceLCDSetBuffer( unsigned char *pbuff );
int pceLCDSetOrientation( int dir );
int pceLCDSetBright( int bright );

// �t���b�V���������݌n�̃T�[�r�X

int pceFlashErase( void *romp );
int pceFlashWrite( void *romp, const void *memp, int len );

// �����x�^�C�}�[�֌W�̃T�[�r�X

void pceTimerSetCallback( int ch, int type, int time, void (*callback)( void ) );
unsigned long pceTimerGetPrecisionCount( void );
unsigned long pceTimerAdjustPrecisionCount( unsigned long st, unsigned long ed );


#define PCE_TT_NONE 0
#define PCE_TT_ONESHOT 1
#define PCE_TT_PERIODIC 2

unsigned long pceTimerGetCount( void );

#define PCE_TSCSF_INCRITICAL 1

void pceTimerSetContextSwitcher( unsigned long (*pContextSwitcher)( unsigned long nowsp, int flag ) );

// CPU ����֌W�̃T�[�r�X

int pceCPUSetSpeed( int no );

#define CPU_SPEED_NORMAL 0
#define CPU_SPEED_HALF   1

// �T�E���h�֌W�̃T�[�r�X

#define PW_TYPE_8BITPCM    0
#define PW_TYPE_16BITPCM   1
#define PW_TYPE_4BITADPCM  2
#define PW_TYPE_CONT       0x10    // �A���o��
#define PW_TYPE_ADP_INI    0x20    // ADPCM�������v��
#define PW_TYPE_VR         0x40    // �σ��[�g

#define PW_TYPE_ADPCM      (PW_TYPE_4BITADPCM|PW_TYPE_ADP_INI)
#define PW_TYPE_ADPCM_V    (PW_TYPE_4BITADPCM|PW_TYPE_VR|PW_TYPE_ADP_INI)
#define PW_TYPE_ADPCM_NI   (PW_TYPE_4BITADPCM)
#define PW_TYPE_ADPCM_V_NI (PW_TYPE_4BITADPCM|PW_TYPE_VR)

#define PW_STAT_START      1       // �Đ��J�n
#define PW_STAT_END        2       // �Đ��I��

typedef struct tagPCEWAVEINFO {
	volatile unsigned char stat;					// 0  �X�e�[�^�X
	unsigned char type;								// 1  �f�[�^�`��
	unsigned short resv;							// 2  �\��
	const void *pData;								// 4  �f�[�^�ւ̃|�C���^
	unsigned long len;								// 8  �f�[�^�̒���(�T���v����)
	struct tagPCEWAVEINFO *next;					// 12 ���ւ̃|�C���^
	void (*pfEndProc)( struct tagPCEWAVEINFO *);	// 16 �I�����R�[���o�b�N
} PCEWAVEINFO;


int pceWaveCheckBuffs( int ch );
int pceWaveDataOut( int ch, PCEWAVEINFO *pwave );
int pceWaveAbort( int ch );
int pceWaveSetChAtt( int ch, int att );
int pceWaveSetMasterAtt( int att );
void pceWaveStop( int hard );

// �t�H���g�֌W�̃T�[�r�X

const unsigned char *pceFontGetAdrs( unsigned short code );
unsigned short pceFontPut( int x, int y, unsigned short code );
void pceFontSetType( int type );
void pceFontSetTxColor( int color );
void pceFontSetBkColor( int color );
void pceFontSetPos( int x, int y );
int pceFontPutStr( const char *pstr );
int pceFontPrintf( const char *fotmat, ... );

#define FC_SPRITE (-1)

// �A�v���P�[�V�����֌W�̃T�[�r�X

int pceAppSetProcPeriod( int period );
void pceAppReqExit( int exitcode );
int pceAppExecFile( const char *fname, int resv );

typedef struct tagMEMBLK {
	unsigned char *top;
	unsigned long len;
} MEMBLK;

int pceAppGetHeap( MEMBLK *pmb );

void pceAppActiveResponse( int flag );

#define AAR_NOACTIVE 0
#define AAR_ACTIVE 1

// �t�@�C���V�X�e���֌W�̃T�[�r�X

#define MAXFILENAME 26

typedef struct tagFILEINFO {
	char filename[MAXFILENAME+1];	// �t�@�C����
	unsigned char attr;				// ����
	unsigned long length;			// ����
	unsigned long adrs;				// �J�n�A�h���X
	unsigned char works[16];		// ������Ɨ̈�
} FILEINFO;

int pceFileFindOpen( FILEINFO *pfi );
int pceFileFindNext( FILEINFO *pfi );
int pceFileFindClose( FILEINFO *pfi );

int pceFileLoad( const char *fname, void *ptr );


typedef struct FILEACC {
	unsigned short valid;		// 0
	unsigned char resv2;
	unsigned char resv3;
	const unsigned char *aptr;	// 4
	unsigned long fsize;		// 8
	unsigned short chain;		// 12
	unsigned short bpos;		// 14
} FILEACC;

#define FOMD_RD  1
#define FOMD_WR  2

int pceFileOpen( FILEACC *pfa, const char *fname, int mode );
int pceFileReadSct( FILEACC *pfa, void *ptr, int sct, int len );
int pceFileWriteSct( FILEACC *pfa, const void *ptr, int sct, int len );
int pceFileClose( FILEACC *pfa );
int pceFileCreate( const char *fname, unsigned long size );
int pceFileDelete( const char *fname );


int pceFileApfSave( int key, const void *ptr, int len );
int pceFileApfLoad( int key, void *ptr, int len );

int pceFileWriteSector( void *ptr, int len );

// ���v�@�\�֌W�̃T�[�r�X

typedef struct tagPCETIME {
	unsigned short yy;	// �N
	unsigned char mm;	// ��
	unsigned char dd;	// ��
	unsigned char hh;	// ��
	unsigned char mi;	// ��
	unsigned char ss;	// �b
	unsigned char s100;	// 1/100�b
} PCETIME;

void pceTimeSet( const PCETIME *ptime );
void pceTimeGet( PCETIME *ptime );


typedef struct tagPCEALTIME {
	unsigned long mode;
	PCETIME time;
} PCEALMTIME;

#define ALM_STOP      0
#define ALM_EVERYHOUR 1 // 2003.02.14 N.SAWA
#define ALM_EVERYDAY  3 // 2003.02.14 N.SAWA
#define ALM_ONESHOT   7

int pceTimeSetAlarm( const PCEALMTIME *ptime );
int pceTimeGetAlarm( PCEALMTIME *ptime );



// �d���֌W�̃T�[�r�X

typedef struct tagPCEPWRSTAT {
	unsigned char status;	// 0 ���d�� 0 : USB���d, 1 : �d�r���d
	unsigned char resv;		// 1 �\��
	unsigned short battvol;	// 2 �d�r�d��(mV)
} PCEPWRSTAT;

#define PWR_RPTOFF 0
#define PWR_RPTON  1

void pcePowerSetReport( int mode );
void pcePowerGetStatus( PCEPWRSTAT *ps );
void pcePowerForceBatt( int fn );
int pcePowerEnterStandby( int flag );

// �ԊO���֌W�̃T�[�r�X

void pceIRStartRx( unsigned char *pData, int len );
void pceIRStartTx( const unsigned char *pData, int len );
void pceIRStartRxEx( unsigned char *pData, int len, int mode, int (*callback)(int rlen) );
void pceIRStartTxEx( const unsigned char *pData, int len, int mode, int (*callback)(void) );
void pceIRStartRxPulse( int mode, void (*rxproc)( int flag, unsigned short time ), int timeout );
void pceIRStartTxPulse( int mode, int (*txproc)( int flag ) );
void pceIRStop( void );
int pceIRGetStat( void );

// PC ���� Piece �A�v���P�[�V�����ԒʐM(USB)�֌W�̃T�[�r�X

typedef struct tagUSBCOMINFO {
	unsigned char signature[16];
} USBCOMINFO;

void pceUSBCOMSetup( USBCOMINFO *puci );
void pceUSBCOMStartRx( unsigned char *pData, int len );
void pceUSBCOMStartTx( const unsigned char *pData, int len );
int pceUSBCOMStop( void );
int pceUSBCOMGetStat( void );

#define UCS_RXWAIT 0x001 // ��M�ҋ@��
#define UCS_RXING  0x002 // ��M��
#define UCS_RXDONE 0x004 // ��M����
#define UCS_TXWAIT 0x100 // ���M�ҋ@��
#define UCS_TXING  0x200 // ���M��
#define UCS_TXDONE 0x400 // ���M����

// �q�[�v�֘A�̃T�[�r�X

int pceHeapGetMaxFreeSize( void );
int pceHeapFree( void *memp );
void *pceHeapAlloc( unsigned long size0 );

typedef struct tagHEAPMEM {
	unsigned short mark;
	unsigned short owner;
	struct tagHEAPMEM *chain;
} HEAPMEM;

#define HEAPMEMMK 0xa431
#define HMO_SYSTEM 0xffff
#define HMO_DEFAULT_USER 1

// ���̑�

typedef struct tagSYSTEMINFO {
	unsigned short size;			//  0 ���̍\���̂̃T�C�Y
	unsigned short hard_ver;		//  2 �n�[�h�E�G�A�E�o�[�W����
	unsigned short bios_ver;		//  4 BIOS�o�[�W����
	unsigned short bios_date;		//  6 BIOS�X�V�� YY(7):MM(4):DD(5)
	unsigned long sys_clock;		//  8 �V�X�e���E�N���b�N(Hz)
	unsigned short vdde_voltage;	// 12 VDDE(����)�d��(mV)
	unsigned short resv1;			// 14 �\��
	unsigned char *sram_top;		// 16 SRAM �J�n�A�h���X
	unsigned char *sram_end;		// 20 SRAM �I���A�h���X+1
	unsigned char *pffs_top;		// 24 pffs �J�n�A�h���X
	unsigned char *pffs_end;		// 28 pffs �I���A�h���X
} SYSTEMINFO;

const SYSTEMINFO *pceSystemGetInfo( void );

void pceDebugSetMon( int mode );

#ifndef _STDARG_H
typedef char *va_list;
#endif //ifndef _STDARG_H

int pcesprintf( char *buff, const char *format, ... );
int pcevsprintf( char *buff, const char *format, va_list argp );
unsigned long pceCRC32( const void *ptr, unsigned long len );

#ifndef NOPCESPRINTF
#define sprintf pcesprintf
#define vsprintf pcevsprintf
#endif //ifndef NOPCESPRINTF

#define PAD_RI 0x01
#define PAD_LF 0x02
#define PAD_DN 0x04
#define PAD_UP 0x08
#define PAD_B  0x10
#define PAD_A  0x20
#define PAD_D  0x40
#define PAD_C  0x80

#define TRG_RI 0x0100
#define TRG_LF 0x0200
#define TRG_DN 0x0400
#define TRG_UP 0x0800
#define TRG_B  0x1000
#define TRG_A  0x2000
#define TRG_D  0x4000
#define TRG_C  0x8000

#define PAD_START PAD_C
#define TRG_START TRG_C

#define PAD_SELECT PAD_D
#define TRG_SELECT TRG_D

//
// �A�v���P�[�V�����E�w�b�_���
//
typedef struct _pceAPPHEAD {
	unsigned long signature;						// +0  �V�O�l�`���[ 'pCeA'
	unsigned short sysver;							// +4  �V�X�e���E�o�[�W����
	unsigned short resv1;							// +6  �\��(=0)
	void (*initialize)( void );						// +8  �������֐�
	void (*periodic_proc)( int cnt );				// +12 �����֐�
	void (*pre_terminate)( void );					// +16 �I���O�֐�
	int (*notify_proc)( int type, int param );		// +20 �ʒm�֐�
	unsigned long stack_size;						// +24 �X�^�b�N�T�C�Y�ւ̃|�C���^
	unsigned char *bss_end;							// +28 BSS�̏I���A�h���X
} pceAPPHEAD;

#define APPSIG 0x41654370
#define APPSIG_OLD 0x416d5369		// ver 0.15�ȑO
//#define APPSYSVER VERSION(0,22)	// ver 0.22
#define APPSYSVER VERSION(1,18)		// ver 1.18

#define APPNF_EXITREQ   1
#define APPNF_SMSTART   2
#define APPNF_SMRESUME  3
#define APPNF_SMREQVBUF 4
#define APPNF_STANDBY   5
#define APPNF_ALARM     6

#define APPNR_IGNORE    0
#define APPNR_ACCEPT    1
#define APPNR_SUSPEND   2
#define APPNR_REJECT    3

//
// �t���b�V���������E�t�@�C���E�V�X�e��
// pffs
//

typedef struct _pffsFileHEADER {
	unsigned char mark;				// +0   �}�[�N
	unsigned char type;				// +1   �t�@�C���E�^�C�v
	unsigned short ofs_data;		// +2   ���f�[�^�ւ̃I�t�Z�b�g
	unsigned short ofs_name;		// +4   �L���v�V�����ւ̃I�t�Z�b�g
	unsigned short ofs_icon;		// +6   �A�C�R���ւ̃I�t�Z�b�g
	unsigned long resv2;			// +8   �\��
	unsigned long top_adrs;			// +12  �擪�̃A�h���X
	unsigned long length;			// +16  ����
	unsigned long crc32;			// +20  CRC32
} pffsFileHEADER;

#define PFFS_FT_EXE1    0x01	// �W�����s�`��(���k�Ȃ�)
#define PFFS_FT_EXE2    0x02	// �W�����s�`��(���k����)

/*
�P�u���b�N��4096B

�Ǘ��u���b�N

�}�[�N                    32B
�f�B���N�g�� 32B�~ 96 = 3072B
//FAT           2B�~256 =  512B
FAT           2B�~496 =  992B
*/

#define MAXDIR 96
//#define MAXFAT 256
#define MAXFAT 496

typedef struct tagpffsMARK {
	unsigned long ptr;
	unsigned long resv;
	char signature[24];
} pffsMARK;

typedef struct tagDIRECTORY {
	char name[24];
	unsigned char attr;
	unsigned char resv;
	unsigned short chain;
	unsigned long size;
} DIRECTORY;

typedef struct tagFAT {
	unsigned short chain;
} FAT;

typedef struct tagpffsMASTERBLOCK {
	pffsMARK mark;
	DIRECTORY dir[MAXDIR];
	FAT fat[MAXFAT];
} pffsMASTERBLOCK;

#define FAT_FREE    0xffff
#define FAT_END     0xeeee
#define FAT_INVALID 0xdddd
#define FAT_SYSTEM  0xcccc


#ifndef __PCEKN__
#include <draw.h>
#endif //#ifndef PCEKN

#ifdef __cplusplus
};
#endif

#endif //_PIECE_H

