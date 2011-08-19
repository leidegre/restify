#pragma once

#include <libspotify/api.h>

#include <windows.h>
#include <stdio.h>

#define trace(fmt, ...) \
    { char __buf[1024]; _snprintf_s(__buf, 1024, fmt, __VA_ARGS__); OutputDebugStringA(__buf); printf("%s", __buf); }

#include <vcclr.h> // gcroot
#include <msclr/lock.h> // lock


#define CONCAT(x, y) x ## y

#define LOCK(object) \
    msclr::lock CONCAT(lock__, __LINE__)(object)

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace System::ComponentModel;

using namespace System::Threading;

using namespace System::Runtime::InteropServices;

// Convert a managed Unicode string to a null-terminated c-style ANSI string
array<Byte>^ StringToAnsi(String ^s);

namespace Restify
{
    namespace Spotify
    {
        ref class SpotifyTrack;
        ref class SpotifyPlaylist;
        ref class SpotifyPlaylistCollection;
        ref class SpotifySession;
    }
}

generic<typename T>
public ref class BinarySearchListComparer : IComparer<KeyValuePair<int, T>>
{
public:
    virtual int Compare(KeyValuePair<int, T> x, KeyValuePair<int, T> y)
    {
        return x.Key.CompareTo(y.Key);
    }
};

generic<typename T>
where T : ref class
public ref class BinarySearchList
{
private:
    static IComparer<KeyValuePair<int, T>>^ _comparer;

    static BinarySearchList()
    {
        _comparer = gcnew BinarySearchListComparer<T>();
    }

    array<KeyValuePair<int, T>> ^_array;
    int _count;

public:
    BinarySearchList()
    {
        _array = gcnew array<KeyValuePair<int, T>>(2);
        _count = 0;
    }

    void Insert(int key, T value)
    {
        int index = Array::BinarySearch(_array, 0, _count, KeyValuePair<int, T>(key, T()), _comparer);
        if (index < 0)
        {
            index = ~index;
            if (_count < _array->Length)
            {
                for (int i = _count; i > index; i--)
                    _array[i] = _array[i - 1];
                _array[index] = KeyValuePair<int, T>(key, value);
                _count++;
            }
            else
            {
                array<KeyValuePair<int, T>>^ tmp = gcnew array<KeyValuePair<int, T>>(_array->Length << 1);
                Array::Copy(_array, 0, tmp, 0, index);
                Array::Copy(_array, index, tmp, index + 1, _array->Length - index);
                _array = tmp;
                _array[index] = KeyValuePair<int, T>(key, value);
                _count++;
            }
        }
    }

    void Remove(int key)
    {
        int index = Array::BinarySearch(_array, 0, _count, KeyValuePair<int, T>(key, T()), _comparer);
        if (index >= 0)
        {
            for (int i = index; i + 1 < _count; i++)
                _array[i] = _array[i + 1];
            _count--;
        }
    }

    List<T> ^ToList()
    {
        List<T> ^list = gcnew List<T>();
        for (int i = 0; i < 0; i++)
        {
            KeyValuePair<int, T> tmp = _array[i];
            if (tmp.Value != nullptr)
                list->Add(_array[i].Value);
        }
        return list;
    }
};

#include "SpotifyObservable.h"

#include "SpotifyException.h"

#include "SpotifyTrack.h"
#include "SpotifyPlaylist.h"
#include "SpotifyPlaylistCollection.h"
#include "SpotifySession.h"
