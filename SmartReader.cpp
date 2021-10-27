#include "SmartReader.h"


#define OUT_BUFFER_SIZE		IDENTIFY_BUFFER_SIZE+16

#define DRIVE_HEAD_REG		0xA0


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

			switch (uiDriveType)
			{
			case DRIVE_FIXED:
			{
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

	return bRet;
}

BOOL CSmartReader::ReadSMARTInfo(BYTE ucDriveIndex)
{
	HANDLE hPhysicalDriveIOCTL = NULL;
	char szDriveName[MAX_PATH] = { 0 };
	BOOL bRet = FALSE;
	DWORD dwRet = 0;

	//
	//  Try to get a handle to PhysicalDrive IOCTL, report failure and exit if can't.
	//
	wsprintf(szDriveName, "\\\\.\\PhysicalDrive%d", ucDriveIndex);

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
			//std::cout << "DeviceIoControl(SMART_GET_VERSION) FAILED with an Error Code = " << std::showbase << std::hex << ::GetLastError() << std::dec << std::endl;

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
				//std::cout << "DeviceIoControl(DFP_GET_VERSION) FAILED with an Error Code = " << std::showbase << std::hex << ::GetLastError() << std::dec << std::endl;
			}
			else
			{
				//std::cout << "Version number of the binary driver = " << static_cast<int>(m_stDrivesInfo[ucDriveIndex].m_stGVIP.bVersion) << std::endl;
				//std::cout << "Revision number of the binary driver = " << static_cast<int>(m_stDrivesInfo[ucDriveIndex].m_stGVIP.bRevision) << std::endl;
				//std::cout << "Device map = " << static_cast<int>(m_stDrivesInfo[ucDriveIndex].m_stGVIP.bIDEDeviceMap) << std::endl;
			}
		}
		else
		{
			// Get the version, etc of PhysicalDrive IOCTL
			memcpy((void*)&(m_stDrivesInfo[ucDriveIndex].m_stGVIP), (void*)&gvip, sizeof(GETVERSIONINPARAMS));

			//std::cout << "Version number of the binary driver = " << static_cast<int>(m_stDrivesInfo[ucDriveIndex].m_stGVIP.bVersion) << std::endl;
			//std::cout << "Revision number of the binary driver = " << static_cast<int>(m_stDrivesInfo[ucDriveIndex].m_stGVIP.bRevision) << std::endl;
			//std::cout << "Device map = " << static_cast<int>(m_stDrivesInfo[ucDriveIndex].m_stGVIP.bIDEDeviceMap) << std::endl;

			if ((m_stDrivesInfo[ucDriveIndex].m_stGVIP.fCapabilities & CAP_SMART_CMD) == CAP_SMART_CMD)
			{
				//std::cout << "Device supports SMART commands" << std::endl;

				if (IsSmartEnabled(hPhysicalDriveIOCTL, ucDriveIndex))
				{
					bRet = CollectDriveInfo(hPhysicalDriveIOCTL, ucDriveIndex);

					bRet = ReadSMARTAttributes(hPhysicalDriveIOCTL, ucDriveIndex);
				}
			}
			else if ((m_stDrivesInfo[ucDriveIndex].m_stGVIP.fCapabilities & CAP_ATA_ID_CMD) == CAP_ATA_ID_CMD)
			{
				//std::cout << "Device supports ATA ID command command" << std::endl;
			}
			else if ((m_stDrivesInfo[ucDriveIndex].m_stGVIP.fCapabilities & CAP_ATAPI_ID_CMD) == CAP_ATAPI_ID_CMD)
			{
				//std::cout << "Device supports ATAPI ID command" << std::endl;
			}
		}

		CloseHandle(hPhysicalDriveIOCTL);
	}
	else
	{
		//std::cout << "DeviceIoControl(SMART_GET_VERSION) FAILED with an Error Code = " << std::showbase << std::hex << ::GetLastError() << std::dec << std::endl;
	}

	return bRet;
}

BOOL CSmartReader::IsSmartEnabled(HANDLE hDevice, UCHAR ucDriveIndex)
{
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
		m_stDrivesInfo[ucDriveIndex].m_csErrorString.Format("Error %d in reading SMART Enabled flag - SMART_SEND_DRIVE_COMMAND", dwRet);
	}
	else
	{
		bRet = TRUE;
	}


	return bRet;
}

BOOL CSmartReader::CollectDriveInfo(HANDLE hDevice, UCHAR ucDriveIndex)
{
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
	}
	else
	{
		CopyMemory(&m_stDrivesInfo[ucDriveIndex].m_stInfo, szOutput + 16, sizeof(ST_IDSECTOR));

		ConvertString(m_stDrivesInfo[ucDriveIndex].m_stInfo.sModelNumber, 39);
		ConvertString(m_stDrivesInfo[ucDriveIndex].m_stInfo.sSerialNumber, 20);
		ConvertString(m_stDrivesInfo[ucDriveIndex].m_stInfo.sFirmwareRev, 8);

		bRet = TRUE;
	}

	return bRet;
}

VOID CSmartReader::ConvertString(PBYTE pString, DWORD cbData)
{
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

	pIt = m_oSMARTDetails.find(ucAttribIndex);
	if (pIt != m_oSMARTDetails.end())
		pRet = &pIt->second;

	return pRet;
}

ST_SMART_INFO* CSmartReader::GetSMARTValue(BYTE ucDriveIndex, BYTE ucAttribIndex)
{
	SMARTINFOMAP::iterator pIt;
	ST_SMART_INFO* pRet = NULL;

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
	return &m_stDrivesInfo[ucDriveIndex];
}
