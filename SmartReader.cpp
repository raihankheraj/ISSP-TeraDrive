#include "SmartReader.h"

#include "Common.h"
#include "CTracer.h"


#define OUT_BUFFER_SIZE		IDENTIFY_BUFFER_SIZE+16
#define DRIVE_HEAD_REG		0xA0

extern CTracer g_objTracer;

CSmartReader::CSmartReader()
{
	InitAll();
	FillAttribGenericDetails();
}

CSmartReader::~CSmartReader()
{
	CloseAll();
}

VOID CSmartReader::InitAll()
{
	m_ucDrivesWithInfo = 0;
	m_ucDrives = 0;
	m_oSmartInfo.clear();
}

VOID CSmartReader::CloseAll()
{
	InitAll();
}

BOOL CSmartReader::ReadSMARTValuesForAllDrives()
{
	DF_TRACE_LOG(L"CSmartReader::ReadSMARTValuesForAllDrives() - ENTER");

	BOOL bFlag = 0;
	char szDrv[MAX_PATH] = { 0 };
	BYTE ucDriveIndex = 0;
	BYTE ucT2 = 0;
	BOOL bRet = FALSE;

	CloseAll();

	//
	// Retrieves a bitmask representing the currently available disk drives
	//
	DWORD dwBits = GetLogicalDrives();

	DF_TRACE_LOG(L"CSmartReader::ReadSMARTValuesForAllDrives() - GetLogicalDrives() returned = 0x%08x", dwBits);

	if (0 == dwBits)
	{
		// API call failed with return value of 0
		return FALSE;
	}
	DWORD dwBitVal = 1;
	ucT2 = 0;

	bFlag = (dwBits & dwBitVal);


	while (ucT2 < 32)
	{
		if (bFlag)
		{
			// Make string of Logical drives (letters)
			wsprintf(szDrv, "%c:\\", 'A' + ucT2);

			//
			// Determines whether a disk drive is a removable, fixed, CD-ROM, RAM disk, or network drive
			//
			UINT uiDriveType = GetDriveType(szDrv);

			PrintDriveType(uiDriveType);

			switch (uiDriveType)
			{
			case DRIVE_FIXED:
			{
				DF_TRACE_LOG(L"CSmartReader::ReadSMARTValuesForAllDrives() - DRIVE_FIXED - Drive=%S", szDrv);

				ucDriveIndex = ucT2 - 2;

				if (ReadSMARTInfo(ucDriveIndex))
				{
					m_ucDrivesWithInfo++;
				}

				m_ucDrives++;
				break;
			}
			default:
			{
				DF_TRACE_LOG(L"CSmartReader::ReadSMARTValuesForAllDrives() - SKIPPING - Drive=%S", szDrv);
				break;
			}
			}
		}
		dwBitVal = dwBitVal * 2;

		bFlag = (dwBits & dwBitVal);

		++ucT2;
	}

	if (m_ucDrives == m_ucDrivesWithInfo)
	{
		bRet = TRUE;
	}

	DF_TRACE_LOG(L"CSmartReader::ReadSMARTValuesForAllDrives() - EXIT");

	return bRet;
}

BOOL CSmartReader::ReadSMARTInfo(BYTE ucDriveIndex)
{
	DF_TRACE_LOG(L"CSmartReader::ReadSMARTInfo() - ENTER");

	HANDLE hPhysicalDriveIOCTL = NULL;
	char szDriveName[MAX_PATH] = { 0 };
	BOOL bRet = FALSE;
	DWORD dwRet = 0;

	//
	//  Try to get a handle to PhysicalDrive IOCTL, report failure and exit if can't.
	//
	wsprintf(szDriveName, "\\\\.\\PhysicalDrive%d", ucDriveIndex);

	DF_TRACE_LOG(L"CSmartReader::ReadSMARTInfo() - DriveName=%S", szDriveName);

	hPhysicalDriveIOCTL = CreateFile(
		szDriveName,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		0, //FILE_ATTRIBUTE_SYSTEM,
		NULL);

	if (hPhysicalDriveIOCTL != INVALID_HANDLE_VALUE)
	{
		DWORD cbBytesReturned = 0;
		GETVERSIONINPARAMS gvip;

		// Get the version, etc of PhysicalDrive IOCTL
		memset((void*)&gvip, 0, sizeof(GETVERSIONINPARAMS));

		//
		// Get the version
		//
		if (!DeviceIoControl(
			hPhysicalDriveIOCTL,
			SMART_GET_VERSION,
			NULL,
			0,
			&gvip, //&(m_stDrivesInfo[ucDriveIndex].m_stGVIP),
			sizeof(GETVERSIONINPARAMS),
			&cbBytesReturned,
			NULL))
		{
			DF_ERROR_LOG(L"DeviceIoControl(SMART_GET_VERSION) FAILED with an Error Code = ", ::GetLastError());

			// Try a different way
			//
			// Get the version
			//
			if (!DeviceIoControl(
				hPhysicalDriveIOCTL,
				DFP_GET_VERSION,
				NULL,
				0,
				&(m_stDrivesInfo[ucDriveIndex].m_stGVIP),
				sizeof(GETVERSIONINPARAMS),
				&cbBytesReturned,
				NULL))
			{
				DF_ERROR_LOG(L"DeviceIoControl(DFP_GET_VERSION) FAILED with an Error Code = ", ::GetLastError());
			}
			else
			{
				DF_TRACE_LOG(L"Version number of the binary driver = %lu", m_stDrivesInfo[ucDriveIndex].m_stGVIP.bVersion);
				DF_TRACE_LOG(L"Revision number of the binary driver = %lu", m_stDrivesInfo[ucDriveIndex].m_stGVIP.bRevision);
				DF_TRACE_LOG(L"Device map = 0x%08x", m_stDrivesInfo[ucDriveIndex].m_stGVIP.bIDEDeviceMap);
			}
		}
		else
		{
			// Get the version, etc of PhysicalDrive IOCTL
			memcpy((void*)&(m_stDrivesInfo[ucDriveIndex].m_stGVIP), (void*)&gvip, sizeof(GETVERSIONINPARAMS));

			DF_TRACE_LOG(L"Version number of the binary driver = %lu", m_stDrivesInfo[ucDriveIndex].m_stGVIP.bVersion);
			DF_TRACE_LOG(L"Revision number of the binary driver = %lu", m_stDrivesInfo[ucDriveIndex].m_stGVIP.bRevision);
			DF_TRACE_LOG(L"Device map = 0x%08x", m_stDrivesInfo[ucDriveIndex].m_stGVIP.bIDEDeviceMap);

			if ((m_stDrivesInfo[ucDriveIndex].m_stGVIP.fCapabilities & CAP_SMART_CMD) == CAP_SMART_CMD)
			{
				DF_TRACE_LOG(L"Device supports SMART commands");

				if (IsSmartEnabled(hPhysicalDriveIOCTL, ucDriveIndex))
				{
					bRet = CollectDriveInfo(hPhysicalDriveIOCTL, ucDriveIndex);

					bRet = ReadSMARTAttributes(hPhysicalDriveIOCTL, ucDriveIndex);
				}
			}
			else if ((m_stDrivesInfo[ucDriveIndex].m_stGVIP.fCapabilities & CAP_ATA_ID_CMD) == CAP_ATA_ID_CMD)
			{
				DF_TRACE_LOG(L"Device supports ATA ID command command");
			}
			else if ((m_stDrivesInfo[ucDriveIndex].m_stGVIP.fCapabilities & CAP_ATAPI_ID_CMD) == CAP_ATAPI_ID_CMD)
			{
				DF_TRACE_LOG(L"Device supports ATAPI ID command");
			}
		}

		CloseHandle(hPhysicalDriveIOCTL);
	}
	else
	{
		DF_ERROR_LOG(L"CreateFile(SMART_GET_VERSION) FAILED with an Error Code = ", ::GetLastError());
	}

	DF_TRACE_LOG(L"CSmartReader::ReadSMARTInfo() - EXIT");

	return bRet;
}

BOOL CSmartReader::IsSmartEnabled(HANDLE hDevice, UCHAR ucDriveIndex)
{
	DF_TRACE_LOG(L"CSmartReader::IsSmartEnabled() - ENTER");

	SENDCMDINPARAMS stCIP = { 0 };
	SENDCMDOUTPARAMS stCOP = { 0 };
	DWORD dwRet = 0;
	BOOL bRet = FALSE;

	stCIP.cBufferSize = 0;
	stCIP.bDriveNumber = ucDriveIndex;
	stCIP.irDriveRegs.bFeaturesReg = ENABLE_SMART;
	stCIP.irDriveRegs.bSectorCountReg = 1;
	stCIP.irDriveRegs.bSectorNumberReg = 1;
	stCIP.irDriveRegs.bCylLowReg = SMART_CYL_LOW;
	stCIP.irDriveRegs.bCylHighReg = SMART_CYL_HI;
	stCIP.irDriveRegs.bDriveHeadReg = DRIVE_HEAD_REG;
	stCIP.irDriveRegs.bCommandReg = SMART_CMD;

	// determind wheter is SMART enable on the drive

	if (!DeviceIoControl(
		hDevice,
		SMART_SEND_DRIVE_COMMAND,
		&stCIP,
		sizeof(SENDCMDINPARAMS),
		&stCOP,
		sizeof(SENDCMDOUTPARAMS),
		&dwRet,
		NULL))
	{
		DF_ERROR_LOG(L"DeviceIoControl(SMART_SEND_DRIVE_COMMAND) FAILED with an Error Code = ", ::GetLastError());
		m_stDrivesInfo[ucDriveIndex].m_csErrorString.Format("Error %d in reading SMART Enabled flag - SMART_SEND_DRIVE_COMMAND", dwRet);
	}
	else
	{
		DF_TRACE_LOG(L"CSmartReader::IsSmartEnabled() - DeviceIoControl(SMAR_SEND_DRIVE_COMMAND) completed SUCCESSFULLY");
		bRet = TRUE;
	}

	DF_TRACE_LOG(L"CSmartReader::IsSmartEnabled() - EXIT");

	return bRet;
}

BOOL CSmartReader::CollectDriveInfo(HANDLE hDevice, UCHAR ucDriveIndex)
{
	DF_TRACE_LOG(L"CSmartReader::CollectDriveInfo() - ENTER");

	BOOL bRet = FALSE;
	SENDCMDINPARAMS stCIP = { 0 };
	DWORD dwRet = 0;

	char szOutput[OUT_BUFFER_SIZE] = { 0 };

	stCIP.cBufferSize = IDENTIFY_BUFFER_SIZE;
	stCIP.bDriveNumber = ucDriveIndex;
	stCIP.irDriveRegs.bFeaturesReg = 0;
	stCIP.irDriveRegs.bSectorCountReg = 1;
	stCIP.irDriveRegs.bSectorNumberReg = 1;
	stCIP.irDriveRegs.bCylLowReg = 0;
	stCIP.irDriveRegs.bCylHighReg = 0;
	stCIP.irDriveRegs.bDriveHeadReg = DRIVE_HEAD_REG;
	stCIP.irDriveRegs.bCommandReg = ID_CMD;

	// Collect drive info, Drive number, Model number, Serial number

	if (!DeviceIoControl(
		hDevice,
		SMART_RCV_DRIVE_DATA,
		&stCIP,
		sizeof(stCIP),
		szOutput,
		OUT_BUFFER_SIZE,
		&dwRet,
		NULL))
	{
		DF_ERROR_LOG(L"DeviceIoControl(SMART_RCV_DRIVE_DATA) FAILED with an Error Code = ", ::GetLastError());
	}
	else
	{
		DF_TRACE_LOG(L"CSmartReader::IsSmartEnabled() - DeviceIoControl(SMART_RCV_DRIVE_DATA) completed SUCCESSFULLY");

		CopyMemory(&m_stDrivesInfo[ucDriveIndex].m_stInfo, szOutput + 16, sizeof(ST_IDSECTOR));

		ConvertString(m_stDrivesInfo[ucDriveIndex].m_stInfo.sModelNumber, 39);
		ConvertString(m_stDrivesInfo[ucDriveIndex].m_stInfo.sSerialNumber, 20);
		ConvertString(m_stDrivesInfo[ucDriveIndex].m_stInfo.sFirmwareRev, 8);

		bRet = TRUE;
	}

	DF_TRACE_LOG(L"CSmartReader::CollectDriveInfo() - EXIT");

	return bRet;
}

VOID CSmartReader::ConvertString(PBYTE pString, DWORD cbData)
{
	// convert string to data
	CString csT1;
	char szT1[MAX_PATH] = { 0 };
	for (DWORD nC1 = 0; nC1 < cbData; nC1 += 2)
	{
		szT1[nC1] = pString[nC1 + 1];
		szT1[nC1 + 1] = pString[nC1];
	}
	csT1 = szT1; csT1.TrimLeft(); csT1.TrimRight();
	lstrcpy(szT1, (PCHAR)(LPCTSTR)csT1);
	memcpy(pString, szT1, cbData);
}

VOID CSmartReader::FillAttribGenericDetails()
{
	char szINIFileName[MAX_PATH] = { 0 }, szKeyName[MAX_PATH] = { 0 }, szValue[1024] = { 0 };
	int nC1, nSmartAttribs;
	ST_SMART_DETAILS stSD;

	m_oSMARTDetails.clear();

	GetModuleFileName(NULL, szINIFileName, MAX_PATH);
	szINIFileName[lstrlen(szINIFileName) - 3] = 0;
	lstrcat(szINIFileName, "ini");

	// fill attributes generica details with Attrib, Id, Critical, Name, and Details

	nSmartAttribs = GetPrivateProfileInt("General", "Max Attributes", 0, szINIFileName);
	for (nC1 = 0; nC1 < nSmartAttribs; ++nC1)
	{
		wsprintf(szKeyName, "Attrib%d", nC1);
		stSD.m_ucAttribId = GetPrivateProfileInt(szKeyName, "Id", 0, szINIFileName);
		stSD.m_bCritical = GetPrivateProfileInt(szKeyName, "Critical", 0, szINIFileName);
		GetPrivateProfileString(szKeyName, "Name", "", szValue, 1024, szINIFileName);
		stSD.m_csAttribName = szValue;
		GetPrivateProfileString(szKeyName, "Details", "", szValue, 1024, szINIFileName);
		stSD.m_csAttribDetails = szValue;
		m_oSMARTDetails.insert(SMARTDETAILSMAP::value_type(stSD.m_ucAttribId, stSD));
	}
}

ST_SMART_DETAILS* CSmartReader::GetSMARTDetails(BYTE ucAttribIndex)
{
	SMARTDETAILSMAP::iterator pIt;
	ST_SMART_DETAILS* pRet = NULL;

	// get SMART details 
	pIt = m_oSMARTDetails.find(ucAttribIndex);
	if (pIt != m_oSMARTDetails.end())
		pRet = &pIt->second;

	return pRet;
}

ST_SMART_INFO* CSmartReader::GetSMARTValue(BYTE ucDriveIndex, BYTE ucAttribIndex)
{
	SMARTINFOMAP::iterator pIt;
	ST_SMART_INFO* pRet = NULL;

	// get SMART values
	pIt = m_oSmartInfo.find(MAKELPARAM(ucAttribIndex, ucDriveIndex));
	if (pIt != m_oSmartInfo.end())
		pRet = (ST_SMART_INFO*)pIt->second;
	return pRet;
}

BOOL CSmartReader::ReadSMARTAttributes(HANDLE hDevice, UCHAR ucDriveIndex)
{
	SENDCMDINPARAMS stCIP = { 0 };
	DWORD dwRet = 0;
	BOOL bRet = FALSE;
	BYTE	szAttributes[sizeof(ST_ATAOUTPARAM) + READ_ATTRIBUTE_BUFFER_SIZE - 1];
	UCHAR ucT1;
	PBYTE pT1, pT3; PDWORD pT2;
	ST_SMART_INFO* pSmartValues;

	stCIP.cBufferSize = READ_ATTRIBUTE_BUFFER_SIZE;
	stCIP.bDriveNumber = ucDriveIndex;
	stCIP.irDriveRegs.bFeaturesReg = READ_ATTRIBUTES;
	stCIP.irDriveRegs.bSectorCountReg = 1;
	stCIP.irDriveRegs.bSectorNumberReg = 1;
	stCIP.irDriveRegs.bCylLowReg = SMART_CYL_LOW;
	stCIP.irDriveRegs.bCylHighReg = SMART_CYL_HI;
	stCIP.irDriveRegs.bDriveHeadReg = DRIVE_HEAD_REG;
	stCIP.irDriveRegs.bCommandReg = SMART_CMD;

	// read SMART attributes

	bRet = DeviceIoControl(
		hDevice,
		SMART_RCV_DRIVE_DATA,
		&stCIP,
		sizeof(stCIP),
		szAttributes,
		sizeof(ST_ATAOUTPARAM) + READ_ATTRIBUTE_BUFFER_SIZE - 1,
		&dwRet,
		NULL);

	if (bRet)
	{
		m_stDrivesInfo[ucDriveIndex].m_ucSmartValues = 0;
		m_stDrivesInfo[ucDriveIndex].m_ucDriveIndex = ucDriveIndex;
		pT1 = (PBYTE)(((ST_ATAOUTPARAM*)szAttributes)->bBuffer);
		for (ucT1 = 0; ucT1 < 30; ++ucT1)
		{
			pT3 = &pT1[2 + ucT1 * 12];
			pT2 = (PDWORD)&pT3[INDEX_ATTRIB_RAW];
			pT3[INDEX_ATTRIB_RAW + 2] = pT3[INDEX_ATTRIB_RAW + 3] = pT3[INDEX_ATTRIB_RAW + 4] = pT3[INDEX_ATTRIB_RAW + 5] = pT3[INDEX_ATTRIB_RAW + 6] = 0;
			if (pT3[INDEX_ATTRIB_INDEX] != 0)
			{
				pSmartValues = &m_stDrivesInfo[ucDriveIndex].m_stSmartInfo[m_stDrivesInfo[ucDriveIndex].m_ucSmartValues];
				pSmartValues->m_ucAttribIndex = pT3[INDEX_ATTRIB_INDEX];
				pSmartValues->m_ucValue = pT3[INDEX_ATTRIB_VALUE];
				pSmartValues->m_ucWorst = pT3[INDEX_ATTRIB_WORST];
				pSmartValues->m_dwAttribValue = pT2[0];
				pSmartValues->m_dwThreshold = MAXDWORD;
				m_oSmartInfo[MAKELPARAM(pSmartValues->m_ucAttribIndex, ucDriveIndex)] = pSmartValues;
				m_stDrivesInfo[ucDriveIndex].m_ucSmartValues++;
			}
		}
	}
	else
	{
		dwRet = GetLastError();
	}


	stCIP.irDriveRegs.bFeaturesReg = READ_THRESHOLDS;
	stCIP.cBufferSize = READ_THRESHOLD_BUFFER_SIZE; // Is same as attrib size
	bRet = DeviceIoControl(
		hDevice,
		SMART_RCV_DRIVE_DATA,
		&stCIP,
		sizeof(stCIP),
		szAttributes,
		sizeof(ST_ATAOUTPARAM) + READ_ATTRIBUTE_BUFFER_SIZE - 1,
		&dwRet,
		NULL);

	if (bRet)
	{
		pT1 = (PBYTE)(((ST_ATAOUTPARAM*)szAttributes)->bBuffer);
		for (ucT1 = 0; ucT1 < 30; ++ucT1)
		{
			pT2 = (PDWORD)&pT1[2 + ucT1 * 12 + 5];
			pT3 = &pT1[2 + ucT1 * 12];
			pT3[INDEX_ATTRIB_RAW + 2] = pT3[INDEX_ATTRIB_RAW + 3] = pT3[INDEX_ATTRIB_RAW + 4] = pT3[INDEX_ATTRIB_RAW + 5] = pT3[INDEX_ATTRIB_RAW + 6] = 0;
			if (pT3[0] != 0)
			{
				pSmartValues = GetSMARTValue(ucDriveIndex, pT3[0]);
				if (pSmartValues)
					pSmartValues->m_dwThreshold = pT2[0];
			}
		}
	}
	return bRet;
}

ST_DRIVE_INFO* CSmartReader::GetDriveInfo(BYTE ucDriveIndex)
{
	// get drive info
	return &m_stDrivesInfo[ucDriveIndex];
}

void CSmartReader::PrintDriveType(UINT uiDriveType)
{
	switch (uiDriveType)
	{
	case DRIVE_UNKNOWN:
	{
		DF_TRACE_LOG(L"CSmartReader::ReadSMARTValuesForAllDrives() - GetDriveType() returned DRIVE_UNKNOWN (0x%08x)", uiDriveType);
		break;
	}
	case DRIVE_NO_ROOT_DIR:
	{
		DF_TRACE_LOG(L"CSmartReader::ReadSMARTValuesForAllDrives() - GetDriveType() returned DRIVE_NO_ROOT_DIR (0x%08x)", uiDriveType);
		break;
	}
	case DRIVE_REMOVABLE:
	{
		DF_TRACE_LOG(L"CSmartReader::ReadSMARTValuesForAllDrives() - GetDriveType() returned DRIVE_REMOVABLE (0x%08x)", uiDriveType);
		break;
	}
	case DRIVE_FIXED:
	{
		DF_TRACE_LOG(L"CSmartReader::ReadSMARTValuesForAllDrives() - GetDriveType() returned DRIVE_FIXED (0x%08x)", uiDriveType);
		break;
	}
	case DRIVE_REMOTE:
	{
		DF_TRACE_LOG(L"CSmartReader::ReadSMARTValuesForAllDrives() - GetDriveType() returned DRIVE_REMOTE (0x%08x)", uiDriveType);
		break;
	}
	case DRIVE_CDROM:
	{
		DF_TRACE_LOG(L"CSmartReader::ReadSMARTValuesForAllDrives() - GetDriveType() returned DRIVE_CDROM (0x%08x)", uiDriveType);
		break;
	}
	case DRIVE_RAMDISK:
	{
		DF_TRACE_LOG(L"CSmartReader::ReadSMARTValuesForAllDrives() - GetDriveType() returned DRIVE_RAMDISK (0x%08x)", uiDriveType);
		break;
	}
	default:
	{
		DF_TRACE_LOG(L"CSmartReader::ReadSMARTValuesForAllDrives() - GetDriveType() returned Unknown value (0x%08x)", uiDriveType);
		break;
	}
	}
}