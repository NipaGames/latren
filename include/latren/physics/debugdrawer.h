#pragma once

#include <btBulletDynamicsCommon.h>
#include <vector>

#include <latren/graphics/shader.h>

namespace Physics {
    class DebugDrawer : public btIDebugDraw {
    private:
        int debugMode_ = btIDebugDraw::DBG_DrawWireframe;
        Shader shader_;
        GLuint vao_ = NULL;
        GLuint vbo_ = NULL;
        std::vector<float> vertices_;
    public:
        LATREN_API virtual ~DebugDrawer();
        LATREN_API DebugDrawer();
        LATREN_API virtual void drawLine(const btVector3&, const btVector3&, const btVector3&) override;
        LATREN_API virtual void setDebugMode(int debugMode) override { debugMode_ = debugMode; }
        LATREN_API virtual int getDebugMode() const override { return debugMode_; }
        LATREN_API virtual void drawContactPoint(const btVector3&, const btVector3&, btScalar, int, const btVector3&) override { }
	    LATREN_API virtual void draw3dText(const btVector3&, const char*) override { }
        LATREN_API virtual void clearLines() override;
        LATREN_API virtual void flushLines() override;
        LATREN_API virtual void reportErrorWarning(const char*) override;
    };
};