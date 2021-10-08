// SmartHDD.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "SmartReader.h"
#include "Common.h"
#include "CTracer.h"

CTracer g_objTracer(L"", L"SmartHDD.log");

int main()
{
	DF_TRACE_LOG(L"main() - ENTER");

	CSmartReader objSmartReader;

	BYTE ucT1 = 0;
	BYTE ucT2 = 0;
	BYTE ucT3 = 0;
	BYTE ucT4 = 0;

	ST_DRIVE_INFO* pDriveInfo = NULL;
	ST_SMART_INFO* pSmartInfo = NULL;
	ST_SMART_DETAILS* pSmartDetails = NULL;
	ST_IDSECTOR* pSectorInfo = NULL;

	char szT1[MAX_PATH];

	DF_TRACE_LOG(L"main() - Call ReadSMARTValuesForAllDrives()");

	// Reading Self-Monitoring Analysis and Reporting Technology (SMART) Values
	objSmartReader.ReadSMARTValuesForAllDrives();

	DF_TRACE_LOG(L"main() - m_ucDrivesWithInfo = %d", objSmartReader.m_ucDrivesWithInfo);

	for (ucT1 = 0, ucT4 = 0; ucT1 < objSmartReader.m_ucDrivesWithInfo; ++ucT1)
	{
		wsprintf(szT1, "");
		std::cout << szT1 << std::endl;

		wsprintf(szT1, ">>>>>>>>>>>>>>>  Drive %d  <<<<<<<<<<<<<<<<", ucT1);
		std::cout << szT1 << std::endl;

		pSectorInfo = &objSmartReader.m_stDrivesInfo[ucT1].m_stInfo;

		std::cout << "Model:  " << pSectorInfo->sModelNumber << std::endl;

		std::cout << "Serial: " << pSectorInfo->sSerialNumber << std::endl;

		std::cout << "Cache = " << pSectorInfo->wBufferSize << std::endl;

		std::cout << "Buffer = " << pSectorInfo->wBufferType << std::endl;

		//wsprintf(szT1, "Cache=%d Buffer=%d", pSectorInfo->wBufferSize, pSectorInfo->wBufferType);

		//std::cout << szT1 << std::endl;

		pDriveInfo = objSmartReader.GetDriveInfo(ucT1);

		for (ucT2 = ucT3 = 0; ucT2 < 255; ++ucT2)
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

					ucT4++;
				}
			}
		}
	}
}