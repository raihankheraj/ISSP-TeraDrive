#pragma once

// This class can be used to communicate with WMI on local machine.
// One can create instance of Wmi Instance Provider and access all methods of class

#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>
#include <wbemprov.h>

#include <atlbase.h>    // For CComBSTR, CComPtr

#include <string>
#include <vector>

#include "StorageDevice.h"

class WMIHelper
{
private:
	BOOL m_bInitialized;

	CComPtr<IWbemServices>    m_spWbemService;

	HRESULT InitializeWMI();

	CComBSTR m_bstrNamespace;

public:
	WMIHelper(string strNamespace);
	~WMIHelper();

	vector<StorageDevice> Get_MSFT_PhysicalDisk_PropertyValues();
	vector<DiskDrive> Get_Win32_DiskDrive_PropertyValues();
};
