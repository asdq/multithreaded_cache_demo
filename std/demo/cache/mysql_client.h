#ifndef MYSQL_CLIENT
#define MYSQL_CLIENT

/*
The MIT License (MIT)

Copyright (c) 2014 Fabio Vaccari

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <mutex>
#include <thread>
#include <tuple>
#include <vector>

namespace sql {
	class Driver;
	class Connection;
}

/*!
	\brief Fetch from, and store to, table 'records' in 'temp' schema.
	
	Table format is:
	
	    CREATE TABLE `records` (
	      `key` varchar(20) NOT NULL,
	      `data` text,
	     PRIMARY KEY (`key`)
	    ) ENGINE=InnoDB DEFAULT CHARSET=utf8
	
	Keeps a connection opened for each thread.
	If 'key' is not in 'records', insert a new entry with empty 'data'.
	
	\author Fabio Vaccari fabio.vaccari@gmail.com
*/
class mysql_client
{
	sql::Driver *mc_driver;
	
	std::string mc_host;
	std::string mc_user;
	std::string mc_password;
	
	std::mutex mc_guard;
	
	typedef std::tuple<std::thread::id, sql::Connection*> conn_entry;
	std::vector<conn_entry> mc_conn_list;
	
	sql::Connection* get_connection();
	void close_connection();
	
public:
	
	// key, data
	typedef std::tuple<std::string, std::string> record;

	mysql_client() : mc_driver(nullptr) {}
	mysql_client(const std::string &url, const std::string usr,
		const std::string &pwd);

	std::string fetch(const std::string &key);
	void store(const std::string &key, const std::string &data);
	void store(const std::vector<record> &list);
	void thread_init();
	void thread_end();
};

#endif