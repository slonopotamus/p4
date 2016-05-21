/*
 * Copyright 1995, 2003 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * fileiouni.cc -- FileIOUnicode methods
 */

# include <stdhdrs.h>

# include <error.h>
# include <errornum.h>
# include <msgsupp.h>
# include <strbuf.h>
# include <i18napi.h>
# include <charcvt.h>

# include "filesys.h"
# include "fileio.h"

void
FileIOUnicode::FillBuffer( Error *e )
{
	// Fill buffer from file.

	if (trans)
	{
	    int cnt;
	    cnt = FileIOCompress::Read( tbuf.Text()+tsz, tbuf.Length()-tsz, e );
	    if ( e->Test() )
		return;

	    tsz += cnt;
	    if (tsz)
	    {
		const char *ss;
		char *ts;
		ss = tbuf.Text();
		ts = iobuf.Text();
		trans->ResetErr();
		trans->Cvt(&ss, tbuf.Text()+tsz, &ts, iobuf.Text()+iobuf.Length());
		if (trans->LastErr() == CharSetCvt::NOMAPPING)
		{
		    // set an error
		    e->Set( MsgSupp::NoTrans ) << trans->LineCnt() << Name();
		    return;
		}
		else if (ts == iobuf.Text())
		{
		    // error
		    e->Set( MsgSupp::PartialChar );
		    return;
		}
		rcv = ts-iobuf.Text();
		tsz += tbuf.Text()-ss;
		if (tsz)
		    memmove(tbuf.Text(), ss, tsz);
	    }
	}
	else
	{
	    FileIOBuffer::FillBuffer( e );
	}

}

void
FileIOUnicode::FlushBuffer( Error *e )
{
	if (trans)
	{
	    const char *ss;
	    char *ts;
	    trans->ResetErr();
	    ss = iobuf.Text();
	    ts = tbuf.Text();
	    trans->Cvt(&ss, iobuf.Text()+snd, &ts, tbuf.Text()+iobuf.Length());
	    if (trans->LastErr() == CharSetCvt::NOMAPPING)
	    {
		// set an error
		e->Set( MsgSupp::NoTrans ) << trans->LineCnt() << Name();
		// prevent close from attempting second flush
		snd = 0;
	    }
	    else if (ts == tbuf.Text())
	    {
		// error
		e->Set( MsgSupp::PartialChar );
		// prevent close from attempting second flush
		snd = 0;
	    }
	    else
	    {
		FileIOCompress::Write( tbuf.Text(), ts-tbuf.Text(), e );
		snd += iobuf.Text()-ss;
		if (snd)
		    memmove(iobuf.Text(), ss, snd);
	    }
	}
	else
	{
	    FileIOBuffer::FlushBuffer( e );
	}
}

void
FileIOUnicode::Close( Error *e )
{
	FileIOBuffer::Close( e );

	tsz = 0;
	trans = NULL;
}

void
FileIOUnicode::Translator( CharSetCvt *c )
{
	trans = c;
	if( c )
	{
	    c->ResetCnt();
	    c->IgnoreBOM();
	}
}

FileIOUTF16::FileIOUTF16( LineType lineType )
    : FileIOUnicode( lineType )
{
	SetContentCharSetPriv( (int)CharSetApi::UTF_16_BOM );
}

void
FileIOUTF16::Set( const StrPtr &name )
{
	Set( name, 0 );
}

void
FileIOUTF16::Set( const StrPtr &name, Error *e )
{
	FileIOUnicode::Set( name, e );
	SetContentCharSetPriv( (int)CharSetApi::UTF_16_BOM );
}

void
FileIOUTF16::Open( FileOpenMode mode, Error *e )
{
	CharSetCvt *cvt;

	if( mode == FOM_READ )
	    cvt = new CharSetCvtUTF168;
	else
	    cvt = new CharSetCvtUTF816( -1, 1 );

	FileIOUnicode::Open( mode, e );

	FileIOUnicode::Translator( cvt );
}

void
FileIOUTF16::Close( Error *e )
{
	CharSetCvt *temp = trans;

	FileIOUnicode::Close( e );

	delete temp;
}

void
FileIOUTF16::Translator( CharSetCvt * )
{
}

FileIOUTF8::FileIOUTF8( LineType lineType )
    : FileIOUTF16( lineType )
{
	SetContentCharSetPriv( (int)CharSetApi::UTF_8_BOM );
}

void
FileIOUTF8::Set( const StrPtr &name, Error *e )
{
	FileIOUnicode::Set( name, e );
	SetContentCharSetPriv( (int)CharSetApi::UTF_8_BOM );
}

void
FileIOUTF8::Open( FileOpenMode mode, Error *e )
{
	CharSetCvt *cvt;

	if( mode == FOM_READ )
	    cvt = new CharSetCvtUTF8UTF8( -1, UTF8_VALID_CHECK );
	else
	    cvt = new CharSetCvtUTF8UTF8( 1, UTF8_WRITE_BOM );

	FileIOUnicode::Open( mode, e );

	FileIOUnicode::Translator( cvt );
}
