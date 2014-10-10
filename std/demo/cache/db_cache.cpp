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
#include <algorithm>

using namespace std;

std::shared_ptr<handle> db_cache_t::locate(const string &key)
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

shared_ptr<handle> db_cache_t::add(const string &key)
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
		h -> timeout = c_handle_timeout;
		h -> set_touched(true);
		h -> data = c_client -> fetch(key);
		h -> lock();
		c_cache[key] = h;
	} else {
		h = i -> second;
		h -> set_touched(true);
	}
	
	--c_cache_write_req;
	if (c_cache_write_req == 0) c_read_lock.notify_all();
	return h;
}

db_cache_t::cache_t db_cache_t::copy_cache()
{
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
}

void db_cache_t::update_db()
{
	vector<tuple<string, string>> list;
	
	for (auto &t : copy_cache()) {
		t.second -> lock();
		if (t.second -> set_touched(false)) {
			list.emplace_back(t.first, t.second -> data);
		}
		t.second -> unlock();
	}
	c_client -> store(list);
}

void db_cache_t::erase_not_touched()
{
	unique_lock<mutex> lk(c_cache_guard);
	
	if (c_cache.size() < c_cache_maxsize) return;
	
	++c_cache_write_req;
	c_write_lock.wait(lk, [this] {
		return c_cache_reading == 0;
	});
	
	auto i = c_cache.begin();
	while (i != c_cache.end()) {
		if ( ! i -> second -> get_touched()) {
			i = c_cache.erase(i);
		} else ++i;
	}
	
	--c_cache_write_req;
	if (c_cache_write_req == 0) c_read_lock.notify_all();
}

void handle::unlock()
{
	data_guard.unlock();
}

void handle::lock()
{
//	It seems that try_lock_for is buggy
//	if ( ! data_guard.try_lock_for(timeout)) {
//		throw std::runtime_error("Timeout: failed to lock the handle.");
//	}
	
// no timer
//	data_guard.lock();
	
//	workaround
	auto now = std::chrono::system_clock::now();
	
	if ( ! data_guard.try_lock_until(now + timeout)) {
		throw std::runtime_error("Timeout: failed to lock the handle.");
	}
}

bool handle::get_touched()
{
	std::lock_guard<std::mutex> lk(touch_guard);
	return touched;
}

bool handle::set_touched(bool flag)
{
	std::lock_guard<std::mutex> lk(touch_guard);
	bool old = touched;
	touched = flag;
	return old;
}
