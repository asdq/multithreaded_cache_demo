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

#include "db_cache.h"
#include "mysql_client.h"
#include <iostream>
#include <cstdlib>
#include <cassert>

const unsigned n_threads = std::thread::hardware_concurrency() * 2;
//    const unsigned n_threads = 1;
const unsigned n_records = 1000;

// cache parameters
const int dt = 500;    // database updates, ms
const int timeout = 100; // try to lock data
const int size = 1000; // start to erase unused data

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
        auto s = random_string(5);
        list.emplace_back(s, s);
    }
    return list;
}

static
std::vector<mysql_client::record> sequential_table()
{
    std::vector<mysql_client::record> list;
    
    for (unsigned i = 0; i < n_records; ++i) {
        list.emplace_back(std::to_string(i), random_string(64));
    }
    return list;
}

static
void print(const std::string &key, const std::string &data,
           const std::string &fetched)
{
    static std::mutex print_guard;
    std::lock_guard<std::mutex> lk(print_guard);
    std::cerr
        << "[" << std::this_thread::get_id() << "] "
        << "key: " << key << " "
        << "data: " << data << " "
        << "fetched: " << fetched
        << std::endl;
}

static
void test_direct(mysql_client &client,
    const std::vector<mysql_client::record> &list)
{
    std::vector<std::thread> vt;
    
    client.store(list);
    for (unsigned i = 0; i < n_threads; ++i) {
        vt.emplace_back([&client, &list] {
            client.thread_init();
                
            for (auto &t : list) {
                auto fetched = client.fetch(std::get<0>(t));
                
//                print(std::get<0>(t), std::get<1>(t), fetched);
                assert(fetched == std::get<1>(t));
            }
            client.thread_end();
        });
    }
    for (auto &t : vt) t.join();
}

static
void test_cached(mysql_client &client,
    const std::vector<mysql_client::record> &list)
{
    db_cache cclient(&client, dt, timeout, size);
    std::vector<std::thread> vt;
    
    for (unsigned i = 0; i < n_threads; ++i) {
        vt.emplace_back( [&client, &cclient, &list] {
            client.thread_init();
            
            for (auto &t : list) {
                *cclient[std::get<0>(t)] = std::get<1>(t);
            }
            
            for (auto &t : list) {
                auto h = cclient[std::get<0>(t)];
                
//                print(std::get<0>(t), std::get<1>(t), *h);
                assert(*h == std::get<1>(t));
            }

            client.thread_end();
        });
    }
    for (auto &t : vt) t.join();
}

int main()
{
    using std::chrono::milliseconds;
    using std::chrono::system_clock;
    using std::chrono::duration_cast;
    
    mysql_client client("localhost", "fabio", "");
    std::vector<mysql_client::record> list;
    std::chrono::time_point<std::chrono::system_clock> t0, t1;
    
    std::srand(std::time(nullptr));
/*
    list = random_table();
    std::cout 
        << "testing without cache" << '\n'
        << "threads: " << n_threads  << '\n'
        << "records: " << n_records << '\n'
        << "..." << std::endl;
        
    t0 = system_clock::now();
    test_direct(client, list);
    t1 = system_clock::now();
    
    std::cout
        << "elapsed time: "
        << duration_cast<milliseconds>(t1 - t0).count()
        << " milliseconds.\n" << std::endl;
*/    
    list = sequential_table();
    std::cout 
        << "testing with cache" << '\n'
        << "threads: " << n_threads  << '\n'
        << "records: " << n_records << '\n'
        << "..." << std::endl;
        
    t0 = system_clock::now();
    test_cached(client, list);
    t1 = system_clock::now();
    
    std::cout
        << "elapsed time: "
        << duration_cast<milliseconds>(t1 - t0).count()
        << " milliseconds.\n" << std::endl;
    
    return 0;
}

