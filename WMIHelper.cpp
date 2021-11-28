#include <comutil.h>

#include "WMIHelper.h"
#include "Common.h"
#include "CTracer.h"

extern CTracer g_objTracer;

// IMPORTANT : Object of this class is USE AND THROW!
// Use only for calling the specified method once and thats all.
using namespace std;

// WMIHelper constructor
WMIHelper::WMIHelper(string strNamespace)
	: m_spWbemService(NULL)
{
	m_bInitialized = FALSE;
	m_bstrNamespace = strNamespace.c_str();

	InitializeWMI();
}

// WMIHelper destructor
WMIHelper::~WMIHelper()
{
	//::CoUninitialize();
}

// Initialize WMI
HRESULT WMIHelper::InitializeWMI()
{
	DF_TRACE_LOG(L"WMIHelper::InitializeWMI() - ENTER");

	HRESULT hres = WBEM_E_FAILED;
	CComPtr<IWbemLocator>pLoc = NULL;
	string strErrorMessage = "";

	do
	{
		hres = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);

		if (RPC_E_CHANGED_MODE == hres)
		{
			DF_TRACE_LOG(L"WMIHelper::InitializeWMI() - CoInitializeEx() was already set earlier in the program.");
		}
		else if (FAILED(hres))
		{
			DF_ERROR_LOG(L"WMIHelper::InitializeWMI() - CoInitializeEx() FAILED with Error = ", hres);
			break;
		}

		// NOTE:
		// When using asynchronous WMI API's remotely in an environment where the "Local System" account
		// has no network identity (such as non-Kerberos domains), the authentication level of
		// RPC_C_AUTHN_LEVEL_NONE is needed. However, lowering the authentication level to
		// RPC_C_AUTHN_LEVEL_NONE makes your application less secure. It is wise to
		// use semi-synchronous API's for accessing WMI data and events instead of the asynchronous ones.

		// hr = CoInitializeSecurity(
		// NULL,
		// -1,
		// NULL,
		// NULL,
		// RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
		// RPC_C_IMP_LEVEL_IMPERSONATE,
		// NULL,
		// EOAC_SECURE_REFS, //change to EOAC_NONE if you change dwAuthnLevel to RPC_C_AUTHN_LEVEL_NONE
		// NULL );
		hres = CoInitializeSecurity(
			NULL,                        // Security descriptor
			-1,                          // COM negotiates authentication service
			NULL,                        // Authentication services
			NULL,                        // Reserved
			RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication level for proxies
			RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation level for proxies
			NULL,                        // Authentication info
			EOAC_NONE,                   // Additional capabilities of the client or server
			NULL                         // Reserved
		);

		if (RPC_E_TOO_LATE == hres)
		{
			DF_TRACE_LOG(L"WMIHelper::InitializeWMI() - CoInitializeSecurity() was already set earlier in the program.");
		}
		else if (FAILED(hres))
		{
			DF_ERROR_LOG(L"WMIHelper::InitializeWMI() - CoInitializeSecurity() FAILED with Error = ", hres);
			break;
		}

		// Get locator
		hres = ::CoCreateInstance(
			CLSID_WbemLocator,
			0,
			CLSCTX_INPROC_SERVER,
			IID_IWbemLocator,
			(LPVOID*)&pLoc
		);

		if (FAILED(hres))
		{
			DF_ERROR_LOG(L"WMIHelper::InitializeWMI() - CoCreateInstance() FAILED with Error = ", hres);
			break;
		}

		// Connect to WMI through the IWbemLocator::ConnectServer method
		// Connect to the local PROVIDER_NAMESPACE namespace
		// and obtain pointer m_spWbemService to make IWbemServices calls.

		//CComBSTR bstrProviderNameSpace = "ROOT\\microsoft\\windows\\storage";

		DF_TRACE_LOG(L"WMIHelper::InitializeWMI() - Provider Namespace=%s", m_bstrNamespace);

		hres = pLoc->ConnectServer(
			m_bstrNamespace,
			NULL,
			NULL,
			NULL,
			NULL,
			0,
			0,
			&m_spWbemService
		);

		if (FAILED(hres))
		{
			DF_ERROR_LOG(L"WMIHelper::InitializeWMI() - ConnectServer() FAILED with Error = ", hres);
			break;
		}

		// Set security levels for the proxy ------------------------
		hres = ::CoSetProxyBlanket(
			m_spWbemService,             // Indicates the proxy to set
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
			DF_ERROR_LOG(L"WMIHelper::InitializeWMI() - CoSetProxyBlanket() FAILED with Error = ", hres);
			break;
		}

		m_bInitialized = TRUE;

		DF_TRACE_LOG(L"WMIHelper::InitializeWMI() - Completed SUCCESSFULLY");

	} while (0);

	DF_TRACE_LOG(L"WMIHelper::InitializeWMI() - EXIT");

	return hres;
}

// Use WMI to retrive all phyical disks on machine
vector<StorageDevice> WMIHelper::Get_MSFT_PhysicalDisk_PropertyValues()
{
	DF_TRACE_LOG(L"WMIHelper::Get_MSFT_PhysicalDisk_PropertyValues() - ENTER");

	HRESULT  hr = WBEM_E_FAILED;
	vector<StorageDevice> storageDevices;
	ULONG uReturn = 0;
	CComPtr<IEnumWbemClassObject> sp_WbemEnumerator;

	do
	{
		hr = m_spWbemService->ExecQuery(
			bstr_t("WQL"),
			bstr_t("SELECT * FROM MSFT_PhysicalDisk"), // Query to select physical devices
			WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
			NULL,
			&sp_WbemEnumerator
		);

		if (FAILED(hr))
		{
			DF_ERROR_LOG(L"WMIHelper::Get_MSFT_PhysicalDisk_PropertyValues() - ExecQuery() API call FAILED with Error = ", hr);
			break;
		}

		while (sp_WbemEnumerator)
		{
			CComPtr<IWbemClassObject> spStorageWbemObject = NULL;

			HRESULT hr = sp_WbemEnumerator->Next(WBEM_INFINITE, 1, &spStorageWbemObject, &uReturn);

			if (0 == uReturn || hr != S_OK)
			{
				break;
			}

			StorageDevice storageDevice;

			VARIANT deviceId;         // An address or other identifier that uniquely names the physical disk
			VARIANT busType;          // The storage bus type of the physical disk
			VARIANT healthStatus;     // A high-level indication of device health
			VARIANT spindleSpeed;     // The rotational speed of spindle-based physical disks
			VARIANT mediaType;        // The media type of the physical disk

			VARIANT friendlyName;     // A user-friendly display name for the physical disk
			VARIANT partNumber;       // A string representation of the physical disk's part number or SKU
			VARIANT firmwareVersion;  // A string representation of the firmware revision
			VARIANT softwareVersion;  // A string representation of the software version number

			VariantInit(&deviceId);
			VariantInit(&busType);
			VariantInit(&healthStatus);
			VariantInit(&spindleSpeed);
			VariantInit(&mediaType);

			VariantInit(&friendlyName);
			VariantInit(&partNumber);
			VariantInit(&firmwareVersion);
			VariantInit(&softwareVersion);

			spStorageWbemObject->Get(L"DeviceId", 0, &deviceId, 0, 0);
			spStorageWbemObject->Get(L"BusType", 0, &busType, 0, 0);
			spStorageWbemObject->Get(L"HealthStatus", 0, &healthStatus, 0, 0);
			spStorageWbemObject->Get(L"SpindleSpeed", 0, &spindleSpeed, 0, 0);
			spStorageWbemObject->Get(L"MediaType", 0, &mediaType, 0, 0);

			spStorageWbemObject->Get(L"FriendlyName", 0, &friendlyName, 0, 0);
			spStorageWbemObject->Get(L"PartNumber", 0, &partNumber, 0, 0);
			spStorageWbemObject->Get(L"FirmwareVersion", 0, &firmwareVersion, 0, 0);
			spStorageWbemObject->Get(L"SoftwareVersion", 0, &softwareVersion, 0, 0);

			string s = (char*) (deviceId.bstrVal == NULL ? "" : _bstr_t(deviceId.bstrVal));
			storageDevice.SetDeviceID(s);

			s = (char*)(friendlyName.bstrVal == NULL ? "" : _bstr_t(friendlyName.bstrVal));
			storageDevice.SetFriendlyName(s);

			s = (char*)(partNumber.bstrVal == NULL ? "" : _bstr_t(partNumber.bstrVal));
			storageDevice.SetPartNumber(s);

			s = (char*)(firmwareVersion.bstrVal == NULL ? "" : _bstr_t(firmwareVersion.bstrVal));
			storageDevice.SetFirmwareVersion(s);

			s = (char*)(softwareVersion.bstrVal == NULL ? "" : _bstr_t(softwareVersion.bstrVal));
			storageDevice.SetSoftwareVersion(s);

			storageDevice.SetBusType (busType.uintVal);
			storageDevice.SetHealthStatus (healthStatus.uintVal);
			storageDevice.SetSpindleSpeed (spindleSpeed.uintVal);
			storageDevice.SetMediaType (mediaType.uintVal);

			storageDevices.push_back(storageDevice);
		}

	} while (0);

	return storageDevices;
}

vector<DiskDrive> WMIHelper::Get_Win32_DiskDrive_PropertyValues()
{
	DF_TRACE_LOG(L"WMIHelper::Get_Win32_DiskDrive_PropertyValues() - ENTER");

	HRESULT  hr = WBEM_E_FAILED;
	vector<DiskDrive> drives;
	ULONG uReturn = 0;
	CComPtr<IEnumWbemClassObject> sp_WbemEnumerator;

	do
	{
		hr = m_spWbemService->ExecQuery(
			bstr_t("WQL"),
			bstr_t("SELECT * FROM Win32_DiskDrive"), // Query for physical disk drives
			WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
			NULL,
			&sp_WbemEnumerator
		);

		if (FAILED(hr))
		{
			DF_ERROR_LOG(L"WMIHelper::Get_Win32_DiskDrive_PropertyValues() - ExecQuery() API call FAILED with Error = ", hr);
			break;
		}

		while (sp_WbemEnumerator)
		{
			CComPtr<IWbemClassObject> spWbemObject = NULL;

			HRESULT hr = sp_WbemEnumerator->Next(WBEM_INFINITE, 1, &spWbemObject, &uReturn);

			if (0 == uReturn || hr != S_OK)
			{
				break;
			}

			DiskDrive drive;

			VARIANT deviceId;         // An address or other identifier that uniquely names the physical disk
			VARIANT model;     // A user-friendly display name for the physical disk
			VARIANT serialNumber;       // A string representation of the physical disk's part number or SKU

			VariantInit(&deviceId);
			VariantInit(&model);
			VariantInit(&serialNumber);

			spWbemObject->Get(L"DeviceId", 0, &deviceId, 0, 0);
			spWbemObject->Get(L"Model", 0, &model, 0, 0);
			spWbemObject->Get(L"SerialNumber", 0, &serialNumber, 0, 0);

			string s = (char*)(deviceId.bstrVal == NULL ? "" : _bstr_t(deviceId.bstrVal));
			drive.SetDeviceID(s);

			s = (char*) (model.bstrVal == NULL ? "" : _bstr_t(model.bstrVal));
			drive.SetModel (s);

			s = (char*) (serialNumber.bstrVal == NULL ? "" : _bstr_t(serialNumber.bstrVal));
			drive.SetSerialNumber (s);

			drives.push_back(drive);
		}

	} while (0);

	return drives;
}

