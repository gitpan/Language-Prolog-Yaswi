SV *call_method__sv(SV *object, char *method);

int call_method__int(SV *object, char *method);

SV *call_method_int__sv(SV *object, char *method, int i);

SV *call_method_sv__sv(SV *object, char *method, SV *arg);

SV *call_sub_sv__sv(char *name, SV *arg);
