/* gpgme.c -  GnuPG Made Easy
 *	Copyright (C) 2000 Werner Koch (dd9jn)
 *
 * This file is part of GPGME.
 *
 * GPGME is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPGME is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "context.h"
#include "ops.h"

#define my_isdigit(a)  ( (a) >='0' && (a) <= '9' )
#define my_isxdigit(a) ( my_isdigit((a))               \
                         || ((a) >= 'A' && (a) <= 'F') \
                         || ((a) >= 'f' && (a) <= 'f') )

/**
 * gpgme_new:
 * @r_ctx: Returns the new context
 * 
 * Create a new context to be used with most of the other GPGME
 * functions.  Use gpgme_release_contect() to release all resources
 *
 * Return value: An error code 
 **/
GpgmeError
gpgme_new (GpgmeCtx *r_ctx)
{
    GpgmeCtx c;

    c = xtrycalloc ( 1, sizeof *c );
    if (!c)
        return mk_error (Out_Of_Core);
    c->verbosity = 1;
    c->use_armor = 1; /* fixme: reset this to 0 */
    *r_ctx = c;
    return 0;
}

/**
 * gpgme_release:
 * @c: Context to be released. 
 * 
 * Release all resources associated with the given context.
 **/
void
gpgme_release ( GpgmeCtx c )
{
    
    _gpgme_gpg_release ( c->gpg ); 
    _gpgme_release_result ( c );
    _gpgme_key_release ( c->tmp_key );
    gpgme_data_release ( c->notation );
    /* fixme: release the key_queue */
    xfree ( c );
}


void
_gpgme_release_result ( GpgmeCtx c )
{
    switch (c->result_type) {
      case RESULT_TYPE_NONE:
        break;
      case RESULT_TYPE_VERIFY:
        _gpgme_release_verify_result ( c->result.verify );
        break;
      case RESULT_TYPE_DECRYPT:
        _gpgme_release_decrypt_result ( c->result.decrypt );
        break;
      case RESULT_TYPE_SIGN:
        _gpgme_release_sign_result ( c->result.sign );
        break;
    }

    c->result.verify = NULL;
    c->result_type = RESULT_TYPE_NONE;
}


char *
gpgme_op_get_notation ( GpgmeCtx c )
{
    if ( !c->notation )
        return NULL;
    return _gpgme_data_get_as_string ( c->notation );
}


void
gpgme_op_set_armor ( GpgmeCtx c, int yes )
{
    if ( !c )
        return; /* oops */
    c->use_armor = yes;
}

void
gpgme_op_set_textmode ( GpgmeCtx c, int yes )
{
    if ( !c )
        return; /* oops */
    c->use_textmode = yes;
}













