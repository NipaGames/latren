#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

typedef size_t ComponentType;

class IComponentMemoryPool {

};

template <typename T>
class ComponentMemoryPool {
private:
    std::vector<T> components_;

};

class MasterMemoryPool {
    // optimally all the pools would be stored contiguously but this is the best i can come up with
    // afaik std any doesn't store the memory itself
    std::unordered_map<ComponentType, std::unique_ptr<IComponentMemoryPool>> componentPools_;

};