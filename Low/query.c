#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include <SWI-Prolog.h>

#include "frames.h"
#include "Low.h"
#include "vars.h"
#include "query.h"

SV *qid;
SV *query;

void boot_query(void) {
    qid=get_sv(PKG "::qid", 1);
    SvREFCNT_inc(qid);
    query=get_sv(PKG "::query", 1);
    SvREFCNT_inc(query);
}

void savestate_query(void) {
    save_item(qid);
    sv_setsv(qid, &PL_sv_undef);
    save_item(query);
    sv_setsv(query, &PL_sv_undef);
}

void closequery(void) {
    /* warn ("close_query(qid=%_)", qid); */
    PL_close_query(SvIV(qid));
    clear_vars();
    sv_setsv(query, &PL_sv_undef);
    sv_setsv(qid, &PL_sv_undef);
    rewind_frame();
}

void cutquery(void) {
    pop_frame();
    closequery();
}

void testnoquery(void) {
    if(SvOK(qid)) {
	croak ("there is already an open query (qid=%_)", qid);
    }
}

void testquery(void) {
    if(!SvOK(qid)) {
	croak ("there is not open query");
    }
}

int isquery(void) {
    return SvOK(qid);
}
