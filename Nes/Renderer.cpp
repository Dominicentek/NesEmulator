#include "Renderer.h"
#include <iostream>
#include <string>
#include <stdlib.h>
#include <SDL.h>
#include <vector>

#define WIN_WIDTH 256 *4 
#define WIN_HEIGHT 240 *4
#define RENDER_WIDTH 256 
#define RENDER_HEIGHT 240 

rndr::Pixel::Pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	this->r = r;
	this->g = g;
	this->b = b;
    this->a = a;

	this->hex = (a << 24) | (r << 16) | (g << 8) | b;
}

rndr::Pixel::Pixel(uint32_t hex)
{
    this->a = (hex >> 24) & 0x00FF;
	this->r = (hex >> 16) & 0x00FF;
	this->g = (hex >> 8) & 0x00FF;
	this->b = hex & 0x00FF;

	this->hex = hex;
}

rndr::Pixel::~Pixel()
{
}


rndr::Sprite::Sprite(uint16_t width, uint16_t height, std::vector<Pixel> pixels)
{
    this->width = width;
    this->height = height;
    this->pixels = pixels;
    if(this->pixels.size() != width*height)
        this->pixels.resize(width * height, Pixel(0, 0, 0, 1));
}

rndr::Sprite::Sprite(uint16_t width, uint16_t height)
{
    this->width = width;
    this->height = height;
    this->pixels = std::vector<Pixel>(width*height, Pixel(0,0,0,1));
}

rndr::Sprite::~Sprite()
{
}

void rndr::Sprite::SetPixel(int index, Pixel pixel)
{
    pixels[index] = pixel;
}

void rndr::Sprite::SetPixel(int x, int y, Pixel pixel)
{
    if(x < width && y < height && x >= 0 && y >= 0)
        pixels[y * width + x] = pixel;
}


rndr::Renderer::Renderer(std::string title, void (*updateCallback)(), void (*renderCallback)(rndr::Renderer* renderer), void (*cleanCallback)(),  void (*guiCallback)())
{
    this->title = title;
    this->updateCallback = updateCallback;
    this->renderCallback = renderCallback;
    this->guiCallback = guiCallback;
    this->cleanCallback = cleanCallback;
    Init();
}


rndr::Renderer::~Renderer()
{
}


void rndr::Renderer::Start()
{
    while (isRunning)
    {
        HandleEvents();
        Update();
        Render();
    }
    Clean();
}

void rndr::Renderer::Init()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) 
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
   
    // create SDL window
    window = SDL_CreateWindow(title.c_str(),SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,WIN_WIDTH,WIN_HEIGHT,SDL_WINDOW_RESIZABLE);
    if (window == NULL) 
        SDL_Log("Unable to create window: %s", SDL_GetError());
    
    // create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) 
        SDL_Log("Unable to create renderer: %s", SDL_GetError());

    SDL_RenderSetLogicalSize(renderer, RENDER_WIDTH, RENDER_HEIGHT);
    
    canvas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, RENDER_WIDTH, RENDER_HEIGHT);
    if (canvas == NULL)
    {
        SDL_Log("Unable to create texture: %s", SDL_GetError());
        return;
    }


    isRunning = true;
}

void rndr::Renderer::Clean()
{
    //SDL_DestroyTexture(texture);
    cleanCallback();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

uint32_t startTime = SDL_GetTicks();
uint32_t frameCount = 0;

void rndr::Renderer::Update()
{

    if (updateCallback)
    {
        updateCallback();
    }

    frameCount++;
    if (SDL_GetTicks() - startTime >= 1000)
    {
        updateWindowTitleWithFPS(window, frameCount, startTime);
        frameCount = 0;
        startTime = SDL_GetTicks();
    }
}

void rndr::Renderer::Render()
{
    SDL_RenderClear(renderer);
   
    int texture_pitch = RENDER_WIDTH * 3;
    if (SDL_LockTexture(canvas, NULL, &canvas_pixels, &texture_pitch) != 0)
    {
        SDL_Log("Unable to lock texture: %s", SDL_GetError());
        return;
    }

    renderCallback(this);
    SDL_UnlockTexture(canvas);
    SDL_Rect destRect = { 0, 0, RENDER_WIDTH, RENDER_HEIGHT };
    SDL_RenderCopy(renderer, canvas, NULL, &destRect);
    guiCallback();
    SDL_RenderPresent(renderer);
}

void rndr::Renderer::HandleEvents()
{
    SDL_Event e;
    SDL_PollEvent(&e);
    switch (e.type)
    {
    case SDL_QUIT:
        isRunning = false;
        break;
    default:
        break;
    }
}

void rndr::Renderer::updateWindowTitleWithFPS(SDL_Window* window, Uint32 frameCount, Uint32 startTime)
{
    Uint32 currentTime = SDL_GetTicks();
    float elapsedSeconds = (currentTime - startTime) / 1000.0f;
    int fps = frameCount / elapsedSeconds;

    std::string title = this->title + " | FPS: " + std::to_string(fps);
    SDL_SetWindowTitle(window, title.c_str());
}


void rndr::Renderer::Draw(const Sprite& sprite,int posX, int posY)
{
     uint8_t* dst = static_cast<uint8_t*>(canvas_pixels);
     for (int y = 0; y < sprite.height; y++)
     {
         for (int x = 0; x < sprite.width; x++)
         {
             const Pixel& pixel = sprite.pixels[y * sprite.width + x];
             dst[(y + posY) * 4 * RENDER_WIDTH + (x + posX) * 4 + 0] = pixel.r;
             dst[(y + posY) * 4 * RENDER_WIDTH + (x + posX) * 4 + 1] = pixel.g;
             dst[(y + posY) * 4 * RENDER_WIDTH + (x + posX) * 4 + 2] = pixel.b;
             dst[(y + posY) * 4 * RENDER_WIDTH + (x + posX) * 4 + 3] = pixel.a;
         }
     }
}
