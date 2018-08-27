#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <memory.h>
#include <sdl.h>
#include <SDL_ttf.h>

#define ARRAY_COUNT(x) (sizeof((x))/(sizeof((x)[0])))

#include "buffer.cpp"

static int screen_width = 800;
static int screen_height = 600;
static int tab_width = 4;

struct State {
	bool quit;
	TTF_Font* font;
	SDL_Renderer* renderer;
	GapBuffer* curr_buf;
};

static void log_sdl_error(const char* msg)
{
	printf("%s error: %s\n", msg, SDL_GetError());
}

static void render_texture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y)
{
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
	SDL_RenderCopy(ren, tex, 0, &dst);
}

static char* get_displayed_text(State* state)
{
	// TODO(scott): determine max amount of chars that could be 
	// displayed dynamically and resize based on that
	char* result = (char*)malloc(4096);
	// protect against empty buffer
	result[0] = 0;

	char* ptr = result;
	size_t len = state->curr_buf->gap_start - state->curr_buf->buffer_start;
	strncpy(ptr, state->curr_buf->buffer_start, len);
	ptr += len;
	len = state->curr_buf->buffer_end - state->curr_buf->gap_end;
	strncpy(ptr, state->curr_buf->gap_end, len);
	ptr += len;
	*ptr = 0;
	return result;
}

static void render_screen(State* state)
{	
	//We need to first render to a surface as that's what TTF_RenderText
	//returns, then load that surface into a texture
	SDL_Color color = { 255, 255, 255, 255 };
	char text[256]; 
	char* end_of_line = NULL;
	end_of_line = copy_next_line(state->curr_buf, text, ARRAY_COUNT(text), end_of_line);
	while (end_of_line < state->curr_buf->buffer_end)
	{
		end_of_line++;
		end_of_line = copy_next_line(state->curr_buf, text, ARRAY_COUNT(text), end_of_line);
	}

	SDL_Surface *surf = TTF_RenderText_Blended(state->font, text, color);
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

static void handle_keydown(State* state, SDL_Keysym key)
{
	switch (key.sym)
	{
	case SDLK_TAB:
		for (int i=0; i<tab_width; i++)
			insert_char(state->curr_buf, ' ');
		break;
	case SDLK_RETURN:
		insert_char(state->curr_buf, '\n');
		break;
	case SDLK_BACKSPACE:
		remove_chars(state->curr_buf, -1);
		break;
	case SDLK_DELETE:
		remove_chars(state->curr_buf, 1);
		break;
	default:
		break;
	}
}


static void handle_input(State* state)
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
			handle_keydown(state, e.key.keysym);
		}
		if (e.type == SDL_TEXTINPUT)
		{
			char* c = e.text.text;
			while (*c)
			{
				insert_char(state->curr_buf, *c++);
			}
		}
		if (e.type == SDL_KEYUP)
		{
		}
		if (e.type == SDL_MOUSEBUTTONDOWN)
		{
			state->quit = true;
		}
	}
}

static bool load_font(State* state, const char* font_name, int font_size)
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
	test_buffer();

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
		if (!load_font(&state, "arial", 16))
		{
			SDL_DestroyWindow(window);
			SDL_Quit();
			return 1;
		}
	}

	state.curr_buf = (GapBuffer*)malloc(sizeof(GapBuffer));
	init_buffer(state.curr_buf);

	while (!state.quit)
	{
		handle_input(&state);
		SDL_RenderClear(state.renderer);
		render_screen(&state);
		SDL_RenderPresent(state.renderer);
	}

	return 0;
}