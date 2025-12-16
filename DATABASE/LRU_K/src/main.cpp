#include <iostream>
#include <memory>
#include "../include/lru_k.h"

int main() {
    // Create a Replacer with 10 frames, K = 2
    lru_k::LRUK lru_k(10, 2);

    // Scenario: Access frame 1, 2, 3
    lru_k.RecordAccess(1);
    lru_k.RecordAccess(2);
    lru_k.RecordAccess(3);
    lru_k.RecordAccess(1);
    lru_k.RecordAccess(1); // Frame 1 has > 2 accesses now

    // Set them as evictable
    lru_k.SetEvictable(1, true);
    lru_k.SetEvictable(2, true);
    lru_k.SetEvictable(3, true);

    std::cout << "Size: " << lru_k.Size() << std::endl;

    // Trigger Eviction
    // Expectation: Frame 2 or 3 should be evicted because they have history < K (infinite distance).
    // Frame 1 has history >= K (finite distance).
    // Between 2 and 3, 2 was accessed earlier, so 2 should go.
    
    auto victim = lru_k.Evict();
    if (victim.has_value()) {
        std::cout << "Evicted Frame: " << victim.value() << std::endl;
    } else {
        std::cout << "No victim found." << std::endl;
    }

    std::cout << "Size after eviction: " << lru_k.Size() << std::endl;

    return 0;
}