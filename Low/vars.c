#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include <SWI-Prolog.h>

#include "Low.h"
#include "swi2perl.h"
#include "callback.h"
#include "vars.h"

GV *vars;
GV *cells;
static GV *cache;

void boot_vars(void) {
    cells=gv_fetchpv(PKG "::cells", GV_ADDMULTI, SVt_PVAV);
    SvREFCNT_inc(cells);
    vars=gv_fetchpv(PKG "::vars", GV_ADDMULTI, SVt_PVAV);
    SvREFCNT_inc(vars);
    cache=gv_fetchpv(PKG "::vars_cache", GV_ADDMULTI, SVt_PVHV);
    SvREFCNT_inc(cache);
}

AV *get_cells(void) {
    AV *av;
    if (av=GvAV(cells)) return av;
    return GvAV(cells)=newAV();
}

AV *get_vars(void) {
    AV *av;
    if (av=GvAV(vars)) return av;
    return GvAV(vars)=newAV();
}

HV *get_cache(void) {
    HV *hv;
    if (hv=GvHV(cache)) return hv;
    return GvHV(cache)=newHV();
}
void savestate_vars(void) {
    save_ary(vars);
    save_ary(cells);
    save_hash(cache);
}

void clear_vars(void) {
    av_clear(get_vars());
    av_clear(get_cells());
    hv_clear(get_cache());
}

void cut_anonymous_vars(void) {
    av_fill(get_cells(), av_len(get_vars()));
}

void set_vars(AV *nrefs, AV *ncells) {
    AV *vars=get_vars();
    AV *cells=get_cells();
    HV *cache=get_cache();
    int i, len;
    if (av_len(vars)>=0 || av_len(cells)>=0) {
	warn ("vars/cells stack is not empty");
	av_clear(vars);
	av_clear(cells);
    }
    
    len=av_len(nrefs)+1;
    for (i=0; i<len; i++) {
	SV **var=av_fetch(nrefs, i, 0);
	if (!var) {
	    die ("corrupted refs/cells stack, ref %i is NULL", i);
	}
	if (sv_derived_from(*var, TYPEPKG "::Variable")) {
	    SV *name=call_method__sv(*var, "name");
	    char *cname;
	    int vlen;
	    cname=SvPV(name, vlen);
	    if (strNE("_", cname)) {
		SV **cell=av_fetch(ncells, i, 0);
		if (!cell) {
		    die ("corrupted refs/cells stack, cell %i is NULL", i);
		}
		SvREFCNT_inc(*cell);
		av_push(cells, *cell);
		SvREFCNT_inc(*cell);
		hv_store(cache, cname, vlen, *cell, 0);
		SvREFCNT_inc(*var);
		av_push(vars, *var);
	    }
	}
    }
}
