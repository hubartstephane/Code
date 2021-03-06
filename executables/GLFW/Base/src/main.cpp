
#include <chaos/Chaos.h> 

uint64_t render_stamp = 0;

chaos::shared_ptr<chaos::GPUFence> render_fence;

class WindowOpenGLTest : public chaos::Window
{
	CHAOS_DECLARE_OBJECT_CLASS2(WindowOpenGLTest, chaos::Window);

protected:

	virtual bool OnDraw(chaos::GPURenderer * renderer, chaos::box2 const & viewport, glm::ivec2 window_size) override
	{
		
		uint64_t ts = renderer->GetTimestamp();

		if (render_fence == nullptr)
		{
			render_fence = renderer->GetCurrentFrameFence();
			render_stamp = ts;
		}
		else
		{
			if (render_fence->WaitForCompletion(0.0f))
			{
				uint64_t dt = ts - render_stamp;
				render_fence = nullptr;
				render_stamp = 0;
			}
		}

		glm::vec4 clear_color(0.1f, 0.0f, 0.0f, 0.0f);
		glClearBufferfv(GL_COLOR, 0, (GLfloat*)&clear_color);

		float far_plane = 1000.0f;
		glClearBufferfi(GL_DEPTH_STENCIL, 0, far_plane, 0);
		return true;
	}

	virtual void TweakHints(chaos::WindowHints & hints, GLFWmonitor * monitor, bool pseudo_fullscreen) const override
	{
		chaos::Window::TweakHints(hints, monitor, pseudo_fullscreen);

	//	hints.toplevel = 1;
		hints.decorated = 1;
		hints.resizable = 1;
	}

	virtual void OnIconifiedStateChange(bool iconified)
	{
		std::cout << "OnIconifiedStateChange : " << iconified << std::endl;
	}

	virtual void OnFocusStateChange(bool gain_focus)
	{
		std::cout << "OnFocusStateChange : " << gain_focus << std::endl;
	}
};

int CHAOS_MAIN(int argc, char ** argv, char ** env)
{
	chaos::WindowParams params;
	params.monitor = nullptr;
	params.width = 300;
	params.height = 300;
	params.monitor_index = 0;

	chaos::WindowHints hints;

	return chaos::RunWindowApplication<WindowOpenGLTest>(argc, argv, env, params, hints);
}

