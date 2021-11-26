#include "DBHelper.h"
#include <iostream>

using namespace std;

// Use DBHelper class in DBHeader.h
DBHelper::DBHelper()
	: driver(nullptr),
	con(nullptr),
	stmt(nullptr),
	res(nullptr)
{

	// Amazon RDS Login Credentials
	const string server = "tcp://smart-rds.cjex0ab26usy.us-west-2.rds.amazonaws.com:3306";
	const string username = "admin";
	const string password = "Passw0rd!";

	try
	{
		// Create a connection 
		driver = get_driver_instance();

		// Pass login credentials to driver
		con = driver->connect(server, username, password);

		// Connect to the RDS - smart_data schema
		con->setSchema("smart_data");
	}
	catch (sql::SQLException& e)
		// Display error message to user if error connecting to database
	{
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}

}

DBHelper::~DBHelper()
	// Clean up results, statements, and connections in database 
{
	if (res != NULL) delete res;
	if (stmt != NULL) delete stmt;
	if (con != NULL) delete con;
}

void DBHelper::SelectAll()
	// Function for Selecting all entries in the database - used mainly for testing purposes
{
	stmt = con->createStatement();

	res = stmt->executeQuery("SELECT * from results");

	while (res->next())
	{
		// Access column data by numeric offset, 1 is the first column 
		std::cout << "Serial: " << res->getString("serial") << std::endl;

		// Access column data by alias or column name 
		std::cout << "Model: " << res->getString("model") << std::endl;

		// Access column data by alias or column name 
		std::cout << "val196: " << res->getInt("val196") << std::endl;

		// Access column data by numeric offset, 1 is the first column 
		std::cout << "worst196: " << res->getInt("worst196") << std::endl;

		// Access column data by alias or column name 
		std::cout << "val197: " << res->getInt("val197") << std::endl;

		// Access column data by numeric offset, 1 is the first column 
		std::cout << "worst197: " << res->getInt("worst197") << std::endl;

		// Access column data by alias or column name 
		std::cout << "val198: " << res->getInt("val198") << std::endl;

		// Access column data by numeric offset, 1 is the first column 
		std::cout << "worst198: " << res->getInt("worst198") << std::endl;

		// Access column data by alias or column name 
		std::cout << "email: " << res->getString("email") << std::endl;

		// Access column data by numeric offset, 1 is the first column 
		std::cout << "firstname: " << res->getString("firstname") << std::endl;

		// Access column data by alias or column name 
		std::cout << "lastname: " << res->getString("lastname") << std::endl;
	}
}

void DBHelper::Insert(const char* fname, const char* lname, const char* email, const char* serial, const char* model, int val196, int worst196, int val197, int worst197, int val198, int worst198)
	// Function for inserting data into database
	// fname, lname, email are passed into function as char pointers (from GUI)
{
	sql::PreparedStatement* pstmt; // Use prepared statement since inserting

	pstmt = con->prepareStatement("INSERT INTO results(serial, model, val196, worst196, val197, worst197, val198, worst198, email, firstname, lastname) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

	// Indiviually set columns in database with corresponding values (order matters!)

	pstmt->setString(1, serial);
	pstmt->setString(2, model);
	pstmt->setInt(3, val196);
	pstmt->setInt(4, worst196);
	pstmt->setInt(5, val197);
	pstmt->setInt(6, worst197);
	pstmt->setInt(7, val198);
	pstmt->setInt(8, worst198);
	pstmt->setString(9, email);
	pstmt->setString(10, fname);
	pstmt->setString(11, lname);

	// Execute update in database
	pstmt->executeUpdate();

	if (pstmt != NULL) delete pstmt;
}
