#pragma once

#include "component/uicomponent.h"

#include <latren/defines/opengl.h>
#include <map>
#include <memory>
#include <latren/graphics/shader.h>
#include <latren/graphics/shape.h>
#include <latren/graphics/material.h>
#include <latren/entity/mempool.h>

namespace UI {
    enum class CanvasBackgroundVerticalAnchor {
        OVER,
        UNDER
    };
    class UIComponent;
    class Canvas {
    protected:
        std::map<int, std::vector<ComponentReference<UIComponent>>> components_;
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
        LATREN_API virtual void AddUIComponent(ComponentReference<UIComponent>, int = 0);
        LATREN_API virtual void RemoveUIComponent(const ComponentReference<UIComponent>&);
        LATREN_API virtual void UpdateWindowSize(int, int);
        LATREN_API virtual glm::mat4 GetProjectionMatrix() const;
        LATREN_API virtual glm::vec2 GetOffset() const;
        LATREN_API Canvas* GetCanvas();
    };
};