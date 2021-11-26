#pragma once

#include <mysql.h>
#include <mysql/jdbc.h>

class DBHelper
{
public:
	DBHelper();
	~DBHelper();

	// Initialize functions to be used in DBHelper.cpp
	void SelectAll();
	void Insert(const char* fname, const char* lname, const char* email, const char* serial, const char* model, int val196, int worst196, int val197, int worst197, int val198, int worst198);
private:
	sql::Driver* driver;
	sql::Connection* con;
	sql::Statement* stmt;
	sql::ResultSet* res;
};
