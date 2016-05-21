/**
 * @file bio.h
 *
 * @brief Perforce's API stubbed out version of openssl/bio.h
 *
 * Threading: 
 *
 * @date   November 18, 2011
 * @author Wendy Heffner (wheffner)
 *
 * Copyright (c) 2011 Perforce Software  
 * Confidential.  All Rights Reserved.
 */

#ifndef HEADER_BIO_H
#define HEADER_BIO_H

# include <stdio.h>
# include <sys/types.h>

////////////////////////////////////////////////////////////////////////////
//			       DEFINES                                    //
////////////////////////////////////////////////////////////////////////////
# ifndef BIO
# define BIO void
# endif //BIO

#define BIO_C_GET_BUF_MEM_PTR			115
# define BIO_METHOD void
typedef struct buf_mem_st
 {
        int length;
        char *data;
        int max;
 } BUF_MEM;

#define BIO_get_mem_ptr(b,pp)	BIO_ctrl(b,BIO_C_GET_BUF_MEM_PTR,0,(char *)pp)
////////////////////////////////////////////////////////////////////////////
//			       METHOD STUBS                               //
////////////////////////////////////////////////////////////////////////////
void BIO_free_all (BIO *a);
BIO * BIO_new_socket (int sock, int close_flag);
BIO * BIO_pop (BIO *b);
BIO * BIO_new_fd(int fd, int close_flag);
BIO * BIO_new_fp(FILE *stream, int close_flag);
BIO * BIO_new(BIO_METHOD *type);
long  BIO_ctrl(BIO *bp,int cmd,long larg,void *parg);
int BIO_printf (BIO *bio, const char *format, ...);
BIO_METHOD *BIO_s_mem(void);
#endif // HEADER_BIO_H

