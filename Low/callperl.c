#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include <SWI-Prolog.h>

#include "frames.h"
#include "perl2swi.h"
#include "swi2perl.h"
#include "query.h"
#include "vars.h"
#include "Low.h"
#include "callperl.h"


static foreign_t swi2perl_sub(term_t name,
			      term_t args,
			      term_t result );

static foreign_t swi2perl_eval(term_t code,
			       term_t result);

static foreign_t swi2perl_method(term_t object,
				 term_t method,
				 term_t args,
				 term_t result);

static PL_extension callperl[] = {
    { "perl5_call",   3, swi2perl_sub,    0 },
    { "perl5_eval",   2, swi2perl_eval,   0 },
    { "perl5_method", 4, swi2perl_method, 0 },
    { NULL, 0, NULL, 0 }, };

void boot_callperl(void) {
    PL_register_extensions(callperl);
}

static savestate(void) {
    savestate_Low();
    savestate_query();
    savestate_vars();
    /*
      save_hptr(&PL_curstash);
      save_item(PL_curstname);
      PL_curstash = gv_stashpvn(CALLPERLPKG, CALLPERLPKG_len, TRUE);
      sv_setpvn(PL_curstname, CALLPERLPKG, CALLPERLPKG_len);
    */
}

static int test_oq(void) {
    if (isquery()) {
	cutquery();
	return 1;
    }
    return 0;
}

static void type_error_atom_exception(term_t nonatom, term_t *e) {
    PL_unify_term(*e=PL_new_term_ref(),
		  PL_FUNCTOR_CHARS, "type_error", 2,
		  PL_CHARS, "atom",
		  PL_TERM, nonatom);
}

static void oq_exception(term_t *e) {
    PL_unify_term(*e=PL_new_term_ref(),
		  PL_FUNCTOR_CHARS, "perl_exception", 1,
		  PL_CHARS, "open_query");
}

static int pop_results(int nret, term_t *r, term_t *e) {
    AV *vars=(AV*)sv_2mortal((SV *)newAV());
    AV *cells=(AV*)sv_2mortal((SV *)newAV());
    if (!nret && SvTRUE(ERRSV)) {
	PL_unify_term(*e=PL_new_term_ref(),
		      PL_FUNCTOR_CHARS, "perl_exception", 1,
		      PL_TERM, perl2swi_sv(ERRSV,
					   vars, cells));
	return 0;
    }
    else {
	dSP;
	term_t ret=PL_new_term_ref();
	PL_put_nil(ret);
	while(--nret>=0) {
	    PL_cons_list(ret,
			 perl2swi_sv(POPs, vars, cells),
			 ret);
	}
	*r=ret;
	return 1;
    }
}

static int push_args(term_t obj, int obj_ok, term_t args, term_t *e) {
    dSP;
    term_t head, list;
    AV *cells=(AV *)sv_2mortal((SV *) newAV());
    if (obj_ok) {
	XPUSHs(sv_2mortal(swi2perl(obj,cells)));
    }
    head=PL_new_term_ref();
    list=PL_copy_term_ref(args);
    while (!PL_get_nil(list)) {
	if (PL_get_list(list, head, list)) {
	    XPUSHs(sv_2mortal(swi2perl(head, cells)));
	}
	else {
	    PL_unify_term(*e=PL_new_term_ref(),
			  PL_FUNCTOR_CHARS, "type_error", 2,
			  PL_CHARS, "list",
			  PL_TERM, args);
	    return 0;
	}
    }
    PUTBACK;
    return 1;
}

static foreign_t swi2perl_sub(term_t name,
			      term_t args,
			      term_t result ) {
    char *cname;
    term_t e;
    if (PL_get_atom_chars(name, &cname)) {
	dSP;
	term_t ret;
	int nret, oq, ok;

	ENTER;
	SAVETMPS;
	savestate();

	PUSHMARK(SP);
	if (ok=push_args(0, 0, args, &e)) {
	    push_frame();
	    nret=call_pv(cname, G_ARRAY | G_EVAL);
	    SPAGAIN;
	    oq=test_oq();
	    pop_frame();
	    if (ok=pop_results(nret, &ret, &e)) {
		if (oq) {
		    ok=0;
		    oq_exception(&e);
		}
	    }
	}

	FREETMPS;
	LEAVE;
	
	if (ok) return PL_unify(result, ret);
    }
    else type_error_atom_exception(name, &e);
    return PL_raise_exception(e);

}

static foreign_t swi2perl_eval(term_t code,
			       term_t result) {
    char *ccode;
    term_t e;
    if (PL_get_atom_chars(code, &ccode)) {
	dSP;
	term_t ret;
	int nret, oq, ok;
	SV *svcode;

	ENTER;
	SAVETMPS;
	savestate();
	PUSHMARK(SP);

	push_frame();
	svcode=sv_2mortal(newSVpv(ccode, 0));
	nret=eval_sv(svcode, G_ARRAY | G_EVAL);
	SPAGAIN;
	oq=test_oq();
	pop_frame();
	if (ok=pop_results(nret, &ret, &e)) {
	    if (oq) {
		ok=0;
		oq_exception(&e);
	    }
	}

	FREETMPS;
	LEAVE;
	
	if (ok) return PL_unify(result, ret);
    }
    else type_error_atom_exception(code, &e);
    return PL_raise_exception(e);
}

static foreign_t swi2perl_method(term_t object,
				 term_t method,
				 term_t args,
				 term_t result) {
    char *cmethod;
    term_t e;
    if (PL_get_atom_chars(method, &cmethod)) {
	dSP;
	term_t ret;
	int nret, oq, ok;

	ENTER;
	SAVETMPS;
	savestate();

	PUSHMARK(SP);
	if (ok=push_args(object, 1, args, &e)) {
	    push_frame();
	    nret=call_method(cmethod, G_ARRAY|G_EVAL);
	    SPAGAIN;
	    oq=test_oq();
	    pop_frame();
	    if (ok=pop_results(nret, &ret, &e)) {
		if (oq) {
		    ok=0;
		    oq_exception(&e);
		}
	    }
	}

	FREETMPS;
	LEAVE;
	
	if (ok) return PL_unify(result, ret);
    }
    else type_error_atom_exception(method, &e);
    return PL_raise_exception(e);

}

