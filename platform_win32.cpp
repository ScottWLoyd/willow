
//#define WIN32_LEAN_AND_MEAN x64
#include "windows.h"

static char* get_save_file_name(void)
{
    OPENFILENAME dialog;
    char file_name[MAX_PATH] = { 0 };
    ZeroMemory(&dialog, sizeof(dialog));
    dialog.lStructSize = sizeof(dialog);
    //dialog.hwndOwner = NULL;
    dialog.lpstrFilter = (LPCWSTR)L"C file(*.c)\0*.c\0H file (*.h)\0*.h\0C++ file (*.cpp)\0*.cpp\0All files (*.*)\0*.*\0";
    dialog.lpstrFile = (LPWSTR)file_name;
    dialog.nMaxFile = MAX_PATH;
    dialog.Flags = OFN_EXPLORER;
    if (GetSaveFileName(&dialog))
    {
        char* name = (char*)dialog.lpstrFile;
        return name;
    }
    return NULL;
}