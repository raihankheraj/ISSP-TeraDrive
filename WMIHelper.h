#pragma once
#include <iostream>

using namespace::std;

class StorageDevice
{
public:
    StorageDevice();
    ~StorageDevice();

    string  DeviceId;
    int BusType;
    int HealthStatus;
    int SpindleSpeed;
    int MediaType;

};