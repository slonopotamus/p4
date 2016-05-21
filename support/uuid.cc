// -*- mode: C++; tab-width: 8; -*-
// vi:ts=8 sw=4 noexpandtab autoindent

/**
 * uuid.cc
 *
 * Description:
 *	A simple implementation of UUIDs, inspired by an old
 *	UUID implementation by Kenneth Laskoski, which is
 *	licensed under the Boost Software Licence, Version 1.0.
 *	No parts of Boost are used.
 *
 *	This version supports only NIL and RFC 4122 random UUIDs,
 *	and does NOT use a strong random number generator.  It is,
 *	however, sufficient for our purposes (generating server ids).
 *	The current (v1.47.0) boost implementation provides a strong
 *	PRNG (Mersenne Twister [mt19937]), but also uses iostreams,
 *	which adds about 500KB to our object size.  This version adds
 *	only 4KB.
 *
 *	This class provides a C++ binding to the UUID type defined in
 *	- ISO/IEC 9834-8:2005 | ITU-T Rec. X.667
 *	  - available at http://www.itu.int/ITU-T/studygroups/com17/oid.html
 *	- IETF RFC 4122
 *	  - available at http://tools.ietf.org/html/rfc4122
 *
 *	See:
 *	    http://en.wikipedia.org/wiki/Uuid
 *	    http://www.boost.org/doc/libs/1_47_0/libs/uuid/uuid.html
 *
 * Copyright 2011 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

#ifdef OS_NT
#  define _CRT_RAND_S
    // define before <stdhdrs.h>/<stdlib.h> to get rand_s()
#endif // OS_NT
#include <stdhdrs.h>
#include <strbuf.h>
#include <time.h>
#include <error.h>
#include <pid.h>

#include "datetime.h"
#include "uuid.h"

#ifdef OS_NT
#  define SRANDOM(x)	::srand(x)
#  define RANDOM()	::rand()
#else
#  define SRANDOM(x)	::srandom(x)
#  define RANDOM()	::random()
#endif // OS_NT

/*
 * initialize our pseudo-random number generator
 */
static void
InitPRNG()
{
	DateTimeHighPrecision	dthp;
	dthp.Now();

	SRANDOM( (dthp.Seconds() + dthp.Nanos()) ^ Pid().GetID() );
}

/*
 * Convenient wrapper for getting a random int
 * after the PRNG has been initialized (if needed).
 * Bury some of the OS-dependent cruft here.
 */
static unsigned int
GetRandomInt()
{
#if defined( OS_NT ) && !defined( OS_MINGW )
	    /*
	     * NB: rand_s() appeared in VS 2005 and is supported only from XP onwards.
	     *     On Windows, the seed used by rand() is stored in TLS and so needs
	     *     to be initialized in each thread.
	     *     rand_s() doesn't use a user-provided seed, and yields a better
	     *     random distribution, so we'll use it instead.
	     */
	    unsigned int	val;
	    ::rand_s( &val );
	    return val;
#else
	    return RANDOM();
#endif // OS_NT
}

/*
 * fill buffer with random bytes
 */
static void
Randomize(
	unsigned char	*buffer,
	unsigned int	count)
{
#if defined( OS_MINGW )
	InitPRNG();
#endif

	for( int i = 0; i < count; ++i )
	{
	    unsigned int when = GetRandomInt();

	    buffer[i] = ((when | (when >> 8)) ^ ((when >> 16) | (when >> 24))) & 0xFF;
	}
}

/*
 * Orthodox Canonical Form (OCF) methods
 */


/**
 * default ctor
 * - generate a Version 4 (Random) UUID
 */
UUID::UUID()
{
#ifndef OS_NT
	// on Windows we use rand_s(), which doesn't use a seed
	MT_STATIC bool	inited = false;

	if( !inited )
	{
	    // initialize the PRNG
	    InitPRNG();
	    inited = true;
	}
#endif // OS_NT

	Randomize( begin(), kDataSize );

	// set variant
	// should be 0b10xxxxxx
	m_uuid[8] &= 0xBF;   // 0b10111111
	m_uuid[8] |= 0x80;   // 0b10000000

	// set version
	// should be 0b0100xxxx
	m_uuid[6] &= 0x4F;   // 0b01001111
	m_uuid[6] |= 0x40;   // 0b01000000
} // default ctor

/**
 * initialize with copies of the given byte value
 * - don't use unless you know what you're doing
 * - mostly useful if you want a NIL UUID.
 */
UUID::UUID(
	int	val)
{
	for( int i = 0; i < kDataSize; ++i )
	    m_uuid[i] = val;
}

/**
 * copy ctor
 */
UUID::UUID(
	const UUID	&rhs)
{
	for( int i = 0; i < kDataSize; ++i )
	    m_uuid[i] = rhs.m_uuid[i];
} // copy ctor

/**
 * dtor
 */
UUID::~UUID()
{
} // dtor

/**
 * assignment op
 */
const UUID	&
UUID::operator=(
	const UUID	&rhs)
{
	if( this != &rhs ) {
	    for( int i = 0; i < kDataSize; ++i )
	        m_uuid[i] = rhs.m_uuid[i];
	}

	return *this;
} // op=

/**
 * op==
 */
bool
UUID::operator==(
	const UUID	&rhs) const
{
	if( this == &rhs ) {
	    return true;
	}

	for( int i = 0; i < kDataSize; ++i )
	{
	    if( m_uuid[i] != rhs.m_uuid[i] )
	        return false;
	}

	return true;
} // op==

/**
 * op!=
 */
bool
UUID::operator!=(
	const UUID	&rhs) const
{
	return !(*this == rhs);
} // op!=


// accessors

bool
UUID::IsNil() const
{
	for( int i = 0; i < kDataSize; ++i )
	{
	    if( m_uuid[i] )
	        return false;
	}

	return true;
}

/*
 * From wikipedia:
 *    In the canonical representation, xxxxxxxx-xxxx-Mxxx-Nxxx-xxxxxxxxxxxx,
 *    the most significant bits of N indicates the variant (depending on the
 *    variant; one, two or three bits are used). The variant covered by the
 *    UUID specification is indicated by the two most significant bits of N
 *    being 1 0 (i.e. the hexadecimal N will always be 8, 9, a, or b).
 *
 *    In the variant covered by the UUID specification, there are five versions.
 *    For this variant, the four bits of M indicates the UUID version
 *    (i.e. the hexadecimal M will either be 1, 2, 3, 4, or 5)
 *
 * NB: The underlying implementation supports only the random version (4) of the
 * normal variant and the NIL variant (all bits 0).
 */

/**
 * VariantType
 * - Return the UUID variant of this UUID.
 * - Returns either 0x80 (RFC 4122)
 *   or 0 (NIL)
 *   - NIL UUIDs return 0, otherwise
 *     high 2 bits are always 10, per wikipedia note above.
 */
unsigned int
UUID::VariantType() const
{
    	return m_uuid[8] & 0xC0;
}

/*
 * VersionType
 * - Return the type of the UUID:
 *    Version 1 (MAC address)
 *    Version 2 (DCE Security)
 *    Version 3 (MD5 hash)
 *    Version 4 (random)
 *    Version 5 (SHA-1 hash)
 * - Returns either 0x40 (random)
 *   or 0 (NIL)
 */
unsigned int
UUID::VersionType() const
{
    	return m_uuid[6] & 0xF0;
}

/**
 * Data
 * - return the raw underlying UUID bytes
 */
void
UUID::Data(DataType &data) const
{
	::memcpy(data, begin(), sizeof(data));
}

/**
 * Size
 * - return the data size of the underlying boost uuid object
 * [static]
 */
unsigned int
UUID::SizeofData()
{
	return kDataSize;
}

// mutators

// swap the underlying uuid data bytes with rhs
void
UUID::Swap(UUID	&rhs)
{
    	DataType	tmp;

	::memcpy(&tmp, begin(), kDataSize);
	::memcpy(begin(), rhs.begin(), kDataSize);
	::memcpy(rhs.begin(), &tmp, kDataSize);
}

/*
 * Other methods
 */


/**
 * NibbleToHexChar
 * - helper routine to convert a nibble value to its
 *   hexadecimal character representation.
 * - assumes ASCII-compatible character set (ie, not EBCDIC).
 */
static char
NibbleToHexChar(unsigned int	val)
{
	if( val < 10 )
	{
	    return val + '0';
	}
	return val - 10 + 'A';
}

/**
 * ToStrBuf
 * - Set "buf" to the formatted hexadecimal character representation of this UUID.
 */
StrPtr
UUID::ToStrBuf(StrBuf	&buf) const
{
	// standard format is 8-4-4-4-12 (32 digits plus 4 dashes)

	// efficiently ensure that we have enough room
	buf.Clear();
	buf.Alloc( kStringSize );
	buf.Clear();

	for( const_iterator	iter = begin();
	    iter != end();
	    ++iter )
	{
	    switch( iter - begin() )
	    {
	    // note: 2 hex digits per iteration
	    case 4:	// fall through
	    case 6:	// fall through
	    case 8:	// fall through
	    case 10:	// fall through
	        buf.Append( "-" );
	        break;
	    default:
	        break;	// do nothing but keep some compilers happy
	    }

	    unsigned int	val = static_cast<unsigned int>(*iter);
	    unsigned int	hi = (val >> 4) & 0xF;
	    unsigned int	lo = val & 0xF;
	    char		hexstr[] = { NibbleToHexChar(hi), NibbleToHexChar(lo), '\0' };
	    buf.Append( hexstr );
	}

	return buf;
} // ToStrBuf
