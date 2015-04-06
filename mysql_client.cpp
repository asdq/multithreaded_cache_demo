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
#include <algorithm>
#include <cassert>
#include <cppconn/driver.h>
#include <cppconn/prepared_statement.h>
#include <mutex>
#include <mysql/mysql.h>
#include <thread>

using statement = std::unique_ptr<sql::PreparedStatement>;
using result_set = std::unique_ptr<sql::ResultSet>;

class mysql_client::mysql_connection_handler
{
    std::string mc_host;
    std::string mc_user;
    std::string mc_password;
    
    // MySQL driver
    sql::Driver *mc_driver;
    
    typedef std::tuple<std::thread::id, sql::Connection*> conn_entry;
    std::vector<conn_entry> mc_conn_list;
    std::mutex mc_guard;
    
public:
    mysql_connection_handler(const std::string &url,
        const std::string usr, const std::string &pwd);
    
    ~mysql_connection_handler();
    
    sql::Connection* get_connection();
    void close_connection();
};

mysql_client::mysql_connection_handler::mysql_connection_handler(
    const std::string &url, const std::string usr, const std::string &pwd
    ) : mc_host(url), mc_user(usr), mc_password(pwd)
{
    mc_driver = get_driver_instance();
}

mysql_client::mysql_connection_handler::~mysql_connection_handler()
{
    assert(mc_conn_list.empty());
}

sql::Connection* mysql_client::mysql_connection_handler::get_connection()
{
    std::lock_guard<std::mutex> lk(mc_guard);
    auto id = std::this_thread::get_id();
    auto i = find_if(mc_conn_list.begin(), mc_conn_list.end(),
        [id] (const conn_entry &t) {
            return id == std::get<0>(t);
        });
    
    if (i == mc_conn_list.end()) {
        mc_conn_list.emplace_back(id, nullptr);
        i = --mc_conn_list.end();
    }
    
    if (std::get<1>(*i) != nullptr && std::get<1>(*i) -> isClosed()) {
    	delete std::get<1>(*i);
    	std::get<1>(*i) = nullptr;
    }
    	
    if (std::get<1>(*i) == nullptr) {
        std::get<1>(*i) = mc_driver -> connect(mc_host, mc_user, mc_password);
        std::get<1>(*i) -> setSchema("test");
    }
    
    return std::get<1>(*i);
}

void mysql_client::mysql_connection_handler::close_connection()
{
    std::lock_guard<std::mutex> lk(mc_guard);
    auto id = std::this_thread::get_id();
    auto i = std::find_if(mc_conn_list.begin(), mc_conn_list.end(),
        [id] (const conn_entry &t) {
            return id == std::get<0>(t);
        }
    );
    
    if (i != mc_conn_list.end()) {
        delete std::get<1>(*i);
        std::swap(*i, mc_conn_list.back());
        mc_conn_list.pop_back();
    }
}

mysql_client::mysql_client(const std::string &url, const std::string usr,
    const std::string &pwd)
    : mc_conn_handler(new mysql_connection_handler(url, usr, pwd))
{}

mysql_client::~mysql_client()
{}

std::string mysql_client::fetch(const std::string &key)
{
    sql::Connection *conn = mc_conn_handler -> get_connection();
    std::string data;
    
    statement pstmt(conn -> prepareStatement(R"mysql(
        SELECT     `data`
        FROM `records`
        WHERE `key` = ?
    )mysql"));
    
    pstmt -> setString(1, key);
    
    result_set res(pstmt -> executeQuery());
    
    if (res -> rowsCount() != 0) {
        res -> next();
        data = res -> getString(1);
    }
    
    return data;
}

void mysql_client::store(const std::vector<record> &list)
{
    sql::Connection *conn = mc_conn_handler -> get_connection();
    
    conn -> setAutoCommit(false);
    
    statement pstmt(conn -> prepareStatement(R"mysql(
        INSERT INTO `records`
        SET  `key` = ?, `data` = ?
        ON DUPLICATE KEY UPDATE `data` = ?
    )mysql"));
    
    for (auto &t : list) {
        pstmt -> setString(1, std::get<0>(t));
        pstmt -> setString(2, std::get<1>(t));
        pstmt -> setString(3, std::get<1>(t));
        pstmt -> executeUpdate();
    }
    conn -> commit();
    
    conn -> setAutoCommit(true);
}

void mysql_client::store(const std::string &key, const std::string &data)
{
    sql::Connection *conn = mc_conn_handler -> get_connection();
    
    statement pstmt(conn -> prepareStatement(R"mysql(
        INSERT INTO `records`
        SET  `key` = ?, `data` = ?
        ON DUPLICATE KEY UPDATE `data` = ?
    )mysql"));
    
    pstmt -> setString(1, key);
    pstmt -> setString(2, data);
    pstmt -> setString(3, data);
    pstmt -> executeUpdate();
}

void mysql_client::thread_init()
{
    ::mysql_thread_init();
    mc_conn_handler -> get_connection();
}

void mysql_client::thread_end()
{
    mc_conn_handler -> close_connection();
    ::mysql_thread_end();
}
