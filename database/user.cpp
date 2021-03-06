#include "database/sqlite3.h"
#include "database/user.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sstream>
#include "guid.h"
#include "database/database.h"
#include "database/activity.h"

using namespace std;

/**
  * Built as part of the Boo QR Logger Project, a class project from the Spring 2017 Software Development I at Stetson University.
  * User model class that creates a row in the users table and loads values into memory in an instance of this class.
  *
  * @author Hayden Estey
  */

User::User(size_t _userid, string _uuid, string _username, string _fname, string _lname, size_t _eventid) {
    this->userid = _userid;
    this->uuid = _uuid;
    this->username = _username;
    this->fname = _fname;
    this->lname = _lname;
    this->eventid = _eventid;
}

User* User::createUser(string username, string fname, string lname, size_t eventid) {
    sqlite3* db = Database::openDatabase();
    int retval;
    sqlite3_stmt* s;

    GuidGenerator generator;
    Guid g = generator.newGuid();
    stringstream guidss;
    guidss << g;
    string uuid = guidss.str();

    const char* sql = "SELECT eventid FROM events WHERE eventid = ?";
    retval = sqlite3_prepare(db, sql, strlen(sql), &s, NULL);
    if (retval != SQLITE_OK) {
        cout << "Error in preparing select statement for events: error code " << sqlite3_errcode(db) << endl;
        return NULL;
    }
    retval = sqlite3_bind_int(s, 1, eventid);
    if (retval != SQLITE_OK) {
        cout << "Error binding int to SQL statement " << sql << endl;
        return NULL;
    }
    if (sqlite3_step(s) == SQLITE_DONE) {
        cout << "Error executing SQL statement " << sql << " with error code " << sqlite3_errcode(db) << endl;
        cout << "Check to make sure that the event exists in the database." << endl;
        return NULL;
    }
    if (eventid != (size_t)sqlite3_column_int(s, 0)) {
        cout << "Something strange happened... " << endl;
        return NULL;
    }
    //sqlite3_reset(s);

    sql = "INSERT INTO users (uuid, username, fname, lname, eventid) values (?, ?, ?, ?, ?)";
    retval = sqlite3_prepare(db, sql, strlen(sql), &s, NULL);
    if (retval != SQLITE_OK) {
        cout << "Error in preparing insert statement for users: error code " << sqlite3_errcode(db) << endl;
        return NULL;
    }
    retval = sqlite3_bind_text(s, 1, uuid.c_str(), uuid.size(), SQLITE_STATIC);
    if (retval != SQLITE_OK) {
        cout << "Error binding uuid text to SQL statement " << sql << endl;
        return NULL;
    }
    retval = sqlite3_bind_text(s, 2, username.c_str(), username.size(), SQLITE_STATIC);
    if (retval != SQLITE_OK) {
        cout << "Error binding username text to SQL statement " << sql << endl;
        return NULL;
    }
    retval = sqlite3_bind_text(s, 3, fname.c_str(), fname.size(), SQLITE_STATIC);
    if (retval != SQLITE_OK) {
        cout << "Error binding fname text to SQL statement " << sql << endl;
        return NULL;
    }
    retval = sqlite3_bind_text(s, 4, lname.c_str(), lname.size(), SQLITE_STATIC);
    if (retval != SQLITE_OK) {
        cout << "Error binding lname text to SQL statement " << sql << endl;
        return NULL;
    }
    retval = sqlite3_bind_int(s, 5, eventid);
    if (retval != SQLITE_OK) {
        cout << "Error binding int to SQL statement " << sql << endl;
        return NULL;
    }
    if (sqlite3_step(s) != SQLITE_DONE) {
        cout << "Error executing sql statement " << sql << ": error code " << sqlite3_errcode(db) <<endl;
        return NULL;
    }
    //sqlite3_reset(s);
    
    size_t id = 0;    
    sql = "SELECT userid FROM users WHERE uuid = ?";
    retval = sqlite3_prepare(db, sql, strlen(sql), &s, NULL);
    if (retval != SQLITE_OK) {
        cout << "Error in preparing SQL statement " << sql << ", error code: " << sqlite3_errcode(db) << endl;
        return NULL;
    }
    retval = sqlite3_bind_text(s, 1, uuid.c_str(), uuid.size(), SQLITE_STATIC);
    if (retval != SQLITE_OK) {
        cout << "Error binding uuid text to SQL statement " << sql << endl;
        return NULL;
    }
    if (sqlite3_step(s) == SQLITE_ROW){
        id = (size_t)sqlite3_column_int(s, 0);
    }
    sqlite3_reset(s);

    User* u = new User(id, uuid, username, fname, lname, eventid);
    return u;
}

User* User::loadUserById(size_t id) {
    sqlite3* db = Database::openDatabase();
    sqlite3_stmt* s;
    int retval;

    string uuid, username, fname, lname;
    size_t eventid = 0;
    
    const char* sql = "SELECT * FROM users WHERE userid = ?";
    retval = sqlite3_prepare(db, sql, strlen(sql), &s, NULL);
    if (retval != SQLITE_OK) {
        cout << "Error preparing SQL statement " << sql << ", error code: " << sqlite3_errcode(db) << endl;
        return NULL;
    }
    retval = sqlite3_bind_int(s, 1, id);
    if (retval != SQLITE_OK) {
        cout << "Error binding text to SQL statement " << sql << endl;
        return NULL;
    }
    if (sqlite3_step(s) == SQLITE_ROW) {
        uuid = string(reinterpret_cast<const char*>(sqlite3_column_text(s, 1)));
        username = string(reinterpret_cast<const char*>(sqlite3_column_text(s, 2)));
        fname = string(reinterpret_cast<const char*>(sqlite3_column_text(s, 3)));
        lname = string(reinterpret_cast<const char*>(sqlite3_column_text(s, 4)));
        eventid = sqlite3_column_int(s, 4);
    }

    User* u = new User(id, uuid, username, fname, lname, eventid);
    return u;
}

void User::setUsername(string _username) {
    username = _username;

    sqlite3* db = Database::openDatabase();
    sqlite3_stmt* s;
    int retval;

    const char* sql = "UPDATE users SET username = ? WHERE uuid = ?";
    retval = sqlite3_prepare(db, sql, strlen(sql), &s, NULL);
    if (retval != SQLITE_OK) {
        cout << "Error preparing SQL statement " << sql << ", error code: " << sqlite3_errcode(db) << endl;
        return;
    }
    retval = sqlite3_bind_text(s, 1, _username.c_str(), _username.size(), SQLITE_STATIC);
    if (retval != SQLITE_OK) {
        cout << "Error binding username text to SQL statement " << sql << endl;
        return;
    }
    retval = sqlite3_bind_text(s, 2, uuid.c_str(), uuid.size(), SQLITE_STATIC);
    if (retval != SQLITE_OK) {
        cout << "Error binding uuid text to SQL statement " << sql << endl;
        return;
    }
    if (sqlite3_step(s) != SQLITE_DONE) {
        cout << "Error executing SQL statement " << sql << ", error code: " << sqlite3_errcode(db) << endl;
        return;
    }
}

void User::setUserFname(string _fname) {
    fname = _fname;

    sqlite3* db = Database::openDatabase();
    sqlite3_stmt* s;
    int retval;

    const char* sql = "UPDATE users SET fname = ? WHERE uuid = ?";
    retval = sqlite3_prepare(db, sql, strlen(sql), &s, NULL);
    if (retval != SQLITE_OK) {
        cout << "Error preparing SQL statement " << sql << ", error code: " << sqlite3_errcode(db) << endl;
        return;
    }
    retval = sqlite3_bind_text(s, 1, _fname.c_str(), _fname.size(), SQLITE_STATIC);
    if (retval != SQLITE_OK) {
        cout << "Error binding fname text to SQL statement " << sql << endl;
        return;
    }
    retval = sqlite3_bind_text(s, 2, uuid.c_str(), uuid.size(), SQLITE_STATIC);
    if (retval != SQLITE_OK) {
        cout << "Error binding uuid text to SQL statement " << sql << endl;
        return;
    }
    if (sqlite3_step(s) != SQLITE_DONE) {
        cout << "Error executing SQL statement " << sql << ", error code: " << sqlite3_errcode(db) << endl;
        return;
    }
}

void User::setUserLname(string _lname) {
    lname = _lname;

    sqlite3* db = Database::openDatabase();
    sqlite3_stmt* s;
    int retval;

    const char* sql = "UPDATE users SET lname = ? WHERE uuid = ?";
    retval = sqlite3_prepare(db, sql, strlen(sql), &s, NULL);
    if (retval != SQLITE_OK) {
        cout << "Error preparing SQL statement " << sql << ", error code: " << sqlite3_errcode(db) << endl;
        return;
    }
    retval = sqlite3_bind_text(s, 1, _lname.c_str(), _lname.size(), SQLITE_STATIC);
    if (retval != SQLITE_OK) {
        cout << "Error binding lname text to SQL statement " << sql << endl;
        return;
    }
    retval = sqlite3_bind_text(s, 2, uuid.c_str(), uuid.size(), SQLITE_STATIC);
    if (retval != SQLITE_OK) {
        cout << "Error binding uuid text to SQL statement " << sql << endl;
        return;
    }
    if (sqlite3_step(s) != SQLITE_DONE) {
        cout << "Error executing SQL statement " << sql << ", error code: " << sqlite3_errcode(db) << endl;
        return;
    }
}

vector<User*> User::searchByLastName(string _name) {
    sqlite3* db = Database::openDatabase();
    int retval;
    sqlite3_stmt *s;
    size_t id = 0;
    size_t _eventid = 0;
    string _username, _fname, _lname;
    vector<User*> results;
    string _uuid;


    const char *sql = "SELECT * FROM users WHERE lname LIKE '%' || ? || '%'";
    retval = sqlite3_prepare(db, sql, strlen(sql), &s, NULL);
    if (retval != SQLITE_OK) {
        cout << "Error preparing select statement for users " << sqlite3_errcode(db) << endl;
        return results;
    }

    retval = sqlite3_bind_text(s, 1, _name.c_str(), _name.size(), SQLITE_STATIC);
    if (retval != SQLITE_OK) {
        cout << "Error binding text to SQL statement " << sql << endl;
        return results;
    }

    while(sqlite3_step(s) == SQLITE_ROW) {
        id = (size_t)sqlite3_column_int(s, 0);
        _uuid = string(reinterpret_cast<const char*>(sqlite3_column_text(s, 1)));
        _username =  string(reinterpret_cast<const char*>(sqlite3_column_text(s, 2)));
        _fname = string(reinterpret_cast<const char*>(sqlite3_column_text(s, 3)));
        _lname = string(reinterpret_cast<const char*>(sqlite3_column_text(s, 4)));
        _eventid = sqlite3_column_int(s, 5);

    User* a = new User(id, _uuid, _username, _fname, _lname, _eventid);
    results.push_back(a);
    }
    return results;

}

vector<User*> User::getAllUsers() {
    sqlite3* db = Database::openDatabase();
    int retval;
    sqlite3_stmt *s;
    size_t id = 0;
    size_t _eventid = 0;
    string _username, _fname, _lname;
    vector<User*> results;
    string _uuid;


    const char *sql = "SELECT * FROM users";
    retval = sqlite3_prepare(db, sql, strlen(sql), &s, NULL);
    if (retval != SQLITE_OK) {
        cout << "Error preparing select statement for users " << sqlite3_errcode(db) << endl;
        return results;
    }


    while(sqlite3_step(s) == SQLITE_ROW) {
        id = (size_t)sqlite3_column_int(s, 0);
        _uuid = string(reinterpret_cast<const char*>(sqlite3_column_text(s, 1)));
        _username =  string(reinterpret_cast<const char*>(sqlite3_column_text(s, 2)));
        _fname = string(reinterpret_cast<const char*>(sqlite3_column_text(s, 3)));
        _lname = string(reinterpret_cast<const char*>(sqlite3_column_text(s, 4)));
        _eventid = sqlite3_column_int(s, 5);


    User* a = new User(id, _uuid, _username, _fname, _lname, _eventid);
    results.push_back(a);
    }
    return results;
}
User* User::getUserWithUUID(std::string guid) {

    sqlite3* db = Database::openDatabase();
    int retval;
    sqlite3_stmt *s;
    size_t id = 0;
    size_t _eventid = 0;
    string _username, _fname, _lname;
    string _uuid;


    const char *sql = "SELECT * FROM users WHERE uuid = ?";
    retval = sqlite3_prepare(db, sql, strlen(sql), &s, NULL);
    if (retval != SQLITE_OK) {
        cout << "Error preparing select statement for users " << sqlite3_errcode(db) << endl;
    }

    retval = sqlite3_bind_text(s, 1, guid.c_str(), guid.size(), SQLITE_STATIC);
    if (retval != SQLITE_OK) {
        cout << "Error binding text to SQL statement " << sql << endl;
    }

    if(sqlite3_step(s) == SQLITE_ROW) {
        id = (size_t)sqlite3_column_int(s, 0);
        _uuid = string(reinterpret_cast<const char*>(sqlite3_column_text(s, 1)));
        _username =  string(reinterpret_cast<const char*>(sqlite3_column_text(s, 2)));
        _fname = string(reinterpret_cast<const char*>(sqlite3_column_text(s, 3)));
        _lname = string(reinterpret_cast<const char*>(sqlite3_column_text(s, 4)));
        _eventid = sqlite3_column_int(s, 5);
    }

    User* a = new User(id, _uuid, _username, _fname, _lname, _eventid);

    return a;





}

User::~User() {
}

size_t User::getUserId() {
    return userid;
}
string User::getUUID() {
    return uuid;
}

string User::getUsername() {
    return username;
}

string User::getUserFname() {
    return fname;
}

string User::getUserLname() {
    return lname;
}

size_t User::getEventID() {
    return eventid;
}


