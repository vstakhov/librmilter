# Librmilter

**WARNING**: this is unstable, fragile product that is not even in alpha 
state. Do **NOT** use it for anything.

This project is intended to implement the Sendmail milter protocol from the 
scratch. Unlike the original `libmilter`, librmilter does not enforce 
specific IO model to the milters designers (namely, threading model). 
Moreover, librmilter does not perform listening on sockets allowing thus more
flexible integration to the existing code.

## Project goals

### Pluggability

`librmilter` is intended to work with different IO models and can bind to 
several events processing libraries (e.g. libevent and libev). `librmilter` 
can also plug external logging libraries.

### Clear design

The core of `librmilter` is milter protocol state machine. The internal state
contains the current state, space for the next command (up to 64K by protocol
definition), macros received and an array of replies that are intended to 
send (e.g. `add_header` or `continue`).

`librmilter` does not define any sendmail like macros, doesn't use some 
"opaque" sockaddr structures and is intended to simplify milters development.

### Performance

`librmilter` is intended to work under high load.

### Dependencies

`librmilter` requires `glib2` - a generic C library that allows to reuse the 
existing data structures and algorithms. `librmilter` uses glib2 containers 
for the public API, so you won't be able to use `librmilter` without it (that
 might be changed in future).

## Backward compatibility

librmilter is *not* compatible with the current libmilter API by many reasons. 
My main claim is that the original API is too complicated and too verbose. 
Anyway, the fact that rmilter doesn't support listening and threads makes it 
incompatible with legacy code.

## License

[BSD-2-Clause](LICENSE).

## Status

The project is a placeholder now. But I'm going to force its development. 
Please examine [the follow gist](https://gist.github.com/vstakhov/7627f356d38e56c5155a) 
for further details.