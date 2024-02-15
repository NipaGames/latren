#pragma once

#include "component/uicomponent.h"

#include <latren/defines/opengl.h>
#include <latren/graphics/shader.h>
#include <latren/graphics/shape.h>
#include <latren/graphics/material.h>
#include <latren/entity/mempool.h>

#include <map>
#include <memory>

namespace UI {
    enum class CanvasBackgroundVerticalAnchor {
        OVER,
        UNDER
    };

    class UIComponent;
    struct UIComponentContainer {
    private:
        std::vector<std::shared_ptr<UIComponent>> owned_;
        std::vector<ComponentReference<UIComponent>> referenced_;
    public:
        void Add(const std::shared_ptr<UIComponent>& c) {
            owned_.push_back(c);
        }
        void Add(const ComponentReference<UIComponent>& c) {
            referenced_.push_back(c);
        }
        bool Remove(const std::shared_ptr<UIComponent>& c) {
            const auto it = std::find(owned_.begin(), owned_.end(), c);
            if (it == owned_.end())
                return false;
            owned_.erase(it);
            return true;
        }
        bool Remove(const ComponentReference<UIComponent>& c) {
            const auto it = std::find(referenced_.begin(), referenced_.end(), c);
            if (it == referenced_.end())
                return false;
            referenced_.erase(it);
            return true;
        }
        void Clear() {
            owned_.clear();
            referenced_.clear();
        }
        void ForEachOwned(const std::function<void(UIComponent&)>& fn) {
            for (const std::shared_ptr<UIComponent>& c : owned_) {
                fn(*c);
            }
        }
        void ForEachReferenced(const std::function<void(UIComponent&)>& fn) {
            for (ComponentReference<UIComponent>& c : referenced_) {
                fn(c);
            }
        }
        void ForEach(const std::function<void(UIComponent&)>& fn) {
            ForEachOwned(fn);
            ForEachReferenced(fn);
        }
    };

    class Canvas {
    private:
        template <typename T>
        void _AddUIComponent(const T& c, int p) {
            components_[p].Add(c);
            ++componentCount_;
        }
        template <typename T>
        void _RemoveUIComponent(const T& c) {
            for (auto& [p, components] : components_) {
                if (components.Remove(c)) {
                    --componentCount_;
                    return;
                }
            }
        }
    protected:
        // the key represents the priority
        std::map<int, UIComponentContainer> components_;
        size_t componentCount_;
        Shape bgShape_;
        void UpdateComponentsOnWindowSize(float);
    public:
        bool isOwnedByRenderer = false;
        bool isVisible = true;
        glm::vec2 offset = glm::vec2(0.0f);
        std::shared_ptr<Material> bgMaterial = nullptr;
        glm::vec2 bgSize = glm::vec2(1280.0f, 720.0f);
        bool bgOverflow = true;
        CanvasBackgroundVerticalAnchor bgVerticalAnchor = CanvasBackgroundVerticalAnchor::UNDER;
        LATREN_API virtual void GenerateBackgroundShape();
        LATREN_API virtual void Draw();

        void AddUIComponent(const std::shared_ptr<UIComponent>& c, int p = 0) { _AddUIComponent(c, p); };
        void RemoveUIComponent(const std::shared_ptr<UIComponent>& c) { _RemoveUIComponent(c); }
        void AddUIComponent(const ComponentReference<UIComponent>& c, int p = 0) { _AddUIComponent(c, p); };
        void RemoveUIComponent(const ComponentReference<UIComponent>& c) { _RemoveUIComponent(c); }

        LATREN_API void ClearComponents();
        LATREN_API size_t GetComponentCount() const;
        LATREN_API virtual void UpdateWindowSize(int, int);
        LATREN_API virtual glm::mat4 GetProjectionMatrix() const;
        LATREN_API virtual glm::vec2 GetOffset() const;
        LATREN_API Canvas* GetCanvas();
    };
};