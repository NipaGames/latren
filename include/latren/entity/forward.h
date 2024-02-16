#pragma once

#include "component.h"

template <class Forward>
class ForwardComponent : public RegisterComponent<Forward> {
private:
    bool _isForwarded = dynamic_cast<IComponent*>(static_cast<Forward*>(this))->ForwardType(typeid(Forward));
};