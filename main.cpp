#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>
#include "qrcodegen.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include <string>
#include <vector>

using qrcodegen::QrCode;

SDL_Texture* createQRTexture(SDL_Renderer* renderer, const QrCode& qr, int scale = 8) {
    int size = qr.getSize();
    int imgSize = size * scale;

    SDL_Surface* surface = SDL_CreateSurface(imgSize, imgSize, SDL_PIXELFORMAT_RGBA32);
    if (!surface) return nullptr;

    SDL_LockSurface(surface);
    Uint32* pixels = (Uint32*)surface->pixels;

    Uint32 black = SDL_MapRGBA(SDL_GetPixelFormatDetails(surface->format), NULL, 0, 0, 0, 255);
    Uint32 white = SDL_MapRGBA(SDL_GetPixelFormatDetails(surface->format), NULL, 255, 255, 255, 255);

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            Uint32 color = qr.getModule(x, y) ? black : white;
            for (int dy = 0; dy < scale; ++dy) {
                for (int dx = 0; dx < scale; ++dx) {
                    int px = (y * scale + dy) * imgSize + (x * scale + dx);
                    pixels[px] = color;
                }
            }
        }
    }

    SDL_UnlockSurface(surface);
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surface);
    //SDL_Texture* tex = SDL_RenderTexture(renderer, surface, NULL, NULL);
    SDL_DestroySurface(surface);
    return tex;
}

bool saveQRToPNG(const QrCode& qr, const char* filename, int scale = 8) {
    int size = qr.getSize();
    int imgSize = size * scale;

    std::vector<unsigned char> pixels(imgSize * imgSize * 3, 255); // blanc par défaut

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            bool module = qr.getModule(x, y);
            for (int dy = 0; dy < scale; ++dy) {
                for (int dx = 0; dx < scale; ++dx) {
                    int px = (y * scale + dy) * imgSize + (x * scale + dx);
                    int offset = px * 3;
                    if (module) {
                        pixels[offset + 0] = 0;
                        pixels[offset + 1] = 0;
                        pixels[offset + 2] = 0;
                    }
                }
            }
        }
    }

    return stbi_write_png(filename, imgSize, imgSize, 3, pixels.data(), imgSize * 3);
}

std::string getDesktopPath() {
#ifdef _WIN32
    char* userProfile = std::getenv("USERPROFILE");
    if (userProfile)
        return std::string(userProfile) + "\\Desktop\\";
    else
        return ".\\";
#elif __APPLE__
    char* home = std::getenv("HOME");
    if (home)
        return std::string(home) + "/Desktop/";
    else
        return "./";
#elif __linux__
    char* home = std::getenv("HOME");
    if (home)
        return std::string(home) + "/Desktop/";
    else
        return "./";
#else
    return "./";
#endif
}

int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("QRBuilder", 420, 400, SDL_WINDOW_RESIZABLE);
    SDL_SetWindowMouseRect(window, nullptr);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    int winWidth, winHeight;
    SDL_GetWindowSize(window, &winWidth, &winHeight);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    std::string inputText = "https://example.com";
    SDL_Texture* qrTexture = nullptr;

    bool running = true;
    SDL_Event e;
    std::string lastInputText = "";
    static char textResult[128] = "";
    int error_correction = 0;
    int last_correction = 0;

    while (running) {
        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL3_ProcessEvent(&e);
            if (e.type == SDL_EVENT_QUIT) running = false;
        }

        ImGui_ImplSDL3_NewFrame();
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2((float)winWidth, (float)winHeight));

        ImGui::Begin("QR Code Generator",
                     nullptr,
                     ImGuiWindowFlags_NoDecoration |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoBringToFrontOnFocus);

        static char buffer[512] = "https://example.com";
        ImGui::InputText("Text to encode", buffer, IM_ARRAYSIZE(buffer));

        if (std::string(buffer) != lastInputText || error_correction != last_correction) {
            lastInputText = buffer;
            inputText = buffer;
            last_correction = error_correction;

            QrCode qr = QrCode::encodeText(inputText.c_str(), (QrCode::Ecc)error_correction);
            if (qrTexture) SDL_DestroyTexture(qrTexture);
            qrTexture = createQRTexture(renderer, qr);
        }

        if (qrTexture) {
            ImGui::Image(qrTexture, ImVec2(256, 256));
        }
        
        std::string desktopPath = getDesktopPath();
        static char buffer_path[256];
        strcpy(buffer_path, desktopPath.c_str());
        
        ImGui::InputText("Path:", buffer_path, IM_ARRAYSIZE(buffer_path));
        
        static char buffer_name[256] = "qr_output";
        ImGui::InputText("Name:", buffer_name, IM_ARRAYSIZE(buffer_name));
        
        ImGui::InputInt("Error Correction", &error_correction);
        if(error_correction > 3) {
            error_correction = 3;
        }
        if(error_correction < 0) {
            error_correction = 0;
        }
        
        if (ImGui::Button("Save")) {
            QrCode qr = QrCode::encodeText(inputText.c_str(), QrCode::Ecc::LOW);
            
            std::string filename = std::string(buffer_path) + buffer_name + ".png";
            
            bool success = saveQRToPNG(qr, filename.c_str());
            if (success) {
                std::string str = std::string("QR sauvegardé : ")+buffer_name+".png";
                strcpy(textResult, str.c_str());
            }
            else {
                strcpy(textResult, "Erreur lors de la sauvegarde !");
            }
        }
        
        ImGui::Text(textResult);

        ImGui::End();

        ImGui::Render();
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    if (qrTexture) SDL_DestroyTexture(qrTexture);
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
