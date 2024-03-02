#pragma once

#include "component/uicomponent.h"

#include <latren/defines/opengl.h>
#include <latren/graphics/shader.h>
#include <latren/graphics/shape.h>
#include <latren/graphics/material.h>
#include <latren/ec/mempool.h>
#include <latren/ec/wrapper.h>

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
        std::vector<SharedComponentPtr<UIComponent>> owned_;
        std::vector<ComponentReference<UIComponent>> referenced_;
    public:
        void Add(const SharedComponentPtr<UIComponent>& c) {
            owned_.push_back(c);
        }
        void Add(const ComponentReference<UIComponent>& c) {
            referenced_.push_back(c);
        }
        bool Remove(const SharedComponentPtr<UIComponent>& c) {
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
            for (const std::shared_ptr<ComponentWrapper<UIComponent>>& c : owned_) {
                fn(c->component);
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
        bool breakUpdates_ = false;
        glm::vec2 mousePos_;
        glm::vec2 offset_ = glm::vec2(0.0f);
        glm::vec2 bgSize_ = glm::vec2(1280.0f, 720.0f);
        void UpdateComponentsOnWindowSize(float);
    public:
        bool isOwnedByRenderer = false;
        bool isVisible = true;
        std::shared_ptr<Material> bgMaterial = nullptr;
        bool bgOverflow = true;
        CanvasBackgroundVerticalAnchor bgVerticalAnchor = CanvasBackgroundVerticalAnchor::UNDER;
        LATREN_API virtual void GenerateBackgroundShape();
        LATREN_API virtual void Draw();
        LATREN_API virtual void Update();
        LATREN_API virtual void UpdateInteractions(UIComponent&);

        template <typename T, typename = std::enable_if_t<std::is_base_of_v<UIComponent, T>>>
        void AddUIComponent(const SharedComponentPtr<T>& gen, int p = 0) {
            auto c = SharedComponentPtrCast<UIComponent>(gen);
            c->Get().parent_ = this;
            _AddUIComponent(c, p);
        };
        template <typename T, typename = std::enable_if_t<std::is_base_of_v<UIComponent, T>>>
        void RemoveUIComponent(const SharedComponentPtr<T>& gen) {
            auto c = SharedComponentPtrCast<UIComponent>(gen);
            c->Get().parent = nullptr;
            _RemoveUIComponent(c);
        }

        void AddUIComponent(ComponentReference<UIComponent> c, int p = 0) {
            c->parent_ = this;
            _AddUIComponent(c, p);
        };
        void RemoveUIComponent(ComponentReference<UIComponent> c) {
            c->parent_ = nullptr;
            _RemoveUIComponent(c);
        }

        LATREN_API void ClearComponents();
        LATREN_API size_t GetComponentCount() const;
        LATREN_API Canvas* GetCanvas();
        LATREN_API void BreakUpdates();
        
        LATREN_API virtual void UpdateWindowSize(int, int);
        LATREN_API virtual glm::mat4 GetProjectionMatrix() const;
        LATREN_API virtual const glm::vec2& GetMousePosition() const;

        LATREN_API virtual glm::vec2 GetOffset() const;
        LATREN_API virtual void SetOffset(const glm::vec2&);
        LATREN_API virtual const glm::vec2& GetBackgroundSize() const;
        LATREN_API virtual void SetBackgroundSize(const glm::vec2&);
    };
};