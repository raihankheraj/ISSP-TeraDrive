#include "StorageDevice.h"

// Storage Device construtor
StorageDevice::StorageDevice()
	: nBusType(0), nHealthStatus(0), nSpindleSpeed(0), nMediaType(0)
{
}

// Storage Device destructor
StorageDevice::~StorageDevice()
{
}

// Setters for Storage Device attributes
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

// Flag for if device is SSD
bool StorageDevice::IsSSD()
{
	bool blnSSD = false;

	// Check BusType, MediaType, SpindleSpeed
	if ((nBusType == 17) && (nMediaType == 4) && (nSpindleSpeed == 0))
	{
		blnSSD = true;
	}

	return blnSSD;
}

// Disk Drive constructor
DiskDrive::DiskDrive()
{
}

// Disk Drive destructor
DiskDrive::~DiskDrive()
{
}


// Disk Drive setters
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
