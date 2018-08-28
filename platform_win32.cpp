
//#define WIN32_LEAN_AND_MEAN x64
#include "windows.h"

static char* get_save_file_name(void)
{
    OPENFILENAMEA dialog;
    char file_name[MAX_PATH] = { 0 };
    ZeroMemory(&dialog, sizeof(dialog));
    dialog.lStructSize = sizeof(dialog);
    //dialog.hwndOwner = NULL;
    dialog.lpstrFilter = "C file(*.c)\0*.c\0H file (*.h)\0*.h\0C++ file (*.cpp)\0*.cpp\0All files (*.*)\0*.*\0";
    dialog.lpstrFile = file_name;
    dialog.nMaxFile = MAX_PATH;
    dialog.Flags = OFN_EXPLORER;
    if (GetSaveFileNameA(&dialog))
    {
		char* result = (char*)malloc(strlen(dialog.lpstrFile) + 1);
		copy_string(result, dialog.lpstrFile, strlen(dialog.lpstrFile));
		return result;
    }
    return NULL;
}