// -*- mode: C++; tab-width: 8; -*-
// vi:ts=8 sw=4 noexpandtab autoindent

/**
 * uuid.h
 *
 * Description:
 *	A simple implementation of UUIDs.
 *
 *	See:
 *	    http://en.wikipedia.org/wiki/Uuid
 *	    http://www.boost.org/doc/libs/1_47_0/libs/uuid/uuid.html
 *
 * Copyright 2011 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

class UUID {
public:
	enum
	{
	    kDataSize = 16,	// 128 bits, per RFC-4122
	    kStringSize = 36	// 2 hex digits per byte + 4 hyphens
	};

	typedef unsigned char	ValueType;
	// buffer big enough to hold the raw uuid data (implementation-dependent)
	typedef ValueType	DataType[kDataSize];

	typedef ValueType	*iterator;
	typedef const ValueType	*const_iterator;

	// Orthodox Canonical Form (OCF) methods
	UUID();		// generate a random UUID

	// generate UUID data as copies of "val" byte
	UUID(
	    int		val);

	UUID(
	    const UUID	&rhs);

	~UUID();

	const UUID &
	operator=(
	    const UUID	&rhs);

	bool
	operator==(
	    const UUID	&rhs) const;

	bool
	operator!=(
	    const UUID	&rhs) const;

	// accessors

	bool
	IsNil() const;

	// return the variant type of this UUID
	unsigned int
	VariantType() const;

	// return the version of this UUID variant
	unsigned int
	VersionType() const;

	// copy the raw uuid data bytes
	void
	Data(DataType &data) const;

	// return the size of the underlying boost uuid data
	static unsigned int
	SizeofData();

	// mutators

	// swap the underlying uuid data bytes with rhs
	void
	Swap(UUID	&rhs);

	// iterators
	
	iterator
	begin()
	{
	    return &m_uuid[0];
	}

	iterator
	end()
	{
	    return &m_uuid[kDataSize];
	}

	const_iterator
	begin() const
	{
	    return &m_uuid[0];
	}

	const_iterator
	end() const
	{
	    return &m_uuid[kDataSize];
	}

	// Other methods

	// Set "buf" to the formatted hexadecimal character representation
	// of this UUID.
	StrPtr
	ToStrBuf(StrBuf	&buf) const;

protected:

private:
	DataType	m_uuid;
};
