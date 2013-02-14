// Inline function definitions requred by queue.h.

// Add job to the queue.
template <class JobT>
void SimpleQueue<JobT>::add(JobT* job) {
    assert(job != nullptr);
    assert(job->nextLink() == nullptr);

    if (_count > 0) {
        // The queue is not empty, so link in the new job.
        this->link(_head, job);
        _head = job;
        _count++;
    } else {
        // The queue is empty, so the new job is both the head and tail.
        _head = _tail = job;
        _count = 1;
    }
}


// Pull the next job from the queue, or nullptr.
template <class JobT>
JobT* SimpleQueue<JobT>::next() {
    switch(_count) {
        case 0: {
            assert(_head == nullptr && _tail == nullptr);
            return nullptr;
        }
        case 1: {
            assert(_head == _tail);
            assert(_head->nextLink() == nullptr);
            auto result = _tail;
            _tail = _head = nullptr;
            _count = 0;
            return result;
        }
        default: {
            assert(_head != _tail);
            auto result = _tail;
            _tail = this->unlink(_tail);
            --_count;
            return result;
        }
    }
}


// Attempt to pull a JobChain of the desired length from the queue.
// If the queue has too few elements, return the biggest chain possible.
template <class JobT>
SimpleQueue<JobT> SimpleQueue<JobT>::next(int desiredLength) {
    assert(desiredLength > 0);
    switch(_count) {
        case 0: {
            assert(_head == nullptr && _tail == nullptr);
            return SimpleQueue<JobT>(nullptr, nullptr, 0);
        }
        case 1: {
            assert(_head == _tail);
            assert(_head->nextLink() == nullptr);
            auto result = _tail;
            _tail = _head = nullptr;
            _count = 0;
            return SimpleQueue<JobT>(result, result, 1);
        }
        default: {
            // If there won't be extra elements left over, just take 'em all.
            if (desiredLength >= _count) {
                auto head = _head;
                auto tail = _tail;
                int count = _count;

                _tail = _head = nullptr;
                _count = 0;

                return SimpleQueue<JobT>(head, tail, count);
            }
            // Otherwise, there are more than enough enqueued elements,
            // so walk through them and detach the desired number.
            else {
                int count = 1;
                auto head = _tail;
                auto tail = _tail;

                while (count < desiredLength) {
                    head = head->nextLink();
                    count++;
                }

                _count -= count;
                _tail = head->nextLink();
                this->unlink(head);

                return SimpleQueue<JobT>(head, tail, count);
            }
        }
    }
}



