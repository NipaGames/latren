#include <latren/ec/serializable.h>

#include <queue>

std::queue<ISerializable*> GLOBAL_SERIALIZABLE_QUEUE;
bool PUSH_TO_SERIALIZATION_QUEUE = false;

void GlobalSerialization::ToggleQueueing(bool toggle) {
    PopSerializables();
    PUSH_TO_SERIALIZATION_QUEUE = toggle;
}

void GlobalSerialization::PushSerializable(ISerializable* field) {
    if (PUSH_TO_SERIALIZATION_QUEUE)
        GLOBAL_SERIALIZABLE_QUEUE.push(field);
}

void GlobalSerialization::PopSerializables() {
    GLOBAL_SERIALIZABLE_QUEUE = { };
}

SerializableFieldMap GlobalSerialization::PopSerializables(const IComponent* parent) {
    SerializableFieldMap serializables;
    int i = 0;
    while (!GLOBAL_SERIALIZABLE_QUEUE.empty()) {
        ISerializable* field = GLOBAL_SERIALIZABLE_QUEUE.front();
        serializables.insert({
            field->GetName(),
            {
                static_cast<const char*>(field->GetPtr()) - reinterpret_cast<const char*>(parent),
                i++,
                field->GetType(),
                field->GetContainerType(),
                field->GetAssignmentOperator()
            }
        });
        GLOBAL_SERIALIZABLE_QUEUE.pop();
    }
    return serializables;
}