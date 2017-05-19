/************************************************************************************

Authors     :   Bradley Austin Davis <bdavis@saintandreas.org>
Copyright   :   Copyright Brad Davis. All Rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************************/


#include <iostream>
#include <memory>
#include <exception>
#include <algorithm>

#include <Windows.h>

#define __STDC_FORMAT_MACROS 1

#define FAIL(X) throw std::runtime_error(X)

///////////////////////////////////////////////////////////////////////////////
//
// GLM is a C++ math library meant to mirror the syntax of GLSL 
//

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

// Import the most commonly used types into the default namespace
using glm::ivec3;
using glm::ivec2;
using glm::uvec2;
using glm::mat3;
using glm::mat4;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::quat;

///////////////////////////////////////////////////////////////////////////////
//
// GLEW gives cross platform access to OpenGL 3.x+ functionality.  
//

#include <GL/glew.h>

bool checkFramebufferStatus(GLenum target = GL_FRAMEBUFFER) {
	GLuint status = glCheckFramebufferStatus(target);
	switch (status) {
	case GL_FRAMEBUFFER_COMPLETE:
		return true;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		std::cerr << "framebuffer incomplete attachment" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		std::cerr << "framebuffer missing attachment" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		std::cerr << "framebuffer incomplete draw buffer" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		std::cerr << "framebuffer incomplete read buffer" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		std::cerr << "framebuffer incomplete multisample" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
		std::cerr << "framebuffer incomplete layer targets" << std::endl;
		break;

	case GL_FRAMEBUFFER_UNSUPPORTED:
		std::cerr << "framebuffer unsupported internal format or image" << std::endl;
		break;

	default:
		std::cerr << "other framebuffer error" << std::endl;
		break;
	}

	return false;
}

bool checkGlError() {
	GLenum error = glGetError();
	if (!error) {
		return false;
	}
	else {
		switch (error) {
		case GL_INVALID_ENUM:
			std::cerr << ": An unacceptable value is specified for an enumerated argument.The offending command is ignored and has no other side effect than to set the error flag.";
			break;
		case GL_INVALID_VALUE:
			std::cerr << ": A numeric argument is out of range.The offending command is ignored and has no other side effect than to set the error flag";
			break;
		case GL_INVALID_OPERATION:
			std::cerr << ": The specified operation is not allowed in the current state.The offending command is ignored and has no other side effect than to set the error flag..";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			std::cerr << ": The framebuffer object is not complete.The offending command is ignored and has no other side effect than to set the error flag.";
			break;
		case GL_OUT_OF_MEMORY:
			std::cerr << ": There is not enough memory left to execute the command.The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
			break;
		case GL_STACK_UNDERFLOW:
			std::cerr << ": An attempt has been made to perform an operation that would cause an internal stack to underflow.";
			break;
		case GL_STACK_OVERFLOW:
			std::cerr << ": An attempt has been made to perform an operation that would cause an internal stack to overflow.";
			break;
		}
		return true;
	}
}

void glDebugCallbackHandler(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, GLvoid* data) {
	OutputDebugStringA(msg);
	std::cout << "debug call: " << msg << std::endl;
}

//////////////////////////////////////////////////////////////////////
//
// GLFW provides cross platform window creation
//

#include <GLFW/glfw3.h>

namespace glfw {
	inline GLFWwindow * createWindow(const uvec2 & size, const ivec2 & position = ivec2(INT_MIN)) {
		GLFWwindow * window = glfwCreateWindow(size.x, size.y, "glfw", nullptr, nullptr);
		if (!window) {
			FAIL("Unable to create rendering window");
		}
		if ((position.x > INT_MIN) && (position.y > INT_MIN)) {
			glfwSetWindowPos(window, position.x, position.y);
		}
		return window;
	}
}

// A class to encapsulate using GLFW to handle input and render a scene
class GlfwApp {

protected:
	uvec2 windowSize;
	ivec2 windowPosition;
	GLFWwindow * window{ nullptr };
	unsigned int frame{ 0 };

public:
	GlfwApp() {
		// Initialize the GLFW system for creating and positioning windows
		if (!glfwInit()) {
			FAIL("Failed to initialize GLFW");
		}
		glfwSetErrorCallback(ErrorCallback);
	}

	virtual ~GlfwApp() {
		if (nullptr != window) {
			glfwDestroyWindow(window);
		}
		glfwTerminate();
	}

	virtual int run() {
		preCreate();

		window = createRenderingTarget(windowSize, windowPosition);

		if (!window) {
			std::cout << "Unable to create OpenGL window" << std::endl;
			return -1;
		}

		postCreate();

		initGl();

		while (!glfwWindowShouldClose(window)) {
			++frame;
			glfwPollEvents();
			update();
			draw();
			finishFrame();
		}

		shutdownGl();

		return 0;
	}


protected:
	virtual GLFWwindow * createRenderingTarget(uvec2 & size, ivec2 & pos) = 0;

	virtual void draw() = 0;

	void preCreate() {
		glfwWindowHint(GLFW_DEPTH_BITS, 16);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
	}


	void postCreate() {
		glfwSetWindowUserPointer(window, this);
		glfwSetKeyCallback(window, KeyCallback);
		glfwSetMouseButtonCallback(window, MouseButtonCallback);
		glfwMakeContextCurrent(window);

		// Initialize the OpenGL bindings
		// For some reason we have to set this experminetal flag to properly
		// init GLEW if we use a core context.
		glewExperimental = GL_TRUE;
		if (0 != glewInit()) {
			FAIL("Failed to initialize GLEW");
		}
		glGetError();

		if (GLEW_KHR_debug) {
			GLint v;
			glGetIntegerv(GL_CONTEXT_FLAGS, &v);
			if (v & GL_CONTEXT_FLAG_DEBUG_BIT) {
				//glDebugMessageCallback(glDebugCallbackHandler, this);
			}
		}
	}

	virtual void initGl() {
	}

	virtual void shutdownGl() {
	}

	virtual void finishFrame() {
		glfwSwapBuffers(window);
	}

	virtual void destroyWindow() {
		glfwSetKeyCallback(window, nullptr);
		glfwSetMouseButtonCallback(window, nullptr);
		glfwDestroyWindow(window);
	}

	virtual void onKey(int key, int scancode, int action, int mods) {
		if (GLFW_PRESS != action) {
			return;
		}

		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, 1);
			return;
		}
	}

	virtual void update() {}

	virtual void onMouseButton(int button, int action, int mods) {}

protected:
	virtual void viewport(const ivec2 & pos, const uvec2 & size) {
		glViewport(pos.x, pos.y, size.x, size.y);
	}

private:

	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		GlfwApp * instance = (GlfwApp *)glfwGetWindowUserPointer(window);
		instance->onKey(key, scancode, action, mods);
	}

	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		GlfwApp * instance = (GlfwApp *)glfwGetWindowUserPointer(window);
		instance->onMouseButton(button, action, mods);
	}

	static void ErrorCallback(int error, const char* description) {
		FAIL(description);
	}
};

//////////////////////////////////////////////////////////////////////
//
// The Oculus VR C API provides access to information about the HMD
//

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>
#include "shader.h"
#include "ScreenQuad.h"
#include "SkyBox.h"
#include "Line.h"

namespace ovr {

	// Convenience method for looping over each eye with a lambda
	template <typename Function>
	inline void for_each_eye(Function function) {
		for (ovrEyeType eye = ovrEyeType::ovrEye_Left;
			eye < ovrEyeType::ovrEye_Count;
			eye = static_cast<ovrEyeType>(eye + 1)) {
			function(eye);
		}
	}

	inline mat4 toGlm(const ovrMatrix4f & om) {
		return glm::transpose(glm::make_mat4(&om.M[0][0]));
	}

	inline mat4 toGlm(const ovrFovPort & fovport, float nearPlane = 0.01f, float farPlane = 10000.0f) {
		return toGlm(ovrMatrix4f_Projection(fovport, nearPlane, farPlane, true));
	}

	inline vec3 toGlm(const ovrVector3f & ov) {
		return glm::make_vec3(&ov.x);
	}

	inline vec2 toGlm(const ovrVector2f & ov) {
		return glm::make_vec2(&ov.x);
	}

	inline uvec2 toGlm(const ovrSizei & ov) {
		return uvec2(ov.w, ov.h);
	}

	inline quat toGlm(const ovrQuatf & oq) {
		return glm::make_quat(&oq.x);
	}

	inline mat4 toGlm(const ovrPosef & op) {
		mat4 orientation = glm::mat4_cast(toGlm(op.Orientation));
		mat4 translation = glm::translate(mat4(), ovr::toGlm(op.Position));
		return translation * orientation;
	}

	inline ovrMatrix4f fromGlm(const mat4 & m) {
		ovrMatrix4f result;
		mat4 transposed(glm::transpose(m));
		memcpy(result.M, &(transposed[0][0]), sizeof(float) * 16);
		return result;
	}

	inline ovrVector3f fromGlm(const vec3 & v) {
		ovrVector3f result;
		result.x = v.x;
		result.y = v.y;
		result.z = v.z;
		return result;
	}

	inline ovrVector2f fromGlm(const vec2 & v) {
		ovrVector2f result;
		result.x = v.x;
		result.y = v.y;
		return result;
	}

	inline ovrSizei fromGlm(const uvec2 & v) {
		ovrSizei result;
		result.w = v.x;
		result.h = v.y;
		return result;
	}

	inline ovrQuatf fromGlm(const quat & q) {
		ovrQuatf result;
		result.x = q.x;
		result.y = q.y;
		result.z = q.z;
		result.w = q.w;
		return result;
	}
}

class RiftManagerApp {
protected:
	ovrSession _session;
	ovrHmdDesc _hmdDesc;
	ovrGraphicsLuid _luid;

public:
	RiftManagerApp() {
		if (!OVR_SUCCESS(ovr_Create(&_session, &_luid))) {
			FAIL("Unable to create HMD session");
		}

		_hmdDesc = ovr_GetHmdDesc(_session);
	}

	~RiftManagerApp() {
		ovr_Destroy(_session);
		_session = nullptr;
	}
};

class RiftApp : public GlfwApp, public RiftManagerApp {
public:

private:
	GLuint _fbo{ 0 };
	GLuint _depthBuffer{ 0 };
	ovrTextureSwapChain _eyeTexture;

	GLuint _mirrorFbo{ 0 };
	ovrMirrorTexture _mirrorTexture;

	ovrEyeRenderDesc _eyeRenderDescs[2];

	mat4 _eyeProjections[2];

	ovrLayerEyeFov _sceneLayer;
	ovrViewScaleDesc _viewScaleDesc;

	uvec2 _renderTargetSize;
	uvec2 _mirrorSize;

	ovrInputState inputState;
	bool pressA, pressB, pressX, pressTrig = false;
	bool isFrozen = false;
	ovrSizei myEyeL, myEyeR;
	ScreenQuad * screen;
	ScreenQuad * screen2;
	ScreenQuad * screen3;
	ScreenQuad * screenR;
	ScreenQuad * screen2R;
	ScreenQuad * screen3R;
	Line * leftBottomL;
	Line * leftTopL;
	Line * frontBottomL;
	Line * frontTopL;
	Line * rightBottomL;
	Line * rightTopL;
	Line * backBottomL;
	Line * leftBottomR;
	Line * leftTopR;
	Line * frontBottomR;
	Line * frontTopR;
	Line * rightBottomR;
	Line * rightTopR;
	Line * backBottomR;
	GLuint screenShader, skyShader, blankShader;
	SkyBox *custom;
	int screenFailure = 0;
	mat4 sceneL = mat4(1.0f);
	mat4 sceneR = mat4(1.0f);

public:

	RiftApp() {
		using namespace ovr;
		
		_viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;

		memset(&_sceneLayer, 0, sizeof(ovrLayerEyeFov));
		_sceneLayer.Header.Type = ovrLayerType_EyeFov;
		_sceneLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;

		ovr::for_each_eye([&](ovrEyeType eye) {
			ovrEyeRenderDesc& erd = _eyeRenderDescs[eye] = ovr_GetRenderDesc(_session, eye, _hmdDesc.DefaultEyeFov[eye]);
			ovrMatrix4f ovrPerspectiveProjection =
				ovrMatrix4f_Projection(erd.Fov, 0.01f, 1000.0f, ovrProjection_ClipRangeOpenGL);
			_eyeProjections[eye] = ovr::toGlm(ovrPerspectiveProjection);
			_viewScaleDesc.HmdToEyeOffset[eye] = erd.HmdToEyeOffset;
			ovrFovPort & fov = _sceneLayer.Fov[eye] = _eyeRenderDescs[eye].Fov;
			auto eyeSize = ovr_GetFovTextureSize(_session, eye, fov, 1.0f);
			if (eye == ovrEye_Left) 
				myEyeL = eyeSize;
			else myEyeR = eyeSize;
			_sceneLayer.Viewport[eye].Size = eyeSize;
			_sceneLayer.Viewport[eye].Pos = { (int)_renderTargetSize.x, 0 };

			_renderTargetSize.y = std::max(_renderTargetSize.y, (uint32_t)eyeSize.h);
			_renderTargetSize.x += eyeSize.w;
		});
		// Make the on screen window 1/4 the resolution of the render target
		_mirrorSize = _renderTargetSize;
		_mirrorSize /= 4;
	}

protected:
	GLFWwindow * createRenderingTarget(uvec2 & outSize, ivec2 & outPosition) override {
		return glfw::createWindow(_mirrorSize);
	}

	void initGl() override {
		GlfwApp::initGl();

		// Disable the v-sync for buffer swap
		glfwSwapInterval(0);

		ovrTextureSwapChainDesc desc = {};
		desc.Type = ovrTexture_2D;
		desc.ArraySize = 1;
		desc.Width = _renderTargetSize.x;
		desc.Height = _renderTargetSize.y;
		desc.MipLevels = 1;
		desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		desc.SampleCount = 1;
		desc.StaticImage = ovrFalse;
		ovrResult result = ovr_CreateTextureSwapChainGL(_session, &desc, &_eyeTexture);
		_sceneLayer.ColorTexture[0] = _eyeTexture;
		if (!OVR_SUCCESS(result)) {
			FAIL("Failed to create swap textures");
		}

		int length = 0;
		result = ovr_GetTextureSwapChainLength(_session, _eyeTexture, &length);
		if (!OVR_SUCCESS(result) || !length) {
			FAIL("Unable to count swap chain textures");
		}
		for (int i = 0; i < length; ++i) {
			GLuint chainTexId;
			ovr_GetTextureSwapChainBufferGL(_session, _eyeTexture, i, &chainTexId);
			glBindTexture(GL_TEXTURE_2D, chainTexId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		glBindTexture(GL_TEXTURE_2D, 0);

		// Set up the framebuffer object
		glGenFramebuffers(1, &_fbo);
		glGenRenderbuffers(1, &_depthBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
		glBindRenderbuffer(GL_RENDERBUFFER, _depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _renderTargetSize.x, _renderTargetSize.y);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		ovrMirrorTextureDesc mirrorDesc;
		memset(&mirrorDesc, 0, sizeof(mirrorDesc));
		mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		mirrorDesc.Width = _mirrorSize.x;
		mirrorDesc.Height = _mirrorSize.y;
		if (!OVR_SUCCESS(ovr_CreateMirrorTextureGL(_session, &mirrorDesc, &_mirrorTexture))) {
			FAIL("Could not create mirror texture");
		}
		glGenFramebuffers(1, &_mirrorFbo);
		screenShader = LoadShaders("../Minimal/screenShader.vert", "../Minimal/screenShader.frag");
		skyShader = LoadShaders("../Minimal/shader.vert", "../Minimal/shader.frag");
		blankShader = LoadShaders("../Minimal/screenShader.vert", "../Minimal/blankShader.frag");
		screen = new ScreenQuad(1);
		screen2 = new ScreenQuad(2);
		screen3 = new ScreenQuad(3);
		screenR = new ScreenQuad(1);
		screen2R = new ScreenQuad(2);
		screen3R = new ScreenQuad(3);
		custom = new SkyBox(3);

		// Initializing Lines for wireframe
		leftBottomL = new Line(screen->getVertex(0));
		frontBottomL = new Line(screen->getVertex(1));
		leftTopL = new Line(screen->getVertex(2));
		frontTopL = new Line(screen->getVertex(3));
		rightBottomL = new Line(screen2->getVertex(1));
		rightTopL = new Line(screen2->getVertex(3));
		backBottomL = new Line(screen3->getVertex(2));
		leftBottomR = new Line(screenR->getVertex(0));
		frontBottomR = new Line(screenR->getVertex(1));
		leftTopR = new Line(screenR->getVertex(2));
		frontTopR = new Line(screenR->getVertex(3));
		rightBottomR = new Line(screen2R->getVertex(1));
		rightTopR = new Line(screen2R->getVertex(3));
		backBottomR = new Line(screen3R->getVertex(2));
		leftBottomR->colorRed();
		frontBottomR->colorRed();
		leftTopR->colorRed();
		frontTopR->colorRed();
		rightBottomR->colorRed();
		rightTopR->colorRed();
		backBottomR->colorRed();
	}

	void onKey(int key, int scancode, int action, int mods) override {
		if (GLFW_PRESS == action) switch (key) {
		case GLFW_KEY_R:
			ovr_RecenterTrackingOrigin(_session);
			return;
		}

		GlfwApp::onKey(key, scancode, action, mods);
	}

	void draw() final override {
		ovrPosef eyePoses[2];
		ovr_GetEyePoses(_session, frame, true, _viewScaleDesc.HmdToEyeOffset, eyePoses, &_sceneLayer.SensorSampleTime);

		double displayMidpointSeconds = ovr_GetPredictedDisplayTime(_session, frame);
		ovrTrackingState trackState = ovr_GetTrackingState(_session, displayMidpointSeconds, ovrTrue);
		//ovrPosef leftHandPose = trackState.HandPoses[ovrHand_Left].ThePose;
		ovrPosef rightHandPose = trackState.HandPoses[ovrHand_Right].ThePose;
		float rightX = rightHandPose.Position.x;
		float rightY = rightHandPose.Position.y;
		float rightZ = rightHandPose.Position.z;

		if (OVR_SUCCESS(ovr_GetInputState(_session, ovrControllerType_Touch, &inputState))) {
			
			// viewpoint on right hand
			if (inputState.HandTrigger[ovrHand_Right] > 0.5f && !pressTrig) {
				std::cerr << "Middle Trigger Pressed\n";
				pressTrig = true;
			}
			else if (!(inputState.HandTrigger[ovrHand_Right] > 0.5f) && pressTrig) {
				std::cerr << "Middle Trigger Released\n";
				pressTrig = false;
			}

			// debug
			if (inputState.Buttons & ovrButton_A && !pressA) {
				std::cerr << "A Pressed\n";
				pressA = true;
			}
			else if (!(inputState.Buttons & ovrButton_A) && pressA) {
				std::cerr << "A Released\n";
				pressA = false;
			}

			// freeze viewpoint
			if (inputState.Buttons & ovrButton_B && !pressB) {
				std::cerr << "B Pressed\n";
				isFrozen = !isFrozen;
				pressB = true;
			}
			else if (!(inputState.Buttons & ovrButton_B) && pressB) {
				std::cerr << "B Released\n";
				pressB = false;
			}

			if (inputState.Buttons & ovrButton_X && !pressX) {
				std::cerr << "X Pressed\n";
				pressX = true;
				screenFailure = (std::rand() % 6) + 1;
			}
			else if (!(inputState.Buttons & ovrButton_X) && pressX) {
				std::cerr << "X Released\n";
				pressX = false;
				screenFailure = 0;
			}

			// left/right
			if (inputState.Thumbstick[ovrHand_Left].x > 0.6f) {
				moveLittleBox(vec3(0.01f, 0.0f, 0.0f));
			}
			else if (inputState.Thumbstick[ovrHand_Left].x < -0.6f) {
				moveLittleBox(vec3(-0.01f, 0.0f, 0.0f));
			}

			// forward/backward
			if (inputState.Thumbstick[ovrHand_Left].y > 0.6f) {
				moveLittleBox(vec3(0.0f, 0.0f, -0.01f));
			}
			else if (inputState.Thumbstick[ovrHand_Left].y < -0.6f) {
				moveLittleBox(vec3(0.0f, 0.0f, 0.01f));
			}

			// cube size
			if (inputState.Thumbstick[ovrHand_Right].x > 0.6f) {
				changeScale(-1);
			}
			else if (inputState.Thumbstick[ovrHand_Right].x < -0.6f) {
				changeScale(1);
			}

			// up/down
			if (inputState.Thumbstick[ovrHand_Right].y > 0.6f) {
				moveLittleBox(vec3(0.0f, 0.01f, 0.0f));
			}
			else if (inputState.Thumbstick[ovrHand_Right].y < -0.6f) {
				moveLittleBox(vec3(0.0f, -0.01f, 0.0f));
			}
		}

		int curIndex;
		ovr_GetTextureSwapChainCurrentIndex(_session, _eyeTexture, &curIndex);
		GLuint curTexId;
		ovr_GetTextureSwapChainBufferGL(_session, _eyeTexture, curIndex, &curTexId);
		
		ovr::for_each_eye([&](ovrEyeType eye) {
			if (isFrozen) {
			}
			else if (pressTrig) {
				if (ovrEye_Left == eye) {
					sceneL = glm::mat4(1.0f);
					sceneL[3].x = rightX - 0.033f;
					sceneL[3].y = rightY;
					sceneL[3].z = rightZ;
				}
				else {
					sceneR = glm::mat4(1.0f);
					sceneR[3].x = rightX + 0.033f;
					sceneR[3].y = rightY;
					sceneR[3].z = rightZ;
				}
			}
			else {
				if (ovrEye_Left == eye) sceneL = ovr::toGlm(eyePoses[eye]);
				else sceneR = ovr::toGlm(eyePoses[eye]);
			}
			if (pressA) {
				leftBottomL->update(sceneL[3].x, sceneL[3].y, sceneL[3].z);
				frontBottomL->update(sceneL[3].x, sceneL[3].y, sceneL[3].z);
				leftTopL->update(sceneL[3].x, sceneL[3].y, sceneL[3].z);
				frontTopL->update(sceneL[3].x, sceneL[3].y, sceneL[3].z);
				rightBottomL->update(sceneL[3].x, sceneL[3].y, sceneL[3].z);
				rightTopL->update(sceneL[3].x, sceneL[3].y, sceneL[3].z);
				backBottomL->update(sceneL[3].x, sceneL[3].y, sceneL[3].z);

				leftBottomR->update(sceneR[3].x, sceneR[3].y, sceneR[3].z);
				frontBottomR->update(sceneR[3].x, sceneR[3].y, sceneR[3].z);
				leftTopR->update(sceneR[3].x, sceneR[3].y, sceneR[3].z);
				frontTopR->update(sceneR[3].x, sceneR[3].y, sceneR[3].z);
				rightBottomR->update(sceneR[3].x, sceneR[3].y, sceneR[3].z);
				rightTopR->update(sceneR[3].x, sceneR[3].y, sceneR[3].z);
				backBottomR->update(sceneR[3].x, sceneR[3].y, sceneR[3].z);
			}
		});

		//LEFT EYE BUFFERS
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, screen->FramebufferName);

		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screen->renderedTexture, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ovr::for_each_eye([&](ovrEyeType eye) {
			const auto& vp = _sceneLayer.Viewport[eye];
			glViewport(0, 0, 1024, 1024);
			_sceneLayer.RenderPose[eye] = eyePoses[eye];

			if (ovrEye_Left == eye) renderScene(_eyeProjections[eye], sceneL, eye, vp, _fbo);

		});
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, screen2->FramebufferName);

		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screen2->renderedTexture, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ovr::for_each_eye([&](ovrEyeType eye) {
			const auto& vp = _sceneLayer.Viewport[eye];
			glViewport(0, 0, 1024, 1024);
			_sceneLayer.RenderPose[eye] = eyePoses[eye];

			if (ovrEye_Left == eye) renderScene(_eyeProjections[eye], sceneL, eye, vp, _fbo);

		});
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, screen3->FramebufferName);

		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screen3->renderedTexture, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ovr::for_each_eye([&](ovrEyeType eye) {
			const auto& vp = _sceneLayer.Viewport[eye];
			glViewport(0, 0, 1024, 1024);
			_sceneLayer.RenderPose[eye] = eyePoses[eye];

			if (ovrEye_Left == eye) renderScene(_eyeProjections[eye], sceneL, eye, vp, _fbo);

		});

		//RIGHT EYE BUFFERS
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, screenR->FramebufferName);

		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenR->renderedTexture, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ovr::for_each_eye([&](ovrEyeType eye) {
			const auto& vp = _sceneLayer.Viewport[eye];
			glViewport(0, 0, 1024, 1024);
			_sceneLayer.RenderPose[eye] = eyePoses[eye];

			if (ovrEye_Right == eye) renderScene(_eyeProjections[eye], sceneR, eye, vp, _fbo);

		});
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, screen2R->FramebufferName);

		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screen2R->renderedTexture, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ovr::for_each_eye([&](ovrEyeType eye) {
			const auto& vp = _sceneLayer.Viewport[eye];
			glViewport(0, 0, 1024, 1024);
			_sceneLayer.RenderPose[eye] = eyePoses[eye];

			if (ovrEye_Right == eye) renderScene(_eyeProjections[eye], sceneR, eye, vp, _fbo);

		});
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, screen3R->FramebufferName);

		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screen3R->renderedTexture, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ovr::for_each_eye([&](ovrEyeType eye) {
			const auto& vp = _sceneLayer.Viewport[eye];
			glViewport(0, 0, 1024, 1024);
			_sceneLayer.RenderPose[eye] = eyePoses[eye];

			if (ovrEye_Right == eye) renderScene(_eyeProjections[eye], sceneR, eye, vp, _fbo);
			
		});
		//glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ovr::for_each_eye([&](ovrEyeType eye) {
			const auto& vp = _sceneLayer.Viewport[eye];
			glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
			_sceneLayer.RenderPose[eye] = eyePoses[eye];
			if (eye == ovrEye_Left) {
				screen->draw(screenShader, blankShader, _eyeProjections[eye], glm::inverse(ovr::toGlm(eyePoses[eye])), (screenFailure == 1));
				screen2->draw(screenShader, blankShader, _eyeProjections[eye], glm::inverse(ovr::toGlm(eyePoses[eye])), (screenFailure == 2));
				screen3->draw(screenShader, blankShader, _eyeProjections[eye], glm::inverse(ovr::toGlm(eyePoses[eye])), (screenFailure == 3));
			}
			else {
				screenR->draw(screenShader, blankShader, _eyeProjections[eye], glm::inverse(ovr::toGlm(eyePoses[eye])), (screenFailure == 4));
				screen2R->draw(screenShader, blankShader, _eyeProjections[eye], glm::inverse(ovr::toGlm(eyePoses[eye])), (screenFailure == 5));
				screen3R->draw(screenShader, blankShader, _eyeProjections[eye], glm::inverse(ovr::toGlm(eyePoses[eye])), (screenFailure == 6));
			}
			if (pressA) {

				leftBottomL->draw(_eyeProjections[eye], glm::inverse(ovr::toGlm(eyePoses[eye])));
				frontBottomL->draw(_eyeProjections[eye], glm::inverse(ovr::toGlm(eyePoses[eye])));
				leftTopL->draw(_eyeProjections[eye], glm::inverse(ovr::toGlm(eyePoses[eye])));
				frontTopL->draw(_eyeProjections[eye], glm::inverse(ovr::toGlm(eyePoses[eye])));
				rightBottomL->draw(_eyeProjections[eye], glm::inverse(ovr::toGlm(eyePoses[eye])));
				rightTopL->draw(_eyeProjections[eye], glm::inverse(ovr::toGlm(eyePoses[eye])));
				backBottomL->draw(_eyeProjections[eye], glm::inverse(ovr::toGlm(eyePoses[eye])));

				leftBottomR->draw(_eyeProjections[eye], glm::inverse(ovr::toGlm(eyePoses[eye])));
				frontBottomR->draw(_eyeProjections[eye], glm::inverse(ovr::toGlm(eyePoses[eye])));
				leftTopR->draw(_eyeProjections[eye], glm::inverse(ovr::toGlm(eyePoses[eye])));
				frontTopR->draw(_eyeProjections[eye], glm::inverse(ovr::toGlm(eyePoses[eye])));
				rightBottomR->draw(_eyeProjections[eye], glm::inverse(ovr::toGlm(eyePoses[eye])));
				rightTopR->draw(_eyeProjections[eye], glm::inverse(ovr::toGlm(eyePoses[eye])));
				backBottomR->draw(_eyeProjections[eye], glm::inverse(ovr::toGlm(eyePoses[eye])));
			}
			custom->draw(skyShader, _eyeProjections[eye], glm::inverse(ovr::toGlm(eyePoses[eye])));
		});
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		ovr_CommitTextureSwapChain(_session, _eyeTexture);
		ovrLayerHeader* headerList = &_sceneLayer.Header;
		ovr_SubmitFrame(_session, frame, &_viewScaleDesc, &headerList, 1);

		GLuint mirrorTextureId;
		ovr_GetMirrorTextureBufferGL(_session, _mirrorTexture, &mirrorTextureId);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, _mirrorFbo);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTextureId, 0);
		glBlitFramebuffer(0, 0, _mirrorSize.x, _mirrorSize.y, 0, _mirrorSize.y, _mirrorSize.x, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR)
		{
			std::cerr << err << " A" << std::endl;
		}
	}

	//TODO Remove the vp and _fbo from the parameters
	virtual void renderScene(const glm::mat4 & projection, const glm::mat4 & headPose, ovrEyeType eye, ovrRecti vp, GLuint _fbo) = 0;
	virtual void changeScale(int direction) = 0;
	virtual void moveLittleBox(vec3 direction) = 0;
};

//////////////////////////////////////////////////////////////////////
//
// The remainder of this code is specific to the scene we want to 
// render.  I use oglplus to render an array of cubes, but your 
// application would perform whatever rendering you want
//



namespace Attribute {
	enum {
		Position = 0,
		TexCoord0 = 1,
		Normal = 2,
		Color = 3,
		TexCoord1 = 4,
		InstanceTransform = 5,
	};
}

#include "SkyBox.h"
//#include "shader.h"
//#include "ScreenQuad.h"

// a class for encapsulating building and rendering an RGB cube
struct Scene {
	int state = 0;
	int viewState = 0;
	mat4 view = mat4(1.0f);
	SkyBox *littleBox;
	SkyBox *left;
	SkyBox *right;
	
	GLuint shader;
	GLuint screenShader;
	float scaleFactor;

public:
	Scene() {
		shader = LoadShaders("../Minimal/shader.vert", "../Minimal/shader.frag");
		
		littleBox = new SkyBox(0);
		left = new SkyBox(1);
		right = new SkyBox(2);


		scaleFactor = .2f;
		littleBox->setScale(scaleFactor);
	}

	void changeScale(int direction) {
		if (scaleFactor > 0.00f && direction == -1) {
			scaleFactor -= .001f;
		}
		else if (scaleFactor < 1.00f && direction == 1) {
			scaleFactor += .001f;
		}
		else if (direction == 0) {
			scaleFactor = 0.2f;
		}
		littleBox->setScale(scaleFactor);
	}

	void moveLittleBox(vec3 direction) {
		littleBox->translate(direction);
	}

	void render(const mat4 & projection, const mat4 & modelview, ovrEyeType eye, ovrRecti vp, GLuint _fbo) {
		// Render to our framebuffer

			
			

		if (eye == ovrEye_Left) { left->draw(shader, projection, modelview); }
		else { right->draw(shader, projection, modelview); }
		littleBox->draw(shader, projection, modelview);

	}
};


// An example application that renders a simple cube
class Immersion : public RiftApp {
	std::shared_ptr<Scene> cubeScene;

public:
	Immersion() { }

protected:
	void initGl() override {
		RiftApp::initGl();
		glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
		glEnable(GL_DEPTH_TEST);
		ovr_RecenterTrackingOrigin(_session);
		cubeScene = std::shared_ptr<Scene>(new Scene());
	}

	void shutdownGl() override {
		//cubeScene.reset();
	}

	void changeScale(int direction) {
		cubeScene->changeScale(direction);
	}

	void moveLittleBox(vec3 direction) {
		cubeScene->moveLittleBox(direction);
	}

	void renderScene(const glm::mat4 & projection, const glm::mat4 & headPose, ovrEyeType eye, ovrRecti vp, GLuint _fbo) override {

		cubeScene->render(projection, glm::inverse(headPose), eye, vp, _fbo);
	}
};

// Execute our example class
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	int result = -1;

	AllocConsole();
	freopen("conin$", "r", stdin);
	freopen("conout$", "w", stdout);
	freopen("conout$", "w", stderr);

	try {
		if (!OVR_SUCCESS(ovr_Initialize(nullptr))) {
			FAIL("Failed to initialize the Oculus SDK");
		}
		result = Immersion().run();
	}
	catch (std::exception & error) {
		OutputDebugStringA(error.what());
		std::cerr << error.what() << std::endl;
	}
	ovr_Shutdown();
	return result;
}