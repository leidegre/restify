
The test project may fail to initialize sometimes, this is typically because:
 
 -  The `Restify.Spotify.dll` cannot load due to missing dependencies
    
    To prevent this the libspotify DLL `spotify.dll` is linked and copied to 
    by default to the output directory.
    The `spotify.dll` file is also a deployed as part of setting up the test

 -  Any project uses the `Any CPU` target on a x64 machine

    This is just not supported, `libspotify` and thus RESTify is purly a 
    32-bit application. Make sure to always build for the x86 platform.
