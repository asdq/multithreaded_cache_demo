/*
The MIT License (MIT)

Copyright (c) 2014 Fabio Vaccari

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shdb_all be included in
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

using namespace std;

std::shared_ptr<handle> db_cache::locate(const string &key)
{
	unique_lock<mutex> lk(c_cache_guard);
	c_read_lock.wait(lk, [this] {
		return c_cache_write_req == 0;
	});
	++c_cache_reading;
	lk.unlock();
	
	std::shared_ptr<handle> h;
	auto i = c_cache.find(key);
	
	// found in the cache
	if (i != c_cache.end()) {
		h = i -> second;
		h -> set_touched(true);
	}
	
	lk.lock();
	--c_cache_reading;
	c_write_lock.notify_all();
	return h;
}

shared_ptr<handle> db_cache::add(const string &key)
{
	unique_lock<mutex> lk(c_cache_guard);
	++c_cache_write_req;
	c_write_lock.wait(lk, [this] {
		return c_cache_reading == 0;
	});
	
	auto i = c_cache.find(key);
	shared_ptr<handle> h;
	
	if (i == c_cache.end()) {
		h.reset(new handle);
		h -> set_touched(true);
		h -> data = c_client -> fetch(key);
		h -> lock(c_handle_timeout);
		c_cache[key] = h;
	} else {
		h = i -> second;
		h -> set_touched(true);
	}
	
	--c_cache_write_req;
	if (c_cache_write_req == 0) c_read_lock.notify_all();
	return h;
}

void db_cache::update_db()
{
	vector<tuple<string, string>> list;
	
	auto copy_cache = [this] {
		unique_lock<mutex> lk(c_cache_guard);
		c_read_lock.wait(lk, [this] {
			return c_cache_write_req == 0;
		});
		++c_cache_reading;
		lk.unlock();
		
		cache_t cache(c_cache);
		
		lk.lock();
		--c_cache_reading;
		c_write_lock.notify_all();
		return cache;
	};
	
	for (auto &t : copy_cache()) {
		t.second -> lock(c_handle_timeout);
		if (t.second -> set_touched(false)) {
			list.emplace_back(t.first, t.second -> data);
		}
		t.second -> unlock();
	}
	c_client -> store(list);
}

void db_cache::erase_not_touched(size_t size)
{
	unique_lock<mutex> lk(c_cache_guard);
	
	if (c_cache.size() < size) return;
	
	++c_cache_write_req;
	c_write_lock.wait(lk, [this] {
		return c_cache_reading == 0;
	});
	
	auto i = c_cache.begin();
	while (i != c_cache.end()) {
		if ( ! i -> second -> touched()) {
			i = c_cache.erase(i);
		} else ++i;
	}
	
	--c_cache_write_req;
	if (c_cache_write_req == 0) c_read_lock.notify_all();
}

void db_cache::timer_loop(unsigned utime)
{
	using namespace std::chrono;
	milliseconds ms(utime);
	auto t0 = system_clock::now();
	
	c_client -> thread_init();
	do {
		auto t1 = system_clock::now();
		auto dt = duration_cast<milliseconds>(t1 - t0);
			
		std::this_thread::sleep_for(ms - dt % ms);
		erase_not_touched(c_cache_maxsize);
		update_db();
	} while ( ! get_exit());
	c_client -> thread_end();
}
