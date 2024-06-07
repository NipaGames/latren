#pragma once

#include "component/uicomponent.h"

#include <latren/defines/opengl.h>
#include <latren/defines/wndmetrics.h>
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

    class LATREN_API Canvas {
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
    public:
        bool isOwnedByRenderer = false;
        bool isVisible = true;
        std::shared_ptr<Material> bgMaterial = nullptr;
        bool bgOverflow = true;
        CanvasBackgroundVerticalAnchor bgVerticalAnchor = CanvasBackgroundVerticalAnchor::UNDER;
        virtual void GenerateBackgroundShape();
        virtual void Draw();
        virtual void Update();
        virtual void UpdateInteractions(UIComponent&);

        template <typename T, typename = std::enable_if_t<std::is_base_of_v<UIComponent, T>>>
        void AddUIComponent(const SharedComponentPtr<T>& gen, int layer = 0) {
            auto c = SharedComponentPtrCast<UIComponent>(gen);
            c->Get().parent_ = this;
            _AddUIComponent(c, layer);
        };
        template <typename T, typename = std::enable_if_t<std::is_base_of_v<UIComponent, T>>>
        void RemoveUIComponent(const SharedComponentPtr<T>& gen) {
            auto c = SharedComponentPtrCast<UIComponent>(gen);
            c->Get().parent_ = nullptr;
            _RemoveUIComponent(c);
        }
        void AddUIComponent(ComponentReference<UIComponent>, int = 0);
        void RemoveUIComponent(ComponentReference<UIComponent>);

        void ClearComponents();
        size_t GetComponentCount() const;
        Canvas* GetCanvas();
        void BreakUpdates();
        
        virtual glm::mat4 GetProjectionMatrix() const;
        virtual const glm::vec2& GetMousePosition() const;

        virtual glm::vec2 GetOffset() const;
        virtual void SetOffset(const glm::vec2&);
        virtual const glm::vec2& GetBackgroundSize() const;
        virtual void SetBackgroundSize(const glm::vec2&);
    };
};