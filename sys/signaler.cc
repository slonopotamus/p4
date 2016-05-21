/*
 * Copyright 1995, 1996 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * Signaler.cc - catch ^C and delete temp files
 */

# define NEED_SIGNAL

# ifdef OS_NT
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# endif // OS_NT

# include <stdhdrs.h>

# include <error.h>
# include <strbuf.h>

# include "signaler.h"

// Watcom on QNX doesn't declare SIG_DFL right - cast it.
// But it isn't (void (*)(int)) on some OS's, so only cast QNX and SUNOS.

# if defined( OS_SUNOS ) || defined( OS_QNX )
# define SIG_TYPECAST(x) (void (*)(int))(x)
# else
# define SIG_TYPECAST(x) (x)
# endif

// Global signaler.

Signaler signaler;

// These two babies have C linkage for signal().

extern "C" {
	static void (*istat)( int ) = SIG_TYPECAST( SIG_DFL );
	void onintr(int v) { signaler.Intr(); exit(-1); }
}

/*
 * SignalMan - control structure for deleting a single file.
 */

struct SignalMan {

	SignalMan	*next;

	SignalFunc	callback;
	void		*ptr;

} ;

Signaler::Signaler()
{
	Catch();

# ifdef OS_NT
	hmutex =	CreateMutex( NULL, FALSE, NULL );
# endif // OS_NT

	list = 0;
	disable = 0;
}

Signaler::~Signaler()
{
# ifdef OS_NT
	// This seems like it would be the right thing to do. Unfortunately,
	// we have a single global Signaler instance, and the methods in it
	// are called by other global objects. Since the order of destructors
	// of global objects is not under our control, we can't close the
	// mutex here, because other objects may continue to try calling the
	// Signaler object even after the Signaler object's destructor has
	// been called. So we leave the mutex handle open, and let the
	// operating system clean it up when we exit.
	//
	// CloseHandle( hmutex );
# endif // OS_NT
}

void
Signaler::Disable()
{
	disable = 1;
}

void
Signaler::Block()
{
	// Ignore signals
	// Apparently, if we just reinstall the default signal
	// handle (SIG_DFL) and we get a SIGINT while in system(),
	// it isn't ignored, dispite manual pages.  FreeBSD.

	signal( SIGINT, SIG_IGN );
}

void
Signaler::Catch()
{
	// Install our handler

	istat = signal( SIGINT, onintr );

	// But if we're not replacing SIG_DFL or SIG_IGN, leave it alone.

        if( istat != SIG_TYPECAST( SIG_DFL ) &&
	    istat != SIG_TYPECAST( SIG_IGN ) )
		signal( SIGINT, istat );
}

void
Signaler::OnIntr( SignalFunc callback, void *ptr )
{
	if( disable )
	    return;
	
# ifdef OS_NT
	WaitForSingleObject( hmutex, INFINITE );
# endif // OS_NT

	SignalMan *d = new SignalMan;

	d->next = list;
	d->callback = callback;
	d->ptr = ptr;
	list = d;

# ifdef OS_NT
	ReleaseMutex( hmutex );
# endif // OS_NT

}

void
Signaler::DeleteOnIntr( void *ptr )
{
	if( disable )
	    return;

# ifdef OS_NT
	WaitForSingleObject( hmutex, INFINITE );
# endif // OS_NT

	SignalMan *p = 0;
	SignalMan *d = list;

	for( ; d; p = d, d = d->next )
	{
	    if( d->ptr == ptr )
	    {
		if( p ) p->next = d->next;
		else list = d->next;
		delete d;
# ifdef OS_NT
		ReleaseMutex( hmutex );
# endif // OS_NT
		return;
	    }
	}

# ifdef OS_NT
	ReleaseMutex( hmutex );
# endif // OS_NT

}

void
Signaler::Intr()
{
	if( disable )
	    return;

	SignalMan *d = list;

	// Reset for sanity.

	signal( SIGINT, SIG_TYPECAST( istat ) );

# ifdef OS_NT
	WaitForSingleObject( hmutex, INFINITE );
# endif // OS_NT

	while( d )
	{
	    // The callback may delete d
	    // so we save it's next pointer now.

	    SignalMan *p = d;
	    d = d->next;
	    p->callback( p->ptr );
	}

# ifdef OS_NT
	ReleaseMutex( hmutex );
# endif // OS_NT
}
