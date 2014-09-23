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
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cassert>

using namespace std;

const unsigned n_threads = thread::hardware_concurrency() * 2;
//	const unsigned n_threads = 1;
const unsigned n_records = 10000;

// cache parameters
const int dt = 500;	// database updates, ms
const int timeout = 250; // try to lock data
const int size = 10000; // start to erase unused data

static
string random_string(int len)
{
	string mask = "a bc def ghij klmno pqrstu vxyz";
	int mask_len = mask.length();
	string s;
	
	for (int i = 0; i < len; ++i) {
		s.push_back(mask[rand() % mask_len]);
	}
	return s;
}

static
vector<mysql_client::record> random_table()
{
	vector<mysql_client::record> list;
	
	for (unsigned i = 0; i < n_records; ++i) {
		auto s = random_string(5);
		list.emplace_back(s, s);
	}
	return list;
}

static
void print(const string &key, const string &data, const string &fetched)
{
	static mutex print_guard;
	lock_guard<mutex> lk(print_guard);
	cerr
		<< "[" << this_thread::get_id() << "] "
		<< "key: " << key << " "
		<< "data: " << data << " "
		<< "fetched: " << fetched
		<< endl;
}

static
void test_direct(mysql_client &client,
	const vector<mysql_client::record> &list)
{
	vector<thread> vt;
	
	client.store(list);
	for (unsigned i = 0; i < n_threads; ++i) {
		vt.emplace_back([&client, &list] {
			client.thread_init();
				
			for (auto &t : list) {
				auto fetched = client.fetch(get<0>(t));
				
//				print(get<0>(t), get<1>(t), fetched);
				assert(fetched == get<1>(t));
			}
			client.thread_end();
		});
	}
	for (auto &t : vt) t.join();
}

static
void test_cached(mysql_client &client,
	const vector<mysql_client::record> &list)
{
	db_cache<mysql_client> cclient(&client, dt, timeout, size);
	vector<thread> vt;
	
	for (unsigned i = 0; i < n_threads; ++i) {
		vt.emplace_back( [&client, &cclient, &list] {
			client.thread_init();
			
			for (auto &t : list) {
				cclient[get<0>(t)].data() = get<1>(t);
			}
			
			for (auto &t : list) {
				string fetched = cclient[get<0>(t)].data();
				
//				print(get<0>(t), get<1>(t), fetched);
				assert(fetched == get<1>(t));
			}

			client.thread_end();
		});
	}
	for (auto &t : vt) t.join();
}

int main()
{
	mysql_client client("localhost", "fabio", "");
	vector<mysql_client::record> list;
	chrono::time_point<chrono::system_clock> t0, t1;
	
	srand(time(nullptr));
/*
	list = random_table();
	cout 
		<< "testing without cache" << '\n'
		<< "threads: " << n_threads  << '\n'
		<< "records: " << n_records << '\n'
		<< "..." << endl;
		
	t0 = chrono::system_clock::now();
	test_direct(client, list);
	t1 = chrono::system_clock::now();
	
	cout
		<< "elapsed time: "
		<< chrono::duration_cast<chrono::milliseconds>(t1 - t0).count()
		<< " milliseconds.\n" << endl;
*/	
	list = random_table();
	cout 
		<< "testing with cache" << '\n'
		<< "threads: " << n_threads  << '\n'
		<< "records: " << n_records << '\n'
		<< "..." << endl;
		
	t0 = chrono::system_clock::now();
	test_cached(client, list);
	t1 = chrono::system_clock::now();
	
	cout
		<< "elapsed time: "
		<< chrono::duration_cast<chrono::milliseconds>(t1 - t0).count()
		<< " milliseconds.\n" << endl;
	
	return 0;
}

