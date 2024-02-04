#include <latren/util/idfactory.h>

CommonID IDFactory::NextID() {
    return counter_++;
}