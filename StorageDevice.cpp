#include "StorageDevice.h"

StorageDevice::StorageDevice()
	: nBusType(0), nHealthStatus(0), nSpindleSpeed(0), nMediaType(0)
{
}


StorageDevice::~StorageDevice()
{
}

void StorageDevice::SetDeviceID(string s)
{
	strDeviceId = s;
}

void StorageDevice::SetFriendlyName(string s)
{
	strFriendlyName = s;
}

void StorageDevice::SetFirmwareVersion(string s)
{
	strFirmwareVersion = s;
}

void StorageDevice::SetPartNumber(string s)
{
	strPartNumber = s;
}

void StorageDevice::SetSoftwareVersion(string s)
{
	strSoftwareVersion = s;
}

void StorageDevice::SetBusType(int i)
{
	nBusType = i;
}
void StorageDevice::SetHealthStatus(int i)
{
	nHealthStatus = i;
}
void StorageDevice::SetSpindleSpeed(int i)
{
	nSpindleSpeed = i;
}
void StorageDevice::SetMediaType(int i)
{
	nMediaType = i;
}


bool StorageDevice::IsSSD()
{
	bool blnSSD = false;

	if ((nBusType == 17) && (nMediaType == 4) && (nSpindleSpeed == 0))
	{
		blnSSD = true;
	}

	return blnSSD;
}

DiskDrive::DiskDrive()
{
}


DiskDrive::~DiskDrive()
{
}

void DiskDrive::SetDeviceID(string s)
{
	strDeviceId = s;
}
void DiskDrive::SetModel(string s)
{
	strModel = s;
}
void DiskDrive::SetSerialNumber(string s)
{
	strSerialNumber = s;
}
