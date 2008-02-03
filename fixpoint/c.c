/* c.sf */
#define MODULE module_c
#define LOAD() module_7(); module_6(); module_5(); module_4(); module_3(); module_2(); module_1(); module_0(); 
extern void module_0(void); /* 0.sf */
extern void module_1(void); /* 1.sf */
extern void module_2(void); /* 2.sf */
extern void module_3(void); /* 3.sf */
extern void module_4(void); /* 4.sf */
extern void module_5(void); /* 5.sf */
extern void module_6(void); /* 6.sf */
extern void module_7(void); /* 7.sf */

/* standard includes */
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
/* extra includes */
#include <math.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

/* standard definitions */
#define REGS_SIZE 5000

typedef ptrdiff_t obj;        /* pointers are this size, lower bit zero */
typedef ptrdiff_t cxoint_t;   /* same thing, used as integer */
typedef struct {              /* type descriptor */
  const char *tname;          /* name (debug) */
  void (*free)(void*);        /* deallocator */
} cxtype_t;

#define notobjptr(o)          (((char*)(o) - (char*)cxg_heap) & cxg_hmask)
#define isobjptr(o)           (!notobjptr(o))
#define notaptr(o)            ((o) & 1)
#define isaptr(o)             (!notaptr(o))

#define obj_from_obj(o)       (o)
#define obj_from_objptr(p)    ((obj)(p))
#define obj_from_size(n)      (((cxoint_t)(n) << 1) | 1)

#define objptr_from_objptr(p) (p)
#define objptr_from_obj(o)    ((obj*)(o))

#define size_from_obj(o)      ((int)((o) >> 1))

#define obj_from_case(n)      obj_from_objptr(cases+(n))
#define case_from_obj(o)      (objptr_from_obj(o)-cases)
#define obj_from_ktrap()      obj_from_size(0x5D56F806)
#define obj_from_void(v)      ((void)(v), obj_from_size(0x6F56DF77))

#define bool_from_obj(o)      (o)
#define bool_from_bool(b)     (b)
#define bool_from_size(s)     (s)

#define void_from_void(v)     (void)(v)
#define void_from_obj(o)      (void)(o)

#define rreserve(m)           if (r > cxg_regs + REGS_SIZE - 2*(m)) r = cxm_rgc(r, r+(m))
#define hpushptr(p, pt, l)    (hreserve(2, l), *--hp = (obj)(p), *--hp = (obj)(pt), (obj)(hp+1))   
#define hbsz(s)               ((s) + 1) /* 1 extra word to store block size */
#define hreserve(n, l)        ((hp < cxg_heap + (n)) ? hp = cxm_hgc(r, r+(l), hp, n) : hp)
#define hendblk(n)            (*--hp = obj_from_size(n), (obj)(hp+1))
#define hblklen(p)            size_from_obj(((obj*)(p))[-1])
#define hblkref(p, i)         (((obj*)(p))[i])

typedef obj (*cxhost_t)(obj);
typedef struct cxroot_tag {
  int globc; obj **globv;
  struct cxroot_tag *next;
} cxroot_t;

extern obj *cxg_heap;
extern obj *cxg_hp;
extern cxoint_t cxg_hmask;
extern cxroot_t *cxg_rootp;
extern obj *cxm_rgc(obj *regs, obj *regp);
extern obj *cxm_hgc(obj *regs, obj *regp, obj *hp, size_t needs);
extern obj cxg_regs[REGS_SIZE];
extern void *cxm_cknull(void *p, char *msg);
#ifndef NDEBUG
extern int cxg_rc;
#endif

/* extra definitions */
/* basic object representation */
#define isimm(o, t) (((o) & 0xff) == (((t) << 1) | 1))
#ifdef NDEBUG
  #define getimmu(o, t) (int)(((o) >> 8) & 0xffffff)
  #define getimms(o, t) (int)(((((o) >> 8) & 0xffffff) ^ 0x800000) - 0x800000)
#else
  extern int getimmu(obj o, int t);
  extern int getimms(obj o, int t);
#endif
#define mkimm(o, t) ((((o) & 0xffffff) << 8) | ((t) << 1) | 1)
#ifdef NDEBUG
   static int isnative(obj o, cxtype_t *tp) 
     { return isobjptr(o) && objptr_from_obj(o)[-1] == (obj)tp;  }
   #define getnative(o, t) ((void*)(*objptr_from_obj(o)))
#else
  extern int isnative(obj o, cxtype_t *tp);
  extern void *getnative(obj o, cxtype_t *tp);
#endif
extern int istagged(obj o, int t);
#ifdef NDEBUG
  #define cktagged(o, t) (o)
  #define taggedlen(o, t) (hblklen(o)-1) 
  #define taggedref(o, t, i) (&hblkref(o, (i)+1))
#else
  extern obj cktagged(obj o, int t);
  extern int taggedlen(obj o, int t);
  extern obj* taggedref(obj o, int t, int i); 
#endif
/* booleans */
#define TRUE_ITAG 0
typedef int bool_t;
#define is_bool_obj(o) (!((o) & ~(obj)1))
#define is_bool_bool(b) ((void)(b), 1)
#define void_from_bool(b) (void)(b)
#define obj_from_bool(b) ((b) ? mkimm(0, TRUE_ITAG) : 0)
/* fixnums */
#define FIXNUM_ITAG 1
typedef int fixnum_t;
#define is_fixnum_obj(o) (isimm(o, FIXNUM_ITAG))
#define is_fixnum_fixnum(i) ((void)(i), 1)
#define fixnum_from_obj(o) (getimms(o, FIXNUM_ITAG))
#define fixnum_from_fixnum(i) (i)
#define void_from_fixnum(i) (void)(i)
#define obj_from_fixnum(i) mkimm(i, FIXNUM_ITAG)
#define FIXNUM_MIN -8388608
#define FIXNUM_MAX 8388607
/* flonums */
extern cxtype_t *FLONUM_NTAG;
typedef double flonum_t;
#define is_flonum_obj(o) (isnative(o, FLONUM_NTAG))
#define is_flonum_flonum(f) ((void)(f), 1)
#define flonum_from_obj(o) (*(flonum_t*)getnative(o, FLONUM_NTAG))
#define flonum_from_flonum(l, f) (f)
#define void_from_flonum(l, f) (void)(f)
#define obj_from_flonum(l, f) hpushptr(dupflonum(f), FLONUM_NTAG, l)
extern flonum_t *dupflonum(flonum_t f);
/* characters */
#define CHAR_ITAG 2
typedef int char_t;
#define is_char_obj(o) (isimm(o, CHAR_ITAG))
#define is_char_char(i) ((i), 1)
#define char_from_obj(o) (getimms(o, CHAR_ITAG))
#define char_from_char(i) (i)
#define void_from_char(i) (void)(i)
#define obj_from_char(i) mkimm(i, CHAR_ITAG)
/* strings */
extern cxtype_t *STRING_NTAG;
#define isstring(o) (isnative(o, STRING_NTAG))
#define stringdata(o) ((int*)getnative(o, STRING_NTAG))
#define stringlen(o) (*stringdata(o))
#define stringchars(o) ((char*)(stringdata(o)+1))
#define hpushstr(l, s) hpushptr(s, STRING_NTAG, l)
#ifdef NDEBUG
  #define stringref(o, i) (stringchars(o)+(i))
#else
  extern char* stringref(obj o, int i);
#endif
extern int *newstring(char *s);
extern int *allocstring(int n, int c);
extern int *substring(int *d, int from, int to);
extern int *stringcat(int *d0, int *d1);
extern int *dupstring(int *d);
extern void stringfill(int *d, int c);
extern int strcmp_ci(char *s1, char*s2);
/* vectors */
#define VECTOR_BTAG 1
#define isvector(o) istagged(o, VECTOR_BTAG)
#define vectorref(v, i) *taggedref(v, VECTOR_BTAG, i)
#define vectorlen(v) taggedlen(v, VECTOR_BTAG)
/* boxes */
#define BOX_BTAG 2
#define isbox(o) istagged(o, BOX_BTAG)
#define boxref(o) *taggedref(o, BOX_BTAG, 0)
/* null */
#define NULL_ITAG 2
#define mknull() mkimm(0, NULL_ITAG)
#define isnull(o) ((o) == mkimm(0, NULL_ITAG))
/* pairs and lists */
#define PAIR_BTAG 3
#define ispair(o) istagged(o, PAIR_BTAG)
#define car(o) *taggedref(o, PAIR_BTAG, 0)
#define cdr(o) *taggedref(o, PAIR_BTAG, 1)
/* symbols */
#define SYMBOL_ITAG 3
#define issymbol(o) (isimm(o, SYMBOL_ITAG))
#define mksymbol(i) mkimm(i, SYMBOL_ITAG)
#define getsymbol(o) getimmu(o, SYMBOL_ITAG)
extern char *symbolname(int sym);
extern int internsym(char *name);
/* eof */
#define EOF_ITAG 127
#define mkeof() mkimm(-1, EOF_ITAG)
#define iseof(o) ((o) == mkimm(-1, EOF_ITAG))
/* input ports */
extern cxtype_t *IPORT_NTAG;
#define isiport(o) (isnative(o, IPORT_NTAG))
#define iportdata(o) ((FILE*)getnative(o, IPORT_NTAG))
#define mkiport(l, fp) hpushptr(fp, IPORT_NTAG, l)
/* output ports */
extern cxtype_t *OPORT_NTAG;
#define isoport(o) (isnative(o, OPORT_NTAG))
#define oportdata(o) ((FILE*)getnative(o, OPORT_NTAG))
#define mkoport(l, fp) hpushptr(fp, OPORT_NTAG, l)

/* cx globals */
extern obj cx__2Acurrent_2Derror_2Dport_2A; /* *current-error-port* */
extern obj cx__2Acurrent_2Doutput_2Dport_2A; /* *current-output-port* */
extern obj cx_analyze_2Dglobals; /* analyze-globals */
extern obj cx_beta_2Dsubstitute; /* beta-substitute */
extern obj cx_box_2Dexp_3F; /* box-exp? */
extern obj cx_boxref_2Dexp_3F; /* boxref-exp? */
extern obj cx_boxset_2Dexp_3F; /* boxset-exp? */
extern obj cx_c_2Derror_2A; /* c-error* */
extern obj cx_c_2Dmangle; /* c-mangle */
extern obj cx_code_2Dgenerate; /* code-generate */
extern obj cx_code_2Dhost_2Depilogue; /* code-host-epilogue */
extern obj cx_code_2Dhost_2Dprologue; /* code-host-prologue */
extern obj cx_code_2Dmain; /* code-main */
extern obj cx_code_2Dmodule; /* code-module */
extern obj cx_code_2Druntime; /* code-runtime */
extern obj cx_code_2Dstandard_2Ddefinitions; /* code-standard-definitions */
extern obj cx_code_2Dstandard_2Dincludes; /* code-standard-includes */
extern obj cx_constant_2Dfold; /* constant-fold */
extern obj cx_cps_2Dconvert; /* cps-convert */
extern obj cx_display_2Dtext; /* display-text */
extern obj cx_equal_3F; /* equal? */
extern obj cx_exp_2D_3Efree_2Dvars; /* exp->free-vars */
extern obj cx_exp_2Dvinfo; /* exp-vinfo */
extern obj cx_expand_2Dtop_2Dlevel_2Dforms_21; /* expand-top-level-forms! */
extern obj cx_file_2Dexpand_2Dtop_2Dlevel_2Dforms_21; /* file-expand-top-level-forms! */
extern obj cx_fix_2Dletrecs; /* fix-letrecs */
extern obj cx_fprintf_2A; /* fprintf* */
extern obj cx_id_2D_3Euname; /* id->uname */
extern obj cx_labelapp_2Dexp_3F; /* labelapp-exp? */
extern obj cx_lambda_2Dlift; /* lambda-lift */
extern obj cx_letrec_2A_2Dexp_2D_3Ebody; /* letrec*-exp->body */
extern obj cx_letrec_2A_2Dexp_2D_3Erands; /* letrec*-exp->rands */
extern obj cx_letrec_2A_2Dexp_3F; /* letrec*-exp? */
extern obj cx_letrec_2Dexp_3F; /* letrec-exp? */
extern obj cx_member; /* member */
extern obj cx_null_2Dmstore; /* null-mstore */
extern obj cx_parse_2Dprogram; /* parse-program */
extern obj cx_path_2Dstrip_2Ddirectory; /* path-strip-directory */
extern obj cx_path_2Dstrip_2Dextension; /* path-strip-extension */
extern obj cx_read_2F1; /* read/1 */
extern obj cx_remove_2Dassignments; /* remove-assignments */
extern obj cx_reset; /* reset */
extern obj cx_reset_2Dtimestamps; /* reset-timestamps */
extern obj cx_sexp_2Dmatch_3F; /* sexp-match? */
extern obj cx_stack_2Dfunctions_2Dcode_2Dgenerate; /* stack-functions-code-generate */
extern obj cx_unbox_2Dvalues; /* unbox-values */
extern obj cx_var_2Dreferenced_2Din_2Dexp_3F; /* var-referenced-in-exp? */
extern obj cx_var_2Duses_2Din_2Dexp; /* var-uses-in-exp */
extern obj cx_with_2Doutput_2Dto_2Dfile; /* with-output-to-file */
extern obj cx_write_2F3; /* write/3 */
obj cx__2Ashowlog_2A; /* *showlog* */
obj cx__2Asilent_2A; /* *silent* */
obj cx_clear_2Dmemoizations; /* clear-memoizations */
obj cx_compile_2Dfile; /* compile-file */
obj cx_display_2Dmodule; /* display-module */
obj cx_main; /* main */
obj cx_maybe_2Dlog; /* maybe-log */
obj cx_maybe_2Dpp_2Dsource; /* maybe-pp-source */
obj cx_maybe_2Dpp_2Dtext; /* maybe-pp-text */
obj cx_parse_2Dfile; /* parse-file */
obj cx_source; /* source */
static obj cx__231002; /* constant #1002 */
static obj cx__231021; /* constant #1021 */
static obj cx__231038; /* constant #1038 */
static obj cx__231043; /* constant #1043 */
static obj cx__231050; /* constant #1050 */
static obj cx__231074; /* constant #1074 */
static obj cx__231082; /* constant #1082 */
static obj cx__231087; /* constant #1087 */
static obj cx__231095; /* constant #1095 */
static obj cx__231103; /* constant #1103 */
static obj cx__231116; /* constant #1116 */
static obj cx__231119; /* constant #1119 */
static obj cx__231167; /* constant #1167 */
static obj cx__231180; /* constant #1180 */
static obj cx__231187; /* constant #1187 */
static obj cx__231194; /* constant #1194 */
static obj cx__231267; /* constant #1267 */
static obj cx__231276; /* constant #1276 */
static obj cx__231280; /* constant #1280 */
static obj cx__231283; /* constant #1283 */
static obj cx__231284; /* constant #1284 */
static obj cx__231288; /* constant #1288 */
static obj cx__231289; /* constant #1289 */
static obj cx__231293; /* constant #1293 */
static obj cx__231294; /* constant #1294 */
static obj cx__231298; /* constant #1298 */
static obj cx__231299; /* constant #1299 */
static obj cx__231303; /* constant #1303 */
static obj cx__231304; /* constant #1304 */
static obj cx__231308; /* constant #1308 */
static obj cx__231309; /* constant #1309 */
static obj cx__231313; /* constant #1313 */
static obj cx__231314; /* constant #1314 */
static obj cx__231318; /* constant #1318 */
static obj cx__231320; /* constant #1320 */
static obj cx__231322; /* constant #1322 */
static obj cx__231326; /* constant #1326 */
static obj cx__231327; /* constant #1327 */
static obj cx__231331; /* constant #1331 */
static obj cx__231332; /* constant #1332 */
static obj cx__231336; /* constant #1336 */
static obj cx__231337; /* constant #1337 */
static obj cx__231358; /* constant #1358 */
static obj cx__231360; /* constant #1360 */
static obj cx__231365; /* constant #1365 */
static obj cx__231368; /* constant #1368 */
static obj cx__231369; /* constant #1369 */
static obj cx__231370; /* constant #1370 */
static obj cx__231381; /* constant #1381 */
static obj cx__231384; /* constant #1384 */
static obj cx__231395; /* constant #1395 */
static obj cx__231399; /* constant #1399 */
static obj cx__2314; /* constant #14 */
static obj cx__231400; /* constant #1400 */
static obj cx__231404; /* constant #1404 */
static obj cx__231470; /* constant #1470 */
static obj cx__231479; /* constant #1479 */
static obj cx__231482; /* constant #1482 */
static obj cx__231486; /* constant #1486 */
static obj cx__231489; /* constant #1489 */
static obj cx__231492; /* constant #1492 */
static obj cx__231495; /* constant #1495 */
static obj cx__2315; /* constant #15 */
static obj cx__23247; /* constant #247 */
static obj cx__23626; /* constant #626 */
static obj cx__23629; /* constant #629 */
static obj cx__23631; /* constant #631 */
static obj cx__23635; /* constant #635 */
static obj cx__23915; /* constant #915 */
static obj cx__23958; /* constant #958 */
static obj cx__23964; /* constant #964 */
static obj cx__23972; /* constant #972 */
static obj cx__23977; /* constant #977 */
static obj cx__23982; /* constant #982 */
static obj cx__23990; /* constant #990 */
static obj cx__23995; /* constant #995 */
static obj cx_begin_2Dexp_3F_2355; /* constant begin-exp?#55 */
static obj cx_curry_2Dexp_3F_23513; /* constant curry-exp?#513 */
static obj cx_void_2Dexp_3F_23912; /* constant void-exp?#912 */

/* helper functions */
/* let-exp?#85 */
static obj cxs_let_2Dexp_3F_2385(obj v87_exp)
{ 
  if (bool_from_obj((isvector((v87_exp))) ? (((vectorlen((v87_exp))) == (3)) ? obj_from_bool((vectorref((v87_exp), (0))) == (mksymbol(internsym("app-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  { /* let */
    obj v91_rator = (vectorref((v87_exp), (1)));
  if (bool_from_obj((isvector((v91_rator))) ? (((vectorlen((v91_rator))) == (3)) ? obj_from_bool((vectorref((v91_rator), (0))) == (mksymbol(internsym("lambda-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  { /* let */
    obj v1675_tmp;
    obj v1674_tmp;
    { /* length */
    int n; obj l = (vectorref((v87_exp), (2)));
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v1675_tmp = obj_from_fixnum(n); };
    { /* length */
    int n; obj l = (vectorref((v91_rator), (1)));
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v1674_tmp = obj_from_fixnum(n); };
    return obj_from_bool(fixnum_from_obj(v1674_tmp) == fixnum_from_obj(v1675_tmp));
  }
  } else {
    return obj_from_bool(0);
  }
  }
  } else {
    return obj_from_bool(0);
  }
}

/* degenerate-let-exp->body#122 */
static obj cxs_degenerate_2Dlet_2Dexp_2D_3Ebody_23122(obj v124_exp)
{ 
    return ((isnull((vectorref((v124_exp), (2))))) ? (vectorref((vectorref((v124_exp), (1))), (2))) : (car((vectorref((v124_exp), (2))))));
}

/* null-let-exp?#148 */
static obj cxs_null_2Dlet_2Dexp_3F_23148(obj v150_exp)
{ 
  if (bool_from_obj((isvector((v150_exp))) ? (((vectorlen((v150_exp))) == (3)) ? obj_from_bool((vectorref((v150_exp), (0))) == (mksymbol(internsym("app-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  if ((isnull((vectorref((v150_exp), (2)))))) {
  { /* let */
    obj v154_rator = (vectorref((v150_exp), (1)));
    return (bool_from_obj((isvector((v154_rator))) ? (((vectorlen((v154_rator))) == (3)) ? obj_from_bool((vectorref((v154_rator), (0))) == (mksymbol(internsym("lambda-exp")))) : obj_from_bool(0)) : obj_from_bool(0)) ? obj_from_bool(isnull((vectorref((v154_rator), (1))))) : obj_from_bool(0));
  }
  } else {
    return obj_from_bool(0);
  }
  } else {
    return obj_from_bool(0);
  }
}

/* identity-lambda-exp?#194 */
static obj cxs_identity_2Dlambda_2Dexp_3F_23194(obj v196_exp)
{ 
  if (bool_from_obj((isvector((v196_exp))) ? (((vectorlen((v196_exp))) == (3)) ? obj_from_bool((vectorref((v196_exp), (0))) == (mksymbol(internsym("lambda-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  { /* let */
    obj v203_body = (vectorref((v196_exp), (2)));
    obj v204_ids = (vectorref((v196_exp), (1)));
    return ((ispair((v204_ids))) ? ((isnull((cdr((v204_ids))))) ? (bool_from_obj((isvector((v203_body))) ? (((vectorlen((v203_body))) == (2)) ? obj_from_bool((vectorref((v203_body), (0))) == (mksymbol(internsym("var-exp")))) : obj_from_bool(0)) : obj_from_bool(0)) ? obj_from_bool((car((v204_ids))) == (vectorref((v203_body), (1)))) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0));
  }
  } else {
    return obj_from_bool(0);
  }
}

/* identity-let-exp?#181 */
static obj cxs_identity_2Dlet_2Dexp_3F_23181(obj v183_exp)
{ 
  if (bool_from_obj((isvector((v183_exp))) ? (((vectorlen((v183_exp))) == (3)) ? obj_from_bool((vectorref((v183_exp), (0))) == (mksymbol(internsym("app-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  if (bool_from_obj(cxs_identity_2Dlambda_2Dexp_3F_23194((vectorref((v183_exp), (1)))))) {
  { /* let */
    obj v187_rands = (vectorref((v183_exp), (2)));
    return ((ispair((v187_rands))) ? obj_from_bool(isnull((cdr((v187_rands))))) : obj_from_bool(0));
  }
  } else {
    return obj_from_bool(0);
  }
  } else {
    return obj_from_bool(0);
  }
}

/* degenerate-let-exp?#145 */
static obj cxs_degenerate_2Dlet_2Dexp_3F_23145(obj v147_exp)
{ 
  { /* let */
    obj v180_x = (cxs_null_2Dlet_2Dexp_3F_23148((v147_exp)));
    return (bool_from_obj(v180_x) ? (v180_x) : (cxs_identity_2Dlet_2Dexp_3F_23181((v147_exp))));
  }
}

/* gc roots */
static obj *globv[] = {
  &cx__2Ashowlog_2A,
  &cx__2Asilent_2A,
  &cx__231002,
  &cx__231021,
  &cx__231038,
  &cx__231043,
  &cx__231050,
  &cx__231074,
  &cx__231082,
  &cx__231087,
  &cx__231095,
  &cx__231103,
  &cx__231116,
  &cx__231119,
  &cx__231167,
  &cx__231180,
  &cx__231187,
  &cx__231194,
  &cx__231267,
  &cx__231276,
  &cx__231280,
  &cx__231283,
  &cx__231284,
  &cx__231288,
  &cx__231289,
  &cx__231293,
  &cx__231294,
  &cx__231298,
  &cx__231299,
  &cx__231303,
  &cx__231304,
  &cx__231308,
  &cx__231309,
  &cx__231313,
  &cx__231314,
  &cx__231318,
  &cx__231320,
  &cx__231322,
  &cx__231326,
  &cx__231327,
  &cx__231331,
  &cx__231332,
  &cx__231336,
  &cx__231337,
  &cx__231358,
  &cx__231360,
  &cx__231365,
  &cx__231368,
  &cx__231369,
  &cx__231370,
  &cx__231381,
  &cx__231384,
  &cx__231395,
  &cx__231399,
  &cx__2314,
  &cx__231400,
  &cx__231404,
  &cx__231470,
  &cx__231479,
  &cx__231482,
  &cx__231486,
  &cx__231489,
  &cx__231492,
  &cx__231495,
  &cx__2315,
  &cx__23247,
  &cx__23626,
  &cx__23629,
  &cx__23631,
  &cx__23635,
  &cx__23915,
  &cx__23958,
  &cx__23964,
  &cx__23972,
  &cx__23977,
  &cx__23982,
  &cx__23990,
  &cx__23995,
};

static cxroot_t root = {
  sizeof(globv)/sizeof(obj *), globv, NULL
};

/* entry points */
static obj host(obj);
static obj cases[226] = {
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,
};

/* host procedure */
#define MAX_LIVEREGS 28
static obj host(obj pc)
{
  register obj *r = cxg_regs;
  register obj *hp = cxg_hp;
#ifndef NDEBUG
  register int rc = cxg_rc;
#endif
  jump: 
  switch (case_from_obj(pc)) {

case 0: /* load module */
    { static char s[] = { 58, 10, 0 };
    cx__2314 = (hpushstr(0, newstring(s))); }
    cx__2315 = (hpushstr(0, newstring("-------------------------- ")));
    { static obj c[] = { obj_from_case(1) }; cx_begin_2Dexp_3F_2355 = (obj)c; }
    cx__23247 = (hpushstr(0, newstring("unknown exp")));
    { static obj c[] = { obj_from_case(3) }; cx_curry_2Dexp_3F_23513 = (obj)c; }
    { static char s[] = { 46, 10, 0 };
    cx__23626 = (hpushstr(0, newstring(s))); }
    cx__23629 = (hpushstr(0, newstring("no clause matches ~s")));
    cx__23631 = (hpushstr(0, newstring("Error: ")));
    cx__23635 = (hpushstr(0, newstring("Error in ~a: ")));
    cx__23915 = (hpushstr(0, newstring("void(0)")));
    { static obj c[] = { obj_from_case(5) }; cx_void_2Dexp_3F_23912 = (obj)c; }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("<string>")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%definition")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23958 = (hendblk(3)); }
    { static char s[] = { 34, 10, 0 };
    cx__23964 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 35, 105, 110, 99, 108, 117, 100, 101, 32, 34, 0 };
    cx__23972 = (hpushstr(0, newstring(s))); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("<string>")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%include")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23977 = (hendblk(3)); }
    { static char s[] = { 10, 0 };
    cx__23982 = (hpushstr(0, newstring(s))); }
    cx__23990 = (hpushstr(0, newstring("#include ")));
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("<symbol>")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%include")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23995 = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("*")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("*")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("define")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__231002 = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("<string>")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("load")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__231021 = (hendblk(3)); }
    cx__231038 = (hpushstr(0, newstring("  visiting ~a~%")));
    cx__231043 = (hpushstr(0, newstring("  already visited: ~a~%")));
    cx__231050 = (hpushstr(0, newstring("module_")));
    { static char s[] = { 32, 42, 47, 10, 0 };
    cx__231074 = (hpushstr(0, newstring(s))); }
    cx__231082 = (hpushstr(0, newstring("(void); /* ")));
    cx__231087 = (hpushstr(0, newstring("extern void ")));
    cx__231095 = (hpushstr(0, newstring("load should precede definitions and expressions")));
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("<string>")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%localdef")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__231103 = (hendblk(3)); }
    cx__231116 = (hpushstr(0, newstring("%% section(s) should precede definitions and expressions")));
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mksymbol(internsym("*")));
    *--hp = (mksymbol(internsym("%%")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__231119 = (hendblk(3)); }
    cx__231167 = (hpushstr(0, newstring("(); ")));
    cx__231180 = (hpushstr(0, newstring("#define LOAD() ")));
    cx__231187 = (hpushstr(0, newstring("#define MODULE ")));
    cx__231194 = (hpushstr(0, newstring("/* ")));
    cx__231267 = (hpushstr(0, newstring(".c")));
    { static char s[] = { 32, 119, 114, 10, 0 };
    cx__231276 = (hpushstr(0, newstring(s))); }
    cx__231280 = (hpushstr(0, newstring("C CODE")));
    cx__231283 = (hpushstr(0, newstring(" cg")));
    cx__231284 = (hpushstr(0, newstring("EXP AFTER VALUE UNBOXING")));
    cx__231288 = (hpushstr(0, newstring(" uv")));
    cx__231289 = (hpushstr(0, newstring("EXP AFTER LABEL CONVERSION")));
    cx__231293 = (hpushstr(0, newstring(" b3")));
    cx__231294 = (hpushstr(0, newstring("EXP AFTER LAMBDA LIFTING")));
    cx__231298 = (hpushstr(0, newstring(" ll")));
    cx__231299 = (hpushstr(0, newstring("EXP AFTER POST-CPS BETA")));
    cx__231303 = (hpushstr(0, newstring(" b2")));
    cx__231304 = (hpushstr(0, newstring("EXP AFTER CPS-CONVERSION")));
    cx__231308 = (hpushstr(0, newstring(" cps")));
    cx__231309 = (hpushstr(0, newstring("EXP AFTER BETA-SUBSTITUTION")));
    cx__231313 = (hpushstr(0, newstring(" b1")));
    cx__231314 = (hpushstr(0, newstring("EXP AFTER CONSTANT-FOLDING")));
    cx__231318 = (hpushstr(0, newstring(" cf")));
    cx__231320 = (hpushstr(0, newstring(" scg")));
    cx__231322 = (hpushstr(0, newstring("EXP ANALYSIS OF GLOBALS")));
    cx__231326 = (hpushstr(0, newstring(" ag")));
    cx__231327 = (hpushstr(0, newstring("EXP AFTER BOXING")));
    cx__231331 = (hpushstr(0, newstring(" ra")));
    cx__231332 = (hpushstr(0, newstring("EXP AFTER LETREC FIXING")));
    cx__231336 = (hpushstr(0, newstring("  fl")));
    cx__231337 = (hpushstr(0, newstring("EXP")));
    { static char s[] = { 10, 47, 42, 32, 111, 115, 32, 101, 110, 116, 114, 121, 32, 112, 111, 105, 110, 116, 32, 42, 47, 0 };
    cx__231358 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 10, 47, 42, 32, 98, 97, 115, 105, 99, 32, 114, 117, 110, 116, 105, 109, 101, 32, 42, 47, 0 };
    cx__231360 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 10, 47, 42, 32, 109, 111, 100, 117, 108, 101, 32, 108, 111, 97, 100, 32, 42, 47, 0 };
    cx__231365 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 10, 47, 42, 32, 104, 111, 115, 116, 32, 112, 114, 111, 99, 101, 100, 117, 114, 101, 32, 42, 47, 10, 0 };
    cx__231368 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 47, 42, 32, 101, 110, 116, 114, 121, 32, 112, 111, 105, 110, 116, 115, 32, 42, 47, 10, 0 };
    cx__231369 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 47, 42, 32, 103, 99, 32, 114, 111, 111, 116, 115, 32, 42, 47, 10, 0 };
    cx__231370 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 47, 42, 32, 104, 101, 108, 112, 101, 114, 32, 102, 117, 110, 99, 116, 105, 111, 110, 115, 32, 42, 47, 10, 0 };
    cx__231381 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 10, 47, 42, 32, 99, 120, 32, 103, 108, 111, 98, 97, 108, 115, 32, 42, 47, 10, 0 };
    cx__231384 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 10, 47, 42, 32, 101, 120, 116, 114, 97, 32, 100, 101, 102, 105, 110, 105, 116, 105, 111, 110, 115, 32, 42, 47, 10, 0 };
    cx__231395 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 10, 47, 42, 32, 115, 116, 97, 110, 100, 97, 114, 100, 32, 100, 101, 102, 105, 110, 105, 116, 105, 111, 110, 115, 32, 42, 47, 10, 0 };
    cx__231399 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 47, 42, 32, 101, 120, 116, 114, 97, 32, 105, 110, 99, 108, 117, 100, 101, 115, 32, 42, 47, 10, 0 };
    cx__231400 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 10, 47, 42, 32, 115, 116, 97, 110, 100, 97, 114, 100, 32, 105, 110, 99, 108, 117, 100, 101, 115, 32, 42, 47, 10, 0 };
    cx__231404 = (hpushstr(0, newstring(s))); }
    cx__231470 = (hpushstr(0, newstring("sfc build 180~%Usage: sfc [-v] file ...~%")));
    cx__231479 = (hpushstr(0, newstring("~a~%")));
    cx__231482 = (hpushstr(0, newstring("compiling ~a~%")));
    cx__231486 = (hpushstr(0, newstring("cannot open input file: ~a~%")));
    cx__231489 = (hpushstr(0, newstring("--help")));
    cx__231492 = (hpushstr(0, newstring("-d")));
    cx__231495 = (hpushstr(0, newstring("-v")));
    { static obj c[] = { obj_from_case(6) }; cx_clear_2Dmemoizations = (obj)c; }
    cx__2Asilent_2A = obj_from_bool(1);
    cx__2Ashowlog_2A = obj_from_bool(0);
    { static obj c[] = { obj_from_case(9) }; cx_maybe_2Dpp_2Dsource = (obj)c; }
    { static obj c[] = { obj_from_case(14) }; cx_maybe_2Dpp_2Dtext = (obj)c; }
    { static obj c[] = { obj_from_case(18) }; cx_maybe_2Dlog = (obj)c; }
    { static obj c[] = { obj_from_case(20) }; cx_source = (obj)c; }
    { static obj c[] = { obj_from_case(95) }; cx_parse_2Dfile = (obj)c; }
    { static obj c[] = { obj_from_case(130) }; cx_compile_2Dfile = (obj)c; }
    { static obj c[] = { obj_from_case(184) }; cx_display_2Dmodule = (obj)c; }
    { static obj c[] = { obj_from_case(219) }; cx_main = (obj)c; }
    r[0] = obj_from_void(0);
    r[1+0] = r[0];
    pc = 0; /* exit from module init */
    r[1+1] = r[0];  
    r += 1; /* shift reg wnd */
    assert(rc = 2);
    goto jump;

case 1: /* begin-exp?#55 k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_begin_2Dexp_3F_2355: /* k exp */
  if (bool_from_obj(cxs_let_2Dexp_3F_2385((r[1])))) {
    r[2] = (vectorref((r[1]), (1)));
    r[2] = (vectorref((r[2]), (1)));
    r[3] = (vectorref((r[1]), (1)));
    r[3] = (vectorref((r[3]), (2)));
  if ((ispair((r[2])))) {
    r[4] = (cdr((r[2])));
    r[4] = obj_from_bool(isnull((r[4])));
  if (bool_from_obj(r[4])) {
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(2);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_var_2Dreferenced_2Din_2Dexp_3F);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (car((r[2])));
    r[5+3] = r[3];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(0);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 2: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    r[3+0] = r[2];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(!bool_from_obj(r[1]));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 3: /* curry-exp?#513 k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_curry_2Dexp_3F_23513: /* k exp */
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[2] = (vectorref((r[1]), (0)));
    r[2] = obj_from_bool((r[2]) == (mksymbol(internsym("lambda-exp"))));
  } else {
    r[2] = obj_from_bool(0);
  }
  } else {
    r[2] = obj_from_bool(0);
  }
  if (bool_from_obj(r[2])) {
    r[2] = (vectorref((r[1]), (1)));
    r[3] = (vectorref((r[1]), (2)));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(4);
    r[4] = (hendblk(3+1));
    r[5+0] = (cx_labelapp_2Dexp_3F);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = r[3];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(0);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 4: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r ids k body */
  if (bool_from_obj(r[1])) {
    r[5] = (vectorref((r[4]), (2)));
    r[0] = r[3];  
    r[1] = r[2];  
    r[2] = r[5];  
    goto s_loop;
  } else {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

s_loop: /* k ids rands */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((ispair((r[2])))) {
    r[3] = (car((r[2])));
  if ((isvector((r[3])))) {
  if (((vectorlen((r[3]))) == (2))) {
    r[4] = (vectorref((r[3]), (0)));
    r[3] = obj_from_bool((r[4]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    r[3] = (car((r[2])));
    r[3] = (vectorref((r[3]), (1)));
    r[4] = (car((r[1])));
    r[3] = obj_from_bool((r[3]) == (r[4]));
  if (bool_from_obj(r[3])) {
    r[3] = (cdr((r[1])));
    r[4] = (cdr((r[2])));
    /* r[0] */    
    r[1] = r[3];  
    r[2] = r[4];  
    goto s_loop;
  } else {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }

case 5: /* void-exp?#912 k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_void_2Dexp_3F_23912: /* k exp */
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[2] = (vectorref((r[1]), (0)));
    r[2] = obj_from_bool((r[2]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[2] = obj_from_bool(0);
  }
  } else {
    r[2] = obj_from_bool(0);
  }
  if (bool_from_obj(r[2])) {
    r[2+0] = (cx_equal_3F);
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = r[0];  
    r[2+2] = (vectorref((r[1]), (2)));
    r[2+3] = (cx__23915);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(0);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 6: /* clear-memoizations k */
    assert(rc == 2);
    r += 1; /* shift reg. wnd */
gs_clear_2Dmemoizations: /* k */
    hreserve(hbsz(1+1), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(7);
    r[1] = (hendblk(1+1));
    r[2+0] = (cx_exp_2Dvinfo);
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = r[1];  
    r[2+2] = obj_from_bool(0);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 7: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek  k */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(8);
    r[3] = (hendblk(1+1));
    r[4+0] = (cx_var_2Duses_2Din_2Dexp);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = obj_from_bool(0);
    r[4+3] = obj_from_bool(0);
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 8: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek  k */
    r[0] = (cx_exp_2D_3Efree_2Dvars);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[2];  
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 9: /* maybe-pp-source k msg exp */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_maybe_2Dpp_2Dsource: /* k msg exp */
  if ((!bool_from_obj(cx__2Asilent_2A))) {
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(10);
    r[3] = (hendblk(3+1));
    r[4+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (cx__2315);
    r[4+3] = obj_from_bool(1);
    r[4+4] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 10: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  msg exp k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(11);
    r[5] = (hendblk(2+1));
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    /* r[2] */    
    r[3] = obj_from_bool(1);
    r[4] = (cx__2Acurrent_2Doutput_2Dport_2A);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 11: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  exp k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(12);
    r[4] = (hendblk(2+1));
    r[5+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cx__2314);
    r[5+3] = obj_from_bool(1);
    r[5+4] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 12: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  exp k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(13);
    r[4] = (hendblk(1+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto gs_source;

case 13: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    r[3+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[2];  
    r[3+2] = r[1];  
    r[3+3] = obj_from_bool(0);
    r[3+4] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 14: /* maybe-pp-text k msg text */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_maybe_2Dpp_2Dtext: /* k msg text */
  if ((!bool_from_obj(cx__2Asilent_2A))) {
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(15);
    r[3] = (hendblk(3+1));
    r[4+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (cx__2315);
    r[4+3] = obj_from_bool(1);
    r[4+4] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 15: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  msg text k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(16);
    r[5] = (hendblk(2+1));
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    /* r[2] */    
    r[3] = obj_from_bool(1);
    r[4] = (cx__2Acurrent_2Doutput_2Dport_2A);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 16: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  text k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(17);
    r[4] = (hendblk(2+1));
    r[5+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cx__2314);
    r[5+3] = obj_from_bool(1);
    r[5+4] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 17: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  text k */
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 18: /* maybe-log k msg */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_maybe_2Dlog: /* k msg */
  if (bool_from_obj(cx__2Ashowlog_2A)) {
    r[2] = (cx__2Acurrent_2Doutput_2Dport_2A);
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = obj_from_case(19);
    r[3] = (hendblk(2+1));
    r[4+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[1];  
    r[4+3] = obj_from_bool(1);
    r[4+4] = r[2];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_void(0);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 19: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  k op */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(fflush(oportdata((r[3]))));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 20: /* source k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_source: /* k exp */
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(90);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(21);
    r[2] = (hendblk(3+1));
    r[0] = r[2];  
    /* r[1] */    
    goto gs_void_2Dexp_3F_23912;

case 21: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r source-body exp k */
  if (bool_from_obj(r[1])) {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mksymbol(internsym("%%void")));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[3])))) {
  if (((vectorlen((r[3]))) == (2))) {
    r[5] = (vectorref((r[3]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[3]), (1)));
    r[0] = (cx_id_2D_3Euname);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[3])))) {
  if (((vectorlen((r[3]))) == (3))) {
    r[5] = (vectorref((r[3]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("varassign-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[3]), (1)));
    r[6] = (vectorref((r[3]), (2)));
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = obj_from_case(22);
    r[7] = (hendblk(2+1));
    r[0] = r[7];  
    r[1] = r[6];  
    goto gs_source;
  } else {
  if ((isvector((r[3])))) {
  if (((vectorlen((r[3]))) == (4))) {
    r[5] = (vectorref((r[3]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("if-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[3]), (1)));
    r[6] = (vectorref((r[3]), (2)));
    r[7] = (vectorref((r[3]), (3)));
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = obj_from_case(24);
    r[8] = (hendblk(3+1));
    r[0] = r[8];  
    r[1] = r[7];  
    goto gs_source;
  } else {
  if (bool_from_obj(cxs_degenerate_2Dlet_2Dexp_3F_23145((r[3])))) {
    r[0] = r[4];  
    r[1] = (cxs_degenerate_2Dlet_2Dexp_2D_3Ebody_23122((r[3])));
    goto gs_source;
  } else {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[2];  
    *--hp = obj_from_case(27);
    r[5] = (hendblk(3+1));
    r[0] = (cx_letrec_2Dexp_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }
  }
  }
  }

case 22: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(23);
    r[5] = (hendblk(2+1));
    r[0] = (cx_id_2D_3Euname);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 23: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("set!")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 24: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r then-exp test-exp k */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(25);
    r[6] = (hendblk(3+1));
    r[0] = r[6];  
    r[1] = r[2];  
    goto gs_source;

case 25: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r test-exp k r */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = obj_from_case(26);
    r[6] = (hendblk(2+1));
    r[0] = r[6];  
    r[1] = r[2];  
    goto gs_source;

case 26: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("if")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 27: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r source-body k exp */
  if (bool_from_obj(r[1])) {
    r[5] = (vectorref((r[4]), (1)));
    r[5] = (vectorref((r[5]), (1)));
    r[6] = (vectorref((r[4]), (1)));
    r[6] = (vectorref((r[6]), (2)));
    r[7] = (vectorref((r[6]), (2)));
    r[6] = (car((r[7])));
    r[6] = (vectorref((r[6]), (2)));
    r[7] = (vectorref((r[4]), (1)));
    r[7] = (vectorref((r[7]), (2)));
    r[8] = (vectorref((r[7]), (1)));
    r[7] = (vectorref((r[8]), (2)));
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = obj_from_case(28);
    r[8] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(33);
    r[5] = (hendblk(3+1));
    r[0] = (cx_letrec_2A_2Dexp_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 28: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r rands ids k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(32);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v4110;

s_loop_v4110: /* k id id */
  if (((isnull((r[1]))) || (isnull((r[2]))))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    r[4] = (cdr((r[2])));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = obj_from_case(29);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[3];  
    r[2] = r[4];  
    goto s_loop_v4110;
  }

case 29: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id id */
    r[5] = (car((r[4])));
    r[6] = (car((r[3])));
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[5];  
    *--hp = obj_from_case(30);
    r[7] = (hendblk(3+1));
    r[0] = r[7];  
    r[1] = r[6];  
    goto gs_source;

case 30: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k r */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(31);
    r[6] = (hendblk(3+1));
    r[0] = (cx_id_2D_3Euname);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 31: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r r */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 32: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* list* */
    obj p;
    hreserve(hbsz(3)*3, 4); /* 4 live regs */
    p = r[3];   /* gc-safe */
    *--hp = p; *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (mksymbol(internsym("letrec")));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    r[4] = (p); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 33: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r source-body k exp */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(34);
    r[5] = (hendblk(3+1));
    r[0] = (cx_letrec_2A_2Dexp_2D_3Ebody);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(41);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[4];  
    goto gs_begin_2Dexp_3F_2355;
  }

case 34: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r source-body k exp */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(35);
    r[5] = (hendblk(4+1));
    r[0] = (cx_letrec_2A_2Dexp_2D_3Erands);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 35: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r source-body r k exp */
    r[6] = (vectorref((r[5]), (1)));
    r[6] = (vectorref((r[6]), (1)));
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = r[1];  
    *--hp = obj_from_case(36);
    r[7] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 36: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r r ids k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(40);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v4061;

s_loop_v4061: /* k id id */
  if (((isnull((r[1]))) || (isnull((r[2]))))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    r[4] = (cdr((r[2])));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = obj_from_case(37);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[3];  
    r[2] = r[4];  
    goto s_loop_v4061;
  }

case 37: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id id */
    r[5] = (car((r[4])));
    r[6] = (car((r[3])));
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[5];  
    *--hp = obj_from_case(38);
    r[7] = (hendblk(3+1));
    r[0] = r[7];  
    r[1] = r[6];  
    goto gs_source;

case 38: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k r */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(39);
    r[6] = (hendblk(3+1));
    r[0] = (cx_id_2D_3Euname);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 39: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r r */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 40: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* list* */
    obj p;
    hreserve(hbsz(3)*3, 4); /* 4 live regs */
    p = r[3];   /* gc-safe */
    *--hp = p; *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (mksymbol(internsym("letrec*")));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    r[4] = (p); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 41: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r source-body k exp */
  if (bool_from_obj(r[1])) {
    r[5] = (vectorref((r[4]), (2)));
    r[5] = (car((r[5])));
    r[6] = (vectorref((r[4]), (1)));
    r[6] = (vectorref((r[6]), (2)));
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_case(42);
    r[7] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (bool_from_obj(cxs_let_2Dexp_3F_2385((r[4])))) {
    r[5] = (vectorref((r[4]), (1)));
    r[5] = (vectorref((r[5]), (1)));
    r[6] = (vectorref((r[4]), (2)));
    r[7] = (vectorref((r[4]), (1)));
    r[7] = (vectorref((r[7]), (2)));
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = obj_from_case(44);
    r[8] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[4])))) {
  if (((vectorlen((r[4]))) == (4))) {
    r[5] = (vectorref((r[4]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("fix-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[4]), (1)));
    r[6] = (vectorref((r[4]), (2)));
    r[7] = (vectorref((r[4]), (3)));
    hreserve(hbsz(4+1), 8); /* 8 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = r[2];  
    *--hp = obj_from_case(49);
    r[8] = (hendblk(4+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[4])))) {
  if (((vectorlen((r[4]))) == (3))) {
    r[5] = (vectorref((r[4]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("app-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[4]), (1)));
    r[6] = (vectorref((r[4]), (2)));
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_case(62);
    r[7] = (hendblk(2+1));
    r[0] = r[7];  
    r[1] = r[6];  
    goto s_loop_v3901;
  } else {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(64);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[4];  
    goto gs_curry_2Dexp_3F_23513;
  }
  }
  }
  }

case 42: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r exp1 k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(43);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto gs_source;

case 43: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* list* */
    obj p;
    hreserve(hbsz(3)*3, 4); /* 4 live regs */
    p = r[3];   /* gc-safe */
    *--hp = p; *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (mksymbol(internsym("begin")));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    r[4] = (p); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 44: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r rands ids k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(48);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v4004;

s_loop_v4004: /* k id id */
  if (((isnull((r[1]))) || (isnull((r[2]))))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    r[4] = (cdr((r[2])));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = obj_from_case(45);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[3];  
    r[2] = r[4];  
    goto s_loop_v4004;
  }

case 45: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id id */
    r[5] = (car((r[4])));
    r[6] = (car((r[3])));
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[5];  
    *--hp = obj_from_case(46);
    r[7] = (hendblk(3+1));
    r[0] = r[7];  
    r[1] = r[6];  
    goto gs_source;

case 46: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k r */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(47);
    r[6] = (hendblk(3+1));
    r[0] = (cx_id_2D_3Euname);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 47: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r r */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 48: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* list* */
    obj p;
    hreserve(hbsz(3)*3, 4); /* 4 live regs */
    p = r[3];   /* gc-safe */
    *--hp = p; *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (mksymbol(internsym("let")));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    r[4] = (p); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 49: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r source-body lams ids k */
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = obj_from_case(59);
    r[6] = (hendblk(2+1));
    r[7+0] = r[6];  
    r[7+1] = r[4];  
    r[7+2] = r[3];  
    r[7+3] = r[2];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v3926;

s_loop_v3926: /* k id id source-body */
  if (((isnull((r[1]))) || (isnull((r[2]))))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[4] = (cdr((r[1])));
    r[5] = (cdr((r[2])));
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[3];  
    *--hp = obj_from_case(50);
    r[6] = (hendblk(4+1));
    r[0] = r[6];  
    r[1] = r[4];  
    r[2] = r[5];  
    /* r[3] */    
    goto s_loop_v3926;
  }

case 50: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r source-body k id id */
    r[6] = (car((r[5])));
    r[7] = (car((r[4])));
    hreserve(hbsz(2+1), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(51);
    r[8] = (hendblk(2+1));
  if ((isvector((r[7])))) {
  if (((vectorlen((r[7]))) == (3))) {
    r[9] = (vectorref((r[7]), (0)));
    r[9] = obj_from_bool((r[9]) == (mksymbol(internsym("lambda-exp"))));
  } else {
    r[9] = obj_from_bool(0);
  }
  } else {
    r[9] = obj_from_bool(0);
  }
  if (bool_from_obj(r[9])) {
    r[9] = (vectorref((r[7]), (1)));
    r[10] = (vectorref((r[7]), (2)));
    hreserve(hbsz(3+1), 11); /* 11 live regs */
    *--hp = r[8];  
    *--hp = r[6];  
    *--hp = r[9];  
    *--hp = obj_from_case(52);
    r[11] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[11]);
    r[2] = (r[10]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[9] = (cx__2Acurrent_2Derror_2Dport_2A);
    (void)(fputc('\n', oportdata((r[9]))));
    hreserve(hbsz(3+1), 10); /* 10 live regs */
    *--hp = r[8];  
    *--hp = r[9];  
    *--hp = r[7];  
    *--hp = obj_from_case(56);
    r[10] = (hendblk(3+1));
  if (bool_from_obj(mksymbol(internsym("variant-case")))) {
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("variant-case")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    r[0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = r[9];  
    r[3] = (cx__23635);
    r[4] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cx__23631);
    r[3] = obj_from_bool(1);
    r[4] = r[9];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }
  }

case 51: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 52: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r ids id k */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(55);
    r[6] = (hendblk(2+1));
    r[0] = r[6];  
    r[1] = r[5];  
    goto s_loop_v3955;

s_loop_v3955: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(53);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v3955;
  }

case 53: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(54);
    r[4] = (hendblk(2+1));
    r[0] = (cx_id_2D_3Euname);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (car((r[2])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 54: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 55: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* list* */
    obj p;
    hreserve(hbsz(3)*3, 4); /* 4 live regs */
    p = r[3];   /* gc-safe */
    *--hp = p; *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (mksymbol(internsym("define")));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    r[4] = (p); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 56: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  lam ep k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(57);
    r[5] = (hendblk(2+1));
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[7+0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[5];  
    r[7+2] = r[3];  
    r[7+3] = (cx__23629);
    r[7+4] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 57: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  ep k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(58);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cx__23626);
    r[5+3] = obj_from_bool(1);
    r[5+4] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 58: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek  k */
    r[0] = (cx_reset);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[2];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 59: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { fixnum_t v4317_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v4317_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v4317_tmp);
    hreserve(hbsz(3)*c, 4); /* 4 live regs */
    l = r[1];   t = r[3];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[4] = (o); } }
    { /* list* */
    obj p;
    hreserve(hbsz(3)*3, 5); /* 5 live regs */
    p = r[4];   /* gc-safe */
    *--hp = p; *--hp = (mknull());
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (mksymbol(internsym("let")));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    r[5] = (p); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v3901: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(60);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v3901;
  }

case 60: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(61);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = (car((r[2])));
    goto gs_source;

case 61: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 62: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r rator k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(63);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto gs_source;

case 63: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 64: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r source-body k exp */
  if (bool_from_obj(r[1])) {
    r[5] = (vectorref((r[4]), (2)));
    r[5] = (vectorref((r[5]), (1)));
    r[5] = (vectorref((r[5]), (1)));
    r[6] = (vectorref((r[4]), (1)));
    { fixnum_t v4316_tmp;
    r[7] = (vectorref((r[4]), (2)));
    r[7] = (vectorref((r[7]), (2)));
    r[8] = (vectorref((r[4]), (1)));
    { /* length */
    int n; obj l = r[8];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v4316_tmp = (n); }
    { /* list-tail */
    obj l = r[7];   int c = (v4316_tmp);
    while (c-- > 0) l = cdr(l);
    r[7] = (l); } }
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = obj_from_case(67);
    r[8] = (hendblk(3+1));
    r[0] = r[8];  
    r[1] = r[7];  
    goto s_loop_v3868;
  } else {
  if ((isvector((r[4])))) {
  if (((vectorlen((r[4]))) == (3))) {
    r[5] = (vectorref((r[4]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("lambda-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[4]), (1)));
    r[6] = (vectorref((r[4]), (2)));
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_case(72);
    r[7] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(76);
    r[5] = (hendblk(3+1));
    r[0] = (cx_box_2Dexp_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }

s_loop_v3868: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(65);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v3868;
  }

case 65: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(66);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = (car((r[2])));
    goto gs_source;

case 66: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 67: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r ids id k */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(70);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[2];  
    goto s_loop_v3847;

s_loop_v3847: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(68);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v3847;
  }

case 68: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(69);
    r[4] = (hendblk(2+1));
    r[0] = (cx_id_2D_3Euname);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (car((r[2])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 69: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 70: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k r */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(71);
    r[5] = (hendblk(3+1));
    r[0] = (cx_id_2D_3Euname);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 71: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r r */
    { /* list* */
    obj p;
    hreserve(hbsz(3)*4, 5); /* 5 live regs */
    p = r[3];   /* gc-safe */
    *--hp = p; *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (mksymbol(internsym("%%curry")));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    r[5] = (p); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 72: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r ids k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(75);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto s_loop_v3812;

s_loop_v3812: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(73);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v3812;
  }

case 73: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(74);
    r[4] = (hendblk(2+1));
    r[0] = (cx_id_2D_3Euname);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (car((r[2])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 74: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 75: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* list* */
    obj p;
    hreserve(hbsz(3)*3, 4); /* 4 live regs */
    p = r[3];   /* gc-safe */
    *--hp = p; *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (mksymbol(internsym("lambda")));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    r[4] = (p); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 76: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r source-body k exp */
  if (bool_from_obj(r[1])) {
    r[5] = (vectorref((r[4]), (3)));
    r[5] = (car((r[5])));
    hreserve(hbsz(1+1), 6); /* 6 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(77);
    r[6] = (hendblk(1+1));
    r[0] = r[6];  
    r[1] = r[5];  
    goto gs_source;
  } else {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(78);
    r[5] = (hendblk(3+1));
    r[0] = (cx_boxref_2Dexp_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 77: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = (mksymbol(internsym("%%box")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 78: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r source-body k exp */
  if (bool_from_obj(r[1])) {
    r[5] = (vectorref((r[4]), (3)));
    r[5] = (car((r[5])));
    hreserve(hbsz(1+1), 6); /* 6 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(79);
    r[6] = (hendblk(1+1));
    r[0] = r[6];  
    r[1] = r[5];  
    goto gs_source;
  } else {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(80);
    r[5] = (hendblk(3+1));
    r[0] = (cx_boxset_2Dexp_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 79: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = (mksymbol(internsym("%%box-ref")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 80: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r source-body k exp */
  if (bool_from_obj(r[1])) {
    r[5] = (vectorref((r[4]), (3)));
    r[5] = (car((r[5])));
    r[6] = (vectorref((r[4]), (3)));
    r[7] = (cdr((r[6])));
    r[6] = (car((r[7])));
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_case(81);
    r[7] = (hendblk(2+1));
    r[0] = r[7];  
    r[1] = r[6];  
    goto gs_source;
  } else {
  if ((isvector((r[4])))) {
  if (((vectorlen((r[4]))) == (4))) {
    r[5] = (vectorref((r[4]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[4]), (1)));
    r[6] = (vectorref((r[4]), (2)));
    r[7] = (vectorref((r[4]), (3)));
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = r[3];  
    *--hp = obj_from_case(85);
    r[8] = (hendblk(3+1));
    r[0] = r[8];  
    r[1] = r[7];  
    goto s_loop_v3751;
  } else {
  if ((isvector((r[4])))) {
  if (((vectorlen((r[4]))) == (3))) {
    r[5] = (vectorref((r[4]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("letcc-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[4]), (1)));
    r[6] = (vectorref((r[4]), (2)));
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_case(86);
    r[7] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[4])))) {
  if (((vectorlen((r[4]))) == (3))) {
    r[5] = (vectorref((r[4]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("withcc-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[4]), (1)));
    r[6] = (vectorref((r[4]), (2)));
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_case(88);
    r[7] = (hendblk(2+1));
    r[0] = r[7];  
    r[1] = r[6];  
    goto gs_source;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = (cx_c_2Derror_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = (cx__23247);
    r[3] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }
  }
  }
  }

case 81: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r box k */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(82);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = r[2];  
    goto gs_source;

case 82: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("%%box-set!")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v3751: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(83);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v3751;
  }

case 83: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(84);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = (car((r[2])));
    goto gs_source;

case 84: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 85: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k prim effect */
    { /* list* */
    obj p;
    hreserve(hbsz(3)*3, 5); /* 5 live regs */
    p = r[1];   /* gc-safe */
    *--hp = p; *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (((((r[4]) == (mksymbol(internsym("no-effect")))) || ((is_flonum_obj(r[4])) && ((is_flonum_obj(mksymbol(internsym("no-effect")))) && (flonum_from_obj(r[4]) == flonum_from_obj(mksymbol(internsym("no-effect"))))))) && (1)) ? (mksymbol(internsym("%prim"))) : (((((r[4]) == (mksymbol(internsym("*-effect")))) || ((is_flonum_obj(r[4])) && ((is_flonum_obj(mksymbol(internsym("*-effect")))) && (flonum_from_obj(r[4]) == flonum_from_obj(mksymbol(internsym("*-effect"))))))) && (1)) ? (mksymbol(internsym("%prim*"))) : (((((r[4]) == (mksymbol(internsym("?-effect")))) || ((is_flonum_obj(r[4])) && ((is_flonum_obj(mksymbol(internsym("?-effect")))) && (flonum_from_obj(r[4]) == flonum_from_obj(mksymbol(internsym("?-effect"))))))) && (1)) ? (mksymbol(internsym("%prim?"))) : (((((r[4]) == (mksymbol(internsym("!-effect")))) || ((is_flonum_obj(r[4])) && ((is_flonum_obj(mksymbol(internsym("!-effect")))) && (flonum_from_obj(r[4]) == flonum_from_obj(mksymbol(internsym("!-effect"))))))) && (1)) ? (mksymbol(internsym("%prim!"))) : (((((r[4]) == (mksymbol(internsym("?!-effect")))) || ((is_flonum_obj(r[4])) && ((is_flonum_obj(mksymbol(internsym("?!-effect")))) && (flonum_from_obj(r[4]) == flonum_from_obj(mksymbol(internsym("?!-effect"))))))) && (1)) ? (mksymbol(internsym("%prim?!"))) : (((((r[4]) == (mksymbol(internsym("*?-effect")))) || ((is_flonum_obj(r[4])) && ((is_flonum_obj(mksymbol(internsym("*?-effect")))) && (flonum_from_obj(r[4]) == flonum_from_obj(mksymbol(internsym("*?-effect"))))))) && (1)) ? (mksymbol(internsym("%prim*?"))) : (((((r[4]) == (mksymbol(internsym("*!-effect")))) || ((is_flonum_obj(r[4])) && ((is_flonum_obj(mksymbol(internsym("*!-effect")))) && (flonum_from_obj(r[4]) == flonum_from_obj(mksymbol(internsym("*!-effect"))))))) && (1)) ? (mksymbol(internsym("%prim*!"))) : (mksymbol(internsym("%prim*?!"))))))))));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    r[5] = (p); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 86: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(87);
    r[4] = (hendblk(2+1));
    r[0] = (cx_id_2D_3Euname);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 87: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* list* */
    obj p;
    hreserve(hbsz(3)*3, 4); /* 4 live regs */
    p = r[3];   /* gc-safe */
    *--hp = p; *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (mksymbol(internsym("letcc")));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    r[4] = (p); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 88: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r cont-exp k */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(89);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = r[2];  
    goto gs_source;

case 89: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("withcc")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 90: /* clo k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
s_source_2Dbody: /* k exp */
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(90);
    r[2] = (hendblk(0+1));
  if (bool_from_obj(cxs_degenerate_2Dlet_2Dexp_3F_23145((r[1])))) {
    r[3] = (cxs_degenerate_2Dlet_2Dexp_2D_3Ebody_23122((r[1])));
    /* r[0] */    
    r[1] = r[3];  
    goto s_source_2Dbody;
  } else {
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(91);
    r[3] = (hendblk(3+1));
    r[0] = r[3];  
    /* r[1] */    
    goto gs_begin_2Dexp_3F_2355;
  }

case 91: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r source-body k exp */
  if (bool_from_obj(r[1])) {
    r[5] = (vectorref((r[4]), (2)));
    r[5] = (car((r[5])));
    r[6] = (vectorref((r[4]), (1)));
    r[6] = (vectorref((r[6]), (2)));
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_case(92);
    r[7] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(94);
    r[5] = (hendblk(1+1));
    r[0] = r[5];  
    r[1] = r[4];  
    goto gs_source;
  }

case 92: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r exp1 k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(93);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto gs_source;

case 93: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 94: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 95: /* parse-file k filename */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_parse_2Dfile: /* k filename */
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(96);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(1), 3); /* 3 live regs */
    *--hp = obj_from_void(0);
    r[3] = (hendblk(1));
    hreserve(hbsz(1), 4); /* 4 live regs */
    *--hp = obj_from_void(0);
    r[4] = (hendblk(1));
    hreserve(hbsz(1), 5); /* 5 live regs */
    *--hp = obj_from_void(0);
    r[5] = (hendblk(1));
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(117);
    r[6] = (hendblk(4+1));
    hreserve(hbsz(7+1), 7); /* 7 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[6];  
    *--hp = obj_from_case(99);
    r[6] = (hendblk(7+1));
    r[7+0] = (cx_null_2Dmstore);
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 96: /* clo k filename */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k filename */
    hreserve(hbsz(1+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(97);
    r[2] = (hendblk(1+1));
    r[3+0] = (cx_path_2Dstrip_2Ddirectory);
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[2];  
    r[3+2] = r[1];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 97: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(98);
    r[3] = (hendblk(1+1));
    r[4+0] = (cx_path_2Dstrip_2Dextension);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 98: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    r[3+0] = (cx_c_2Dmangle);
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[2];  
    r[3+2] = r[1];  
    r[3+3] = (cx__231050);
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 99: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r module-add-load! module-name k filename module-visits module-loads mstore */
    (void)(objptr_from_obj(r[8])[0] = (r[1]));
    (void)(objptr_from_obj(r[7])[0] = (mknull()));
    (void)(objptr_from_obj(r[6])[0] = (mknull()));
    r[9] = (mkiport(9, cxm_cknull(fopen(stringchars((r[5])), "r"), "fopen")));
    hreserve(hbsz(2+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[4];  
    *--hp = obj_from_case(116);
    r[10] = (hendblk(2+1));
    r[11+0] = (r[10]);
    r[11+1] = (mknull());
    r[11+2] = (mknull());
    r[11+3] = (mknull());
    r[11+4] = (mknull());
    r[11+5] = r[9];  
    r[11+6] = r[2];  
    r[11+7] = r[8];  
    r[11+8] = r[7];  
    r[11+9] = r[3];  
    r[11+10] = r[5];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v3537;

case 100: /* clo k specs incs defs forms */
    assert(rc == 6);
    { obj* p = objptr_from_obj(r[0]);
    r[1+5] = p[1];
    r[1+6] = p[2];
    r[1+7] = p[3];
    r[1+8] = p[4];
    r[1+9] = p[5];
    r[1+10] = p[6]; }
    r += 1; /* shift reg. wnd */
s_loop_v3537: /* k specs incs defs forms p module-add-load! mstore module-loads module-name filename */
    hreserve(hbsz(6+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(100);
    r[11] = (hendblk(6+1));
    hreserve(hbsz(11+1), 12); /* 12 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = (r[10]);
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[4];  
    *--hp = r[7];  
    *--hp = (r[11]);
    *--hp = r[6];  
    *--hp = obj_from_case(101);
    r[11] = (hendblk(11+1));
    r[12+0] = (cx_read_2F1);
    pc = objptr_from_obj(r[12+0])[0];
    r[12+1] = (r[11]);
    r[12+2] = r[5];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 101: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9];
    r[1+11] = p[10];
    r[1+12] = p[11]; }
    r += 1; /* shift reg. wnd */
    /* ek r module-add-load! loop mstore forms module-loads module-name k specs filename incs defs */
  if ((iseof((r[1])))) {
    hreserve(hbsz(7+1), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_case(102);
    r[13] = (hendblk(7+1));
    { fixnum_t v4315_tmp;
    { /* length */
    int n; obj l = r[5];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v4315_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v4315_tmp);
    hreserve(hbsz(3)*c, 14); /* 14 live regs */
    l = r[5];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[14] = (o); } }
    r[0] = (cx_expand_2Dtop_2Dlevel_2Dforms_21);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[13]);
    r[2] = (r[14]);
    r[3] = (objptr_from_obj(r[4])[0]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    hreserve(hbsz(8+1), 13); /* 13 live regs */
    *--hp = r[8];  
    *--hp = r[9];  
    *--hp = r[1];  
    *--hp = (r[11]);
    *--hp = (r[12]);
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(107);
    r[13] = (hendblk(8+1));
    r[14+0] = (cx_sexp_2Dmatch_3F);
    pc = objptr_from_obj(r[14+0])[0];
    r[14+1] = (r[13]);
    r[14+2] = (cx__23995);
    r[14+3] = r[1];  
    r += 14; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 102: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r module-loads module-name k specs filename incs defs */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(103);
    r[9] = (hendblk(7+1));
    r[10+0] = (cx_parse_2Dprogram);
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[9];  
    r[10+2] = r[1];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 103: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r module-loads module-name k specs filename incs defs */
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { fixnum_t v4313_tmp;
    { /* length */
    int n; obj l = r[8];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v4313_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v4313_tmp);
    hreserve(hbsz(3)*c, 10); /* 10 live regs */
    l = r[8];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[10] = (o); } }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[9];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { fixnum_t v4314_tmp;
    { /* length */
    int n; obj l = r[7];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v4314_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v4314_tmp);
    hreserve(hbsz(3)*c, 11); /* 11 live regs */
    l = r[7];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[11] = (o); } }
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (r[10]);
    *--hp = (r[11]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    r[12] = (objptr_from_obj(r[2])[0]);
    hreserve(hbsz(5+1), 13); /* 13 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = (r[11]);
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(105);
    r[13] = (hendblk(5+1));
    r[0] = (r[13]);
    r[1] = (r[12]);
    goto s_loop_v3614;

s_loop_v3614: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(104);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v3614;
  }

case 104: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = (cx__231167);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 105: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r module-name k r specs filename */
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = (cx__23982);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = (cx__231180);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = (cx__23982);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    hreserve(hbsz(5+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(106);
    r[10] = (hendblk(5+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 106: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r specs filename r */
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = (cx__231187);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = (cx__231074);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = (cx__231194);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    { fixnum_t v4312_tmp;
    { /* length */
    int n; obj l = r[4];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v4312_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v4312_tmp);
    hreserve(hbsz(3)*c, 12); /* 12 live regs */
    l = r[4];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[12] = (o); } }
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = r[3];  
    *--hp = (r[12]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (r[13]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 107: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r module-add-load! loop forms defs incs r specs k */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = (mknull());
    *--hp = (cx__23982);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[11] = (cdr((r[7])));
    r[11] = (car((r[11])));
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (r[10]);
    *--hp = (r[11]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = (cx__23990);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[6];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[11+0] = r[3];  
    pc = objptr_from_obj(r[11+0])[0];
    r[11+1] = r[9];  
    r[11+2] = r[8];  
    r[11+3] = (r[10]);
    r[11+4] = r[5];  
    r[11+5] = r[4];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(108);
    r[10] = (hendblk(8+1));
    r[0] = (cx_sexp_2Dmatch_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cx__23977);
    r[3] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 108: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r module-add-load! loop forms defs incs r specs k */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = (mknull());
    *--hp = (cx__23964);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[11] = (cdr((r[7])));
    r[11] = (car((r[11])));
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (r[10]);
    *--hp = (r[11]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = (cx__23972);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[6];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[11+0] = r[3];  
    pc = objptr_from_obj(r[11+0])[0];
    r[11+1] = r[9];  
    r[11+2] = r[8];  
    r[11+3] = (r[10]);
    r[11+4] = r[5];  
    r[11+5] = r[4];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[7];  
    *--hp = r[8];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(109);
    r[10] = (hendblk(8+1));
    r[0] = (cx_sexp_2Dmatch_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cx__231119);
    r[3] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 109: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r module-add-load! loop forms defs incs specs r k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(7+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(110);
    r[10] = (hendblk(7+1));
  if ((ispair((r[4])))) {
    r[0] = (cx_c_2Derror_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cx__231116);
    r[3] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_void(0);
    r[2] = r[3];  
    r[3] = r[4];  
    r[4] = r[5];  
    r[5] = r[6];  
    r[6] = r[7];  
    r[7] = r[8];  
    r[8] = r[9];  
    goto s_l_v3581;
  }
  } else {
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[8];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(111);
    r[10] = (hendblk(8+1));
    r[0] = (cx_sexp_2Dmatch_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cx__23958);
    r[3] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 110: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
s_l_v3581: /* ek  loop forms defs incs specs r k */
    r[9] = (cdr((r[7])));
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[6];  
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[10+0] = r[2];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[8];  
    r[10+2] = r[9];  
    r[10+3] = r[5];  
    r[10+4] = r[4];  
    r[10+5] = r[3];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;

case 111: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r module-add-load! loop forms defs r incs specs k */
  if (bool_from_obj(r[1])) {
    r[10] = (cdr((r[6])));
    r[10] = (car((r[10])));
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[5];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[11+0] = r[3];  
    pc = objptr_from_obj(r[11+0])[0];
    r[11+1] = r[9];  
    r[11+2] = r[8];  
    r[11+3] = r[7];  
    r[11+4] = (r[10]);
    r[11+5] = r[4];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(112);
    r[10] = (hendblk(8+1));
    r[0] = (cx_sexp_2Dmatch_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cx__231103);
    r[3] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 112: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r module-add-load! loop forms defs r incs specs k */
  if (bool_from_obj(r[1])) {
    r[10] = (cdr((r[6])));
    r[10] = (car((r[10])));
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[5];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[11+0] = r[3];  
    pc = objptr_from_obj(r[11+0])[0];
    r[11+1] = r[9];  
    r[11+2] = r[8];  
    r[11+3] = r[7];  
    r[11+4] = (r[10]);
    r[11+5] = r[4];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[6];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(113);
    r[10] = (hendblk(8+1));
    r[0] = (cx_sexp_2Dmatch_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cx__231021);
    r[3] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 113: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r module-add-load! loop forms defs incs specs r k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(114);
    r[10] = (hendblk(8+1));
  if ((ispair((r[4])))) {
    r[0] = (cx_c_2Derror_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cx__231095);
    r[3] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_void(0);
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    /* r[7] */    
    /* r[8] */    
    /* r[9] */    
    goto s_l_v3553;
  }
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[4];  
    *--hp = r[8];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[11+0] = r[3];  
    pc = objptr_from_obj(r[11+0])[0];
    r[11+1] = r[9];  
    r[11+2] = r[7];  
    r[11+3] = r[6];  
    r[11+4] = r[5];  
    r[11+5] = (r[10]);
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  }

case 114: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
s_l_v3553: /* ek  module-add-load! loop forms defs incs specs r k */
    r[10] = (cdr((r[8])));
    r[10] = (car((r[10])));
    hreserve(hbsz(6+1), 11); /* 11 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(115);
    r[11] = (hendblk(6+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (r[10]);
    r[3] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 115: /* clo k name xincs xdefs */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1];
    r[1+5] = p[2];
    r[1+6] = p[3];
    r[1+7] = p[4];
    r[1+8] = p[5];
    r[1+9] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* k name xincs xdefs loop forms defs incs specs r */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = (mknull());
    *--hp = (cx__231074);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[11] = (cdr((r[9])));
    r[11] = (car((r[11])));
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (r[10]);
    *--hp = (r[11]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = (cx__231082);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = (cx__231087);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[8];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
  } else {
    r[10] = r[8];  
  }
    { fixnum_t v4311_tmp;
    { /* length */
    int n; obj l = r[2];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v4311_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v4311_tmp);
    hreserve(hbsz(3)*c, 11); /* 11 live regs */
    l = r[2];   t = r[7];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[11] = (o); } }
    { fixnum_t v4310_tmp;
    { /* length */
    int n; obj l = r[3];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v4310_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v4310_tmp);
    hreserve(hbsz(3)*c, 12); /* 12 live regs */
    l = r[3];   t = r[6];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[12] = (o); } }
    r[13+0] = r[4];  
    pc = objptr_from_obj(r[13+0])[0];
    r[13+1] = r[0];  
    r[13+2] = (r[10]);
    r[13+3] = (r[11]);
    r[13+4] = (r[12]);
    r[13+5] = r[5];  
    r += 13; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;

case 116: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k p */
    (void)(fclose(iportdata((r[3]))));
    r[4] = r[1];  
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 117: /* clo k str return */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2];
    r[1+5] = p[3];
    r[1+6] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* k str return module-name mstore module-loads module-visits */
    hreserve(hbsz(1), 7); /* 7 live regs */
    *--hp = obj_from_void(0);
    r[7] = (hendblk(1));
    hreserve(hbsz(4+1), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(117);
    r[8] = (hendblk(4+1));
    hreserve(hbsz(8+1), 9); /* 9 live regs */
    *--hp = r[7];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[8];  
    *--hp = r[4];  
    *--hp = obj_from_case(118);
    r[8] = (hendblk(8+1));
    r[9+0] = r[3];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[8];  
    r[9+2] = r[1];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 118: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r mstore module-add-load! module-loads module-visits str return k load-name */
    (void)(objptr_from_obj(r[9])[0] = (r[1]));
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[9];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(119);
    r[10] = (hendblk(8+1));
    r[0] = (cx_member);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (objptr_from_obj(r[9])[0]);
    r[3] = (objptr_from_obj(r[5])[0]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 119: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r mstore module-add-load! module-loads load-name module-visits str return k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(2+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = obj_from_case(120);
    r[10] = (hendblk(2+1));
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (mknull());
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    r[0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r[3] = (cx__231043);
    r[4] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[9];  
    *--hp = r[4];  
    *--hp = r[8];  
    *--hp = r[3];  
    *--hp = r[7];  
    *--hp = r[2];  
    *--hp = obj_from_case(121);
    r[10] = (hendblk(8+1));
  if (bool_from_obj(cx__2Ashowlog_2A)) {
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (mknull());
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    r[0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r[3] = (cx__231038);
    r[4] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[11+0] = obj_from_ktrap();
    r[11+1] = obj_from_void(0);
    r[11+2] = r[2];  
    r[11+3] = r[7];  
    r[11+4] = r[3];  
    r[11+5] = r[8];  
    r[11+6] = r[4];  
    r[11+7] = r[9];  
    r[11+8] = r[5];  
    r[11+9] = r[6];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v3450;
  }
  }

case 120: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  return k */
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = obj_from_bool(0);
    r[4+3] = (mknull());
    r[4+4] = (mknull());
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 121: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
s_l_v3450: /* ek  mstore str module-add-load! return module-loads k load-name module-visits */
    r[10] = (objptr_from_obj(r[9])[0]);
    r[11] = (objptr_from_obj(r[8])[0]);
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (r[10]);
    *--hp = (r[11]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    (void)(objptr_from_obj(r[9])[0] = (r[10]));
    hreserve(hbsz(5+1), 10); /* 10 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[8];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(122);
    r[10] = (hendblk(5+1));
    r[11+0] = (cx_file_2Dexpand_2Dtop_2Dlevel_2Dforms_21);
    pc = objptr_from_obj(r[11+0])[0];
    r[11+1] = (r[10]);
    r[11+2] = r[3];  
    r[11+3] = (objptr_from_obj(r[2])[0]);
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 122: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r module-add-load! return load-name module-loads k */
    r[7+0] = r[6];  
    r[7+1] = r[1];  
    r[7+2] = (mknull());
    r[7+3] = (mknull());
    r[7+4] = (mknull());
    r[7+5] = r[2];  
    r[7+6] = r[3];  
    r[7+7] = r[4];  
    r[7+8] = r[5];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v3453;

case 123: /* clo k forms incs defs sdefs */
    assert(rc == 6);
    { obj* p = objptr_from_obj(r[0]);
    r[1+5] = p[1];
    r[1+6] = p[2];
    r[1+7] = p[3];
    r[1+8] = p[4]; }
    r += 1; /* shift reg. wnd */
s_loop_v3453: /* k forms incs defs sdefs module-add-load! return load-name module-loads */
  if ((isnull((r[1])))) {
  if ((isnull((r[4])))) {
    r[9] = obj_from_bool(0);
  } else {
    r[9] = (objptr_from_obj(r[8])[0]);
    r[10] = (objptr_from_obj(r[7])[0]);
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[9];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    (void)(objptr_from_obj(r[8])[0] = (r[9]));
    r[9] = (objptr_from_obj(r[7])[0]);
  }
    r[10+0] = r[6];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[0];  
    r[10+2] = r[9];  
    r[10+3] = r[2];  
    r[10+4] = r[3];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[9] = (car((r[1])));
    r[10] = (cdr((r[1])));
    hreserve(hbsz(4+1), 11); /* 11 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(123);
    r[11] = (hendblk(4+1));
    hreserve(hbsz(8+1), 12); /* 12 live regs */
    *--hp = r[0];  
    *--hp = r[9];  
    *--hp = (r[10]);
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = (r[11]);
    *--hp = r[5];  
    *--hp = obj_from_case(124);
    r[11] = (hendblk(8+1));
    r[12+0] = (cx_sexp_2Dmatch_3F);
    pc = objptr_from_obj(r[12+0])[0];
    r[12+1] = (r[11]);
    r[12+2] = (cx__231021);
    r[12+3] = r[9];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 124: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r module-add-load! loop sdefs defs incs forms s k */
  if (bool_from_obj(r[1])) {
    r[10] = (cdr((r[8])));
    r[10] = (car((r[10])));
    hreserve(hbsz(5+1), 11); /* 11 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(125);
    r[11] = (hendblk(5+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (r[10]);
    r[3] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    hreserve(hbsz(7+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[8];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(126);
    r[10] = (hendblk(7+1));
    r[0] = (cx_sexp_2Dmatch_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cx__231002);
    r[3] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 125: /* clo k name xincs xdefs */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1];
    r[1+5] = p[2];
    r[1+6] = p[3];
    r[1+7] = p[4];
    r[1+8] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* k name xincs xdefs loop sdefs defs incs forms */
    { fixnum_t v4309_tmp;
    { /* length */
    int n; obj l = r[2];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v4309_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v4309_tmp);
    hreserve(hbsz(3)*c, 9); /* 9 live regs */
    l = r[2];   t = r[7];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[9] = (o); } }
    { fixnum_t v4308_tmp;
    { /* length */
    int n; obj l = r[3];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v4308_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v4308_tmp);
    hreserve(hbsz(3)*c, 10); /* 10 live regs */
    l = r[3];   t = r[6];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[10] = (o); } }
    r[11+0] = r[4];  
    pc = objptr_from_obj(r[11+0])[0];
    r[11+1] = r[0];  
    r[11+2] = r[8];  
    r[11+3] = r[9];  
    r[11+4] = (r[10]);
    r[11+5] = r[5];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;

case 126: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop sdefs s defs incs forms k */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[10+0] = r[2];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[8];  
    r[10+2] = r[7];  
    r[10+3] = r[6];  
    r[10+4] = r[5];  
    r[10+5] = r[9];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(127);
    r[9] = (hendblk(7+1));
    r[0] = (cx_sexp_2Dmatch_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (cx__23995);
    r[3] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 127: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop sdefs defs incs s forms k */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = (mknull());
    *--hp = (cx__23982);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[10] = (cdr((r[6])));
    r[10] = (car((r[10])));
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[9];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = (cx__23990);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[5];  
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[10+0] = r[2];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[8];  
    r[10+2] = r[7];  
    r[10+3] = r[9];  
    r[10+4] = r[4];  
    r[10+5] = r[3];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(128);
    r[9] = (hendblk(7+1));
    r[0] = (cx_sexp_2Dmatch_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (cx__23977);
    r[3] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 128: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop sdefs defs incs s forms k */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = (mknull());
    *--hp = (cx__23964);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[10] = (cdr((r[6])));
    r[10] = (car((r[10])));
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[9];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = (cx__23972);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[5];  
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[10+0] = r[2];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[8];  
    r[10+2] = r[7];  
    r[10+3] = r[9];  
    r[10+4] = r[4];  
    r[10+5] = r[3];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(129);
    r[9] = (hendblk(7+1));
    r[0] = (cx_sexp_2Dmatch_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (cx__23958);
    r[3] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 129: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop sdefs defs s incs forms k */
  if (bool_from_obj(r[1])) {
    r[9] = (cdr((r[5])));
    r[9] = (car((r[9])));
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[4];  
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[10+0] = r[2];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[8];  
    r[10+2] = r[7];  
    r[10+3] = r[6];  
    r[10+4] = r[9];  
    r[10+5] = r[3];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
    r[9+0] = r[2];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[8];  
    r[9+2] = r[7];  
    r[9+3] = r[6];  
    r[9+4] = r[4];  
    r[9+5] = r[3];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  }

case 130: /* compile-file k filename */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_compile_2Dfile: /* k filename */
    hreserve(hbsz(2+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(131);
    r[2] = (hendblk(2+1));
    r[3+0] = (cx_reset_2Dtimestamps);
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 131: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  filename k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(132);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    goto gs_clear_2Dmemoizations;

case 132: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  filename k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(133);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto gs_parse_2Dfile;

case 133: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r filename k */
    r[4] = (car((r[1])));
    r[5] = (cdr((r[1])));
    r[5] = (car((r[5])));
    r[6] = (cdr((r[1])));
    r[6] = (cdr((r[6])));
    r[6] = (car((r[6])));
    r[7] = (cdr((r[1])));
    r[7] = (cdr((r[7])));
    r[7] = (cdr((r[7])));
    r[7] = (car((r[7])));
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = r[2];  
    *--hp = r[7];  
    *--hp = obj_from_case(134);
    r[8] = (hendblk(6+1));
    r[0] = r[8];  
    r[1] = (cx__231337);
    r[2] = r[7];  
    goto gs_maybe_2Dpp_2Dsource;

case 134: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek  exp filename defs incs specs k */
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(135);
    r[8] = (hendblk(6+1));
    r[0] = r[8];  
    r[1] = (cx__231336);
    goto gs_maybe_2Dlog;

case 135: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek  exp filename defs incs specs k */
    hreserve(hbsz(5+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(136);
    r[8] = (hendblk(5+1));
    r[0] = (cx_fix_2Dletrecs);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 136: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r filename defs incs specs k */
    hreserve(hbsz(6+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(137);
    r[7] = (hendblk(6+1));
    r[8+0] = r[7];  
    r[8+1] = (cx__231332);
    r[8+2] = r[1];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_maybe_2Dpp_2Dsource;

case 137: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename defs incs specs k */
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(138);
    r[8] = (hendblk(6+1));
    r[0] = r[8];  
    goto gs_clear_2Dmemoizations;

case 138: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename defs incs specs k */
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(139);
    r[8] = (hendblk(6+1));
    r[0] = r[8];  
    r[1] = (cx__231331);
    goto gs_maybe_2Dlog;

case 139: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename defs incs specs k */
    hreserve(hbsz(5+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(140);
    r[8] = (hendblk(5+1));
    r[0] = (cx_remove_2Dassignments);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 140: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r filename defs incs specs k */
    hreserve(hbsz(6+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(141);
    r[7] = (hendblk(6+1));
    r[8+0] = r[7];  
    r[8+1] = (cx__231327);
    r[8+2] = r[1];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_maybe_2Dpp_2Dsource;

case 141: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename defs incs specs k */
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(142);
    r[8] = (hendblk(6+1));
    r[0] = r[8];  
    goto gs_clear_2Dmemoizations;

case 142: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename defs incs specs k */
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(143);
    r[8] = (hendblk(6+1));
    r[0] = r[8];  
    r[1] = (cx__231326);
    goto gs_maybe_2Dlog;

case 143: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename defs incs specs k */
    hreserve(hbsz(5+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(144);
    r[8] = (hendblk(5+1));
    r[0] = (cx_analyze_2Dglobals);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 144: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r filename defs incs specs k */
    hreserve(hbsz(6+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(145);
    r[7] = (hendblk(6+1));
    r[8+0] = r[7];  
    r[8+1] = (cx__231322);
    r[8+2] = (car((r[1])));
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_maybe_2Dpp_2Dsource;

case 145: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename defs incs specs k */
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(146);
    r[8] = (hendblk(6+1));
    r[0] = r[8];  
    goto gs_clear_2Dmemoizations;

case 146: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename defs incs specs k */
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(147);
    r[8] = (hendblk(6+1));
    r[0] = r[8];  
    r[1] = (cx__231320);
    goto gs_maybe_2Dlog;

case 147: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename defs incs specs k */
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(148);
    r[8] = (hendblk(6+1));
    r[0] = (cx_stack_2Dfunctions_2Dcode_2Dgenerate);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = (cdr((r[2])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 148: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r r filename defs incs specs k */
    hreserve(hbsz(7+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(149);
    r[8] = (hendblk(7+1));
    r[0] = r[8];  
    r[1] = (cx__231318);
    goto gs_maybe_2Dlog;

case 149: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(150);
    r[9] = (hendblk(6+1));
    r[0] = (cx_constant_2Dfold);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (car((r[2])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 150: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r filename r defs incs specs k */
    hreserve(hbsz(7+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(151);
    r[8] = (hendblk(7+1));
    r[9+0] = r[8];  
    r[9+1] = (cx__231314);
    r[9+2] = r[1];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_maybe_2Dpp_2Dsource;

case 151: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(152);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    goto gs_clear_2Dmemoizations;

case 152: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(153);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    r[1] = (cx__231313);
    goto gs_maybe_2Dlog;

case 153: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(154);
    r[9] = (hendblk(6+1));
    r[0] = (cx_beta_2Dsubstitute);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    /* r[2] */    
    r[3] = obj_from_bool(0);
    r[4] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 154: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r filename r defs incs specs k */
    hreserve(hbsz(7+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(155);
    r[8] = (hendblk(7+1));
    r[9+0] = r[8];  
    r[9+1] = (cx__231309);
    r[9+2] = r[1];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_maybe_2Dpp_2Dsource;

case 155: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(156);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    goto gs_clear_2Dmemoizations;

case 156: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(157);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    r[1] = (cx__231308);
    goto gs_maybe_2Dlog;

case 157: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(158);
    r[9] = (hendblk(6+1));
    r[0] = (cx_cps_2Dconvert);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 158: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r filename r defs incs specs k */
    hreserve(hbsz(7+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(159);
    r[8] = (hendblk(7+1));
    r[9+0] = r[8];  
    r[9+1] = (cx__231304);
    r[9+2] = r[1];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_maybe_2Dpp_2Dsource;

case 159: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(160);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    goto gs_clear_2Dmemoizations;

case 160: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(161);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    r[1] = (cx__231303);
    goto gs_maybe_2Dlog;

case 161: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(162);
    r[9] = (hendblk(6+1));
    r[0] = (cx_beta_2Dsubstitute);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    /* r[2] */    
    r[3] = obj_from_bool(1);
    r[4] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 162: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r filename r defs incs specs k */
    hreserve(hbsz(7+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(163);
    r[8] = (hendblk(7+1));
    r[9+0] = r[8];  
    r[9+1] = (cx__231299);
    r[9+2] = r[1];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_maybe_2Dpp_2Dsource;

case 163: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(164);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    goto gs_clear_2Dmemoizations;

case 164: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(165);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    r[1] = (cx__231298);
    goto gs_maybe_2Dlog;

case 165: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(166);
    r[9] = (hendblk(6+1));
    r[0] = (cx_lambda_2Dlift);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 166: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r filename r defs incs specs k */
    hreserve(hbsz(7+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(167);
    r[8] = (hendblk(7+1));
    r[9+0] = r[8];  
    r[9+1] = (cx__231294);
    r[9+2] = r[1];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_maybe_2Dpp_2Dsource;

case 167: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(168);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    goto gs_clear_2Dmemoizations;

case 168: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(169);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    r[1] = (cx__231293);
    goto gs_maybe_2Dlog;

case 169: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(170);
    r[9] = (hendblk(6+1));
    r[0] = (cx_beta_2Dsubstitute);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    /* r[2] */    
    r[3] = obj_from_bool(1);
    r[4] = obj_from_bool(1);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 170: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r filename r defs incs specs k */
    hreserve(hbsz(7+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(171);
    r[8] = (hendblk(7+1));
    r[9+0] = r[8];  
    r[9+1] = (cx__231289);
    r[9+2] = r[1];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_maybe_2Dpp_2Dsource;

case 171: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(172);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    goto gs_clear_2Dmemoizations;

case 172: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(173);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    r[1] = (cx__231288);
    goto gs_maybe_2Dlog;

case 173: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(174);
    r[9] = (hendblk(6+1));
    r[0] = (cx_unbox_2Dvalues);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 174: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r filename r defs incs specs k */
    hreserve(hbsz(7+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(175);
    r[8] = (hendblk(7+1));
    r[9+0] = r[8];  
    r[9+1] = (cx__231284);
    r[9+2] = r[1];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_maybe_2Dpp_2Dsource;

case 175: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(176);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    goto gs_clear_2Dmemoizations;

case 176: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(177);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    r[1] = (cx__231283);
    goto gs_maybe_2Dlog;

case 177: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(178);
    r[9] = (hendblk(6+1));
    r[0] = (cx_code_2Dgenerate);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 178: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r filename r defs incs specs k */
    hreserve(hbsz(7+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(179);
    r[8] = (hendblk(7+1));
    r[9] = (cdr((r[1])));
    r[9] = (car((r[9])));
    r[10+0] = r[8];  
    r[10+1] = (cx__231280);
    r[10+2] = r[9];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_maybe_2Dpp_2Dtext;

case 179: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  filename r r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(180);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    r[1] = (cx__231276);
    goto gs_maybe_2Dlog;

case 180: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  filename r r defs incs specs k */
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(181);
    r[9] = (hendblk(6+1));
    r[0] = (cx_path_2Dstrip_2Dextension);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 181: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r r r defs incs specs k */
    { /* string-append */
    int *d = stringcat(stringdata((r[1])), stringdata((cx__231267)));
    r[8] = (hpushstr(8, d)); }
    { /* delete-file */ 
    int res = remove(stringchars((r[8])));
    (void)(res == 0); }
    hreserve(hbsz(2+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = obj_from_case(182);
    r[9] = (hendblk(2+1));
    hreserve(hbsz(5+1), 10); /* 10 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(183);
    r[10] = (hendblk(5+1));
    r[11+0] = (cx_with_2Doutput_2Dto_2Dfile);
    pc = objptr_from_obj(r[11+0])[0];
    r[11+1] = r[9];  
    r[11+2] = r[8];  
    r[11+3] = (r[10]);
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 182: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  k r */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 183: /* clo k */
    assert(rc == 2);
    { obj* p = objptr_from_obj(r[0]);
    r[1+1] = p[1];
    r[1+2] = p[2];
    r[1+3] = p[3];
    r[1+4] = p[4];
    r[1+5] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* k r r defs incs specs */
    r[6+0] = r[0];  
    r[6+1] = r[5];  
    r[6+2] = r[4];  
    r[6+3] = r[3];  
    r[6+4] = r[2];  
    r[6+5] = r[1];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_display_2Dmodule;

case 184: /* display-module k specs incs defs sfcs module-code */
    assert(rc == 7);
    r += 1; /* shift reg. wnd */
gs_display_2Dmodule: /* k specs incs defs sfcs module-code */
    r[6] = (car((r[5])));
    r[7] = (cdr((r[5])));
    r[7] = (car((r[7])));
    r[8] = (cdr((r[5])));
    r[8] = (cdr((r[8])));
    r[8] = (car((r[8])));
    r[9] = (cdr((r[5])));
    r[9] = (cdr((r[9])));
    r[9] = (cdr((r[9])));
    r[9] = (car((r[9])));
    { /* list-ref */
    obj l = r[5];   int c = (4);
    while (c-- > 0) l = cdr(l);
    r[10] = (car(l)); }
    { /* list-ref */
    obj l = r[5];   int c = (5);
    while (c-- > 0) l = cdr(l);
    r[11] = (car(l)); }
    hreserve(hbsz(0+1), 12); /* 12 live regs */
    *--hp = obj_from_case(185);
    r[12] = (hendblk(0+1));
    hreserve(hbsz(11+1), 13); /* 13 live regs */
    *--hp = r[6];  
    *--hp = r[0];  
    *--hp = (r[11]);
    *--hp = r[7];  
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[4];  
    *--hp = (r[12]);
    *--hp = r[8];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(186);
    r[12] = (hendblk(11+1));
    r[13+0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[13+0])[0];
    r[13+1] = (r[12]);
    r[13+2] = r[1];  
    r += 13; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 185: /* clo k s */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k s */
    { /* cons */ 
    hreserve(hbsz(3), 2); /* 2 live regs */
    *--hp = (mknull());
    *--hp = (cx__23982);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 186: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9];
    r[1+11] = p[10];
    r[1+12] = p[11]; }
    r += 1; /* shift reg. wnd */
    /* ek  incs defs globals add-nl sfcs roots cases liveregs code k main? */
    hreserve(hbsz(11+1), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(187);
    r[13] = (hendblk(11+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[13]);
    r[2] = (cx__231404);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 187: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9];
    r[1+11] = p[10];
    r[1+12] = p[11]; }
    r += 1; /* shift reg. wnd */
    /* ek  incs defs globals add-nl sfcs roots cases liveregs code k main? */
    hreserve(hbsz(11+1), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(188);
    r[13] = (hendblk(11+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[13]);
    r[2] = (cx_code_2Dstandard_2Dincludes);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 188: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9];
    r[1+11] = p[10];
    r[1+12] = p[11]; }
    r += 1; /* shift reg. wnd */
    /* ek  incs defs globals add-nl sfcs roots cases liveregs code k main? */
    hreserve(hbsz(10+1), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(189);
    r[13] = (hendblk(10+1));
  if ((ispair((r[2])))) {
    hreserve(hbsz(2+1), 14); /* 14 live regs */
    *--hp = (r[13]);
    *--hp = r[2];  
    *--hp = obj_from_case(218);
    r[14] = (hendblk(2+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[14]);
    r[2] = (cx__231400);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_void(0);
    r[2] = r[3];  
    r[3] = r[4];  
    r[4] = r[5];  
    r[5] = r[6];  
    r[6] = r[7];  
    r[7] = r[8];  
    r[8] = r[9];  
    r[9] = (r[10]);
    r[10] = (r[11]);
    r[11] = (r[12]);
    goto s_l_v2907;
  }

case 189: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9];
    r[1+11] = p[10]; }
    r += 1; /* shift reg. wnd */
s_l_v2907: /* ek  defs globals add-nl sfcs roots cases liveregs code k main? */
    hreserve(hbsz(10+1), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(190);
    r[12] = (hendblk(10+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[12]);
    r[2] = (cx__231399);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 190: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9];
    r[1+11] = p[10]; }
    r += 1; /* shift reg. wnd */
    /* ek  defs globals add-nl sfcs roots cases liveregs code k main? */
    hreserve(hbsz(10+1), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(191);
    r[12] = (hendblk(10+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[12]);
    r[2] = (cx_code_2Dstandard_2Ddefinitions);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 191: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9];
    r[1+11] = p[10]; }
    r += 1; /* shift reg. wnd */
    /* ek  defs globals add-nl sfcs roots cases liveregs code k main? */
    hreserve(hbsz(9+1), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(192);
    r[12] = (hendblk(9+1));
  if ((ispair((r[2])))) {
    hreserve(hbsz(3+1), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = r[2];  
    *--hp = r[4];  
    *--hp = obj_from_case(214);
    r[13] = (hendblk(3+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[13]);
    r[2] = (cx__231395);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_void(0);
    r[2] = r[3];  
    r[3] = r[4];  
    r[4] = r[5];  
    r[5] = r[6];  
    r[6] = r[7];  
    r[7] = r[8];  
    r[8] = r[9];  
    r[9] = (r[10]);
    r[10] = (r[11]);
    goto s_l_v2942;
  }

case 192: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9]; }
    r += 1; /* shift reg. wnd */
s_l_v2942: /* ek  globals add-nl sfcs roots cases liveregs code k main? */
    hreserve(hbsz(9+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(193);
    r[11] = (hendblk(9+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[11]);
    r[2] = (cx__231384);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 193: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9]; }
    r += 1; /* shift reg. wnd */
    /* ek  globals add-nl sfcs roots cases liveregs code k main? */
    hreserve(hbsz(8+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(194);
    r[11] = (hendblk(8+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[11]);
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 194: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek  add-nl sfcs roots cases liveregs code k main? */
    hreserve(hbsz(6+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(195);
    r[10] = (hendblk(6+1));
  if ((ispair((r[3])))) {
    hreserve(hbsz(3+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(210);
    r[11] = (hendblk(3+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[11]);
    r[2] = (cx__231381);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_void(0);
    r[2] = r[4];  
    r[3] = r[5];  
    r[4] = r[6];  
    r[5] = r[7];  
    r[6] = r[8];  
    r[7] = r[9];  
    goto s_l_v2977;
  }

case 195: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
s_l_v2977: /* ek  roots cases liveregs code k main? */
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(196);
    r[8] = (hendblk(6+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = (cx__231370);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 196: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek  roots cases liveregs code k main? */
    hreserve(hbsz(5+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(197);
    r[8] = (hendblk(5+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 197: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek  cases liveregs code k main? */
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(198);
    r[7] = (hendblk(5+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = (cx__231369);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 198: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek  cases liveregs code k main? */
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(199);
    r[7] = (hendblk(4+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 199: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek  liveregs code k main? */
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(200);
    r[6] = (hendblk(4+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (cx__231368);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 200: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek  liveregs code k main? */
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(201);
    r[6] = (hendblk(3+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 201: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  code k main? */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(202);
    r[5] = (hendblk(3+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (cx_code_2Dhost_2Dprologue);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 202: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  code k main? */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(203);
    r[5] = (hendblk(2+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 203: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  k main? */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(204);
    r[4] = (hendblk(2+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cx_code_2Dhost_2Depilogue);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 204: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  k main? */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(205);
    r[4] = (hendblk(2+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cx__231365);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 205: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  k main? */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(206);
    r[4] = (hendblk(2+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cx_code_2Dmodule);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 206: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  k main? */
  if (bool_from_obj(r[3])) {
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(207);
    r[4] = (hendblk(1+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cx__231360);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 207: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek  k */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(208);
    r[3] = (hendblk(1+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = (cx_code_2Druntime);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 208: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek  k */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(209);
    r[3] = (hendblk(1+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = (cx__231358);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 209: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek  k */
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[2];  
    r[2] = (cx_code_2Dmain);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 210: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  add-nl sfcs k */
    hreserve(hbsz(1+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = obj_from_case(213);
    r[5] = (hendblk(1+1));
    r[0] = r[5];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v2955;

s_loop_v2955: /* k id add-nl */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(211);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v2955;
  }

case 211: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r add-nl id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(212);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 212: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 213: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    r[3+0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[2];  
    r[3+2] = r[1];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 214: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  add-nl defs k */
    hreserve(hbsz(1+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = obj_from_case(217);
    r[5] = (hendblk(1+1));
    r[0] = r[5];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v2920;

s_loop_v2920: /* k id add-nl */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(215);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v2920;
  }

case 215: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r add-nl id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(216);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 216: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 217: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    r[3+0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[2];  
    r[3+2] = r[1];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 218: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  incs k */
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 219: /* main k argv */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k argv */
    hreserve(hbsz(1), 2); /* 2 live regs */
    *--hp = obj_from_void(0);
    r[2] = (hendblk(1));
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(220);
    r[3] = (hendblk(0+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[3];  
    *--hp = obj_from_case(221);
    r[3] = (hendblk(3+1));
    r[4+0] = r[3];  
    r[4+1] = (mknull());
    r[4+2] = obj_from_fixnum(0);
    r[4+3] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v2877;

case 220: /* clo k */
    assert(rc == 2);
    r += 1; /* shift reg. wnd */
    /* k */
    r[1+0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[1+0])[0];
    r[1+1] = r[0];  
    r[1+2] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r[1+3] = (cx__231470);
    r[1+4] = (mknull());
    r += 1; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

s_loop_v2877: /* k r i argv */
    { const fixnum_t v4307_i = fixnum_from_obj(r[2]);
    { /* argv-ref */
    int i = (v4307_i);
    char *s = ((char **)((r[3])))[i];
    if (s) r[4] = (hpushstr(4, newstring(s)));
    else r[4] = obj_from_bool(0); }
  if (bool_from_obj(r[4])) {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    /* r[0] */    
    r[1] = r[5];  
    r[2] = obj_from_fixnum((v4307_i) + (1));
    /* r[3] */    
    goto s_loop_v2877;
  } else {
    { /* reverse! */
    obj t, v = mknull(), l = r[1];  
    while (l != mknull()) t = cdr(l), cdr(l) = v, v = l, l = t;
    r[5] = (v); }
    r[6+0] = r[0];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } } 

case 221: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r about k args */
    r[5] = (cdr((r[1])));
    (void)(objptr_from_obj(r[4])[0] = (r[5]));
    r[6] = (objptr_from_obj(r[4])[0]);
    r[6] = obj_from_bool(isnull((r[6])));
  if (bool_from_obj(r[6])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;
  } else {
    r[6] = (objptr_from_obj(r[4])[0]);
    r[0] = r[3];  
    r[1] = r[6];  
    /* r[2] */    
    goto s_loop_v2850;
  }

case 222: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v2850: /* k id about */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (car((r[1])));
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(222);
    r[4] = (hendblk(1+1));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(223);
    r[4] = (hendblk(3+1));
  if ((strcmp(stringchars((r[3])), stringchars((cx__231495))) == 0)) {
    cx__2Ashowlog_2A = obj_from_bool(1);
    r[5] = obj_from_void(0);
    hreserve(hbsz(1+1), 6); /* 6 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(222);
    r[6] = (hendblk(1+1));
    r[7+0] = obj_from_ktrap();
    r[7+1] = r[5];  
    r[7+2] = r[6];  
    r[7+3] = r[1];  
    r[7+4] = r[0];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v2864;
  } else {
  if ((strcmp(stringchars((r[3])), stringchars((cx__231492))) == 0)) {
    cx__2Asilent_2A = obj_from_bool(0);
    r[5] = obj_from_void(0);
    hreserve(hbsz(1+1), 6); /* 6 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(222);
    r[6] = (hendblk(1+1));
    r[7+0] = obj_from_ktrap();
    r[7+1] = r[5];  
    r[7+2] = r[6];  
    r[7+3] = r[1];  
    r[7+4] = r[0];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v2864;
  } else {
  if ((strcmp(stringchars((r[3])), stringchars((cx__231489))) == 0)) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;
  } else {
    { bool_t v4306_tmp;
    { /* file-exists? */ 
    FILE *f = fopen(stringchars((r[3])), "r");
    if (f != NULL) fclose(f);
    v4306_tmp = (f != NULL); }
    r[5] = obj_from_bool(!(v4306_tmp)); }
  if (bool_from_obj(r[5])) {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[6+0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[4];  
    r[6+2] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r[6+3] = (cx__231486);
    r[6+4] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(224);
    r[5] = (hendblk(2+1));
  if (bool_from_obj(cx__2Ashowlog_2A)) {
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[7+0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[5];  
    r[7+2] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r[7+3] = (cx__231482);
    r[7+4] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_void(0);
    r[2] = r[3];  
    r[3] = r[4];  
    goto s_l_v2852;
  }
  }
  }
  }
  }
  }

case 223: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
s_l_v2864: /* ek  loop id k */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 224: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
s_l_v2852: /* ek  fn k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(225);
    r[4] = (hendblk(1+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto gs_compile_2Dfile;

case 225: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
  if (bool_from_obj(cx__2Ashowlog_2A)) {
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    r[4+0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[2];  
    r[4+2] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r[4+3] = (cx__231479);
    r[4+4] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

default: /* inter-host call */
    cxg_hp = hp;
    cxm_rgc(r, r + MAX_LIVEREGS);
#ifndef NDEBUG
    cxg_rc = rc;
#endif
    return pc;
  }
}

/* module load */
void MODULE(void)
{
  obj pc;
  if (!root.next) {
    root.next = cxg_rootp;
    cxg_rootp = &root;
    LOAD();
    pc = obj_from_case(0);
    assert((cxg_rc = 0, 1));
    while (pc) pc = (*(cxhost_t*)pc)(pc); 
    assert(cxg_rc == 2);
  }
}

/* basic runtime */
#define HEAP_SIZE 131072 /* 2^17 */

obj *cxg_heap = NULL;
cxoint_t cxg_hmask = 0;
obj *cxg_hp = NULL;
cxroot_t *cxg_rootp = NULL;
obj cxg_regs[REGS_SIZE];
#ifndef NDEBUG
int cxg_rc = 0;
#endif

static obj *heap2 = NULL;
static size_t hsize = 0; 
static cxoint_t hmask2 = 0;
static int gccount = 0, bumpcount = 0;

#define notfwdptr(o) (((char*)(o) - (char*)heap2) & hmask2)

static void toheap2(obj* p)
{
  obj o = *p, *op, fo, *fop;
  if (notobjptr(o)) return;
  fo = (op = objptr_from_obj(o))[-1]; assert(fo);
  if (notaptr(fo)) {
    fop = op + size_from_obj(fo); while (fop >= op) *--cxg_hp = *--fop;
    *p = *fop = obj_from_objptr(cxg_hp+1);
  } else if (notfwdptr(fo)) {
    *--cxg_hp = *op--; *--cxg_hp = *op;
    *p = *op = obj_from_objptr(cxg_hp+1);
  } else *p = fo;
}

static void finalize(obj *hp, obj *end)
{
  while (hp < end) {
    obj fo = *hp++; assert(fo);
    if (notaptr(fo)) hp += size_from_obj(fo);
    else if (notfwdptr(fo)) ((cxtype_t*)fo)->free((void*)*hp++);
    else if (notaptr(fo = objptr_from_obj(fo)[-1])) hp += size_from_obj(fo);
    else ++hp;
  } assert(hp == end);
}

static size_t relocate(obj *regs, obj *regp, obj *end2, obj *end1) 
{
  obj *p, *hp1 = cxg_hp; cxroot_t *pr; 
  cxg_hp = end2; if (!cxg_heap) return 0;
  for (p = regs; p < regp; ++p) toheap2(p);
  for (pr = cxg_rootp; pr; pr = pr->next) {
    obj **pp = pr->globv; int c = pr->globc;
    while (c-- > 0) toheap2(*pp++);
  }
  for (p = end2; p > cxg_hp; --p) toheap2(p-1);
  if (end1) finalize(hp1, end1);
  return end2 - cxg_hp;
}

obj *cxm_hgc(obj *regs, obj *regp, obj *hp, size_t needs) 
{
  obj *p = cxg_heap; cxg_hp = hp; ++gccount;
  if (hsize < (needs += relocate(regs, regp, heap2 + hsize, p + hsize))) {
    size_t s = HEAP_SIZE; while (s < needs) s *= 2;
    hmask2 = 1 | ~(s*sizeof(obj)-1);
    if (!(p = realloc(cxg_heap, s*sizeof(obj)))) { perror("alloc[h]"); exit(2); }
    cxg_heap = heap2; heap2 = p; relocate(regs, regp, heap2 + s, 0);
    if (!(p = realloc(cxg_heap, s*sizeof(obj)))) { perror("alloc[h]"); exit(2); }
    hsize = s; cxg_hmask = hmask2; ++bumpcount;
  }
  cxg_heap = heap2; heap2 = p;
  return cxg_hp;
}

obj *cxm_rgc(obj *regs, obj *regp) 
{
  obj *p = cxg_regs;
  assert(regp <= cxg_regs + REGS_SIZE);
  while (regs < regp) *p++ = *regs++;
  return cxg_regs;
}

void *cxm_cknull(void *p, char *msg)
{
  if (!p) { 
    perror(msg); exit(2); 
  }
  return p;
}

/* os entry point */
int main(int argc, char **argv) {
  int res; obj pc;
  obj retcl[1] = { 0 }; 
  MODULE();
  cxg_regs[0] = cx_main;
  cxg_regs[1] = (obj)retcl;
  cxg_regs[2] = (obj)argv;
  assert(cxg_rc = 3);
  pc = objptr_from_obj(cx_main)[0];
  while (pc) pc = (*(cxhost_t*)pc)(pc); 
  assert(cxg_rc == 3);
  res = (cxg_regs[2] != 0); 
  /* fprintf(stderr, "%d collections, %d reallocs\n", gccount, bumpcount); */
  return res; 
}
