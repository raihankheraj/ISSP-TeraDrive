
// SmartHDDGUIDlg.cpp : implementation file
//

//#include "pch.h"
#include "framework.h"
#include "SmartHDDGUI.h"
#include "SmartHDDGUIDlg.h"
#include "afxdialogex.h"

#include <iostream>
#include <windows.h>
#include <Wbemidl.h>
#include <comdef.h>
#include <vector>

#include "WMIHelper.h"
#include "SmartReader.h"
#include "StorageDevice.h"
#include "DBHelper.h"
#include "Common.h"
#include "CTracer.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern CTracer g_objTracer;

using namespace::std;

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}


// Setup passing of firstname, lastname, and email
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	// Call OnBnClickedOk when button clicked
	ON_BN_CLICKED(IDOK, &CAboutDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CSmartHDDGUIDlg dialog



CSmartHDDGUIDlg::CSmartHDDGUIDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SMARTHDDGUI_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSmartHDDGUIDlg::DoDataExchange(CDataExchange* pDX)
{
	// Do data exchange with edit boxes: firstname, lastname, email
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_FIRSTNAME, edtBoxFirstName);
	DDX_Control(pDX, IDC_EDIT_LASTNAME, edtBoxLastName);
	DDX_Control(pDX, IDC_EDIT_EMAIL, edtBoxEmail);
}

BEGIN_MESSAGE_MAP(CSmartHDDGUIDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CSmartHDDGUIDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CSmartHDDGUIDlg message handlers

BOOL CSmartHDDGUIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSmartHDDGUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSmartHDDGUIDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSmartHDDGUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CAboutDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}


void CSmartHDDGUIDlg::OnBnClickedBtnBack()
{
	// TODO: Add your control notification handler code here
}


void CSmartHDDGUIDlg::OnBnClickedOk()
	// Function called when OK button clicked on GUI
{
	DF_TRACE_LOG(L"::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");
	DF_TRACE_LOG(L"ENTER");

	if (edtBoxFirstName.GetWindowTextLength() == 0)
		// Check user entry for first name not empty
	{
		MessageBox("First name cannot be empty. Please enter the First name.", "TeraDrive SMART HDD Data Collection", MB_ICONERROR | MB_OK);
		return;
	}
	else
		// Save user entry for first name so it can be sent to the database
	{
		edtBoxFirstName.GetWindowText(csFirstName);
		DF_TRACE_LOG(L"From Control First Name=%S", csFirstName.GetString());
	}

	if (edtBoxLastName.GetWindowTextLength() == 0)
		// Check user entry for last name not empty
	{
		MessageBox("Last name cannot be empty. Please enter the Last name.", "TeraDrive SMART HDD Data Collection", MB_ICONERROR | MB_OK);
		return;
	}
	else
		// Save user entry for last name so it can be sent to the database
	{
		edtBoxLastName.GetWindowText(csLastName);
		DF_TRACE_LOG(L"From Control Last Name=%S", csLastName.GetString());
	}

	if (edtBoxEmail.GetWindowTextLength() == 0)
		// Check user entry for email not empty
	{
		MessageBox("Email cannot be empty. Please enter the Email in correct format e.g. support@email.com", "TeraDrive SMART HDD Data Collection", MB_ICONERROR | MB_OK);
		return;
	}
	else
		// Save user entry for email so it can be sent to the database
	{
		edtBoxEmail.GetWindowText(csEmail);
		DF_TRACE_LOG(L"From Control Email=%S", csEmail.GetString());
	}


	// Create Smart Reader object
	CSmartReader objSmartReader;

	BYTE ucT1 = 0;
	BYTE ucT2 = 0;
	BYTE ucT3 = 0;

	// Create variables to store Smart info and initialize to null
	ST_DRIVE_INFO* pDriveInfo = NULL;
	ST_SMART_INFO* pSmartInfo = NULL;
	ST_SMART_DETAILS* pSmartDetails = NULL;
	ST_IDSECTOR* pSectorInfo = NULL;

	char szT1[MAX_PATH] = { 0 };


	// Use WMI Helper to get storage devices
	vector<StorageDevice> storageDevices;
	WMIHelper wmi("ROOT\\microsoft\\windows\\storage");
	storageDevices = wmi.Get_MSFT_PhysicalDisk_PropertyValues();

	// Use WMI Helper to get drives 
	vector<DiskDrive> drives;
	WMIHelper wmi2("root\\CIMV2");
	drives = wmi2.Get_Win32_DiskDrive_PropertyValues();

	DF_TRACE_LOG(L"Call ReadSMARTValuesForAllDrives()");

	// Reading Self-Monitoring Analysis and Reporting Technology (SMART) Values

	objSmartReader.ReadSMARTValuesForAllDrives();

	DF_TRACE_LOG(L"m_ucDrivesWithInfo = %d", objSmartReader.m_ucDrivesWithInfo);

	// Iterate through drives found by Smart Object Reader (Could be SSD or HDD)
	for (ucT1 = 0; ucT1 < objSmartReader.m_ucDrivesWithInfo; ++ucT1)
	{
		char sz[3] = { 0 };
		wsprintf(sz, "%d", ucT1);
		string strDeviceId = sz;
		
		// Set SSD flag to false
		bool blnSSD = false;

		// Iterage through storage devices found
		for (StorageDevice sd : storageDevices)
		{
			// If drive is SSD change flag
			if ((sd.GetDeviceID() == strDeviceId) && sd.IsSSD())
			{
				blnSSD = true;
				break;
			}
		}

		// SSD drive was detected
		if (blnSSD)
		{			
			continue;
		}

		// Collect Smart Sector info from HDDs (Not SSD)
		pSectorInfo = &objSmartReader.m_stDrivesInfo[ucT1].m_stInfo;

		DF_TRACE_LOG(L"");

		DF_TRACE_LOG(L">>>>>>>>>>>>>>>  Drive %d  <<<<<<<<<<<<<<<<", ucT1);

		//std::cout << "Model:  " << pSectorInfo->sModelNumber << std::endl;
		//std::cout << "Serial: " << pSectorInfo->sSerialNumber << std::endl;
		//std::cout << "Cache = " << pSectorInfo->wBufferSize << std::endl;
		//std::cout << "Buffer = " << pSectorInfo->wBufferType << std::endl;

		DF_TRACE_LOG(L"Model:  %S", pSectorInfo->sModelNumber);
		DF_TRACE_LOG(L"Serial: %S", pSectorInfo->sSerialNumber);

		// Get Drive information for each relevant drive
		pDriveInfo = objSmartReader.GetDriveInfo(ucT1);

		// Initialize Smart values
		int val196 = 0;
		int worst196 = 0;
		int val197 = 0;
		int worst197 = 0;
		int val198 = 0;
		int worst198 = 0;

		bool bln196 = true;
		bool bln197 = true;
		bool bln198 = true;

		// Only iterate through Smart values 196, 197, 198 
		for (ucT2 = 196; ucT2 < 199; ++ucT2)
		{
			pSmartInfo = objSmartReader.GetSMARTValue(pDriveInfo->m_ucDriveIndex, ucT2);

			// Check if drive has Smart info
			if (pSmartInfo)
			{
				pSmartDetails = objSmartReader.GetSMARTDetails(pSmartInfo->m_ucAttribIndex);

				// Check if drive has Smart details
				if (pSmartDetails)
				{
					if (pSmartDetails->m_bCritical)
						wsprintf(szT1, "(CRITICAL) ATTRIBUTE Name=%s Id=%d Value=%d Worst=%d AttributeValue=%lu Threshold=%lu", pSmartDetails->m_csAttribName.GetString(), pSmartDetails->m_ucAttribId, pSmartInfo->m_ucValue, pSmartInfo->m_ucWorst, pSmartInfo->m_dwAttribValue, pSmartInfo->m_dwThreshold);
					else
						wsprintf(szT1, "ATTRIBUTE Name=%s Id=%d Value=%d Worst=%d AttributeValue=%lu Threshold=%lu", pSmartDetails->m_csAttribName.GetString(), pSmartDetails->m_ucAttribId, pSmartInfo->m_ucValue, pSmartInfo->m_ucWorst, pSmartInfo->m_dwAttribValue, pSmartInfo->m_dwThreshold);

					DF_TRACE_LOG(L"%S", szT1);

					// If found Smart 196
					if (ucT2 == 196)
					{
						val196 = pSmartInfo->m_ucValue;
						worst196 = pSmartInfo->m_ucWorst;
					}
					// If found Smart 197
					else if (ucT2 == 197)
					{
						val197 = pSmartInfo->m_ucValue;
						worst197 = pSmartInfo->m_ucWorst;
					}
					// If found Smart 198
					else if (ucT2 == 198)
					{
						val198 = pSmartInfo->m_ucValue;
						worst198 = pSmartInfo->m_ucWorst;
					}
				}
			}
			// Could not find 196 AND 197 AND 198
			else
			{
				if (ucT2 == 196)
				{
					bln196 = false;
				}
				else if (ucT2 == 197)
				{
					bln197 = false;
				}
				else if (ucT2 == 198)
				{
					bln198 = false;
				}
			}
		}

		// Failed to retrieve all 3 Smart Values: 196, 197, 198
		if (!bln196 && !bln197 && !bln198)
		{
			DF_TRACE_LOG(L"S.M.A.R.T attributes (IDs 196, 197 and 198) could NOT be retrieved from the Physical Drive.");

			MessageBox("Failed to capture S.M.A.R.T attribute values for 196 AND 197 AND 198.", "TeraDrive SMART HDD Data Collection", MB_ICONINFORMATION | MB_OK);
		}
		else
		{
			// Succesfully retrieved Smart Values: 196, 197, 198
			try
			{
				DBHelper db;
				
				// Initialize variables for first name, last name, email
				char szFirstName[MAX_PATH] = { 0 };
				char szLastName[MAX_PATH] = { 0 };
				char szEmail[MAX_PATH] = { 0 };

				DF_TRACE_LOG(L"Reached after sz var stuff");

				strcpy_s(szFirstName, csFirstName.GetLength()+1, csFirstName);
				DF_TRACE_LOG(L"DB First name = %S", szFirstName);

				strcpy_s(szLastName, csLastName.GetLength()+1, csLastName);
				DF_TRACE_LOG(L"DB Last name = %S", szLastName);

				strcpy_s(szEmail, csEmail.GetLength()+1, csEmail);
				DF_TRACE_LOG(L"DB Email = %S", szEmail);

				DF_TRACE_LOG(L"First name = %S", szFirstName);
				DF_TRACE_LOG(L"Last name = %S", szLastName);
				DF_TRACE_LOG(L"Email = %S", szEmail);
				DF_TRACE_LOG(L"Serial = %S", pSectorInfo->sSerialNumber);
				DF_TRACE_LOG(L"Model = %S", pSectorInfo->sModelNumber);
				DF_TRACE_LOG(L"Val 196 = %d", val196);
				DF_TRACE_LOG(L"Worst 196 = %d", worst196);
				DF_TRACE_LOG(L"Val 197 = %d", val197);
				DF_TRACE_LOG(L"Worst 197 = %d", worst197);
				DF_TRACE_LOG(L"Val 198 = %d", val198);
				DF_TRACE_LOG(L"Worst 198 = %d", worst198);

				//Insert(char* fname, char* lname, char* email, char* serial, char* model, int val196, int worst196, int val197, int worst197, int val198, int worst198);
				db.Insert(szFirstName,
					szLastName,
					szEmail,
					(char*)pSectorInfo->sSerialNumber,
					(char*)pSectorInfo->sModelNumber,
					val196,
					worst196,
					val197,
					worst197,
					val198,
					worst198);

				MessageBox("Successfully captured and saved S.M.A.R.T data.", "TeraDrive SMART HDD Data Collection", MB_ICONINFORMATION | MB_OK);

			}
			catch (sql::SQLException& e)
			{
				DF_TRACE_LOG(L"ERR: SQLException in %S", __FILE__);
				DF_TRACE_LOG(L"%S on line %S", __FUNCTION__, __LINE__);
			}
		}
	}

	for (StorageDevice sd : storageDevices)
	{
		string s = sd.GetDeviceID();

		// If device is SSD enter this block
		if (sd.IsSSD())
		{
			DF_TRACE_LOG(L"");

			wsprintf(szT1, ">>>>>>>>>>>>>>>  Drive %s  <<<<<<<<<<<<<<<<", s.c_str());
			DF_TRACE_LOG(szT1);


			DF_TRACE_LOG(L"Model:  %S", sd.GetFriendlyName().c_str());

			for (DiskDrive drive : drives)
			{
				if (drive.GetModel() == sd.GetFriendlyName())
				{
					DF_TRACE_LOG(L"Serial: %S", drive.GetSerialNumber().c_str());
				}
			}

			DF_TRACE_LOG(L"S.M.A.R.T attributes (IDs 196, 197 and 198) could NOT be retrieved for SSD Drive.");

			MessageBox("Failed to capture S.M.A.R.T attribute values for SSD (NVME) Drive(s).", "TeraDrive SMART HDD Data Collection", MB_ICONINFORMATION | MB_OK);
		}
	}

	DF_TRACE_LOG(L"EXIT");
	DF_TRACE_LOG(L"::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::");

	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}
