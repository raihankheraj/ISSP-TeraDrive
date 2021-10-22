#include <iostream>
#include <windows.h>;
#include <Wbemidl.h>
#include <comdef.h>
#include "StorageDevice.h"
#include <vector>

#pragma comment(lib, "wbemuuid.lib")

using namespace::std;


void IntializeCOM()
{
    HRESULT hres;
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);

    if (FAILED(hres))
    {
        cout << "Failed to initialize COM library. Error code = 0x" << hex << hres << endl;
        // Program has failed.
    }

    // Step 2: --------------------------------------------------
    // Set general COM security levels --------------------------

    hres = CoInitializeSecurity(
        NULL,
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities 
        NULL                         // Reserved
    );

    if (FAILED(hres))
    {
        cout << "Failed to initialize security. Error code = 0x" << hex << hres << endl;
        CoUninitialize();             // Program has failed.
    }
}

void SetupWBEM(IWbemLocator*& pLoc, IWbemServices*& pSvc)
{
    // Step 3: ---------------------------------------------------
    // Obtain the initial locator to WMI -------------------------

    HRESULT hres;
    //IWbemLocator *pLoc = NULL;

    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&pLoc);

    if (FAILED(hres))
    {
        cout << "Failed to create IWbemLocator object." << " Err code = 0x" << hex << hres << endl;
        CoUninitialize();
    }

    // Step 4: -----------------------------------------------------
    // Connect to WMI through the IWbemLocator::ConnectServer method

    //IWbemServices *pSvc = NULL;

    // Connect to the ROOT\\\microsoft\\windows\\storage namespace with
    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\microsoft\\windows\\storage"), // Object path of WMI namespace
        NULL,                    // User name. NULL = current user
        NULL,                    // User password. NULL = current
        0,                       // Locale. NULL indicates current
        NULL,                    // Security flags.
        0,                       // Authority (for example, Kerberos)
        0,                       // Context object 
        &pSvc                    // pointer to IWbemServices proxy
    );

    if (FAILED(hres))
    {
        cout << "Could not connect. Error code = 0x" << hex << hres << endl;
        pLoc->Release();
        CoUninitialize();
    }


    // Step 5: --------------------------------------------------
    // Set security levels on the proxy -------------------------

    hres = CoSetProxyBlanket(
        pSvc,                        // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
        NULL,                        // Server principal name 
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // client identity
        EOAC_NONE                    // proxy capabilities 
    );

    if (FAILED(hres))
    {
       cout << "Could not set proxy blanket. Error code = 0x" << hex << hres << endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
    }

}


int main()
{
    IWbemLocator *wbemLocator = NULL;
    IWbemServices *wbemServices = NULL;

    IntializeCOM();
    SetupWBEM(wbemLocator, wbemServices);

    IEnumWbemClassObject* storageEnumerator = NULL;
    HRESULT hres = wbemServices->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM MSFT_PhysicalDisk"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &storageEnumerator);

    if (FAILED(hres))
    {
        cout << "Query for MSFT_PhysicalDisk. Error code = 0x" << hex << hres << endl;
        wbemServices->Release();
        wbemLocator->Release();
        CoUninitialize();
    }

    IWbemClassObject *storageWbemObject = NULL;
    ULONG uReturn = 0;

    vector<StorageDevice> storageDevices;

    while (storageEnumerator)
    {
        HRESULT hr = storageEnumerator->Next(WBEM_INFINITE, 1, &storageWbemObject, &uReturn);
        if (0 == uReturn || hr != S_OK)
        {
            break;
        }

        StorageDevice storageDevice;

        VARIANT deviceId;
        VARIANT busType;
        VARIANT healthStatus;
        VARIANT spindleSpeed;
        VARIANT mediaType;

        storageWbemObject->Get(L"DeviceId", 0, &deviceId, 0, 0);
        storageWbemObject->Get(L"BusType", 0, &busType, 0, 0);
        storageWbemObject->Get(L"HealthStatus", 0, &healthStatus, 0, 0);
        storageWbemObject->Get(L"SpindleSpeed", 0, &spindleSpeed, 0, 0);
        storageWbemObject->Get(L"MediaType", 0, &mediaType, 0, 0);

        storageDevice.DeviceId = deviceId.bstrVal == NULL ? "" : _bstr_t(deviceId.bstrVal);
        storageDevice.BusType = busType.uintVal;
        storageDevice.HealthStatus = healthStatus.uintVal;
        storageDevice.SpindleSpeed = spindleSpeed.uintVal;
        storageDevice.MediaType = mediaType.uintVal;

        storageDevices.push_back(storageDevice);
        storageWbemObject->Release();
    }


}