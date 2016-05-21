/*
 * Copyright 1995, 1996 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * filegen.cc -- generic part of filesystem interface FileSys
 */

# ifdef USE_EBCDIC
# define NEED_EBCDIC
# endif

# include <stdhdrs.h>

# include <error.h>
# include <strbuf.h>

# include "filesys.h"
# include "md5.h"

void
FileSys::Digest( StrBuf *digest, Error *e )
{
	// Use MD5 message digest algorithm to fingerprint the file.

	MD5 md5;

	Open( FOM_READ, e );

	// Read some 

	StrFixed buf( BufferSize() );

	while( !e->Test() )
	{
	    int l = Read( buf.Text(), buf.Length(), e );

	    if( !l || e->Test() )
		break;

	    // Since the server MD5 is done as ASCII, so must this.

# ifdef USE_EBCDIC
	    if( IsTextual() )
		__etoa_l( buf.Text(), l );
# endif

	    StrRef z;
	    z.Set( buf.Text(), l );
	    md5.Update( z );
	}

	Close( e );

	md5.Final( *digest );
}

int
FileSys::ReadLine( StrBuf *buf, Error *e )
{
	char b = 0;
	buf->Clear();
	int size = BufferSize();

	// Read line into buffer; return 0 on eof
	// At most BufferSize, to be bug-for-bug compatible
	// w/ previous versions.

	while( buf->Length() < size &&
	    Read( &b, 1, e ) == 1 && b != '\n' )
		buf->Extend( b );

	if( !buf->Length() && !b )
	    return 0;

	// Make sure our null is there

	buf->Terminate();

	return b == '\n' ? 1 : -1;
}

void
FileSys::ReadWhole( StrBuf *buf, Error *e )
{
	buf->Clear();
	int size = BufferSize();

	for(;;)
	{
	    char *b = buf->Alloc( size );
	    int l = Read( b, size, e );
	    if( l < 0 )
		l = 0;
	    buf->SetEnd( b + l );

	    if( e->Test() || !l )
		break;
	}

	buf->Terminate();
}

/*
 * FileSys::ReadFile() - open, read whole file into string, close
 */

void
FileSys::ReadFile( StrBuf *buf, Error *e )
{
	Open( FOM_READ, e );

	if( e->Test() )
	    return;

	ReadWhole( buf, e );

	if( e->Test() )
	    return;

	Close( e );
}

/*
 * FileSys::WriteFile() - open, write whole file from string, close
 */

void
FileSys::WriteFile( const StrPtr *buf, Error *e )
{
	Open( FOM_WRITE, e );

	if( e->Test() )
	    return;

	Write( buf->Text(), buf->Length(), e );

	if( e->Test() )
	    return;

	Close( e );
}

/*
 * FileSys::Compare() - do direct comparison of files
 */

int
FileSys::Compare( FileSys *other, Error *e )
{
	int diff = 0;

	Open( FOM_READ, e );

	if( e->Test() )
	    return 0;

	other->Open( FOM_READ, e );

	if( e->Test() )
	{
	    Close( e );
	    return 0;
	}

	StrFixed buf1( BufferSize() );
	StrFixed buf2( BufferSize() );

	while( !diff )
	{
	    int l1 = Read( buf1.Text(), buf1.Length(), e );
	    int l2 = other->Read( buf2.Text(), buf2.Length(), e );

	    if( e->Test() )
		break;

	    diff = l1 != l2 || memcmp( buf1.Text(), buf2.Text(), l1 );

	    if( !l1 )
		break;
	}

	Close( e );
	other->Close( e );

	return diff;
}

/*
 * FileSys::Copy - copy one file to another
 */

void
FileSys::Copy( FileSys *targetFile, FilePerm perms, Error *e )
{
	// Open source and target files.

	Open( FOM_READ, e );

	if( e->Test() )
	    return;

	targetFile->Perms( perms );
	targetFile->Open( FOM_WRITE, e );

	if( e->Test() )
	{
	    Close( e );
	    return;
	}

	// Copy text

	StrFixed buf( BufferSize() );

	while( !e->Test() )
	{
	    int l = Read( buf.Text(), buf.Length(), e );

	    if( !l || e->Test() )
		break;

	    targetFile->Write( buf.Text(), l, e );
	}

	// Close off files

	Close( e );
	targetFile->Close( e );
	targetFile->Chmod( perms, e );
}

/*
 * FileSys::Chmod2()
 *
 * If Chmod() doesn't work, it may because we don't own the file.
 * Another approach is to copy the file to get ownership and thus
 * set the permission.
 */

void
FileSys::Chmod2( FilePerm perms, Error *e )
{
	// First just try chmod -- maybe we own it

	Chmod( perms, e );

	if( !e->Test() )
	    return;

	// No, now try copying to get ownership and perms.

	Error ec;

	FileSys *tmpFile = CreateTemp( GetType() );
	tmpFile->MakeLocalTemp( Name() );

	if( !ec.Test() ) Copy( tmpFile, perms, &ec );
	if( !ec.Test() ) tmpFile->Rename( this, &ec );

	delete tmpFile;

	// If copy unsuccessful -- use just return Chmod() error

	if( !ec.Test() ) e->Clear();
}

