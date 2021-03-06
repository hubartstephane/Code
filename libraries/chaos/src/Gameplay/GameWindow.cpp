#include <chaos/Chaos.h>

namespace chaos
{
	void GameWindow::OnInputModeChanged(InputMode new_mode, InputMode old_mode)
	{
		if (game != nullptr)
			game->OnInputModeChanged(new_mode, old_mode);
	}

	bool GameWindow::OnCharEventImpl(unsigned int c)
	{
		// give inputs to the game
		if (game != nullptr)
			if (game->OnCharEvent(c))
				return true;
		return Window::OnCharEventImpl(c);
	}

	bool GameWindow::OnKeyEventImpl(KeyEvent const& event)
	{
		// super method
		if (Window::OnKeyEventImpl(event))
			return true;
		// give inputs to the game
		if (game != nullptr)
			if (game->OnKeyEvent(event))
				return true;
		return false;
	}

	bool GameWindow::OnMouseButtonImpl(int button, int action, int modifier)
	{
		if (game != nullptr)
			if (game->OnMouseButton(button, action, modifier))
				return true;
		return Window::OnMouseButtonImpl(button, action, modifier);
	}

	bool GameWindow::OnMouseMoveImpl(double x, double y)
	{
		if (game != nullptr)
			if (game->OnMouseMove(x, y))
				return true;
		return Window::OnMouseMoveImpl(x, y);
	}

	box2 GameWindow::GetRequiredViewport(glm::ivec2 const & size) const
	{
		if (game != nullptr)
			return game->GetRequiredViewport(size);
		return Window::GetRequiredViewport(size);
	}

	bool GameWindow::OnDraw(GPURenderer * renderer, box2 const & viewport, glm::ivec2 window_size)
	{
		if (game != nullptr)
		{
			GPUProgramProvider provider;


			// shu48 ???
#if 1
			WindowApplication* application = Application::GetInstance();
			if (application != nullptr)
				if (application->GetTextureAtlas() != nullptr)
					provider.AddTexture("material", application->GetTextureAtlas()->GetTexture());
#endif

			GPURenderParams render_params;
			render_params.viewport = viewport;
			render_params.screen_size = window_size;
			game->Display(renderer, &provider, render_params);
		}
		return true;
	}

	void GameWindow::SetGame(Game* in_game)
	{
		game = in_game;
	}

	void GameWindow::Finalize()
	{
		game = nullptr;
	}

	CHAOS_HELP_TEXT(CMD, "-NoAutoPause");

	void GameWindow::OnIconifiedStateChange(bool iconified)
	{
		// do not execute following code in debug because it does not fit well with debugger
#if _DEBUG
		if (!Application::HasApplicationCommandLineFlag("-NoAutoPause"))
#endif
		if (iconified)
			game->RequirePauseGame();
	}

	void GameWindow::OnFocusStateChange(bool gain_focus)
	{
		// do not execute following code in debug because it does not fit well with debugger
#if _DEBUG
		if (!Application::HasApplicationCommandLineFlag("-NoAutoPause"))
#endif
		if (!gain_focus)
			game->RequirePauseGame();

	}

}; // namespace chaos

