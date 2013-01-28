#ifndef __schwa__job__worker__
#define __schwa__job__worker__


#include "job01/job_pool.h"

#include <atomic>
#include <bitset>
#include <thread>


template <class JobPoolT>
class Worker {

public:
    Worker(JobPoolT& global_pool) : _pool(global_pool), _running(false) { }

    // Start the Worker, unless it has already been started.
    void start() {
        lock l(_mutex);

        // TODO: throw exception
        if (_running) return;

        _running = true;

        _thread = std::thread([this](){
            this->loop();
        });
    }

    // Stop the Worker, unless it has already stopped.
    void stop() {
        {
            lock l(_mutex);
            if (!_running) return;
            Flags flags = _flags.load();
            flags[STOP_WORKING] = true;
            _flags.store(flags);
        }
        _thread.join();
    }

private:
    typename JobPoolT::LocalPoolT _pool;
    typedef std::lock_guard<std::mutex> lock;
    typedef std::bitset<32> Flags;

    enum FlagNames {
        STOP_WORKING = 0
    };

    void work() {

    }

    Flags processFlags(Flags flags) {

        if (flags[STOP_WORKING]) {
            flags.reset(STOP_WORKING);
            _running = false;
        }

        return flags;
    }

    void loop() {
        while(true) {
            // Fast path... if there is no interrupt, work.
            while(_flags.load().none()) {
                work();
            }

            // Lock worker during interrupt processing.
            {
                lock l(_mutex);

                // Load flags again, now that we have the lock.
                Flags flags = _flags.load();

                // Clear all flags which were processed.
                flags = processFlags(flags);
                _flags.store(flags);

                // Ensure there were no flags left unprocessed.
                assert(flags.none());

                // We processed the STOP_WORKING flag.
                if (!_running) return;
            }
        }
    }

    std::mutex          _mutex;
    std::thread         _thread;

    std::atomic<Flags>  _flags;

    bool                _running;
};


#endif  // #ifndef __schwa__job__worker__