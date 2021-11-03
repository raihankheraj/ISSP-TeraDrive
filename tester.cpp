#include <iostream>
#include <windows.h>
#include <Wbemidl.h>
#include <comdef.h>
#include <vector>

#include "StorageDevice.h"
#include "SmartReader.h"
#include "Common.h"
#include "CTracer.h"
#include "WMIHelper.h"

#pragma comment(lib, "wbemuuid.lib")

using namespace::std;

CTracer g_objTracer(L"", L"SmartHDD.log");

int main()
{
	DF_TRACE_LOG(L"::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");
	DF_TRACE_LOG(L"main() - ENTER");

	CSmartReader objSmartReader;

	BYTE ucT1 = 0;
	BYTE ucT2 = 0;
	BYTE ucT3 = 0;

	ST_DRIVE_INFO* pDriveInfo = NULL;
	ST_SMART_INFO* pSmartInfo = NULL;
	ST_SMART_DETAILS* pSmartDetails = NULL;
	ST_IDSECTOR* pSectorInfo = NULL;

	char szT1[MAX_PATH] = { 0 };

	vector<StorageDevice> storageDevices;
	WMIHelper wmi("ROOT\\microsoft\\windows\\storage");
	storageDevices = wmi.Get_MSFT_PhysicalDisk_PropertyValues();

	vector<DiskDrive> drives;
	WMIHelper wmi2("root\\CIMV2");
	drives = wmi2.Get_Win32_DiskDrive_PropertyValues();
	
	DF_TRACE_LOG(L"main() - Call ReadSMARTValuesForAllDrives()");

	// Reading Self-Monitoring Analysis and Reporting Technology (SMART) Values

	objSmartReader.ReadSMARTValuesForAllDrives();

	DF_TRACE_LOG(L"main() - m_ucDrivesWithInfo = %d", objSmartReader.m_ucDrivesWithInfo);

	for (ucT1 = 0; ucT1 < objSmartReader.m_ucDrivesWithInfo; ++ucT1)
	{
		char sz[3] = { 0 };
		wsprintf(sz, "%d", ucT1);
		string strDeviceId = sz;
		bool blnSSD = false;

		for (StorageDevice sd : storageDevices)
		{
			if ((sd.GetDeviceID() == strDeviceId) && sd.IsSSD())
			{
				blnSSD = true;
				break;
			}
		}

		if (blnSSD)
		{
			// SSD drive was detected
			continue;
		}

		pSectorInfo = &objSmartReader.m_stDrivesInfo[ucT1].m_stInfo;

		wsprintf(szT1, "");
		std::cout << szT1 << std::endl;

		wsprintf(szT1, ">>>>>>>>>>>>>>>  Drive %d  <<<<<<<<<<<<<<<<", ucT1);
		std::cout << szT1 << std::endl;

		std::cout << "Model:  " << pSectorInfo->sModelNumber << std::endl;
		std::cout << "Serial: " << pSectorInfo->sSerialNumber << std::endl;
		//std::cout << "Cache = " << pSectorInfo->wBufferSize << std::endl;
		//std::cout << "Buffer = " << pSectorInfo->wBufferType << std::endl;

		pDriveInfo = objSmartReader.GetDriveInfo(ucT1);

		bool blnRetrieved = true;

		//for (ucT2 = 0; ucT2 < 255; ++ucT2)
		for (ucT2 = 196; ucT2 < 199; ++ucT2)
		{
			pSmartInfo = objSmartReader.GetSMARTValue(pDriveInfo->m_ucDriveIndex, ucT2);

			if (pSmartInfo)
			{
				pSmartDetails = objSmartReader.GetSMARTDetails(pSmartInfo->m_ucAttribIndex);

				if (pSmartDetails)
				{
					if (pSmartDetails->m_bCritical)
						wsprintf(szT1, "(CRITICAL) ATTRIBUTE Name=%s Id=%d Value=%d Worst=%d AttributeValue=%lu Threshold=%lu", pSmartDetails->m_csAttribName.GetString(), pSmartDetails->m_ucAttribId, pSmartInfo->m_ucValue, pSmartInfo->m_ucWorst, pSmartInfo->m_dwAttribValue, pSmartInfo->m_dwThreshold);
					else
						wsprintf(szT1, "ATTRIBUTE Name=%s Id=%d Value=%d Worst=%d AttributeValue=%lu Threshold=%lu", pSmartDetails->m_csAttribName.GetString(), pSmartDetails->m_ucAttribId, pSmartInfo->m_ucValue, pSmartInfo->m_ucWorst, pSmartInfo->m_dwAttribValue, pSmartInfo->m_dwThreshold);

					std::cout << szT1 << std::endl;
				}
			}
			else
			{
				blnRetrieved = false;
			}
		}

		if (!blnRetrieved)
		{
			std::cout << "S.M.A.R.T attributes (IDs 196, 197 and 198) could NOT be retrieved from the Physical Drive." << std::endl;
		}
	}

	for (StorageDevice sd : storageDevices)
	{
		string s = sd.GetDeviceID();

		if (sd.IsSSD())
		{
			wsprintf(szT1, "");
			std::cout << szT1 << std::endl;

			wsprintf(szT1, ">>>>>>>>>>>>>>>  Drive %s  <<<<<<<<<<<<<<<<", s.c_str());
			std::cout << szT1 << std::endl;

			std::cout << "Model:  " << sd.GetFriendlyName() << std::endl;

			for (DiskDrive drive : drives)
			{
				if (drive.GetModel() == sd.GetFriendlyName())
				{
					std::cout << "Serial: " << drive.GetSerialNumber() << std::endl;
				}
			}
			std::cout << "S.M.A.R.T attributes (IDs 196, 197 and 198) could NOT be retrieved for SSD drive." << std::endl;
		}
	}

	DF_TRACE_LOG(L"main() - EXIT");
	DF_TRACE_LOG(L"::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");
}
