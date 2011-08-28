
Restify.Spotify

    This is a C++/CLI interop layer.

    The idea is to hide as much of the asynchronous callback behavior of libspotify as possible
    and make it thread-safe and a .NET'ish but we do this in two steps.
    
    C++/CLI interop layer doesn't solve everything and while C++/CLI is nice there are some constructs (such as anonymous delegates) 
    that can't be created in C++/CLI. Very little abstraction is put into the interop layer. 
    Most of it is handled by the managed library.

    However:

        THE API ITSELF IS NOT THREAD-SAFE. THUS, YOU MUST TAKE CARE NOT TO CALL THE API FUNCTIONS FROM MORE THAN ONE OF YOUR OWN THREADS.

        We use the main thread for all intents and purposes. You can post messages (either synchronously or asynchronously) 
        to this thread to call into libspotify
        
        If all call is made to a member function using the wrong thread-context an exception will be thrown
        (this is a debug feature primarily, but it can prevent design problems further on to)

