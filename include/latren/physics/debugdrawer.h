#pragma once

#include <btBulletDynamicsCommon.h>
#include <vector>

#include <latren/graphics/shader.h>

namespace Physics {
    class  DebugDrawer : public btIDebugDraw {
    private:
        int debugMode_ = btIDebugDraw::DBG_DrawWireframe;
        Shader shader_;
        GLuint vao_ = GL_NONE;
        GLuint vbo_ = GL_NONE;
        std::vector<float> vertices_;
    public:
        virtual ~DebugDrawer();
        DebugDrawer();
        virtual void drawLine(const btVector3&, const btVector3&, const btVector3&) override;
        virtual void clearLines() override;
        virtual void flushLines() override;
        virtual void reportErrorWarning(const char*) override;
        virtual void setDebugMode(int debugMode) override { debugMode_ = debugMode; }
        virtual int getDebugMode() const override { return debugMode_; }
        virtual void drawContactPoint(const btVector3&, const btVector3&, btScalar, int, const btVector3&) override { }
	    virtual void draw3dText(const btVector3&, const char*) override { }
    };
};