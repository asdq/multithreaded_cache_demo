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

#include <memory>
#include <string>
#include <tuple>
#include <vector>

/*!
    \brief Client for mysql table.
    
    Table format is:
    
        CREATE TABLE `records` (
          `key` varchar(20) NOT NULL,
          `data` text,
         PRIMARY KEY (`key`)
        )
    
    If key is not in table, insert a new entry with empty data.
    
    This class handles a single connection for each thread. Since mysql
    requires to call mysql_thread_end() before thread ends, you must
    call the following methods within the thread:
    
    - thread_init() to start table operations
    - thread_end() at the end of table operations
    
    this is true for the main thread too.
*/
class mysql_client
{
    class mysql_connection_handler;
    
    std::unique_ptr<mysql_connection_handler> mc_conn_handler;
    
public:
    
    // key, data
    typedef std::tuple<std::string, std::string> record;
    
    mysql_client(const std::string &url, const std::string usr,
        const std::string &pwd);
        
    ~mysql_client();
    
    std::string fetch(const std::string &key);
    void store(const std::string &key, const std::string &data);
    void store(const std::vector<record> &list);
    void thread_init();
    void thread_end();
};

#endif