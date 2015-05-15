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

// TODO read parameters from command line

#include "db_cache.h"
#include "mysql_client.h"
#include <iostream>

using std::chrono::milliseconds;
using std::chrono::system_clock;
using std::chrono::duration_cast;

const unsigned n_threads = std::thread::hardware_concurrency() * 2;
const unsigned n_records = 10000;

// cache parameters
const int dt = 1000;    // database updates, ms
const int timeout = 100; // try to lock data
const int max_size = 15000; // start to erase unused data
const int key_length = 3;
const int data_length = 5;

// database connection
// see mysql_client.h
const std::string user = "fabio";
const std::string password = "";
const std::string host = "localhost";

// use to synchronize output stream
std::mutex print_guard;

static
std::string random_string(int len)
{
    std::string mask = "a bc def ghij klmno pqrstu vxyz";
    int mask_len = mask.length();
    std::string s;
    
    for (int i = 0; i < len; ++i) {
        s.push_back(mask[std::rand() % mask_len]);
    }
    return s;
}

static
std::vector<mysql_client::record> random_table()
{
    std::vector<mysql_client::record> list;
    
    for (unsigned i = 0; i < n_records; ++i) {
        auto key = random_string(key_length);
        auto value = random_string(data_length);
        list.emplace_back(key, value);
    }
    return list;
}

// start different threads
// each of them fetches a list of records and compares values
// if the values don't correspond, store the value on the list
static
void test_cached(mysql_client &client)
{
    db_cache cclient(&client, dt, timeout, max_size);
    std::vector<std::thread> vt;
    
    for (unsigned i = 0; i < n_threads; ++i) {
        vt.emplace_back( [&client, &cclient] {
            client.thread_init();
            
            auto list = random_table();
            
            auto t0 = system_clock::now();
            for (auto &t : list) {
                for (;;) try {
                    auto h = cclient[std::get<0>(t)];
                    
                    if (*h != std::get<1>(t)) { *h = std::get<1>(t); }
                    break;
                    
                } catch(db_cache_timeout e) {
                	std::unique_lock<std::mutex> lk(print_guard);
                	std::cerr << "thread " << std::this_thread::get_id()
                	          << " timeout on key '" << std::get<0>(t)
                	          << "'\n";
                	lk.unlock();
                	
                	std::this_thread::yield();
                }
            }
            auto t1 = system_clock::now();
            
            std::unique_lock<std::mutex> lk(print_guard);
            std::cerr << "thread " << std::this_thread::get_id()
                      << " elapsed time: "
                      << duration_cast<milliseconds>(t1 - t0).count()
                      << " milliseconds.\n";
            lk.unlock();
            
            client.thread_end();
        });
    }
    for (auto &t : vt) t.join();
}

int main()
{
    mysql_client client(host, user, password);
    std::chrono::time_point<std::chrono::system_clock> t0, t1;
    
    std::srand(std::time(nullptr));
    
    std::cout
        << "Testing cache with random entries\n"
        << "key length: " << key_length << '\n'
        << "data length: " << data_length << '\n'
        << std::endl;
    
    std::cout 
        << "testing with cache" << '\n'
        << "threads: " << n_threads  << '\n'
        << "records per thread: " << n_records << '\n'
        << "cache max size: " << max_size << '\n'
        << "timeout locking: " << timeout << " milliseconds\n"
        << "database updates every " << dt << " milliseconds\n"
        << "..." << std::endl;
    
    t0 = system_clock::now();
    test_cached(client);
    t1 = system_clock::now();
    
    std::cout
        << "elapsed time: "
        << duration_cast<milliseconds>(t1 - t0).count()
        << " milliseconds.\n" << std::endl;
    
    return 0;
}

