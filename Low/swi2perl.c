#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include <SWI-Prolog.h>

#include "Low.h"
#include "opaque.h"
#include "callback.h"
#include "swi2perl.h"


SV *swi2perl(term_t t, AV *cells) {
    if (PL_is_integer(t)) {
	long v;
	PL_get_long(t, &v);
	return newSViv(v);
    }
    if (PL_is_float(t)) {
	double v;
	PL_get_float(t, &v);
	return newSVnv(v);
    }
    if (PL_is_atom(t)) {
	char *v;
	PL_get_atom_chars(t, &v);
	return newSVpv(v, 0);
    }
    if (PL_is_list(t)) {
	AV *array=newAV();
	SV *ref=newRV_noinc((SV *)array);
	int len=0;
	term_t head, tail;
	while(PL_is_list(t)) {
	    if(PL_get_nil(t)) {
		sv_bless(ref, gv_stashpv( len ?
					  TYPEINTPKG "::list" :
					  TYPEINTPKG "::nil", 1));
		return ref;
	    }
	    head=PL_new_term_refs(2);
	    tail=head+1;
	    PL_get_list(t, head, tail);
	    av_push(array, swi2perl(head, cells));
	    t=tail;
	    len++;
	}
	av_push(array, swi2perl(tail, cells));
	sv_bless(ref, gv_stashpv(TYPEINTPKG "::ulist", 1));
	return ref;
    }
    if (PL_is_compound(t)) {
	SV *ref;
	int i;
	int arity;
	atom_t atom;
	PL_get_name_arity(t, &atom, &arity);
	if (arity==3 && !strcmp(OPAQUE_FUNCTOR, PL_atom_chars(atom))) {
	    term_t arg=PL_new_term_ref();
	    PL_get_arg(1, t, arg);
	    ref=call_sub_sv__sv (PKG "::get_opaque",
				 sv_2mortal(swi2perl(arg, cells)));
	    SvREFCNT_inc(ref);
	    return ref;
	}
	else {
	    AV *functor=newAV();
	    ref=newRV_noinc((SV *)functor);
	    sv_bless(ref, gv_stashpv(TYPEINTPKG "::functor", 1));

	    av_extend(functor, arity+1);
	    av_store(functor, 0, newSVpv(PL_atom_chars(atom), 0));
	    for (i=1; i<=arity; i++) {
		term_t arg=PL_new_term_ref();
		PL_get_arg(i, t, arg);
		av_store(functor, i, swi2perl(arg, cells));
	    }
	    return ref;
	}
    }
    if (PL_is_variable(t)) {
	term_t var;
	int len=av_len(cells)+1;
	int i;
	SV *cell;
	SV *ref;
	for(i=0; i<len; i++) {
	    term_t vterm;
	    SV **ref_p=av_fetch(cells, i, 0);
	    if (!ref_p)
		die ("internal error, unable to fetch var from cache");
	    var=SvIV(*ref_p);
	    if (PL_compare(t, var)==0) {
		cell=*ref_p;
		break;
	    }
	}
	if (i==len) {
	    cell=newSViv(t);
	    /* SvREADONLY_on(cell); */
	    av_push(cells, cell);
	}
	ref=newRV_inc(cell);
	sv_bless(ref, gv_stashpv(TYPEINTPKG "::variable",1));
	return ref;
    }
    if (PL_is_string(t)) {
	char *v;
	int len;
	PL_get_string_chars(t, &v, &len);
	return newSVpv(v, len);
    }
}
