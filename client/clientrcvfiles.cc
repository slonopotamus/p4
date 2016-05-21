/*
 * Copyright 1995, 2001 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>

# include <strbuf.h>
# include <strdict.h>
# include <strops.h>
# include <strarray.h>
# include <strtable.h>
# include <error.h>
# include <mapapi.h>
# include <runcmd.h>
# include <handler.h>
# include <rpc.h>
# include <md5.h>
# include <mangle.h>
# include <i18napi.h>
# include <charcvt.h>
# include <transdict.h>
# include <debug.h>
# include <tunable.h>
# include <ignore.h>
# include <timer.h>
# include <progress.h>

# include <p4tags.h>

# include <filesys.h>
# include <pathsys.h>
# include <enviro.h>
# include <ticket.h>

# include "clientuser.h"
# include <msgclient.h>
# include <msgsupp.h>
# include "clientservice.h"
# include "client.h"
# include "clientprog.h"

class TransmitChild
{
    public:

	RunArgv args;
	RunCommand cmd;
	int opts;
	int fds[2];
	Error e;
} ;

void
clientReceiveFiles( Client *client, Error *e )
{
	StrPtr *token = client->GetVar( P4Tag::v_token, e );
	StrPtr *threads = client->GetVar( P4Tag::v_peer, e );
	StrPtr *blockCount = client->GetVar( P4Tag::v_blockCount );
	StrPtr *blockSize = client->GetVar( P4Tag::v_scanSize );
	StrPtr *proxyload = client->GetVar( "proxyload" );
	StrPtr *proxyverbose = client->GetVar( "proxyverbose" );
	StrPtr *applicense = client->GetVar( P4Tag::v_app );
	StrPtr *clientSend = client->GetVar( "clientSend" );
	StrPtr *confirm = client->GetVar( P4Tag::v_confirm );

	if( e->Test() )
	{
	    client->OutputError( e );
	    return;
	}
	int nThreads = threads->Atoi();

	StrBuf exe( client->GetExecutable() );
	if( !exe.Length() )
	    exe.Set( "p4" );

	TransmitChild *tc = new TransmitChild[ nThreads ];

	for( int i = 0; i < nThreads; i++ )
	{
	    tc[i].args.AddArg( exe );
	    tc[i].args.AddArg( "-p" );
	    tc[i].args.AddArg( client->GetPort() );
	    tc[i].args.AddArg( "-u" );
	    tc[i].args.AddArg( client->GetUser() );
	    tc[i].args.AddArg( "-c" );
	    tc[i].args.AddArg( client->GetClient() );
	    if( proxyload )
	        tc[i].args.AddArg( "-Zproxyload" );
	    if( proxyverbose )
	        tc[i].args.AddArg( "-Zproxyverbose" );
	    if( applicense )
	    {
	        StrBuf appArg; appArg << "-Zapp=" << applicense;
	        tc[i].args.AddArg( appArg );
	    }
	    if( client->GetPassword().Length() )
	    {
	        tc[i].args.AddArg( "-P" );
	        tc[i].args.AddArg( client->GetPassword() );
	    }
	    tc[i].args.AddArg( "transmit" );
	    tc[i].args.AddArg( "-t" );
	    tc[i].args.AddArg( *token );
	    if( blockCount )
	    {
	        tc[i].args.AddArg( "-b" );
	        tc[i].args.AddArg( *blockCount );
	    }
	    if( blockSize )
	    {
	        tc[i].args.AddArg( "-s" );
	        tc[i].args.AddArg( *blockSize );
	    }
	    if( clientSend )
	    {
		tc[i].args.AddArg( "-r" );
	    }
	    //StrBuf x;
	    //p4debug.printf("Argv: %s\n", tc[i].args.Text( x ) );

	    tc[ i ].opts = (RCO_AS_SHELL|RCO_USE_STDOUT);
	    tc[ i ].fds[0] = tc[ i ].fds[1] = -1;

	    tc[ i ].cmd.RunChild(
	            tc[ i ].args, tc[ i ].opts, tc[ i ].fds, &tc[ i ].e );

	    if( tc[ i ].e.Test() )
	    {
		    //p4debug.printf("RunChild apparently failed\n");
	        *e = tc[ i ].e;
	        delete []tc;
		return;
	    }
	}

	int errSet = 0;
	for( int i = 0; i < nThreads; i++ )
	{
		//p4debug.printf("Waiting for child %d\n", i);
	    int status = tc[ i ].cmd.WaitChild();
	    if( status )
		++errSet;
	}

	if( errSet )
	    client->SetError();

	delete []tc;

	if( errSet && confirm )
	    client->Confirm( confirm );
}
