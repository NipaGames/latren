#pragma once

#include "canvas.h"
#include "component/uicomponent.h"
#include <latren/graphics/renderer.h>

namespace UI {
    class  CanvasLayout : public Canvas {
    private:
        std::unordered_map<std::string, std::vector<SharedComponentPtr<UIComponent>>> pages_;
    protected:
        std::string currentPage_;
        Renderer* renderer_ = nullptr;
        const std::string assignmentId_;
    public:
        virtual ~CanvasLayout();
        CanvasLayout(const std::string& id) : assignmentId_(id) { }
        virtual CanvasLayout& operator=(const CanvasLayout&);
        virtual void AssignToRenderer(Renderer*);
        virtual void Destroy();
        virtual void CreateHUDElements() = 0;
        virtual void Start();
        virtual void SetCurrentPage(const std::string&);
        virtual void CloseCurrentPage();
        template <typename T>
        SharedComponentPtr<T> CreateUIComponent(const std::unordered_set<std::string>& pages = {}, int layer = 0) {
            SharedComponentPtr<T> component = ComponentWrapper<T>::CreateInstance();
            component->Get().transformFrom = UITransformFrom::UI_TRANSFORM;
            SharedComponentPtr<UIComponent> base = std::static_pointer_cast<ComponentWrapper<UIComponent>>(std::static_pointer_cast<IComponentWrapper>(component));
            for (const std::string& page : pages) {
                pages_[page].push_back(base);
            }
            AddUIComponent(base, layer);
            return component;
        }
    };
};