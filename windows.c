/* *****************
 * GameBoy emulator written in C.
 * Credits: Greg Tourville
********************/

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include "gameboy.h"

// Class name
const char* g_szClassName = "GameBoy Emulator";

#define REFRESH_RATE 15
char  window_caption[100];
char  window_caption_fps[100];
char  rom_file_buf[260];
char* rom_file = rom_file_buf;
char  save_file[260];

// Data
u8*   rom = NULL;
u32   rom_size = 0;
u8*   save = NULL;
u32   save_size = 0;
FILE* rom_f = 0l;
FILE* save_f = 0l;

// Declarations
void  DrawFrame(HWND hWnd, HDC hDC, HBITMAP screen, u32* pixels, HBRUSH brush);
HMENU MakeMenus(HWND hWnd);
void  PickROM(HWND hWnd);
int   LoadGame(HWND hWnd);

// Menu options
enum{ 
    ID_FILE_OPEN, 
    ID_FILE_LOAD, 
    ID_FILE_SAVE, 
    ID_EXIT, 
    // video scaling
    ID_STUFF_SCALE_1, 
    ID_STUFF_SCALE_2, 
    ID_STUFF_SCALE_3, 
    ID_STUFF_SCALE_4, 
#ifdef DMG_BIOS_ENABLE
    ID_STUFF_USE_BIOS,
#endif
    ID_RESET,
    // color scheme
    ID_COLORS_RED,     
    ID_COLORS_GREEN,   
    ID_COLORS_BLUE,    
    ID_COLORS_ORANGE,  
    ID_COLORS_GRAYSCALE,
    ID_COLORS_NIGHTTIME,
    ID_COLORS_INVERTED,
    // speed / frameskip
    ID_SPEED_NORMAL,  // 0
    ID_SPEED_FAST,    // 1
    ID_SPEED_FASTER,  // 2
    ID_SPEED_FASTEST  // -1
    };

// Emulator data
u32  started = 0;
u8   showtext = 1;
u8   quit_seq = 0;
u16  frames = 0;
u16  rendered = 0;
u16  fps = 0;
u8   running = 1;
UINT speed_setting = ID_SPEED_NORMAL;
int  frame_limit = 0;
u32  cur_t, old_t;

// Screen surfaces
int SCR_WIDTH = LCD_WIDTH * 2;
int SCR_HEIGHT = LCD_HEIGHT * 2;
u32 fb[LCD_HEIGHT][LCD_WIDTH];
u8  window_scale = 2;

// Color scheme
u32 COLORS_Y[4] = {0xFFFFFFFF, 0xFF999999, 0xFF444444, 0xFF000000};
u32 COLORS_R[4] = {0xFFFFFFFF, 0xFFBC0000, 0xFF444499, 0xFF000000};
u32 COLORS_G[4] = {0xFFFFFFFF, 0xFF22DD22, 0xFF994444, 0xFF000000};
u32 COLORS_B[4] = {0xFFFFFFFF, 0xFF7777EE, 0xFF444499, 0xFF000000};
u32 COLORS_O[4] = {0xFFFFFFEE, 0xFFFFFF66, 0xFF444499, 0xFF000000};
u32 COLORS_N[4] = {0xFF000000, 0xFF224466, 0xFFDD66FF, 0xFF9988AA};
u32 COLORS_I[4] = {0xFF000000, 0xFF444444, 0xFF999999, 0xFFF0F0F0};
u32* color_maps[] = {COLORS_R, COLORS_G, COLORS_B, COLORS_O, COLORS_Y, COLORS_N, COLORS_I};
u32* color_map;

// GameBoy Color palette conversion
u32 ColorTo32(u16 cgb)
    {
    u8 r = (cgb & 0x001F) << 3;
    u8 g = ((cgb >>  5) & 0x001F) << 3;
    u8 b = ((cgb >> 10) & 0x001F) << 3;

    return 0xFF000000 | (r << 16) | (g << 8) | b;
    }

// Key Mappings
#define NUM_KEYS    8
const int VK_Q = 0x51;
u32 KEYS[] =
    { 
    VK_RIGHT, VK_LEFT, VK_UP,     VK_DOWN, 
    0x5A,     0x58,    VK_RSHIFT, VK_RETURN, 
    0x44,     0x41,    0x57,      0x53, 
    VK_SPACE, VK_BACK, VK_LSHIFT, VK_ESCAPE
    };


void PickROM(HWND hWnd)
    {
    OPENFILENAME ofn;       // common dialog box structure

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    rom_file[0] = '\0';
    ofn.lpstrFile = rom_file;
    ofn.nMaxFile = sizeof(rom_file_buf);
    ofn.lpstrFilter = "All\0*.*\0GB\0*.GB\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // Display the Open dialogue box. 
    if (!GetOpenFileName(&ofn))
        rom_file[0] = '\0';
    }

int LoadGame(HWND hWnd)
    {
    u32 romread;
    u32 i;

    if (!hWnd) return 1;

    started = 0;

    // File chooser
    PickROM(hWnd);

    // Check for ROM
    if (!rom_file || !rom_file[0])
        return 0;
    else
        {
        char* s = rom_file;
        u16 i;
        for (i = 0; rom_file[i] != '\0'; i++)
            {
            if (rom_file[i] == '\\' || rom_file[i] == '/')
                s = &rom_file[i+1];
            }
        // sprintf(window_caption, "gameboy - %s", s);
        }

    // Load ROM file
    rom_f = fopen(rom_file, "rb");
    if (rom_f) 
        {
        fseek(rom_f, 0, SEEK_END);
        rom_size = ftell(rom_f);
        rewind(rom_f);
        rom = (u8*)malloc(rom_size);
        for (i = 0; i < rom_size; i++)
            rom[i] = 0xFF;
        romread = fread(rom, sizeof(u8), rom_size, rom_f);
        fclose(rom_f);
        }
    else
        {
        return 1;
        }

    // Load SAVE file (if it exists)
    sprintf(save_file, "%s.sav", rom_file);
    save_size = GetSaveSize(rom);
    save = (u8*)malloc(save_size);
    save_f = fopen(save_file, "rb");
    if (save_f)
        {
        fseek(save_f,0,SEEK_SET);
        fread(save, sizeof(u8), save_size, save_f);
        fclose(save_f);
        }

    // Fire up the emulator
    LoadROM(rom, rom_size, save, save_size);

    started = 1;
    return 0;
    }


void Input(u32 Key, u8 state)
    {
    u8 j;
    for (j = 0; j < 2 * NUM_KEYS; j++)
        {
        if (KEYS[j] == Key)
            {
            if (state)
                KeyPress(j % NUM_KEYS);
            else
                KeyRelease(j % NUM_KEYS);
            }
        }
    }

void SetDCPixelFormat(HDC hDC)
    {
    int nPixelFormat;
    
    static PIXELFORMATDESCRIPTOR pfd =
        {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0, 0,
        0, 0, 0, 0, 0,
        16, 0, 0, 0, 0,
        0, 0, 0
        };
    
    nPixelFormat = ChoosePixelFormat(hDC, &pfd);
    SetPixelFormat(hDC, nPixelFormat, &pfd);
    }

/* create bitmap font */
HFONT SetupRC(HDC hDC)
    {
    HFONT hFont;
    LOGFONT logfont;

    logfont.lfHeight = -20;
    logfont.lfWidth = 0;
    logfont.lfEscapement = 0;
    logfont.lfOrientation = 0;
    logfont.lfWeight = FW_BOLD;
    logfont.lfItalic = 0;
    logfont.lfUnderline = 0;
    logfont.lfStrikeOut = 0;
    logfont.lfCharSet = ANSI_CHARSET;
    logfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
    logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    logfont.lfQuality = DEFAULT_QUALITY;
    logfont.lfPitchAndFamily = DEFAULT_PITCH;
    strcpy(logfont.lfFaceName, "Arial");

    hFont = CreateFontIndirect(&logfont);
    SelectObject(hDC, hFont);

    return hFont;
    }

void DrawFrame(HWND hWnd, HDC hDC, HBITMAP screen, u32* pixels, HBRUSH brush)
    {
    u8 sx, sy;
    PAINTSTRUCT ps;
    HDC hdcMem;
    char buffer[256];
    RECT rect = {0, 0, SCR_WIDTH, SCR_HEIGHT};
    RECT text;


    BeginPaint(hWnd, &ps);

    // clear screen
    if (started == 0)
        {
        FillRect(hDC, &rect, brush);
        }

    // draw framebuffer
    if (started)
        {
        for (sy = 0; sy < LCD_HEIGHT; sy++)
            for (sx = 0; sx < LCD_WIDTH; sx++)
                {
                pixels[sx + sy*LCD_WIDTH] = color_map[gb_fb[LCD_HEIGHT-sy-1][sx] & 3];
                }
            
        GdiFlush();

        hdcMem = CreateCompatibleDC(hDC);
        SelectObject(hdcMem, screen);
        StretchBlt(hDC, 0, 0, SCR_WIDTH, SCR_HEIGHT, hdcMem, 0, 0, LCD_WIDTH, LCD_HEIGHT, SRCCOPY);
        //BitBlt(hDC, 0, 0, SCR_WIDTH, SCR_HEIGHT, hdcMem, 0, 0, SRCCOPY);
        SelectObject(hdcMem, 0);
        DeleteDC(hdcMem);
        } 

    // text
    if (showtext)
        {
        SetTextColor(hDC, RGB(186, 0, 0));
        SetBkMode(hDC, TRANSPARENT);
        text.top = 1;  text.left = 2;
        sprintf(buffer, "FPS: %i", fps);
        DrawText(hDC, buffer, -1, &text, DT_SINGLELINE | DT_NOCLIP);
        text.top = SCR_HEIGHT - 28;  text.left = 2;
        sprintf(buffer, "File: %s", rom_file);
        DrawText(hDC, buffer, -1, &text, DT_SINGLELINE | DT_NOCLIP);
        }

    EndPaint(hWnd, &ps);
    }

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
    static HDC        hDC;
    static HMENU      hMenu;
    static HBITMAP    screen;
    static HBRUSH      brush;
    static u32*       pixels;
    static HFONT      font;
    static BITMAPINFO BMI = { 
        { sizeof(BITMAPINFOHEADER), 160, 144, 1, 32, BI_RGB, 0, 0, 0, 0, 0}, 0 };

    u8 scale;
    u8 color;

    switch (message)
        {
    case WM_CREATE:
        hMenu = MakeMenus(hWnd);
        hDC = GetDC(hWnd);
        SetDCPixelFormat(hDC);
        font = SetupRC(hDC);
        screen = CreateDIBSection(hDC, &BMI, DIB_RGB_COLORS, (void**)&pixels, NULL, 0);
        brush = CreateSolidBrush(RGB(0, 0, 0));
        InvalidateRect(hWnd, NULL, 0);
        SetTimer(hWnd, 102, 1000, NULL);
        break;
    case WM_DESTROY:
        KillTimer(hWnd, 102);
        DeleteObject(font);
        DeleteObject(screen);
        DeleteObject(brush);
        PostQuitMessage(1);
        running = 0;
        break;
    case WM_TIMER:
        switch (LOWORD(wParam))
            {
        case 102:
            fps = frames;
            frames = 0;
            break;
            }
        break;
    case WM_PAINT:
        DrawFrame(hWnd, hDC, screen, pixels, brush);
        SwapBuffers(hDC);
        ValidateRect(hWnd, NULL);
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
            {
        case ID_FILE_OPEN:
            LoadGame(hWnd);
            if (started) {

            }
            break;
        case ID_FILE_LOAD:
            break;
        case ID_FILE_SAVE:
            if (save_size && save) 
                {
                save_f = fopen(save_file, "wb");
                if (save_f)
                    {
                    fseek(save_f,0,SEEK_SET);
                    fwrite(save, 1, save_size, save_f);
                    fclose(save_f);
                    }
                }
            break;
        case ID_STUFF_SCALE_1:
        case ID_STUFF_SCALE_2:
        case ID_STUFF_SCALE_3:
        case ID_STUFF_SCALE_4:
            scale = LOWORD(wParam) - ID_STUFF_SCALE_1 + 1;
            if (window_scale != scale)
                {
                int x, y, width, height, uflags;
                RECT winRect;

                GetWindowRect(hWnd, &winRect);

                width = LCD_WIDTH * scale + GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
                height = LCD_HEIGHT * scale + GetSystemMetrics(SM_CYFIXEDFRAME) * 2
                             + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENUSIZE);
                x = winRect.left - (scale - window_scale) * LCD_WIDTH / 2;
                y = winRect.top - (scale - window_scale) * LCD_HEIGHT / 2;
                uflags = SWP_NOZORDER;

                if (scale == 1)
                    {
                    SetWindowText(hWnd, "GB");
                    }
                if (window_scale == 1)
                    {
                    SetWindowText(hWnd, g_szClassName);
                    }

                SetWindowPos(hWnd, NULL, x, y, width, height, uflags);
                window_scale = scale;
                SCR_WIDTH = LCD_WIDTH * window_scale;
                SCR_HEIGHT = LCD_HEIGHT * window_scale;
                }
            break;
#ifdef DMG_BIOS_ENABLE
        case ID_STUFF_USE_BIOS:
            if (opt_use_gb_bios)
                {
                opt_use_gb_bios = 0;
                CheckMenuItem(hMenu, ID_STUFF_USE_BIOS, MF_UNCHECKED);
                }
            else
                {
                opt_use_gb_bios = 1;
                CheckMenuItem(hMenu, ID_STUFF_USE_BIOS, MF_CHECKED);
                }
            break;
#endif
        case ID_RESET:
            if (started) 
                {
                started = 0;
                LoadROM(rom, rom_size, save, save_size);
                started = 1;
                }
            break;

        case ID_COLORS_RED:   
        case ID_COLORS_GREEN:
        case ID_COLORS_BLUE:  
        case ID_COLORS_ORANGE:
        case ID_COLORS_GRAYSCALE:
        case ID_COLORS_NIGHTTIME:
        case ID_COLORS_INVERTED:
            color = LOWORD(wParam) - ID_COLORS_RED;
            if (color < sizeof(color_maps)/sizeof(u32*))
                {
                u8 i;
                u32* old = color_map;
                color_map = color_maps[color];
                for (i = 0; i < sizeof(color_maps)/sizeof(u32*); i++)
                    {
                    if (color_maps[i] == old)
                        CheckMenuItem(hMenu, ID_COLORS_RED + i, MF_UNCHECKED);    
                    }
                }
            CheckMenuItem(hMenu, wParam, MF_CHECKED);
            break;

        case ID_SPEED_NORMAL:
        case ID_SPEED_FAST:
        case ID_SPEED_FASTER:
        case ID_SPEED_FASTEST:
            {
            int speeds[] = {0, 1, 2, -1};
            if (LOWORD(wParam) != speed_setting)
                {
                CheckMenuItem(hMenu, speed_setting, MF_UNCHECKED);
                CheckMenuItem(hMenu, wParam, MF_CHECKED);
                speed_setting = LOWORD(wParam);
                frame_limit = speeds[speed_setting - ID_SPEED_NORMAL];
                }
            break;
            }
        case ID_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            break;
            }
        break;
    case WM_RBUTTONDOWN:
        showtext = !showtext;
        break;
    case WM_KEYDOWN:
        if (wParam == VK_Q)
            {
            quit_seq |= 1;
            }
        else if (wParam == VK_LCONTROL ||
            wParam == VK_RCONTROL)
            {
            quit_seq |= 2;
            }
        else
            {
            Input(wParam, 1);
            }

        if (quit_seq & 0x3)
            {
            DestroyWindow(hWnd);
            }
        break;
    case WM_KEYUP:
        if (wParam == VK_LCONTROL ||
            wParam == VK_RCONTROL ||
            wParam == VK_Q)
            {
            quit_seq = 0;
            }
        else if (wParam == VK_F5)
            {
            if (started) 
                {
                started = 0;
                LoadROM(rom, rom_size, save, save_size);
                started = 1;
                }
            }
        else
            {
            Input(wParam, 0);
            }
        break;
    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;
    case WM_QUIT:
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        }
    return 0;
    }

HMENU MakeMenus(HWND hWnd)
    {
    HMENU hMenu;
    HMENU hSubMenu;

    hMenu = CreateMenu();
    hSubMenu = CreatePopupMenu();
    AppendMenu(hSubMenu, MF_STRING, ID_FILE_OPEN, "&Open");
    AppendMenu(hSubMenu, MF_STRING, ID_FILE_LOAD, "&Load");
    AppendMenu(hSubMenu, MF_STRING, ID_FILE_SAVE, "&Save");
    AppendMenu(hSubMenu, MF_STRING, ID_EXIT, "E&xit");
    AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&File");

    hSubMenu = CreatePopupMenu();
    AppendMenu(hSubMenu, MF_STRING, ID_STUFF_SCALE_1, "Scale &1x");
    AppendMenu(hSubMenu, MF_STRING, ID_STUFF_SCALE_2, "Scale &2x");
    AppendMenu(hSubMenu, MF_STRING, ID_STUFF_SCALE_3, "Scale &3x");
    AppendMenu(hSubMenu, MF_STRING, ID_STUFF_SCALE_4, "Scale &4x");
#ifdef DMG_BIOS_ENABLE
    AppendMenu(hSubMenu, MF_STRING, ID_STUFF_USE_BIOS, "&Use DMG Bios");
    CheckMenuItem(hSubMenu, ID_STUFF_USE_BIOS, MF_CHECKED);
#endif

    AppendMenu(hSubMenu, MF_STRING, ID_SPEED_NORMAL, "&Normal Speed");
    AppendMenu(hSubMenu, MF_STRING, ID_SPEED_FAST, "&Fast (200%)");
    AppendMenu(hSubMenu, MF_STRING, ID_SPEED_FASTER, "Faste&r (300%)");
    AppendMenu(hSubMenu, MF_STRING, ID_SPEED_FASTEST, "Faste&st");
    CheckMenuItem(hSubMenu, ID_SPEED_NORMAL, MF_CHECKED);

    AppendMenu(hSubMenu, MF_STRING, ID_RESET, "&Reset");
    AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&Stuff");

    hSubMenu = CreatePopupMenu();
    AppendMenu(hSubMenu, MF_STRING, ID_COLORS_RED,       "&Red");
    AppendMenu(hSubMenu, MF_STRING, ID_COLORS_GREEN,     "&Green");
    AppendMenu(hSubMenu, MF_STRING, ID_COLORS_BLUE,      "&Blue");
    AppendMenu(hSubMenu, MF_STRING, ID_COLORS_ORANGE,    "Yelll&ow");
    AppendMenu(hSubMenu, MF_STRING, ID_COLORS_GRAYSCALE, "Gra&yscale");
    AppendMenu(hSubMenu, MF_STRING, ID_COLORS_NIGHTTIME, "&Night Time");
    AppendMenu(hSubMenu, MF_STRING, ID_COLORS_INVERTED,  "&Inverted");
    CheckMenuItem(hSubMenu, ID_COLORS_GRAYSCALE, MF_CHECKED);
    AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&Colors");

    //hSubMenu = CreatePopupMenu();
    //AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "S&peed");

    SetMenu(hWnd, hMenu);
    return hMenu;
    }

// Main windows function, program entry-point
int CALLBACK WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nCmdShow)
    {
    WNDCLASSEX wincl;
    HWND hWnd;
    MSG  message;
    int x, y, width, height, windowType;

    // Colors
    color_map = COLORS_Y;

    // Window struct
    wincl.hInstance = hInstance;
    wincl.lpszClassName = g_szClassName;
    wincl.lpfnWndProc = WndProc;
    wincl.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wincl.cbSize = sizeof(WNDCLASSEX);
    wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;
    wincl.hbrBackground = NULL;

    // Register with Windows
    if (!RegisterClassEx(&wincl)) return 0;

    // Screen placement
    width = LCD_WIDTH * window_scale + GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
    height = LCD_HEIGHT * window_scale + GetSystemMetrics(SM_CYFIXEDFRAME) * 2
                 + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENUSIZE);
    x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    y = (GetSystemMetrics(SM_CYSCREEN) - height)/2;
    windowType = WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX |
        WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

    SCR_WIDTH = LCD_WIDTH * window_scale;
    SCR_HEIGHT = LCD_HEIGHT * window_scale;

    // Create window
    hWnd = CreateWindowEx(
        WS_EX_TOPMOST, g_szClassName, g_szClassName,
        windowType, x, y, width, height,
        HWND_DESKTOP, NULL, hInstance, NULL
    );

    // Init
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    old_t = GetTickCount();
    // Message loop
    while (running)
        {
        cur_t = GetTickCount();
        if (cur_t - old_t >= REFRESH_RATE)
            {
            RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
            rendered = 0;
            old_t = cur_t;
            }
        while (PeekMessage(&message, hWnd, 0, 0, PM_REMOVE))
            {
            DispatchMessage(&message);
            }
        if ((rendered <= frame_limit || frame_limit == -1)) 
            { 
            if (started) { RunFrame(); }
            rendered++;
            frames++;
            }
        }

    // Battery file
    if (save_size && save) 
        {
        save_f = fopen(save_file, "wb");
        if (save_f)
            {
            fseek(save_f,0,SEEK_SET);
            fwrite(save, 1, save_size, save_f);
            fclose(save_f);
            }
        }

    // Clean up
    free (rom);
    free (save);

    // Quit
    return 0;
    }
