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
	std::shared_ptr<handle> h;
	
	c_read_lock.wait(lk, [this] {
		return c_cache_write_req == 0;
	});
	++c_cache_reading;
	
	// [a, b) ordered , [b, c) not ordered
	auto a = c_cache.cbegin();
	auto b = c_cache.cend() - c_unsorted_sz;
	auto c = c_cache.cend();
	
	lk.unlock();
	
	// binary search on [a, b)
	auto i = lower_bound(a, b, forward_as_tuple(key, nullptr),
		[] (const cache_entry &a, const cache_entry &b) {
			return get<0>(a) < get<0>(b);
		}
	);
	
	// linear search on [b, c)
	if (i == b || key != get<0>(*i)) {
		i = find_if(b, c, [&key] (const cache_entry &x) {
				return key == get<0>(x);
			}
		);
	}
	
	lk.lock();
	
	// found in the cache
	if (i != c && key == get<0>(*i)) {
		h = get<1>(*i);
		h -> touched = true;
	}
	
	--c_cache_reading;
	c_write_lock.notify_all();
	return h;
}

std::shared_ptr<handle> db_cache_t::merge(const string &key)
{
	unique_lock<mutex> lk(c_cache_guard);
	
	++c_cache_write_req;
	c_write_lock.wait(lk, [this] {
		return c_cache_reading == 0;
	});
	
	std::shared_ptr<handle> h(new handle);
	h -> touched = true;
	h -> timeout = c_handle_timeout;
	
	// append
	c_cache.emplace_back(key, h);
	++c_unsorted_sz;
	
	// if unordered side is too long, merge it
	if (log2(c_cache.size() - c_unsorted_sz) < c_unsorted_sz) {
		
		// [a, b) ordered , [b, c) not ordered
		auto a = c_cache.begin();
		auto b = c_cache.end() - c_unsorted_sz;
		auto c = c_cache.end();
		
		sort(b, c);
		inplace_merge(a, b, c);
		c_unsorted_sz = 0;
	}
	
	--c_cache_write_req;
	if (c_cache_write_req == 0) c_read_lock.notify_all();
	return h;
}

vector<tuple<string, string>> db_cache_t::update_db()
{
	unique_lock<mutex> lk(c_cache_guard);
	
	c_read_lock.wait(lk, [this] {
		return c_cache_write_req == 0;
	});
	++c_cache_reading;
	
	vector<tuple<string, string>> list;
	
	for (auto &t : c_cache) {
		auto h = get<1>(t);
		
		h -> lock();
		if (h -> touched) {
			list.emplace_back(get<0>(t), h -> data);
			h -> touched = false;
		}
		h -> unlock();
	}
	
	--c_cache_reading;
	c_write_lock.notify_all();
	return list;
}

void db_cache_t::erase_not_touched()
{
	unique_lock<mutex> lk(c_cache_guard);
	
	if (c_cache.size() < c_cache_maxsize) return;
	
	++c_cache_write_req;
	c_write_lock.wait(lk, [this] {
		return c_cache_reading == 0;
	});
	
	auto a = c_cache.begin();
	auto b = c_cache.end();
	auto i = stable_partition(a, b,
		[] (const cache_entry &t) {
			return get<1>(t) -> touched;
	});
	
	c_unsorted_sz = i - is_sorted_until(a, i);
	c_cache.erase(i, b);
	
	--c_cache_write_req;
	if (c_cache_write_req == 0) c_read_lock.notify_all();
}