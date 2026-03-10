#include <GL/glew.h>
#include <SDL_main.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_ttf.h>

#include <iostream>
#include <string>

#include "SquareMeshVbo.h"
#include "GameEngine.h"
#include "GameStateController.h"

using namespace std;

// ---------------------------------------------
// Global variables
// ---------------------------------------------
bool quit;
const GLint WIDTH = 1280, HEIGHT = 720;  // 1920 x 1080 , 1280 x 720
int currentFrame = 0, lastFrame = 0;

Uint32 mouseDownTime = 0;
bool mouseHeld = false;
int mouseX = 0, mouseY = 0;

SDL_Window* window = nullptr;
SDL_GLContext glContext;
SDL_Event sdlEvent;

GameEngine* engine = nullptr;
GameStateController* gameStateController = nullptr;
GameData* gameData = nullptr;

// Main function

int main(int argc, char* argv[])
{
	quit = false;

	// Request OpenGL 3.3 Core
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	// Initialize SDL video
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
		return false;
	}

	if (TTF_Init() < 0) { // Initialize SDL_ttf
		printf("Couldn't initialize SDL TTF: %s\\n", SDL_GetError());
		return false;
	}

	// Create the main window
	window = SDL_CreateWindow(
		"My Game",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WIDTH, HEIGHT,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
	);
	if (!window)
	{
		cout << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
		return false;
	}

	// Create OpenGL context
	glContext = SDL_GL_CreateContext(window);
	if (!glContext)
	{
		cout << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << endl;
		return false;
	}

	// Initialize GLEW
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK)
	{
		cout << "Error initializing GLEW! " << glewGetErrorString(glewError) << endl;
		return false;
	}

	// Enable VSync
	if (SDL_GL_SetSwapInterval(1) < 0)
	{
		cout << "Warning: Unable to set VSync! SDL Error: " << SDL_GetError() << endl;
	}

	// Initialize game engine
	gameData = GameData::GetInstance();
	engine = GameEngine::GetInstance();
	engine->Init(WIDTH, HEIGHT);

	gameStateController = new GameStateController();
	gameStateController->Init(GameState::GS_LEVEL1);

	// Game Loop
	while (GameData::GetInstance()->gGameStateCurr != GameState::GS_QUIT)
	{
		// Frame loop for current state
		while (GameData::GetInstance()->gGameStateCurr == GameData::GetInstance()->gGameStateNext)
		{
			currentFrame = SDL_GetTicks();
			GameEngine::GetInstance()->SetDeltaTime(currentFrame - lastFrame);

			// ---------------------------------------------
			// Event Handling
			// ---------------------------------------------
			while (SDL_PollEvent(&sdlEvent) != 0)
			{
				// Window close
				if (sdlEvent.type == SDL_QUIT)
				{
					GameData::GetInstance()->gGameStateNext = GameState::GS_QUIT;
				}

				// Keyboard input
				else if (sdlEvent.type == SDL_KEYDOWN)
				{
					switch (sdlEvent.key.keysym.sym)
					{
					case SDLK_LEFT:
					case SDLK_a:	gameStateController->currentLevel->HandleKey('a'); break;
					case SDLK_RIGHT:
					case SDLK_d:	gameStateController->currentLevel->HandleKey('d'); break;
					case SDLK_UP:
					case SDLK_w:	gameStateController->currentLevel->HandleKey('w'); break;
					case SDLK_DOWN:
					case SDLK_s:	gameStateController->currentLevel->HandleKey('s'); break;
					case SDLK_ESCAPE: gameStateController->currentLevel->HandleKey('q'); break;
					case SDLK_r: gameStateController->currentLevel->HandleKey('r'); break;
					case SDLK_e: gameStateController->currentLevel->HandleKey('e'); break;
					case SDLK_c: gameStateController->currentLevel->HandleKey('c'); break;
					case SDLK_x: gameStateController->currentLevel->HandleKey('x'); break;
					case SDLK_SPACE: gameStateController->currentLevel->HandleKey(' '); break;
					case SDLK_p: gameStateController->currentLevel->HandleKey('p'); break;
					case SDLK_o: gameStateController->currentLevel->HandleKey('o'); break;
					case SDLK_l: gameStateController->currentLevel->HandleKey('l'); break;
					case SDLK_v: gameStateController->currentLevel->HandleKey('v'); break;
					}
				}

				// Mouse pressed
				else if (sdlEvent.type == SDL_MOUSEBUTTONDOWN)
				{
					if (sdlEvent.button.button == SDL_BUTTON_LEFT)
					{
						mouseHeld = true;
						mouseDownTime = SDL_GetTicks();
						SDL_GetMouseState(&mouseX, &mouseY);
						gameStateController->currentLevel->HandleMouse(0, mouseX, mouseY);
					}
					else if (sdlEvent.button.button == SDL_BUTTON_RIGHT)
					{
						SDL_GetMouseState(&mouseX, &mouseY);
						gameStateController->currentLevel->HandleMouse(4, mouseX, mouseY);
					}
				}

				// Mouse released
				else if (sdlEvent.type == SDL_MOUSEBUTTONUP)
				{
					if (sdlEvent.button.button == SDL_BUTTON_LEFT)
					{
						mouseHeld = false;
						SDL_GetMouseState(&mouseX, &mouseY);
						gameStateController->currentLevel->HandleMouse(2, mouseX, mouseY);
					}
					else if (sdlEvent.button.button == SDL_BUTTON_RIGHT)
					{
						SDL_GetMouseState(&mouseX, &mouseY);
						gameStateController->currentLevel->HandleMouse(5, mouseX, mouseY);
					}
				}

				// Mouse moved (hover)
				else if (sdlEvent.type == SDL_MOUSEMOTION)
				{
					SDL_GetMouseState(&mouseX, &mouseY);
					gameStateController->currentLevel->HandleMouse(3, mouseX, mouseY);
				}
			}

			// ---------------------------------------------
			// Handle hold & drag
			// ---------------------------------------------
			if (mouseHeld)
			{
				Uint32 heldDuration = SDL_GetTicks() - mouseDownTime;
				if (heldDuration >= 250)
				{
					SDL_GetMouseState(&mouseX, &mouseY);
					gameStateController->currentLevel->HandleMouse(1, mouseX, mouseY);
				}
			}

			// ---------------------------------------------
			// Update and Render
			// ---------------------------------------------
			if (gameStateController->currentLevel)
			{
				gameStateController->currentLevel->LevelUpdate();
				
				SDL_GetMouseState(&mouseX, &mouseY);
				gameStateController->currentLevel->HandleMouse(3, mouseX, mouseY);
				
				gameStateController->currentLevel->LevelDraw();
			}
			
			SDL_GL_SwapWindow(window);
			lastFrame = currentFrame;
		}

		// State transition detected - handle it
		gameStateController->Update();
	}

	SDL_DestroyWindow(window);
	window = nullptr;
	SDL_Quit();
	TTF_Quit();

	return 0;
}
