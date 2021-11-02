#pragma once
#include <iostream>

using namespace::std;

class StorageDevice
{
public:
    StorageDevice();
    ~StorageDevice();

    bool IsSSD();

    string GetDeviceID() { return strDeviceId; }
    string GetFriendlyName() { return strFriendlyName; }
    string GetFirmwareVersion() { return strFirmwareVersion;  }
    string GetPartNumber() { return strPartNumber; }
    string GetSoftwareVersion() { return strSoftwareVersion; }

    void SetDeviceID(string);
    void SetFriendlyName(string);
    void SetFirmwareVersion(string);
    void SetPartNumber(string);
    void SetSoftwareVersion(string);

    void SetBusType(int);
    void SetHealthStatus(int);
    void SetSpindleSpeed(int);
    void SetMediaType(int);

private:
    int nBusType;
    int nHealthStatus;
    int nSpindleSpeed;
    int nMediaType;

    string strDeviceId;
    string strFirmwareVersion;
    string strFriendlyName;
    string strPartNumber;
    string strSoftwareVersion;
};

class DiskDrive
{
public:
    DiskDrive();
    ~DiskDrive();

    string GetDeviceID() { return strDeviceId; }
    string GetModel() { return strModel; }
    string GetSerialNumber() { return strSerialNumber; }

    void SetDeviceID(string);
    void SetModel(string);
    void SetSerialNumber(string);

private:
    string strDeviceId;
    string strModel;
    string strSerialNumber;

};
