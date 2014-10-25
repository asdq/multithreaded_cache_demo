#ifndef TIMER_H
#define TIMER_H

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

// See discussion at
// http://stackoverflow.com/questions/14650885/how-to-create-timer-events-using-c-11

#include <chrono>
#include <functional>
#include <thread>
#include <mutex>

namespace futil {

/*!
    \brief A timer.
    
    Run code periodically in a separate thread.
    Start from specific time point,
    then wake up the thread at specific intervals.
*/
class timer
{
    std::thread t_thread;
    std::mutex t_guard;
    bool t_exit;
    bool t_run;
    
    bool exit()
    {
        std::lock_guard<std::mutex> lk(t_guard);
        return t_exit;
    }
    
    void set_exit(bool flag)
    {
        std::lock_guard<std::mutex> lk(t_guard);
        t_exit = flag;
    }
    
    bool run()
    {
        std::lock_guard<std::mutex> lk(t_guard);
        return t_run;
    }
    
    void set_run(bool flag)
    {
        std::lock_guard<std::mutex> lk(t_guard);
        t_run = flag;
    }
    
public:
    
    /*!
        \brief Instantiate a timer.
        \param t0 starting time
        \param delay time delay.
        \param f what to call.
        \param args list of arguments.
        
        Start from time point, then repeat until stop.
    */
    template <class TimePoint, class Duration, class Callable, class... Args>
    timer(TimePoint t0, Duration delay, Callable&& f, Args... args) :
        t_exit(false), t_run(true)
    {
        std::function<typename std::result_of<Callable(Args...)>::type ()>
            task(std::bind(std::forward<Callable>(f), std::forward<Args>(args)...));
            
        t_thread = std::thread([this, t0, delay, task] {
            std::this_thread::sleep_until(t0);
            while ( ! exit()) {
                if (run()) task();
                
                auto t1 = std::chrono::system_clock::now();
                auto dt = std::chrono::duration_cast<Duration>(t1 - t0);
                std::this_thread::sleep_for(delay - dt % delay);
            }
        });
    }
    
    ~timer()
    {
        set_exit(true);
        if (t_thread.joinable()) t_thread.join();
    }
    
    /*!
        \brief Check timer.
        \return true if running, otherwise false.
    */
    operator bool() { return run(); }

    /*!
        \brief Wakeup the timer.
    */
    void start() { set_run(true); }
    
    /*!
        \brief Pause the timer.
    */
    void stop() { set_run(false); }
};

} // end futil

#endif
