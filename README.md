# ISSP-TeraDrive
ACIT 3900/4900 ISSP TeraDrive Code repo

SmartHDD GUI
Created by: 

Our software program SmartHDD GUI was created using Visual Studio and c++. The program retrieves SMART values 197, 197, 198 for HDD only. If it cannot retrieve all 3, the entry will not be saved in the database.
TeraDrive requested this software tool in order to gain insight on customer HDDs before providing extended warranty. 

Program Setup

First the Setup.exe must be created using the NSI Installer script (bundles up dependencies). We anticipate users to download the Setup.exe from TeraDrive, and not worry about compiling the NSI script to make the Setup.exe
  - libcrypto-1_1.dll
  - libssl-1_1.dll
  - License.rtf
  - mysqlcppconn-9-vs14.dll
  - SmartHDDGUI.ini
Once the Setup.exe has been created, run the executable (accept to run as admin)

Installing the Program

Once you have launched the Setup.exe, follow the following steps to complete the installation of the program and dependencies (15.1 MB space required)
  - Click Next to continue into the installation guide
  - Agree to TeraDrive Terms and Conditions
  - Choose destination folder for program download (default = C:\Program Files (x86)\TeraDrive)
  - Click Install
  - Click Finish

Running the Program

Now that the SmartHDDGUI.exe and the required dependencies are downloaded, navigate to your destination folder (default = C:\Program Files (x86)\TeraDrive
  - Run the SmartHDDGUI.exe       *Note: SmartHDDGUI.ini must be in the same directory as SmartHDDGUI.exe when running the program*
  -  Submit your first name, last name, and email
  -  Click OK
Congratulations! You have just made an entry into the TeraDrive Smart Database
