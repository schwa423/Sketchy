#include "job01/queue.h"


// schwa::job01 ===============================================================
namespace schwa { namespace job01 {


// Quick sanity check... ensure that _head and _tail values
// are consistent with _count.
void Queue::sanityCheck() {
    assert(_count >= 0);
    switch (_count) {
        case 0:
            assert(_head == nullptr && _tail == nullptr);
            break;
        case 1:
            assert(_head == _tail && _head != nullptr);
            break;
        default:
            assert(_head != nullptr && _tail != nullptr && _head != _tail);
    }
}


}}  // schwa::job01 ===========================================================