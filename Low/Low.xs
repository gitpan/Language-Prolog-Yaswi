#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include <SWI-Prolog.h>

#include "plconfig.h"
#include "argv.h"
#include "swi2perl.h"
#include "perl2swi.h"
#include "opaque.h"
#include "frames.h"
#include "vars.h"
#include "query.h"
#include "Low.h"


static SV *depth;

void savestate_Low() {
    save_item(depth);
    sv_inc(depth);
}

MODULE = Language::Prolog::Yaswi::Low   PACKAGE = Language::Prolog::Yaswi::Low   PREFIX = yaswi_

BOOT:
depth=get_sv(PKG "::depth", 1);
SvREFCNT_inc(depth);
converter=get_sv(PKG "::converter", 1);
SvREFCNT_inc(converter);
boot_query();
boot_args();
boot_frames();
boot_vars();
boot_callperl();


PROTOTYPES: ENABLE


SV *
yaswi_OPAQUE_PREFIX()
CODE:
    RETVAL=newSVpv(OPAQUE_PREFIX, 0);
OUTPUT:
    RETVAL

SV *
yaswi_plexe()
CODE:
    RETVAL=newSVpv(PL_exe, 0);
OUTPUT:
    RETVAL


int
yaswi_init(...)
CODE:
    if(!PL_is_initialised(NULL, NULL)) {
	args2argv();
        RETVAL=PL_initialise(PL_argc, PL_argv);
	push_frame();
    }
    else {
        RETVAL=1;
    }
OUTPUT:
    RETVAL


# int
# yaswi_cleanup(status)
#     int status
# CODE:
#     RETVAL=PL_cleanup(status);
# OUTPUT:
#     RETVAL

int
yaswi_toplevel()
CODE:
    RETVAL=PL_toplevel();
OUTPUT:
    RETVAL


SV *
yaswi_swi2perl(term)
    SV *term;
CODE:
    if (!SvIOK(term)) {
	croak ("'%_' is not a valid SWI-Prolog term", term);
    }
    RETVAL=swi2perl(SvIV(term), get_cells());
OUTPUT:
    RETVAL


void
yaswi_openquery(query_obj, module, ctx_module)
    SV *query_obj;
    SV *module;
    SV *ctx_module
PREINIT:
    term_t q, arg0;
    functor_t functor;
    module_t m, cm;
    predicate_t predicate;
    AV *refs, *cells;
PPCODE:
    testnoquery();
    q=perl2swi_sv(query_obj,
		  refs=(AV *)sv_2mortal((SV *)newAV()),
		  cells=(AV *)sv_2mortal((SV *)newAV()));
    if (PL_is_atom(q)) {
	atom_t name;
	PL_get_atom(q, &name);
	functor=PL_new_functor(name, 0);
	arg0=PL_new_term_ref();
    }
    else if (PL_is_compound(q)) {
	int arity, i;
	PL_get_functor(q, &functor);
	arity=PL_functor_arity(functor);
	arg0=PL_new_term_refs(arity);
	for (i=0; i<arity; i++) {
	    PL_unify_arg(i+1, q, arg0+i);
	}
    }
    else {
	die ("query is unknow\n");
    }
    perl2swi_module(module, &m);
    predicate=PL_pred(functor, m);
    perl2swi_module(ctx_module, &cm);
    sv_setiv(qid, PL_open_query(cm,
				PL_Q_NODEBUG|PL_Q_CATCH_EXCEPTION,
				predicate, arg0));
    /* warn("open_query(%_)", qid); */
    sv_setiv(query, q);
    push_frame();
    set_vars(refs, cells);
    XPUSHs(sv_2mortal(newRV_inc((SV *)refs)));

void
yaswi_cutquery()
CODE:
    testquery();
    cutquery();

int
yaswi_nextsolution()
CODE:
    testquery(); 
    cut_anonymous_vars();
    pop_frame();
    /* warn ("next_solution(qid=%_)", qid); */
    if(PL_next_solution(SvIV(qid))) {
	push_frame();
	RETVAL=1;
    }
    else {
	term_t e;
	RETVAL=0;
	if (e=PL_exception(SvIV(qid))) {
	    /* warn ("exception cached"); */
	    SV *errsv = get_sv("@", TRUE);
	    sv_setsv(errsv, sv_2mortal(swi2perl(e, get_cells())));
	    closequery();
	    croak(Nullch);
	    /* croak ("exception pop up from Prolog"); */
	}
	else {
	    closequery();
	}
    }
OUTPUT:
    RETVAL
    
void
yaswi_testquery()
CODE:
    testquery();


IV
yaswi_ref2int(rv)
    SV *rv;
CODE:
    if (!SvROK(rv)) {
	croak ("value passed to ref2int is not a reference");
    }
    RETVAL=(IV) SvRV(rv);
OUTPUT:
    RETVAL
