
extern SV *converter;

term_t perl2swi_sv(SV *sv, AV *refs, AV *cells);

void perl2swi_module(SV *sv, module_t *m);
