#pragma once

#include "component.h"

template <class Forward>
class ForwardComponent : public RegisterComponent<Forward> {
private:
    bool _isForwarded = dynamic_cast<IComponent*>(static_cast<Forward*>(this))->ForwardType(typeid(Forward), [](const IComponent* c) {
        return new Forward(dynamic_cast<const Forward&>(*c));
    });
};

class TestComponent1 : public Component<TestComponent1> { };
class TestComponent2 : public TestComponent1 { };
class TestComponent3 : public TestComponent2, public ForwardComponent<TestComponent3> { };
class TestComponent4 : public TestComponent3, public ForwardComponent<TestComponent4> { };