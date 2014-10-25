
## A multithreaded cache for database connections ##

Requirements are:

 - Only one thread at a time can work with a data entry.
 - Many threads can work with different data entries at the same time.
 - A thread can wait for accessing a data entry only for a limited time,
   afterwards it should throw an excepton.
 - The cache should keep track of data changes and save them periodically
   in the database.
 - If requested data is not in the cache, the cache should fetch it
   from the database.
 - If requested data is neither in the cache nor in the database,
   an empty entry is added.

Class 'mysql_client' fetches and writes records entries in a mysql database,
it mantains an open connection for each thread. Methods thread_init() and
thread_end() should be called at the beginning and at the end of the work
with a thread respectively. It uses connector/c++.

Class 'db_cache' performs the following tasks:

 - find data in the cache: method locate()
 - add new data in the cache: method add()
 - remove unused data if the cache exceeds the given size:
   method erase_not_touched()
 - take the data for database update: method update_db()

these methods are either readers or writers.
Readers can lookup the container, but cannot insert or delete an entry. They
can lock and unlock a data entry. Readers are locate() and copy_cache in update_db().
Writers can lookup the container and can insert or delete an entry in the
container. They cannot lock or unlock a data entry. Writers are add(),
merge() and erase_not_touched().
Writers have precedence on readers and only one writer at a time can access
the container.
The only public method of 'db_cache' is operator [] which returns a moveable-
only object of type 'data_handle'. It ensures unlocking of the data. The
data itself is available trought operator * (), for example:

    {
        data_handle dh = cache[key];
        std::cout << *dh;
    }

On creation 'db_cache' starts a thread which updates the database
periodically and removes untouched data if the cache exceeds a given size.
This thread is the only one that performs these actions, other actions are
performed by the task which made a data request. The sequence of actions
implemented for a thread is:

    timer_loop -> erase_not_touched -> copy_cache -> update_db -> store -> timer_loop
    [] -> locate -> lock -> data_handle -> .. ~data_handle -> unlock
    [] -> locate -> fetch -> add -> lock -> data_handle -> .. ~data_handle -> unlock

File test.cpp contains code used for testing.
