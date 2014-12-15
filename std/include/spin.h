#ifndef SPIN_H
#define SPIN_H

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

#include <thread>
#include <atomic>

namespace futil {

/*!
    \brief Spin two processes.
    
    Implements the algorithm of Peterson.
    
    First add the processes with add_first() and add_second().
    Then lock and unlock.
    The first process will spin untill the second process is added.
    Which process will start first is unpredictable.
    This class is BasicLockable.
 */
class spin2
{
    std::atomic<std::thread::id> p_threads[2];
    std::atomic<short> p_turn;
    std::atomic<bool> p_flag[2];
    
public:
    
    spin2()
    {
        p_flag[0].store(false);
        p_flag[1].store(false);
        p_threads[0].store(std::thread::id());
        p_threads[1].store(std::thread::id());
    }
    
    bool add_first()
    {
        auto t = std::this_thread::get_id();
        auto t_null = std::thread::id();
        
        return p_threads[0].compare_exchange_strong(t_null, t);
    }
    
    bool add_second()
    {
        auto t = std::this_thread::get_id();
        auto t_null = std::thread::id();
        
        return p_threads[1].compare_exchange_strong(t_null, t);
    }
    
    void lock()
    {
        auto t = std::this_thread::get_id();
        std::thread::id t0, t1, t_null;
        
        // spin until both threads are assigned
        t0 = t1 = t_null;
        do {
            t0 = p_threads[0].load();
            t1 = p_threads[1].load();
        } while(t0 == t_null || t1 == t_null);
        
        if (t == t0) {
            p_flag[0].store(true);
            p_turn.store(1);
            
            // spin
            while (p_flag[1].load() && p_turn.load() == 1); 
            
        } else if (t == t1) {
            p_flag[1].store(true);
            p_turn.store(0);
            
            // spin
            while (p_flag[0].load() && p_turn.load() == 0);
        }
    }
    
    void unlock()
    {
        auto t = std::this_thread::get_id();
        auto t0 = p_threads[0].load();
        auto t1 = p_threads[1].load();
        
        if (t == t0) {
            p_flag[0].store(false);
        } else if (t == t1) {
            p_flag[1].store(false);
        }
    }
};

} // end futil

#endif
