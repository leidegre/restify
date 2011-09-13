RESTify

    There are some registry files and bat files to help with the configuration of Windows
    they are not necessary if you run as admin but can be helpful under certain situations

    The debugger reg files will allow you to debug the Restify.Server child processes

    the two files "spotify_appkey.key" and "spotify_user" are special and if found anywhere
    up the directory tree they will be used to load the application key and security credentials
    this is however only something which is enabled in a debug build for testing purposes
    and you need to put your own application key and/or account information in those files

    The file "spotify_appkey.key" is downloaded from the libspotify website

    The "spotify_user" is just a text file where the first line is your userName 
    and the second line is your password (the password is optional)
    but you won't be automatically logged in if you don't enter a password in this file
    (this is to simplify testing until a "remember me" feature is implemented properly)

Good to know

    Methods that block like `CreateMetaobject` must never be called from the 
    message loop thread.

    session.Post(() => {
        session.CreateMetaobject("..."); // this will dead-lock (or time-out)
    })

    Never write code like the above.

    This will block message loop itself and not events will be processed
    the metadata will arrive too late and nothing will happen.