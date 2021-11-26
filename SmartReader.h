#pragma once

#include <windows.h>
#include <devioctl.h>
#include <Ntdddisk.h>

#include <iostream>
#include <string>
#include <map>
#include <atlstr.h>

#define SMART_ATTRIB_RAW_READ_ERROR_RATE					1
#define SMART_ATTRIB_THROUGHPUT_PERFORMANCE					2
#define SMART_ATTRIB_SPIN_UP_TIME							3
#define SMART_ATTRIB_START_STOP_COUNT						4
#define SMART_ATTRIB_START_REALLOCATION_SECTOR_COUNT		5
#define SMART_ATTRIB_SEEK_ERROR_RATE						7
#define SMART_ATTRIB_POWER_ON_HOURS_COUNT					9
#define SMART_ATTRIB_SPIN_RETRY_COUNT						10
#define SMART_ATTRIB_RECALIBRATION_RETRIES					11
#define SMART_ATTRIB_DEVICE_POWER_CYCLE_COUNT				12
#define SMART_ATTRIB_SOFT_READ_ERROR_RATE					13
#define SMART_ATTRIB_LOAD_UNLOAD_CYCLE_COUNT				193
#define SMART_ATTRIB_TEMPERATURE							194
#define SMART_ATTRIB_ECC_ON_THE_FLY_COUNT					195
#define SMART_ATTRIB_REALLOCATION_EVENT_COUNT				196
#define SMART_ATTRIB_CURRENT_PENDING_SECTOR_COUNT			197
#define SMART_ATTRIB_UNCORRECTABLE_SECTOR_COUNT				198
#define SMART_ATTRIB_ULTRA_DMA_CRC_ERROR_COUNT				199
#define SMART_ATTRIB_WRITE_ERROR_RATE						200
#define SMART_ATTRIB_TA_COUNTER_INCREASED					202
#define SMART_ATTRIB_GSENSE_ERROR_RATE						221
#define SMART_ATTRIB_POWER_OFF_RETRACT_COUNT				228
#define MAX_ATTRIBUTES	                                    256

#define INDEX_ATTRIB_INDEX									0
#define INDEX_ATTRIB_UNKNOWN1								1
#define INDEX_ATTRIB_UNKNOWN2								2
#define INDEX_ATTRIB_VALUE									3
#define INDEX_ATTRIB_WORST									4
#define INDEX_ATTRIB_RAW									5

   //  IOCTL commands
#define  DFP_GET_VERSION          0x00074080
#define  DFP_SEND_DRIVE_COMMAND   0x0007c084
#define  DFP_RECEIVE_DRIVE_DATA   0x0007c088

//  Required to ensure correct PhysicalDrive IOCTL structure setup
#pragma pack(1)

typedef struct
{
	WORD wGenConfig;
	WORD wNumCyls;
	WORD wReserved;
	WORD wNumHeads;
	WORD wBytesPerTrack;
	WORD wBytesPerSector;
	WORD wSectorsPerTrack;
	WORD wVendorUnique[3];
	BYTE sSerialNumber[20];
	WORD wBufferType;
	WORD wBufferSize;
	WORD wECCSize;
	BYTE sFirmwareRev[8];
	BYTE sModelNumber[39];
	WORD wMoreVendorUnique;
	WORD wDoubleWordIO;
	WORD wCapabilities;
	WORD wReserved1;
	WORD wPIOTiming;
	WORD wDMATiming;
	WORD wBS;
	WORD wNumCurrentCyls;
	WORD wNumCurrentHeads;
	WORD wNumCurrentSectorsPerTrack;
	WORD ulCurrentSectorCapacity;
	WORD wMultSectorStuff;
	DWORD ulTotalAddressableSectors;
	WORD wSingleWordDMA;
	WORD wMultiWordDMA;
	BYTE bReserved[127];
}ST_IDSECTOR;

typedef struct
{
	BYTE  bDriverError;
	BYTE  bIDEStatus;
	BYTE  bReserved[2];
	DWORD dwReserved[2];
} ST_DRIVERSTAT;

typedef struct
{
	DWORD      cBufferSize;
	ST_DRIVERSTAT DriverStatus;
	BYTE       bBuffer[1];
} ST_ATAOUTPARAM;

typedef struct
{
	BYTE m_ucAttribIndex;
	DWORD m_dwAttribValue;
	BYTE m_ucValue;
	BYTE m_ucWorst;
	DWORD m_dwThreshold;
}ST_SMART_INFO;

typedef struct
{
	GETVERSIONINPARAMS m_stGVIP;
	ST_IDSECTOR m_stInfo;
	ST_SMART_INFO m_stSmartInfo[256];
	BYTE m_ucSmartValues;
	BYTE m_ucDriveIndex;
	CString m_csErrorString;
}ST_DRIVE_INFO;

typedef struct
{
	BOOL m_bCritical;
	BYTE m_ucAttribId;
	CString m_csAttribName;
	CString m_csAttribDetails;
}ST_SMART_DETAILS;

#pragma pack()

typedef std::map<DWORD, LPVOID> SMARTINFOMAP;

typedef std::map<BYTE, ST_SMART_DETAILS> SMARTDETAILSMAP;

class CSmartReader
{
public:
	CSmartReader();
	~CSmartReader();

	BOOL ReadSMARTValuesForAllDrives();
	ST_SMART_DETAILS* GetSMARTDetails(BYTE ucAttribIndex);
	ST_SMART_INFO* GetSMARTValue(BYTE ucDriveIndex, BYTE ucAttribIndex);
	ST_DRIVE_INFO* GetDriveInfo(BYTE ucDriveIndex);

	BYTE m_ucDrivesWithInfo; // Number of drives with information read
	BYTE m_ucDrives;// Fixed HDD's
	ST_DRIVE_INFO m_stDrivesInfo[16];

private:
	VOID InitAll();
	VOID CloseAll();
	VOID FillAttribGenericDetails();
	VOID ConvertString(PBYTE pString, DWORD cbData);
	void PrintDriveType(UINT uiDriveType);

	BOOL ReadSMARTInfo(BYTE ucDriveIndex);
	BOOL IsSmartEnabled(HANDLE hDevice, UCHAR ucDriveIndex);
	BOOL CollectDriveInfo(HANDLE hDevice, UCHAR ucDriveIndex);
	BOOL ReadSMARTAttributes(HANDLE hDevice, UCHAR ucDriveIndex);

	ST_SMART_DETAILS m_stSmartDetails;
	SMARTINFOMAP m_oSmartInfo;
	SMARTDETAILSMAP m_oSMARTDetails;
};