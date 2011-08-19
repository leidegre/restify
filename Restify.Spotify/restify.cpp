
#include "restify.h"

array<Byte>^ StringToAnsi(String^ s)
{
    if (s == nullptr || s->Length == 0)
    {
        return gcnew array<Byte>(1);
    }
    int length = Encoding::Default->GetByteCount(s);
    array<Byte>^ cstr = gcnew array<Byte>(length + 1);
    Encoding::Default->GetBytes(s, 0, s->Length, cstr, 0);
    return cstr;
}
