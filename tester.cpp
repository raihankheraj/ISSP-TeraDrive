// SmartHDD.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "SmartReader.h"

int main()
{
	CSmartReader objSmartReader;
	ST_DRIVE_INFO* pDriveInfo = NULL;
	BYTE ucT1, ucT2, ucT3, ucT4;
	ST_SMART_INFO* pSmartInfo = NULL;
	ST_SMART_DETAILS* pSmartDetails = NULL;
	ST_IDSECTOR* pSectorInfo = NULL;
	char szT1[MAX_PATH];

	std::cout << "Reading Self-Monitoring Analysis and Reporting Technology (SMART) Values ..." << std::endl;

	objSmartReader.ReadSMARTValuesForAllDrives();

	for (ucT1 = 0, ucT4 = 0; ucT1 < objSmartReader.m_ucDrivesWithInfo; ++ucT1)
	{
		wsprintf(szT1, "");
		std::cout << szT1 << std::endl;

		wsprintf(szT1, ">>>>>>>>>>>>>>>  Drive %d  <<<<<<<<<<<<<<<<", ucT1);
		std::cout << szT1 << std::endl;

		pSectorInfo = &objSmartReader.m_stDrivesInfo[ucT1].m_stInfo;

		std::cout << "Model: " << pSectorInfo->sModelNumber << std::endl;

		std::cout << "Serial: " << pSectorInfo->sSerialNumber << std::endl;

		wsprintf(szT1, "Cache=%d Buffer=%d", pSectorInfo->wBufferSize, pSectorInfo->wBufferType);
		std::cout << szT1 << std::endl;

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
						wsprintf(szT1, "%03d (CR)", pSmartDetails->m_ucAttribId);
					else
						wsprintf(szT1, "%03d", pSmartDetails->m_ucAttribId);

					std::cout << szT1 << std::endl;

					wsprintf(szT1, "%s", pSmartDetails->m_csAttribName);
					std::cout << szT1 << std::endl;;

					wsprintf(szT1, "%d", pSmartInfo->m_ucValue);
					std::cout << szT1 << std::endl;

					wsprintf(szT1, "%d", pSmartInfo->m_ucWorst);
					std::cout << szT1 << std::endl;

					wsprintf(szT1, "%d", pSmartInfo->m_dwAttribValue);
					std::cout << szT1 << std::endl;

					wsprintf(szT1, "%d", pSmartInfo->m_dwThreshold);
					std::cout << szT1 << std::endl;

					ucT4++;
				}
			}
		}
	}
}