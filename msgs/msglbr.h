/*
 * Copyright 1995, 2000 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * msglbr.h - definitions of errors for Lbr subsystem.
 */


class MsgLbr {

    public:

	static ErrorId BadType1;
	static ErrorId Purged;
	static ErrorId ScriptFailed;

	static ErrorId After;
	static ErrorId Checkin;
	static ErrorId Checkout;
	static ErrorId Commit;
	static ErrorId Diff;
	static ErrorId Edit0;
	static ErrorId Edit1;
	static ErrorId Edit2;
	static ErrorId Empty;
	static ErrorId EofAt;
	static ErrorId Expect;
	static ErrorId ExpDesc;
	static ErrorId ExpEof;
	static ErrorId ExpRev;
	static ErrorId ExpSemi;
	static ErrorId Lock;
	static ErrorId Loop;
	static ErrorId Mangled;
	static ErrorId MkDir;
	static ErrorId NoBrRev;
	static ErrorId NoBranch;
	static ErrorId NoRev;
	static ErrorId NoRev3;
	static ErrorId NoRevDel;
	static ErrorId Parse;
	static ErrorId RevLess;
	static ErrorId TooBig;
	static ErrorId RcsTooBig;
	static ErrorId FmtLbrStat;
	static ErrorId FmtLbrStat2;
	static ErrorId FmtLbrStat3;
	static ErrorId LbrOpenFail;
	static ErrorId AlreadyOpen;
	static ErrorId BadKeyword;
	static ErrorId KeywordUnterminated;

	// Retired ErrorIds. We need to keep these so that clients 
	// built with newer apis can commnunicate with older servers 
	// still sending these.

} ;
