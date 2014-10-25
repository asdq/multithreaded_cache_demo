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

#include "mysql_client.h"
#include "mysql_connection.h"
#include <algorithm>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <mysql/mysql.h>
#include <stdlib.h>

using namespace std;

mysql_client::mysql_client(const string &url, const string usr,
    const string &pwd) : mc_host(url), mc_user(usr), mc_password(pwd)
{
    mc_driver = get_driver_instance();
}

string mysql_client::fetch(const string &key)
{
    sql::Connection *conn = get_connection();
    sql::PreparedStatement *pstmt = nullptr;
    sql::ResultSet *res = nullptr;
    string data;
    
    pstmt = conn -> prepareStatement(R"mysql(
        SELECT     `data`
        FROM `records`
        WHERE `key` = ?
    )mysql");
    
    pstmt -> setString(1, key);
    res = pstmt -> executeQuery();
    
    if (res -> rowsCount() != 0) {
        res -> next();
        data = res -> getString(1);
    }
    
    delete res;
    delete pstmt;
    return data;
}

void mysql_client::store(const vector<record> &list)
{
    sql::Connection *conn = get_connection();
    sql::PreparedStatement *pstmt = nullptr;
    
    conn -> setAutoCommit(false);
    
    pstmt = conn -> prepareStatement(R"mysql(
        INSERT INTO `records`
        SET  `key` = ?, `data` = ?
        ON DUPLICATE KEY UPDATE `data` = ?
    )mysql");
    
    for (auto &t : list) {
        pstmt -> setString(1, get<0>(t));
        pstmt -> setString(2, get<1>(t));
        pstmt -> setString(3, get<1>(t));
        pstmt -> executeUpdate();
    }
    conn -> commit();
    
    conn -> setAutoCommit(true);
    delete pstmt;
}

void mysql_client::store(const string &key, const string &data)
{
    sql::Connection *conn = get_connection();
    sql::PreparedStatement *pstmt = nullptr;
    
    pstmt = conn -> prepareStatement(R"mysql(
        INSERT INTO `records`
        SET  `key` = ?, `data` = ?
        ON DUPLICATE KEY UPDATE `data` = ?
    )mysql");
    
    pstmt -> setString(1, key);
    pstmt -> setString(2, data);
    pstmt -> setString(3, data);
    pstmt -> executeUpdate();
    
    delete pstmt;
}

void mysql_client::thread_init()
{
    ::mysql_thread_init();
    get_connection();
}

void mysql_client::thread_end()
{
    close_connection();
    ::mysql_thread_end();
}

sql::Connection* mysql_client::get_connection()
{
    lock_guard<mutex> lk(mc_guard);
    auto id = this_thread::get_id();
    auto i = find_if(mc_conn_list.begin(), mc_conn_list.end(),
        [id] (const conn_entry &t) {
            return id == get<0>(t);
        }
    );
    
    if (i == mc_conn_list.end()) {
        mc_conn_list.emplace_back(id, nullptr);
        i = --mc_conn_list.end();
    }
    
    if (get<1>(*i) != nullptr && get<1>(*i) -> isClosed()) {
    	delete get<1>(*i);
    }
    	
    if (get<1>(*i) == nullptr)
    {
        get<1>(*i) = mc_driver -> connect(mc_host, mc_user, mc_password);
        get<1>(*i) -> setSchema("test");
    }
    
    return get<1>(*i);
}

void mysql_client::close_connection()
{
    lock_guard<mutex> lk(mc_guard);
    auto id = this_thread::get_id();
    auto i = find_if(mc_conn_list.begin(), mc_conn_list.end(),
        [id] (const conn_entry &t) {
            return id == get<0>(t);
        }
    );
    
    if (i != mc_conn_list.end()) {
        delete get<1>(*i);
        swap(*i, mc_conn_list.back());
        mc_conn_list.pop_back();
    }
}