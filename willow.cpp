#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <memory.h>
#include <sdl.h>
#include <SDL_ttf.h>

#define ARRAY_COUNT(x) (sizeof((x))/(sizeof((x)[0])))

inline void copy_string(char* dst, char* src, size_t count)
{
    while (count--)
        *dst++ = *src++;
}

union Vec2 {
    struct {
        int x, y;
    };
    struct {
        int line, col;
    };
};

#include "platform.h"
#include "buffer.cpp"

#ifdef _WIN32
#include "platform_win32.cpp"
#else

#endif

SDL_Color ColorForeground = { 255, 255, 255, 255 };
SDL_Color ColorBackground = { 0, 0, 0, 255 };

static int screen_width = 800;
static int screen_height = 600;
static int tab_width = 4;

struct CursorPos {
    int line;
    int col;
    Vec2 screen_pos;
};

struct State {
	bool quit;
	TTF_Font* font;
    int font_height;
	SDL_Renderer* renderer;
	GapBuffer* curr_buf;
    CursorPos cursor_pos;
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
	copy_string(ptr, state->curr_buf->buffer_start, len);
	ptr += len;
	len = state->curr_buf->buffer_end - state->curr_buf->gap_end;
    copy_string(ptr, state->curr_buf->gap_end, len);
	ptr += len;
	*ptr = 0;
	return result;
}

static void render_screen(State* state)
{	
	//We need to first render to a surface as that's what TTF_RenderText
	//returns, then load that surface into a texture
    int y_inc = TTF_FontLineSkip(state->font);

    char* text = get_displayed_text(state);
    if (strlen(text) == 0)
    {
        return;
    }

    SDL_Surface *surf = TTF_RenderText_Blended_Wrapped(state->font, text, ColorForeground, screen_width);        
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
    SDL_FreeSurface(surf);
    render_texture(texture, state->renderer, 0, 0);
    SDL_DestroyTexture(texture);

    /*
     * Render the buffer information
     */
    sprintf(text, "(%d,%d)", state->cursor_pos.line, state->cursor_pos.col);
    //surf = TTF_RenderText_Blended(state->font, text, ColorForeground);
    surf = TTF_RenderText_Shaded(state->font, text, ColorBackground, ColorForeground);
    if (!surf)
    {
        TTF_CloseFont(state->font);
        log_sdl_error("TTF_RenderText");
        return;
    }
    texture = SDL_CreateTextureFromSurface(state->renderer, surf);
    if (!texture)
    {
        log_sdl_error("CreateTexture");
    }
    //Clean up the surface and font
    SDL_FreeSurface(surf);
    int w, h, x, y;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    x = 0;
    y = screen_height - h;
    render_texture(texture, state->renderer, x, y);
    SDL_DestroyTexture(texture);
}

static void save_buffer(GapBuffer* buf, bool save_as = false)
{
    char* file_name = buf->file_name;
    if (!file_name || save_as)
    {
       file_name = get_save_file_name();
    }
    if (file_name)
    {
        buf->file_name = file_name;
        FILE* file = fopen(buf->file_name, "r");
        char* ptr = buf->buffer_start;
        while (*ptr)
        {
            if (ptr >= buf->gap_start && ptr <= buf->gap_end)
                ptr++;
#ifdef _WIN32
            if (*ptr == '\n')
            {
                putc('\r', file);
            }
#endif
            putc(*ptr, file);
            ptr++;
        }
        fclose(file);
    }
}

static void handle_keydown(State* state, SDL_Keysym key)
{
    bool shift_pressed = key.mod & KMOD_LSHIFT || key.mod & KMOD_RSHIFT;
    bool ctrl_pressed = key.mod & KMOD_LCTRL || key.mod & KMOD_RCTRL;

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
    case SDLK_s:
        if (ctrl_pressed)
        {
            save_buffer(state->curr_buf);
        }
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
		else if (e.type == SDL_KEYDOWN)
		{
			handle_keydown(state, e.key.keysym);
		}
		else if (e.type == SDL_TEXTINPUT)
		{
			char* c = e.text.text;
			while (*c)
			{
				insert_char(state->curr_buf, *c++);
			}
		}
		else if (e.type == SDL_KEYUP)
		{
		}
		else if (e.type == SDL_MOUSEBUTTONDOWN)
		{
		}

        if (e.type == SDL_KEYDOWN || e.type == SDL_TEXTINPUT)
        {
            Vec2 pos = get_point_location(state->curr_buf);
            state->cursor_pos.line = pos.line;
            state->cursor_pos.col = pos.col;
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

    state->font_height = TTF_FontLineSkip(state->font);

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