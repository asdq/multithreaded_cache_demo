#ifndef DB_CACHE_H
#define DB_CACHE_H
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

#include <iostream>
#include "mysql_client.h"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <tuple>
#include <vector>
#include <unordered_map>

struct handle
{
	std::string data;
	std::timed_mutex data_guard;
	std::mutex touch_guard;
	bool touched;
	
	void unlock()
	{
		data_guard.unlock();
	}
	
	void lock(const std::chrono::milliseconds &timeout)
	{
	//	It seems that try_lock_for is buggy
	//	if ( ! data_guard.try_lock_for(timeout)) {
	//		throw std::runtime_error("Timeout: failed to lock the handle.");
	//	}
		
	//	workaround
		auto now = std::chrono::system_clock::now();
		
		if ( ! data_guard.try_lock_until(now + timeout)) {
			throw std::runtime_error("Timeout: failed to lock the handle.");
		}
	}
	
	bool get_touched()
	{
		std::lock_guard<std::mutex> lk(touch_guard);
		return touched;
	}
	
	bool set_touched(bool flag)
	{
		std::lock_guard<std::mutex> lk(touch_guard);
		bool old = touched;
		touched = flag;
		return old;
	}
};

// handle locking
class data_handle
{
	std::shared_ptr<handle> h_data;
	
public:
	data_handle() : h_data() {}
	~data_handle() { h_data -> unlock(); }
	data_handle(const data_handle&) = delete;
	data_handle(data_handle&& dh) : h_data(dh.h_data) {}
	data_handle(const std::shared_ptr<handle> &d) : h_data(d) {}
	
	data_handle& operator = (data_handle&& dh)
	{
		h_data = dh.h_data;
		return *this;
	}
	
	std::string& data()
	{
		return h_data -> data;
	}
};

class db_cache
{
	mysql_client *c_client;
	
	// cache code
	
	typedef std::unordered_map<
		std::string,
		std::shared_ptr<handle>
	> cache_t;
	
	cache_t c_cache;
	const std::chrono::milliseconds c_handle_timeout;
	const size_t c_cache_maxsize;
	
	std::shared_ptr<handle> locate(const std::string &key);
	std::shared_ptr<handle> add(const std::string &key);
	void erase_not_touched(size_t size);
	void update_db();
	
	// cache locking
	
	std::mutex c_cache_guard;
	std::condition_variable c_read_lock;
	std::condition_variable c_write_lock;
	int c_cache_reading; // number of readers
	int c_cache_write_req; // requests for writing
	
	// timer code
	
	bool c_timer_exit;
	std::thread c_timer;
	std::mutex guard;
	
	void set_exit(bool flag)
	{
		std::lock_guard<std::mutex> lk(guard);
		c_timer_exit = flag;
	}
	
	bool get_exit()
	{
		std::lock_guard<std::mutex> lk(guard);
		return c_timer_exit;
	}
	
	void timer_loop(unsigned utime);
	
public:
	
	/*!
		\brief Instantiate a cache.
		
		\param c a database connection.
		\param utime time interval for db to update, in ms.
		\param timeout for data lock.
		\param size when start to clean the cache.
	*/
	db_cache(mysql_client *c, unsigned utime, int timeout, size_t size) :
		c_client(c), c_handle_timeout(timeout), c_cache_maxsize(size),
		c_cache_reading(0), c_cache_write_req(0), c_timer_exit(false),
		c_timer([this, utime] { timer_loop(utime); })
	{}
	
	~db_cache()
	{
		set_exit(true);
		c_timer.join();
		
		while( ! c_cache.empty()) {
			erase_not_touched(0);
			update_db();
		}
	}
	
	data_handle operator [] (const std::string &key)
	{
		auto h = locate(key);
		
		if (h) {
			h -> lock(c_handle_timeout);
		} else {
			h = add(key);
		}
		return h;
	}
};

#endif
