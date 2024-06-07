#include <latren/ui/canvaslayout.h>
#include <latren/systems.h>

using namespace UI;

void UI::CanvasLayout::Destroy() {
    ClearComponents();
    pages_.clear();
    if (renderer_ != nullptr)
        renderer_->RemoveCanvas(assignmentId_);
}

UI::CanvasLayout::~CanvasLayout() {
    Destroy();
}

void UI::CanvasLayout::SetCurrentPage(const std::string& page) {
    currentPage_ = page;
    BreakUpdates();
    for (const auto& [p, components] : pages_) {
        for (const auto& c : components) {
            c->Get().isVisible = false;
            c->Get().isActive = false;
        }
    }
    for (const auto& c : pages_[page]) {
        c->Get().isVisible = true;
        c->Get().isActive = true;
    }
}

CanvasLayout& UI::CanvasLayout::operator=(const CanvasLayout& c) {
    Canvas::operator=(c);
    if (renderer_ != nullptr)
        renderer_->RemoveCanvas(assignmentId_);
    renderer_ = c.renderer_;
    pages_ = c.pages_;
    currentPage_ = c.currentPage_;
    return *this;
}

void UI::CanvasLayout::AssignToRenderer(Renderer* renderer) {
    renderer_ = renderer;
    renderer_->AssignCanvas(assignmentId_, GetCanvas());
}

void UI::CanvasLayout::CloseCurrentPage() {
    currentPage_ = "";
    for (const auto& [p, components] : pages_) {
        for (const auto& c : components) {
            c->Get().isVisible = false;
            c->Get().isActive = false;
        }
    }
}

void UI::CanvasLayout::Start() {
    AssignToRenderer(&Systems::GetRenderer());
    for (auto& [p, layer] : components_) {
        layer.ForEachOwned([](UIComponent& c) {
            c.IStart();
        });
    }
}