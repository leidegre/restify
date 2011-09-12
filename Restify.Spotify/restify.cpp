
#include "restify.h"

//
// Thread safety helper functions
//

static DWORD sp_main_thread_id;

void sp_set_thread_access()
{
    sp_main_thread_id = GetCurrentThreadId();
}

bool sp_has_thread_access()
{
    return GetCurrentThreadId() == sp_main_thread_id;
}

//
// Managed to unmanaged (Spotfy strings) and vice versa
//

array<Byte> ^stringify(String^ s)
{
    if (s == nullptr || s->Length == 0)
        return gcnew array<Byte>(1);
    int length = Encoding::UTF8->GetByteCount(s);
    array<Byte> ^cstr = gcnew array<Byte>(length + 1);
    Encoding::UTF8->GetBytes(s, 0, s->Length, cstr, 0);
    return cstr;
}

String ^unstringify(const char *s)
{
    int byteCount = strlen(s);
    int charCount = Encoding::UTF8->GetCharCount((Byte *)const_cast<char *>(s), byteCount);
    if (charCount > 0)
    {
        array<wchar_t> ^chBuffer = gcnew array<wchar_t>(charCount);
        pin_ptr<wchar_t> chPtr = &chBuffer[0];
        int count = Encoding::UTF8->GetChars((Byte *)const_cast<char *>(s), byteCount, chPtr, charCount);
        return gcnew String(chBuffer, 0, count);
    }
    return String::Empty;
}