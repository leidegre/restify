
Spotify

    This is a C++/CLI introp library that tries do hide as much of the asynchrnous stuff going on in libspotify as possible
    as well as making it thread-safe and .NET'ish

    Note:

        Only member functions that start with "Spotify_" should call the libspotify API because they do this from the main thread
        which is the only point at which you may call the libspotify API according with the documentation

        Only exception to this rule is constructors that run with accordance to the documentation