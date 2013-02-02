#include "job01/generic_worker.h"


// schwa::job::impl
namespace schwa { namespace job { namespace impl {


BaseWorker::BaseWorker() : _running(false)
{
    // Because _flags is atomic, me must jump through hoops to initialize it.
    Flags flags;
    flags.reset();
    _flags.store(flags);
}


BaseWorker::~BaseWorker()
{
    // Ensure thread is stopped before we die.
    stop();
}


// Start the Worker, unless it has already been started.
void BaseWorker::start()
{
    lock l(_mutex);

    // TODO: throw exception
    if (_running) return;

    _running = true;

    _thread = std::thread([this](){
        this->workerThreadFunc();
    });
}


void BaseWorker::stop()
{
    {
        lock l(_mutex);
        if (!_running) return;
        Flags flags = _flags.load();
        flags[STOP_WORKING] = true;
        _flags.store(flags);
    }
    _thread.join();
}


void BaseWorker::processInterrupt()
{
    cerr << "processInterrupt()     " << *this << endl;

    // Load flags again, now that we have the lock...
    // since this is infrequent, it's OK.
    Flags flags = _flags.load();

    // If set, process STOP_WORKING flag, and reset it.
    if (flags[STOP_WORKING]) {
        flags.reset(STOP_WORKING);
        _running = false;
    }

    // Ensure there were no flags left unprocessed.
    assert(flags.none());

    // Now that we've finished processing, store the current set of flags.
    _flags.store(flags);
}


void BaseWorker::printOn(std::ostream& stream) const
{
    Flags flags = _flags.load();
    stream << "Worker" << flags << " !!";
}


// Print out worker flags in better way than as a simple bitset.
std::ostream& operator<<(std::ostream& stream, const BaseWorker::Flags& flags) {
    if (flags.none()) {
        stream << "<no flags>";
        return stream;
    }

    stream << "<flags: ";

    if (flags[BaseWorker::STOP_WORKING]) {
        stream << "STOP_WORKING";
    }

    stream << ">";
    return stream;
}

// Print out worker... delegate to printOn().
std::ostream& operator<<(std::ostream& stream, const BaseWorker& worker) {
    worker.printOn(stream);
    return stream;
}


}}}  // schwa::job::impl

