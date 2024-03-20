#include "win32_os.h"

void OS_Message_Box(string Message, string Title) {
    Assert(Message.Size < WIN32_MESSAGE_BOX_CAPACITY);
    Assert(Title.Size < WIN32_MESSAGE_BOX_CAPACITY);

    wchar_t MessageW[WIN32_MESSAGE_BOX_CAPACITY+1] = {};
    wchar_t TitleW[WIN32_MESSAGE_BOX_CAPACITY+1] = {0};

    MultiByteToWideChar(CP_UTF8, 0, Message.Str, (int)Message.Size, MessageW, WIN32_MESSAGE_BOX_CAPACITY+1);
    MultiByteToWideChar(CP_UTF8, 0, Title.Str, (int)Title.Size, TitleW, WIN32_MESSAGE_BOX_CAPACITY+1);

    MessageBoxW(NULL, MessageW, TitleW, MB_OK);
}