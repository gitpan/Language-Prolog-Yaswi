#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include <SWI-Prolog.h>


#include "Low.h"
#include "frames.h"

AV *fids;

void boot_frames(void) {
    fids=get_av(PKG "::fids", 1);
    SvREFCNT_inc(fids);
}

fid_t frame(void) {
    SV **w;
    int len=av_len(fids);
    if (len<0) {
	die ("frame called and frame stack is empty");
    }
    w=av_fetch(fids, len, 0);
    if (!w) {
	die ("corrupted frame stack");
    }
    return SvIV(*w);
}

void push_frame(void) {
    SV *fid=newSViv(PL_open_foreign_frame());
    /* warn ("push_frame(%_)", fid); */
    av_push(fids, fid);
}

void pop_frame(void) {
    SV *fid=av_pop(fids);
    /* warn ("pop_frame(%_)", fid); */
    if (!SvOK(fid)) {
	die ("pop_frame called but frame stack is empty");
    }
    PL_discard_foreign_frame(SvIV(fid));
    SvREFCNT_dec(fid);
}

void rewind_frame(void) {
    fid_t fid=frame();
    /* warn ("rewind_frame(%i)", fid); */
    PL_rewind_foreign_frame(fid);
}

