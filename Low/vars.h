
extern GV *cells;
extern GV *vars;

void boot_vars(void);
void savestate_vars(void);
AV *get_cells(void);
AV *get_vars(void);
void clear_vars(void);
void cut_anonymous_vars(void);
SV *get_var(SV *name);
void set_vars(AV *refs, AV *cells);
