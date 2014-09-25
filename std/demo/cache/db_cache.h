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

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

struct handle
{
	std::string data;
	std::timed_mutex guard;
	std::atomic_bool touched;
};

// implement cache
class db_cache_t
{
	typedef std::tuple<
		std::string,	// key
		std::shared_ptr<handle>	// data
	> cache_entry;
	
	std::vector<cache_entry> c_cache;
	size_t c_unsorted_sz;
	
	std::mutex c_cache_guard;
	std::condition_variable c_read_lock;
	std::condition_variable c_write_lock;
	int c_cache_reading; // number of readers
	int c_cache_write_req; // requests for writing
	
	const std::chrono::milliseconds c_handle_timeout;
	const size_t c_cache_maxsize;
	
public:

	explicit
	db_cache_t(int timeout, size_t size) :
		c_unsorted_sz(0),
		c_cache_reading(0),
		c_cache_write_req(0),
		c_handle_timeout(timeout),
		c_cache_maxsize(size)
	{}
	
	std::shared_ptr<handle> locate(const std::string &key);
	std::shared_ptr<handle> merge(const std::string &key);
	void unlock(std::shared_ptr<handle> &h);
	void lock(std::shared_ptr<handle> &h);
	void erase_not_touched();
	std::vector<std::tuple<std::string, std::string>> update_db();
};

// handle locking
class data_handle
{
	friend class db_cache_t;
	
	db_cache_t *h_cache;
	std::shared_ptr<handle> h_data;
	
public:
	data_handle(db_cache_t *c, const std::shared_ptr<handle> &d)
	: h_cache(c), h_data(d) {}
	
	data_handle() : h_cache(nullptr), h_data() {}
	~data_handle() { h_cache -> unlock(h_data); }
	data_handle(const data_handle&) = delete;
	
	data_handle(data_handle&& dh)
	: h_cache(dh.h_cache), h_data(dh.h_data) {}
	
	data_handle& operator = (data_handle&& dh)
	{
		h_cache = dh.h_cache;
		h_data = dh.h_data;
		return *this;
	}
	
	std::string& data()
	{
		return h_data -> data;
	}
};

template<class DbClient>
class db_cache : private db_cache_t
{
	DbClient *c_client;
	
	std::atomic_bool c_timer_exit;
	std::thread c_timer;
	
	std::shared_ptr<handle> get_handle(const std::string &key)
	{
		auto h = locate(key);
		
		if ( ! h) {
			h = merge(key); // h must be already locked
			h -> data = c_client -> fetch(key);
		} else lock(h);
		return h;
	}
	
public:
	
	/*!
		\brief Instantiate a cache.
		
		\param c a database connection.
		\param dt time interval for db to update, in ms.
		\param timeout for data lock.
		\param size when start to clean the cache.
	*/
	db_cache(DbClient *c, int dt, int timeout, size_t size) :
		db_cache_t(timeout, size),
		c_client(c)
	{
		c_timer_exit = ATOMIC_VAR_INIT(false);
		c_timer = std::thread([this, dt] {
			std::chrono::milliseconds ms(dt);
			
			c_client -> thread_init();
			do {
				std::this_thread::sleep_for(ms);
				erase_not_touched();
				c_client -> store(update_db());
			} while ( ! c_timer_exit.load());
			c_client -> thread_end();
		});
	}
	
	~db_cache()
	{
		c_timer_exit.store(true);
		c_timer.join();
	}
	
	data_handle operator [] (const std::string &key)
	{
		auto h = get_handle(key);
		return data_handle(this, h);
	}
};

#endif
