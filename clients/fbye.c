    /*********************************************************************\
    *  Copyright (c) 2004 by Radim Kolar (hsn@cybermail.net)              *
    *                                                                     *
    *  You may copy or modify this file in any manner you wish, provided  *
    *  that this notice is always included, and that you hold the author  *
    *  harmless for any loss or damage resulting from the installation or *
    *  use of this software.                                              *
    \*********************************************************************/

#include "tweak.h"
#include "client_def.h"
#include "c_extern.h"
#include "merge.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

int main (int argc, char ** argv)
{
    env_client();
    client_finish();
    return 0;
}
