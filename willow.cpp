#include <stdio.h>
#include <sdl.h>
#include <SDL_ttf.h>

#define ARRAY_COUNT(x) (sizeof((x))/(sizeof((x)[0])))

int screen_width = 800;
int screen_height = 600;

struct State {
	bool quit;
	TTF_Font* font;
	SDL_Renderer* renderer;
};

void log_sdl_error(const char* msg)
{
	printf("%s error: %s\n", msg, SDL_GetError());
}

void render_texture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y)
{
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
	SDL_RenderCopy(ren, tex, 0, &dst);
}

void render_screen(State* state)
{	
	//We need to first render to a surface as that's what TTF_RenderText
	//returns, then load that surface into a texture
	SDL_Color color = { 255, 255, 255, 255 };
	char* message = "Hello world!!!";
	SDL_Surface *surf = TTF_RenderText_Blended(state->font, message, color);
	if (!surf) 
	{
		TTF_CloseFont(state->font);
		log_sdl_error("TTF_RenderText");
		return;
	}
	SDL_Texture *texture = SDL_CreateTextureFromSurface(state->renderer, surf);
	if (!texture) 
	{
		log_sdl_error("CreateTexture");
	}
	//Clean up the surface and font
	SDL_FreeSurface(surf);

	//Get the texture w/h so we can center it in the screen
	int w, h;
	SDL_QueryTexture(texture, NULL, NULL, &w, &h);
	int x = screen_width / 2 - w / 2;
	int y = screen_height / 2 - h / 2;

	render_texture(texture, state->renderer, 0, 0);
}

void handle_input(State* state)
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT)
		{
			state->quit = true;
		}
		if (e.type == SDL_KEYDOWN)
		{
			state->quit = true;
		}
		if (e.type == SDL_MOUSEBUTTONDOWN)
		{
			state->quit = true;
		}
	}
}

bool load_font(State* state, const char* font_name, int font_size)
{
#ifdef _WIN32
	const char* font_path = "c:\\windows\\fonts\\";
#else
	// TODO(scott): figure out a better way to do this
	const char* font_path = "";
#endif
	char buffer[256];
	snprintf(buffer, ARRAY_COUNT(buffer), "%s%s.ttf", font_path, font_name);

	state->font = TTF_OpenFont(buffer, font_size);
	if (!state->font)
	{
		log_sdl_error("TTF_OpenFont");
		return false;
	}

	return true;
}

int main(int argc, char** argv)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
	{
		log_sdl_error("SDL_Init");
		return 1;
	}

	if (TTF_Init())
	{
		log_sdl_error("TTF_Init");
		SDL_Quit();
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("Willow", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_RESIZABLE);
	if (!window)
	{
		log_sdl_error("SDL_CreateWindow");
		SDL_Quit();
		return 1;
	}

	State state = { 0 };

	state.renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!state.renderer)
	{
		log_sdl_error("SDL_CreateRenderer");
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}
		
	if (!load_font(&state, "veramono", 16))
	{
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	while (!state.quit)
	{
		handle_input(&state);
		SDL_RenderClear(state.renderer);
		render_screen(&state);
		SDL_RenderPresent(state.renderer);
	}

	return 0;
}