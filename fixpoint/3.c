/* 3.sf */
#define MODULE module_3
#define LOAD() module_0(); 
extern void module_0(void); /* 0.sf */

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
extern obj cx_assoc; /* assoc */
extern obj cx_c_2Dargref_2Dctype; /* c-argref-ctype */
extern obj cx_c_2Dundecorate_2Dalvar; /* c-undecorate-alvar */
extern obj cx_fixnum_2D_3Estring; /* fixnum->string */
extern obj cx_flonum_2D_3Estring; /* flonum->string */
extern obj cx_fprintf_2A; /* fprintf* */
extern obj cx_letrec_2Dexp; /* letrec-exp */
extern obj cx_list_3F; /* list? */
extern obj cx_member; /* member */
extern obj cx_prim_2Dctype; /* prim-ctype */
extern obj cx_reset; /* reset */
extern obj cx_symbol_2A; /* symbol* */
extern obj cx_timestamp; /* timestamp */
extern obj cx_write_2F3; /* write/3 */
obj cx__3Cid_3E_3F; /* <id>? */
obj cx__3Cliteral_3E_3F; /* <literal>? */
obj cx_c_2Derror_2A; /* c-error* */
obj cx_exp_2D_3Efree_2Dlexvars; /* exp->free-lexvars */
obj cx_exp_2D_3Efree_2Dvars; /* exp->free-vars */
obj cx_exp_2Dctype; /* exp-ctype */
obj cx_exp_2Dvinfo; /* exp-vinfo */
obj cx_id_2D_3Euname; /* id->uname */
obj cx_id_3C_3F; /* id<? */
obj cx_keywords; /* keywords */
obj cx_labelapp_2Dexp; /* labelapp-exp */
obj cx_labelapp_2Dexp_3F; /* labelapp-exp? */
obj cx_last_2Did_2Dtimestamp; /* last-id-timestamp */
obj cx_memoizate_2Dexp_2Dattribute; /* memoizate-exp-attribute */
obj cx_memoizate_2Dvar_2Dattribute; /* memoizate-var-attribute */
obj cx_parse_2Dprogram; /* parse-program */
obj cx_posq; /* posq */
obj cx_reduce_2Dleft; /* reduce-left */
obj cx_reduce_2Dright; /* reduce-right */
obj cx_reduce_2Dright_2Fright_2Dseed; /* reduce-right/right-seed */
obj cx_reduce_2Dright_2Fsingular_2Didentity; /* reduce-right/singular-identity */
obj cx_reset_2Dtimestamps; /* reset-timestamps */
obj cx_sexp_2Dmatch_3F; /* sexp-match? */
obj cx_string_2Dappend_2A; /* string-append* */
obj cx_string_2Dforest_2D_3Estring; /* string-forest->string */
obj cx_string_2Dforest_2Dlength; /* string-forest-length */
obj cx_string_2Dforest_3F; /* string-forest? */
obj cx_symbol_2A; /* symbol* */
obj cx_the_2Dhalt_2Dprim; /* the-halt-prim */
obj cx_timestamp; /* timestamp */
obj cx_var_2Dassigned_2Din_2Dexp_3F; /* var-assigned-in-exp? */
obj cx_var_2Dassignment_2Dcount; /* var-assignment-count */
obj cx_var_2Donly_2Dapplied_2Din_2Dexp_3F; /* var-only-applied-in-exp? */
obj cx_var_2Dreference_2Dcount; /* var-reference-count */
obj cx_var_2Dreferenced_2Din_2Dexp_3F; /* var-referenced-in-exp? */
obj cx_var_2Dunboxed_2Dctype_2Din_2Dexp; /* var-unboxed-ctype-in-exp */
obj cx_var_2Duse_2Dunboxed_2Dctype; /* var-use-unboxed-ctype */
obj cx_var_2Duses_2Din_2Dexp; /* var-uses-in-exp */
static obj cx__231007; /* constant #1007 */
static obj cx__231036; /* constant #1036 */
static obj cx__231064; /* constant #1064 */
static obj cx__231075; /* constant #1075 */
static obj cx__231076; /* constant #1076 */
static obj cx__231093; /* constant #1093 */
static obj cx__231098; /* constant #1098 */
static obj cx__2311; /* constant #11 */
static obj cx__231113; /* constant #1113 */
static obj cx__231138; /* constant #1138 */
static obj cx__231208; /* constant #1208 */
static obj cx__231248; /* constant #1248 */
static obj cx__231270; /* constant #1270 */
static obj cx__231287; /* constant #1287 */
static obj cx__231305; /* constant #1305 */
static obj cx__231327; /* constant #1327 */
static obj cx__231360; /* constant #1360 */
static obj cx__231363; /* constant #1363 */
static obj cx__231365; /* constant #1365 */
static obj cx__231369; /* constant #1369 */
static obj cx__23169; /* constant #169 */
static obj cx__231709; /* constant #1709 */
static obj cx__2318; /* constant #18 */
static obj cx__23183; /* constant #183 */
static obj cx__23203; /* constant #203 */
static obj cx__23239; /* constant #239 */
static obj cx__23333; /* constant #333 */
static obj cx__23390; /* constant #390 */
static obj cx__23711; /* constant #711 */
static obj cx__23746; /* constant #746 */
static obj cx__23752; /* constant #752 */
static obj cx__23778; /* constant #778 */
static obj cx__23801; /* constant #801 */
static obj cx__23819; /* constant #819 */
static obj cx__23835; /* constant #835 */
static obj cx__23862; /* constant #862 */
static obj cx__23891; /* constant #891 */
static obj cx__23920; /* constant #920 */
static obj cx__23949; /* constant #949 */
static obj cx__23978; /* constant #978 */
static obj cx_keep_231406; /* constant keep#1406 */

/* helper functions */
/* global-id-constant?#247 */
static obj cxs_global_2Did_2Dconstant_3F_23247(obj v249_id)
{ 
    return ((fixnum_from_obj(car((cdr((cdr((v249_id))))))) < (0)) ? (car((cdr((cdr((cdr((v249_id))))))))) : obj_from_bool(0));
}

/* global-id-private-constant?#266 */
static obj cxs_global_2Did_2Dprivate_2Dconstant_3F_23266(obj v268_id)
{ 
    return ((fixnum_from_obj(car((cdr((cdr((v268_id))))))) < (0)) ? obj_from_bool((car((cdr((cdr((cdr((v268_id))))))))) == (mksymbol(internsym("private")))) : obj_from_bool(0));
}

/* label-id?#301 */
static obj cxs_label_2Did_3F_23301(obj v303_id)
{ 
    return ((!(fixnum_from_obj(car((cdr((cdr((v303_id))))))) < (0))) ? obj_from_bool((car((cdr((cdr((cdr((v303_id))))))))) == obj_from_bool(1)) : obj_from_bool(0));
}

/* lookupicate#615 */
static obj cxs_lookupicate_23615(obj v618_sym, obj v617_env)
{ 
  s_lookupicate_23615:
  if ((isnull((v617_env)))) {
    return obj_from_bool(0);
  } else {
  { /* let */
    obj v2083_tmp;
  { /* let */
    obj v2086_tmp;
  { /* let */
    obj v622_x = (car((v617_env)));
    v2086_tmp = (car((cdr((v622_x)))));
  }
    v2083_tmp = obj_from_bool((v2086_tmp) == (v618_sym));
  }
  if (bool_from_obj(v2083_tmp)) {
    return (car((v617_env)));
  } else {
  { /* let */
    obj v2085_tmp = (cdr((v617_env)));
    obj v2084_tmp = (v618_sym);
    /* tail call */
    v618_sym = (v2084_tmp);
    v617_env = (v2085_tmp);
    goto s_lookupicate_23615;
  }
  }
  }
  }
}

/* let-exp?#1827 */
static obj cxs_let_2Dexp_3F_231827(obj v1829_exp)
{ 
  if (bool_from_obj((isvector((v1829_exp))) ? (((vectorlen((v1829_exp))) == (3)) ? obj_from_bool((vectorref((v1829_exp), (0))) == (mksymbol(internsym("app-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  { /* let */
    obj v1833_rator = (vectorref((v1829_exp), (1)));
  if (bool_from_obj((isvector((v1833_rator))) ? (((vectorlen((v1833_rator))) == (3)) ? obj_from_bool((vectorref((v1833_rator), (0))) == (mksymbol(internsym("lambda-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  { /* let */
    obj v2082_tmp;
    obj v2081_tmp;
    { /* length */
    int n; obj l = (vectorref((v1829_exp), (2)));
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v2082_tmp = obj_from_fixnum(n); };
    { /* length */
    int n; obj l = (vectorref((v1833_rator), (1)));
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v2081_tmp = obj_from_fixnum(n); };
    return obj_from_bool(fixnum_from_obj(v2081_tmp) == fixnum_from_obj(v2082_tmp));
  }
  } else {
    return obj_from_bool(0);
  }
  }
  } else {
    return obj_from_bool(0);
  }
}

/* posq */
static obj cxs_posq(obj v70_x, obj v69_lst)
{ 
  { /* letrec */
    obj v73_lst;
    obj v72_i;
  { /* let */
    obj v2080_tmp = obj_from_fixnum(0);
    obj v2079_tmp = (v69_lst);
    /* tail call */
    v73_lst = (v2079_tmp);
    v72_i = (v2080_tmp);
    goto s_loop;
  }
  s_loop:
  if ((isnull((v73_lst)))) {
    return obj_from_bool(0);
  } else {
  if (((car((v73_lst))) == (v70_x))) {
    return (v72_i);
  } else {
  { /* let */
    obj v2078_tmp = obj_from_fixnum(fixnum_from_obj(v72_i) + (1));
    obj v2077_tmp = (cdr((v73_lst)));
    /* tail call */
    v73_lst = (v2077_tmp);
    v72_i = (v2078_tmp);
    goto s_loop;
  }
  }
  }
  }
}

/* labelapp-exp? */
static obj cxs_labelapp_2Dexp_3F(obj v294_exp)
{ 
  if (bool_from_obj((isvector((v294_exp))) ? (((vectorlen((v294_exp))) == (3)) ? obj_from_bool((vectorref((v294_exp), (0))) == (mksymbol(internsym("app-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  { /* let */
    obj v2076_tmp;
  { /* let */
    obj v318_object = (vectorref((v294_exp), (1)));
    v2076_tmp = ((isvector((v318_object))) ? (((vectorlen((v318_object))) == (2)) ? obj_from_bool((vectorref((v318_object), (0))) == (mksymbol(internsym("var-exp")))) : obj_from_bool(0)) : obj_from_bool(0));
  }
    return (bool_from_obj(v2076_tmp) ? (cxs_label_2Did_3F_23301((vectorref((vectorref((v294_exp), (1))), (1))))) : obj_from_bool(0));
  }
  } else {
    return obj_from_bool(0);
  }
}

/* <literal>? */
static obj cxs__3Cliteral_3E_3F(obj v434_x)
{ 
    return ((is_fixnum_obj(v434_x)) ? obj_from_bool(is_fixnum_obj(v434_x)) : obj_from_bool(is_bool_obj(v434_x)));
}

/* string-forest-length */
static obj cxs_string_2Dforest_2Dlength(obj v467_x)
{ 
  if ((isstring((v467_x)))) {
    return obj_from_fixnum(stringlen((v467_x)));
  } else {
  if ((isnull((v467_x)))) {
    return obj_from_fixnum(0);
  } else {
  if ((ispair((v467_x)))) {
    return obj_from_fixnum(fixnum_from_obj(cxs_string_2Dforest_2Dlength((car((v467_x))))) + fixnum_from_obj(cxs_string_2Dforest_2Dlength((cdr((v467_x))))));
  } else {
  { /* letrec */
    obj v472_i;
    obj v471_sum;
  { /* let */
    obj v2075_tmp = obj_from_fixnum(0);
    obj v2074_tmp = obj_from_fixnum(0);
    /* tail call */
    v472_i = (v2074_tmp);
    v471_sum = (v2075_tmp);
    goto s_loop;
  }
  s_loop:
  if ((fixnum_from_obj(v472_i) < (vectorlen((v467_x))))) {
  { /* let */
    obj v2073_tmp = obj_from_fixnum(fixnum_from_obj(v471_sum) + fixnum_from_obj(cxs_string_2Dforest_2Dlength((vectorref((v467_x), fixnum_from_obj(v472_i))))));
    obj v2072_tmp = obj_from_fixnum(fixnum_from_obj(v472_i) + (1));
    /* tail call */
    v472_i = (v2072_tmp);
    v471_sum = (v2073_tmp);
    goto s_loop;
  }
  } else {
    return (v471_sum);
  }
  }
  }
  }
  }
}

/* gc roots */
static obj *globv[] = {
  &cx_exp_2D_3Efree_2Dlexvars,
  &cx_exp_2D_3Efree_2Dvars,
  &cx_exp_2Dvinfo,
  &cx_keywords,
  &cx_last_2Did_2Dtimestamp,
  &cx_the_2Dhalt_2Dprim,
  &cx_var_2Duses_2Din_2Dexp,
  &cx__231007,
  &cx__231036,
  &cx__231064,
  &cx__231075,
  &cx__231076,
  &cx__231093,
  &cx__231098,
  &cx__2311,
  &cx__231113,
  &cx__231138,
  &cx__231208,
  &cx__231248,
  &cx__231270,
  &cx__231287,
  &cx__231305,
  &cx__231327,
  &cx__231360,
  &cx__231363,
  &cx__231365,
  &cx__231369,
  &cx__23169,
  &cx__231709,
  &cx__2318,
  &cx__23183,
  &cx__23203,
  &cx__23239,
  &cx__23333,
  &cx__23390,
  &cx__23711,
  &cx__23746,
  &cx__23752,
  &cx__23778,
  &cx__23801,
  &cx__23819,
  &cx__23835,
  &cx__23862,
  &cx__23891,
  &cx__23920,
  &cx__23949,
  &cx__23978,
};

static cxroot_t root = {
  sizeof(globv)/sizeof(obj *), globv, NULL
};

/* entry points */
static obj host(obj);
static obj cases[237] = {
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
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,
};

/* host procedure */
#define MAX_LIVEREGS 18
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
    cx__2311 = (hpushstr(0, newstring(" ")));
    cx__2318 = (hpushstr(0, newstring("Compiler error: ")));
    cx__23169 = (hpushstr(0, newstring(".")));
    cx__23183 = (hpushstr(0, newstring(":")));
    cx__23203 = (hpushstr(0, newstring("::")));
    { static char s[] = { 36, 0 };
    cx__23239 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 114, 91, 36, 108, 105, 118, 101, 43, 48, 93, 32, 61, 32, 114, 91, 48, 93, 59, 10, 32, 32, 32, 32, 112, 99, 32, 61, 32, 48, 59, 32, 47, 42, 32, 101, 120, 105, 116, 32, 102, 114, 111, 109, 32, 109, 111, 100, 117, 108, 101, 32, 105, 110, 105, 116, 32, 42, 47, 10, 32, 32, 32, 32, 114, 91, 36, 108, 105, 118, 101, 43, 49, 93, 32, 61, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 59, 10, 32, 32, 32, 32, 114, 32, 43, 61, 32, 36, 108, 105, 118, 101, 59, 32, 47, 42, 32, 115, 104, 105, 102, 116, 32, 114, 101, 103, 32, 119, 110, 100, 32, 42, 47, 10, 32, 32, 32, 32, 97, 115, 115, 101, 114, 116, 40, 114, 99, 32, 61, 32, 50, 41, 59, 10, 32, 32, 32, 32, 103, 111, 116, 111, 32, 106, 117, 109, 112, 59, 0 };
    cx__23333 = (hpushstr(0, newstring(s))); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("%localdef")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%definition")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%prim*?!")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%prim*")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%prim?!")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%prim")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%const")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%quote")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("load")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%%")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%include")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("withcc")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("letcc")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("letrec")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("lambda")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("if")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("define")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("begin")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23390 = (hendblk(3)); }
    cx__23711 = (hpushstr(0, newstring("#")));
    cx__23746 = (hpushstr(0, newstring("")));
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("<id>")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("*")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[1] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 2); /* 2 live regs */
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("<id>")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[1] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 2); /* 2 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[1] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("letrec")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23752 = (hendblk(3)); }
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
    cx__23778 = (hendblk(3)); }
    cx__23801 = (hpushstr(0, newstring("void(0)")));
    cx__23819 = (hpushstr(0, newstring("syntax error in expression")));
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("...")));
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
    *--hp = (mksymbol(internsym("*")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23835 = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("...")));
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
    *--hp = (mksymbol(internsym("<string-forest>")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%prim*?!")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23862 = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("...")));
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
    *--hp = (mksymbol(internsym("<string-forest>")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%prim*!")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23891 = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("...")));
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
    *--hp = (mksymbol(internsym("<string-forest>")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%prim*?")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23920 = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("...")));
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
    *--hp = (mksymbol(internsym("<string-forest>")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%prim?!")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23949 = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("...")));
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
    *--hp = (mksymbol(internsym("<string-forest>")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%prim!")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23978 = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("...")));
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
    *--hp = (mksymbol(internsym("<string-forest>")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%prim?")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__231007 = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("...")));
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
    *--hp = (mksymbol(internsym("<string-forest>")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%prim*")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__231036 = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("...")));
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
    *--hp = (mksymbol(internsym("<string-forest>")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%prim")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__231064 = (hendblk(3)); }
    cx__231075 = (hpushstr(0, newstring("unsupported constant syntax")));
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("...")));
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
    *--hp = (mksymbol(internsym("%const")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__231076 = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("<string-forest>")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%prim")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__231093 = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("*")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%quote")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__231098 = (hendblk(3)); }
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
    *--hp = (mksymbol(internsym("withcc")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__231113 = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("*")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("<id>")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("letcc")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__231138 = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("*")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("...")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[1] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 2); /* 2 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("*")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = (mksymbol(internsym("<id>")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[1] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("letrec")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__231208 = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("*")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("...")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[1] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 2); /* 2 live regs */
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("<id>")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[1] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("lambda")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__231248 = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("...")));
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
    *--hp = (mksymbol(internsym("begin")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__231270 = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("*")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("<id>")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("set!")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__231287 = (hendblk(3)); }
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
    *--hp = (mksymbol(internsym("if")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__231305 = (hendblk(3)); }
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
    *--hp = (mksymbol(internsym("*")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("if")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__231327 = (hendblk(3)); }
    { static char s[] = { 46, 10, 0 };
    cx__231360 = (hpushstr(0, newstring(s))); }
    cx__231363 = (hpushstr(0, newstring("no clause matches ~s")));
    cx__231365 = (hpushstr(0, newstring("Error: ")));
    cx__231369 = (hpushstr(0, newstring("Error in ~a: ")));
    { static obj c[] = { obj_from_case(1) }; cx_keep_231406 = (obj)c; }
    r[0] = (hpushstr(0, newstring("void")));
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = (mknull());
    *--hp = r[0];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    r[1] = (hpushstr(1, newstring("objptr")));
    { /* cons */ 
    hreserve(hbsz(3), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    r[1] = (hpushstr(1, newstring("obj")));
    { /* cons */ 
    hreserve(hbsz(3), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__231709 = (hendblk(3)); }
    { static obj c[] = { obj_from_case(4) }; cx_c_2Derror_2A = (obj)c; }
    { static obj c[] = { obj_from_case(11) }; cx_reduce_2Dright = (obj)c; }
    { static obj c[] = { obj_from_case(13) }; cx_reduce_2Dleft = (obj)c; }
    { static obj c[] = { obj_from_case(15) }; cx_reduce_2Dright_2Fsingular_2Didentity = (obj)c; }
    { static obj c[] = { obj_from_case(17) }; cx_reduce_2Dright_2Fright_2Dseed = (obj)c; }
    { static obj c[] = { obj_from_case(19) }; cx_posq = (obj)c; }
    { static obj c[] = { obj_from_case(20) }; cx_string_2Dappend_2A = (obj)c; }
    { static obj c[] = { obj_from_case(23) }; cx_symbol_2A = (obj)c; }
    { static obj c[] = { obj_from_case(28) }; cx_id_3C_3F = (obj)c; }
    cx_last_2Did_2Dtimestamp = obj_from_fixnum(0);
    { static obj c[] = { obj_from_case(29) }; cx_reset_2Dtimestamps = (obj)c; }
    { static obj c[] = { obj_from_case(30) }; cx_timestamp = (obj)c; }
    { static obj c[] = { obj_from_case(31) }; cx_id_2D_3Euname = (obj)c; }
    { static obj c[] = { obj_from_case(32) }; cx_labelapp_2Dexp = (obj)c; }
    { static obj c[] = { obj_from_case(33) }; cx_labelapp_2Dexp_3F = (obj)c; }
    cx_the_2Dhalt_2Dprim = (cx__23333);
    { static obj c[] = { obj_from_case(34) }; cx_memoizate_2Dexp_2Dattribute = (obj)c; }
    { static obj c[] = { obj_from_case(37) }; cx_memoizate_2Dvar_2Dattribute = (obj)c; }
    cx_keywords = (cx__23390);
    { static obj c[] = { obj_from_case(41) }; cx__3Cid_3E_3F = (obj)c; }
    { static obj c[] = { obj_from_case(42) }; cx__3Cliteral_3E_3F = (obj)c; }
    { static obj c[] = { obj_from_case(43) }; cx_string_2Dforest_3F = (obj)c; }
    { static obj c[] = { obj_from_case(50) }; cx_string_2Dforest_2Dlength = (obj)c; }
    { static obj c[] = { obj_from_case(51) }; cx_string_2Dforest_2D_3Estring = (obj)c; }
    { static obj c[] = { obj_from_case(57) }; cx_sexp_2Dmatch_3F = (obj)c; }
    { static obj c[] = { obj_from_case(63) }; cx_parse_2Dprogram = (obj)c; }
    hreserve(hbsz(0+1), 0); /* 0 live regs */
    *--hp = obj_from_case(177);
    r[0] = (hendblk(0+1));
    hreserve(hbsz(0+1), 1); /* 1 live regs */
    *--hp = obj_from_case(212);
    r[1] = (hendblk(0+1));
    r[2+0] = r[0];  
    r[2+1] = r[1];  
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_memoizate_2Dexp_2Dattribute;

case 1: /* keep#1406 k f lst */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_keep_231406: /* k f lst */
  if ((isnull((r[2])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(1);
    r[3] = (hendblk(0+1));
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(2);
    r[3] = (hendblk(4+1));
    r[4+0] = r[1];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[2])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 2: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r keep f k lst */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(3);
    r[6] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[3];  
    r[3] = (cdr((r[5])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = r[3];  
    r[3] = (cdr((r[5])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 3: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k lst */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = (car((r[3])));
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

case 4: /* c-error* k reason args */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_c_2Derror_2A: /* k reason args */
    (void)(fputc('\n', oportdata((cx__2Acurrent_2Doutput_2Dport_2A))));
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(5);
    r[3] = (hendblk(3+1));
    r[4+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (cx__2318);
    r[4+3] = obj_from_bool(1);
    r[4+4] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 5: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  reason args k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(6);
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

case 6: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  args k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(10);
    r[4] = (hendblk(1+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto s_loop;

case 7: /* clo k id */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
s_loop: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (car((r[1])));
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(7);
    r[3] = (hendblk(0+1));
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(8);
    r[3] = (hendblk(4+1));
    r[4+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (cx__2311);
    r[4+3] = obj_from_bool(1);
    r[4+4] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }

case 8: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek  arg loop id k */
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(9);
    r[6] = (hendblk(3+1));
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    /* r[2] */    
    r[3] = obj_from_bool(0);
    r[4] = (cx__2Acurrent_2Doutput_2Dport_2A);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 9: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  loop id k */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 10: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek  k */
    (void)(fputc('\n', oportdata((cx__2Acurrent_2Doutput_2Dport_2A))));
    r[0] = (cx_reset);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[2];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 11: /* reduce-right k f base lst */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
gs_reduce_2Dright: /* k f base lst */
  if ((isnull((r[3])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_case(12);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    /* r[1] */    
    /* r[2] */    
    r[3] = (cdr((r[3])));
    goto gs_reduce_2Dright;
  }

case 12: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r f lst k */
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (car((r[3])));
    r[5+3] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 13: /* reduce-left k f base lst */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
gs_reduce_2Dleft: /* k f base lst */
  if ((isnull((r[3])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(14);
    r[4] = (hendblk(3+1));
    r[5+0] = r[1];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (car((r[3])));
    r[5+3] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 14: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r lst f k */
    r[5+0] = r[4];  
    r[5+1] = r[3];  
    r[5+2] = r[1];  
    r[5+3] = (cdr((r[2])));
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_reduce_2Dleft;

case 15: /* reduce-right/singular-identity k f base lst */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
gs_reduce_2Dright_2Fsingular_2Didentity: /* k f base lst */
  if ((isnull((r[3])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[4+0] = r[0];  
    r[4+1] = r[3];  
    r[4+2] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v5056;
  }

s_loop_v5056: /* k lst f */
    r[3] = (cdr((r[1])));
    r[3] = obj_from_bool(isnull((r[3])));
  if (bool_from_obj(r[3])) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = (car((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(16);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v5056;
  }

case 16: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r f lst k */
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (car((r[3])));
    r[5+3] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 17: /* reduce-right/right-seed k f base lst */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
    /* k f base lst */
  if ((isnull((r[3])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[4+0] = r[0];  
    r[4+1] = r[3];  
    r[4+2] = r[1];  
    r[4+3] = r[2];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v5035;
  }

s_loop_v5035: /* k lst f base */
    r[4] = (cdr((r[1])));
    r[4] = obj_from_bool(isnull((r[4])));
  if (bool_from_obj(r[4])) {
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[0];  
    r[4+2] = (car((r[1])));
    r[4+3] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[4] = (cdr((r[1])));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(18);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[4];  
    /* r[2] */    
    /* r[3] */    
    goto s_loop_v5035;
  }

case 18: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r f lst k */
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (car((r[3])));
    r[5+3] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 19: /* posq k x lst */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k x lst */
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = (cxs_posq((r[1]), (r[2])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 20: /* string-append* k lst */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_string_2Dappend_2A: /* k lst */
    hreserve(hbsz(2+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(21);
    r[2] = (hendblk(2+1));
    r[3+0] = r[2];  
    r[3+1] = r[1];  
    r[3+2] = obj_from_fixnum(0);
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v5014;

s_loop_v5014: /* k lst len */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    { fixnum_t v5185_tmp;
    r[4] = (car((r[1])));
    v5185_tmp = (stringlen((r[4])));
    r[4] = obj_from_fixnum(fixnum_from_obj(r[2]) + (v5185_tmp)); }
    /* r[0] */    
    r[1] = r[3];  
    r[2] = r[4];  
    goto s_loop_v5014;
  }

case 21: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r lst k */
    r[4] = (hpushstr(4, allocstring(fixnum_from_obj(r[1]), '?')));
    r[0] = r[3];  
    r[1] = r[2];  
    r[2] = obj_from_fixnum(0);
    r[3] = r[4];  
    goto s_copy;

case 22: /* clo k lst pos */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1]; }
    r += 1; /* shift reg. wnd */
s_copy: /* k lst pos r */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[4] = (car((r[1])));
    hreserve(hbsz(1+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(22);
    r[5] = (hendblk(1+1));
    r[6+0] = r[0];  
    r[6+1] = obj_from_fixnum(0);
    r[6+2] = r[3];  
    r[6+3] = r[4];  
    r[6+4] = r[5];  
    r[6+5] = r[1];  
    r[6+6] = r[2];  
    r[6+7] = obj_from_fixnum(fixnum_from_obj(r[2]) + (stringlen((r[4]))));
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v4999;
  }

s_loop_v4999: /* k i r s copy lst pos endpos */
    { const fixnum_t v5183_i = fixnum_from_obj(r[1]);
  if (((fixnum_from_obj(r[6]) + (v5183_i)) == fixnum_from_obj(r[7]))) {
    r[8+0] = r[4];  
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[0];  
    r[8+2] = (cdr((r[5])));
    r[8+3] = r[7];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    { const char_t v5184_tmp = (*stringref((r[3]), (v5183_i)));
    (void)(*stringref((r[2]), (fixnum_from_obj(r[6]) + (v5183_i))) = (v5184_tmp)); } 
    /* r[0] */    
    r[1] = obj_from_fixnum((v5183_i) + (1));
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    /* r[7] */    
    goto s_loop_v4999;
  } } 

case 23: /* symbol* k args */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_symbol_2A: /* k args */
    hreserve(hbsz(1+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(26);
    r[2] = (hendblk(1+1));
    r[0] = r[2];  
    /* r[1] */    
    goto s_loop_v4973;

s_loop_v4973: /* k id */
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
    *--hp = obj_from_case(24);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v4973;
  }

case 24: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(25);
    r[5] = (hendblk(2+1));
  if ((issymbol((r[4])))) {
    r[6] = (hpushstr(6, newstring(symbolname(getsymbol((r[4]))))));
    r[7+0] = obj_from_ktrap();
    r[7+1] = r[6];  
    r[7+2] = r[2];  
    r[7+3] = r[1];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v4977;
  } else {
  if (((is_fixnum_obj(r[4])) || (is_flonum_obj(r[4])))) {
  if ((is_fixnum_obj(r[4]))) {
    r[0] = (cx_fixnum_2D_3Estring);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[4];  
    r[3] = obj_from_fixnum(10);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = (cx_flonum_2D_3Estring);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
    r[6+0] = obj_from_ktrap();
    r[6+1] = r[4];  
    r[6+2] = r[2];  
    r[6+3] = r[1];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v4977;
  }
  }

case 25: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
s_l_v4977: /* ek r k r */
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

case 26: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(27);
    r[3] = (hendblk(1+1));
    r[0] = r[3];  
    /* r[1] */    
    goto gs_string_2Dappend_2A;

case 27: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    r[3+0] = r[2];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = (mksymbol(internsym(stringchars((r[1])))));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 28: /* id<? k i1 i2 */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k i1 i2 */
    r[3] = (cdr((r[1])));
    r[3] = (cdr((r[3])));
    r[3] = (car((r[3])));
    r[4] = (cdr((r[2])));
    r[4] = (cdr((r[4])));
    r[4] = (car((r[4])));
    r[3] = obj_from_bool(fixnum_from_obj(r[3]) < fixnum_from_obj(r[4]));
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 29: /* reset-timestamps k */
    assert(rc == 2);
    r += 1; /* shift reg. wnd */
    /* k */
    cx_last_2Did_2Dtimestamp = obj_from_fixnum(0);
    r[1] = obj_from_void(0);
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = r[1];  
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 30: /* timestamp k */
    assert(rc == 2);
    r += 1; /* shift reg. wnd */
    /* k */
    cx_last_2Did_2Dtimestamp = obj_from_fixnum(fixnum_from_obj(cx_last_2Did_2Dtimestamp) + (1));
    r[1] = (cx_last_2Did_2Dtimestamp);
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = r[1];  
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 31: /* id->uname k id */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k id */
  if (bool_from_obj(cxs_global_2Did_2Dprivate_2Dconstant_3F_23266((r[1])))) {
    r[2] = (cdr((r[1])));
    r[2] = (car((r[2])));
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (bool_from_obj(cxs_global_2Did_2Dconstant_3F_23247((r[1])))) {
    { /* cons */ 
    hreserve(hbsz(3), 2); /* 2 live regs */
    *--hp = (mknull());
    *--hp = (cx__23239);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    r[3] = (cdr((r[1])));
    r[3] = (car((r[3])));
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    /* r[0] */    
    r[1] = r[2];  
    goto gs_symbol_2A;
  } else {
    r[2] = (cdr((r[1])));
    r[2] = (cdr((r[2])));
    r[2] = (car((r[2])));
    r[2] = obj_from_bool(fixnum_from_obj(r[2]) < (0));
  if (bool_from_obj(r[2])) {
    r[2] = (cdr((r[1])));
    r[2] = (car((r[2])));
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    r[2] = (cdr((r[2])));
    r[2] = (cdr((r[2])));
    r[2] = (car((r[2])));
    r[2] = obj_from_bool(isstring((r[2])));
  if (bool_from_obj(r[2])) {
    r[2] = (cdr((r[1])));
    r[2] = (cdr((r[2])));
    r[2] = (cdr((r[2])));
    r[2] = (car((r[2])));
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = (cx__23203);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    r[3] = (cdr((r[1])));
    r[3] = (cdr((r[3])));
    r[3] = (car((r[3])));
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = (cx__23169);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    r[3] = (cdr((r[1])));
    r[3] = (car((r[3])));
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    /* r[0] */    
    r[1] = r[2];  
    goto gs_symbol_2A;
  } else {
    r[2] = (cdr((r[1])));
    r[2] = (cdr((r[2])));
    r[2] = (cdr((r[2])));
    r[2] = (car((r[2])));
  if (bool_from_obj(r[2])) {
    r[2] = (cdr((r[1])));
    r[2] = (cdr((r[2])));
    r[2] = (car((r[2])));
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = (cx__23183);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    r[3] = (cdr((r[1])));
    r[3] = (car((r[3])));
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    /* r[0] */    
    r[1] = r[2];  
    goto gs_symbol_2A;
  } else {
    r[2] = (cdr((r[1])));
    r[2] = (cdr((r[2])));
    r[2] = (car((r[2])));
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = (cx__23169);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    r[3] = (cdr((r[1])));
    r[3] = (car((r[3])));
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    /* r[0] */    
    r[1] = r[2];  
    goto gs_symbol_2A;
  }
  }
  }
  }
  }

case 32: /* labelapp-exp k id rands */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k id rands */
    { /* vector */
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("var-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[3] = (hendblk(2+1)); }
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[3] = (hendblk(3+1)); }
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 33: /* labelapp-exp? k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k exp */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cxs_labelapp_2Dexp_3F((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 34: /* memoizate-exp-attribute k fn */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_memoizate_2Dexp_2Dattribute: /* k fn */
    hreserve(hbsz(1), 2); /* 2 live regs */
    *--hp = obj_from_void(0);
    r[2] = (hendblk(1));
    (void)(objptr_from_obj(r[2])[0] = (mknull()));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(35);
    r[2] = (hendblk(2+1));
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 35: /* clo k exp */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* k exp fn memo */
  if ((!bool_from_obj(r[1]))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (objptr_from_obj(r[3])[0] = (mknull()));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[4] = (objptr_from_obj(r[3])[0]);
    { /* assq */
    obj x = (r[1]), l = (r[4]), p = mknull();
    for (; l != mknull(); l = cdr(l)) { p = car(l); if (car(p) == x) break; }
    r[4] = (l == mknull() ? obj_from_bool(0) : p); }
  if (bool_from_obj(r[4])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (cdr((r[4])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(36);
    r[5] = (hendblk(3+1));
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[5];  
    r[6+2] = r[1];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }

case 36: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k exp memo */
    r[5] = (objptr_from_obj(r[4])[0]);
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    (void)(objptr_from_obj(r[4])[0] = (r[5]));
    r[5] = r[1];  
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 37: /* memoizate-var-attribute k fn */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_memoizate_2Dvar_2Dattribute: /* k fn */
    hreserve(hbsz(1), 2); /* 2 live regs */
    *--hp = obj_from_void(0);
    r[2] = (hendblk(1));
    (void)(objptr_from_obj(r[2])[0] = (mknull()));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(38);
    r[2] = (hendblk(2+1));
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 38: /* clo k id exp */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* k id exp fn memo */
  if ((!bool_from_obj(r[1]))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (objptr_from_obj(r[4])[0] = (mknull()));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[5] = (objptr_from_obj(r[4])[0]);
    { /* assq */
    obj x = (r[2]), l = (r[5]), p = mknull();
    for (; l != mknull(); l = cdr(l)) { p = car(l); if (car(p) == x) break; }
    r[5] = (l == mknull() ? obj_from_bool(0) : p); }
  if (bool_from_obj(r[5])) {
    r[6] = (cdr((r[5])));
    { /* assq */
    obj x = (r[1]), l = (r[6]), p = mknull();
    for (; l != mknull(); l = cdr(l)) { p = car(l); if (car(p) == x) break; }
    r[6] = (l == mknull() ? obj_from_bool(0) : p); }
  if (bool_from_obj(r[6])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (cdr((r[6])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(39);
    r[7] = (hendblk(3+1));
    r[8+0] = r[3];  
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[7];  
    r[8+2] = r[1];  
    r[8+3] = r[2];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }
  } else {
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = obj_from_case(40);
    r[6] = (hendblk(4+1));
    r[7+0] = r[3];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[6];  
    r[7+2] = r[1];  
    r[7+3] = r[2];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }
  }

case 39: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id tmp */
    r[5] = (cdr((r[4])));
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    (void)(cdr((r[4])) = (r[5]));
    r[5] = r[1];  
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 40: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r k exp id memo */
    r[6] = (objptr_from_obj(r[5])[0]);
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    (void)(objptr_from_obj(r[5])[0] = (r[6]));
    r[6] = r[1];  
    r[7+0] = r[2];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = obj_from_ktrap();
    r[7+2] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 41: /* <id>? k x */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs__3Cid_3E_3F: /* k x */
  if ((issymbol((r[1])))) {
    { /* memq */
    obj x = (r[1]), l = (cx_keywords);
    for (; l != mknull(); l = cdr(l)) if (car(l) == x) break;
    r[2] = (l == mknull() ? obj_from_bool(0) : l); }
    r[2] = obj_from_bool(!bool_from_obj(r[2]));
  } else {
    r[2] = obj_from_bool(0);
  }
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 42: /* <literal>? k x */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k x */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cxs__3Cliteral_3E_3F((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 43: /* string-forest? k x */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_string_2Dforest_3F: /* k x */
  if ((isstring((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(isstring((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(2+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(44);
    r[2] = (hendblk(2+1));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(47);
    r[2] = (hendblk(2+1));
    r[3+0] = (cx_list_3F);
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[2];  
    r[3+2] = r[1];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 44: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r x k */
  if (bool_from_obj(r[1])) {
    r[4+0] = r[3];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[2])))) {
    { /* vector->list */
    obj v, l = mknull(); int c = (vectorlen((r[2])));
    hreserve(hbsz(3)*c, 4); /* 4 live regs */
    v = r[2];   /* gc-safe */
    while (c-- > 0) { *--hp = l; *--hp = hblkref(v, 1+c);
    *--hp = obj_from_size(PAIR_BTAG); l = hendblk(3); }
    r[4] = (l); }
    r[0] = r[3];  
    r[1] = r[4];  
    goto s_loop_v4848;
  } else {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }

case 45: /* clo k id */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
s_loop_v4848: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(isnull((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(45);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(46);
    r[2] = (hendblk(3+1));
    r[0] = r[2];  
    r[1] = (car((r[1])));
    goto gs_string_2Dforest_3F;
  }

case 46: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 47: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r x k */
  if (bool_from_obj(r[1])) {
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v4831;
  } else {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 48: /* clo k id */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
s_loop_v4831: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(isnull((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(48);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(49);
    r[2] = (hendblk(3+1));
    r[0] = r[2];  
    r[1] = (car((r[1])));
    goto gs_string_2Dforest_3F;
  }

case 49: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 50: /* string-forest-length k x */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k x */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cxs_string_2Dforest_2Dlength((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 51: /* string-forest->string k x */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_string_2Dforest_2D_3Estring: /* k x */
    hreserve(hbsz(1), 2); /* 2 live regs */
    *--hp = obj_from_void(0);
    r[2] = (hendblk(1));
    r[3] = (cxs_string_2Dforest_2Dlength((r[1])));
    r[3] = (hpushstr(4, allocstring(fixnum_from_obj(r[3]), '?')));
    (void)(objptr_from_obj(r[2])[0] = (r[3]));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = obj_from_case(52);
    r[3] = (hendblk(2+1));
    r[4+0] = r[3];  
    r[4+1] = r[1];  
    r[4+2] = obj_from_fixnum(0);
    r[4+3] = r[2];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_copy_v4780;

case 52: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  k str */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (objptr_from_obj(r[3])[0]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 53: /* clo k x pos */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1]; }
    r += 1; /* shift reg. wnd */
s_copy_v4780: /* k x pos str */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(53);
    r[4] = (hendblk(1+1));
  if ((isstring((r[1])))) {
    r[5+0] = r[0];  
    r[5+1] = obj_from_fixnum(0);
    r[5+2] = r[1];  
    r[5+3] = r[3];  
    r[5+4] = r[2];  
    r[5+5] = obj_from_fixnum(fixnum_from_obj(r[2]) + (stringlen((r[1]))));
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v4800;
  } else {
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((ispair((r[1])))) {
    r[5] = (car((r[1])));
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(54);
    r[6] = (hendblk(3+1));
    r[0] = r[6];  
    r[1] = r[5];  
    /* r[2] */    
    /* r[3] */    
    goto s_copy_v4780;
  } else {
    r[5+0] = r[0];  
    r[5+1] = obj_from_fixnum(0);
    r[5+2] = r[2];  
    r[5+3] = r[4];  
    r[5+4] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v4781;
  }
  }
  }

s_loop_v4800: /* k i x str pos endpos */
    { const fixnum_t v5181_i = fixnum_from_obj(r[1]);
  if (((fixnum_from_obj(r[4]) + (v5181_i)) == fixnum_from_obj(r[5]))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    { const char_t v5182_tmp = (*stringref((r[2]), (v5181_i)));
    r[6] = (objptr_from_obj(r[3])[0]);
    (void)(*stringref((r[6]), (fixnum_from_obj(r[4]) + (v5181_i))) = (v5182_tmp)); } 
    /* r[0] */    
    r[1] = obj_from_fixnum((v5181_i) + (1));
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    goto s_loop_v4800;
  } } 

case 54: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r copy x k */
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cdr((r[3])));
    r[5+3] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 55: /* clo k i pos */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2]; }
    r += 1; /* shift reg. wnd */
s_loop_v4781: /* k i pos copy x */
  if ((fixnum_from_obj(r[1]) < (vectorlen((r[4]))))) {
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(55);
    r[5] = (hendblk(2+1));
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = obj_from_case(56);
    r[5] = (hendblk(3+1));
    r[6+0] = r[3];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[5];  
    r[6+2] = (vectorref((r[4]), fixnum_from_obj(r[1])));
    r[6+3] = r[2];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 56: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop i k */
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = obj_from_fixnum(fixnum_from_obj(r[3]) + (1));
    r[5+3] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 57: /* sexp-match? k pat x */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_sexp_2Dmatch_3F: /* k pat x */
  if (((r[1]) == (mksymbol(internsym("*"))))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool((r[1]) == (mksymbol(internsym("*"))));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(58);
    r[3] = (hendblk(3+1));
  if (((r[1]) == (mksymbol(internsym("<id>"))))) {
    r[0] = r[3];  
    r[1] = r[2];  
    goto gs__3Cid_3E_3F;
  } else {
    r[4+0] = obj_from_ktrap();
    r[4+1] = obj_from_bool(0);
    r[4+2] = r[2];  
    r[4+3] = r[1];  
    r[4+4] = r[0];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v4744;
  }
  }

case 58: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
s_l_v4744: /* ek r x pat k */
  if (bool_from_obj(r[1])) {
    r[5+0] = r[4];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[5] = obj_from_bool(((r[3]) == (mksymbol(internsym("<symbol>")))) && (issymbol((r[2]))));
  if (bool_from_obj(r[5])) {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[6] = obj_from_bool(((r[3]) == (mksymbol(internsym("<string>")))) && (isstring((r[2]))));
  if (bool_from_obj(r[6])) {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[7] = (((r[3]) == (mksymbol(internsym("<literal>")))) ? (cxs__3Cliteral_3E_3F((r[2]))) : obj_from_bool(0));
  if (bool_from_obj(r[7])) {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(59);
    r[8] = (hendblk(3+1));
  if (((r[3]) == (mksymbol(internsym("<string-forest>"))))) {
    r[0] = r[8];  
    r[1] = r[2];  
    goto gs_string_2Dforest_3F;
  } else {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_bool(0);
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    goto s_l_v4746;
  }
  }
  }
  }
  }

case 59: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
s_l_v4746: /* ek r x pat k */
  if (bool_from_obj(r[1])) {
    r[5+0] = r[4];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (((r[2]) == (r[3]))) {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool((r[2]) == (r[3]));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((ispair((r[3])))) {
    r[5] = (car((r[3])));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("..."))));
  if (bool_from_obj(r[5])) {
    r[5] = (cdr((r[3])));
    r[5] = obj_from_bool(ispair((r[5])));
  if (bool_from_obj(r[5])) {
    r[5] = (cdr((r[3])));
    r[5] = (cdr((r[5])));
    r[5] = obj_from_bool(isnull((r[5])));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (cdr((r[3])));
    r[5] = (car((r[5])));
    r[5] = obj_from_bool((r[2]) == (r[5]));
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[5] = (cdr((r[3])));
    r[5] = obj_from_bool(ispair((r[5])));
  if (bool_from_obj(r[5])) {
    r[5] = (cdr((r[3])));
    r[5] = (car((r[5])));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("..."))));
  if (bool_from_obj(r[5])) {
    r[5] = (cdr((r[3])));
    r[5] = (cdr((r[5])));
    r[5] = obj_from_bool(isnull((r[5])));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (car((r[3])));
  if (((r[5]) == (mksymbol(internsym("*"))))) {
    r[0] = (cx_list_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    r[1] = r[2];  
    r[2] = r[5];  
    goto s_loop_v4753;
  }
  } else {
  if ((ispair((r[2])))) {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(62);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = (car((r[3])));
    r[2] = (car((r[2])));
    goto gs_sexp_2Dmatch_3F;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }
  }
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }
  }

case 60: /* clo k lst */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v4753: /* k lst pat */
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
  if ((ispair((r[1])))) {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(60);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(61);
    r[3] = (hendblk(3+1));
    r[4+0] = r[3];  
    r[4+1] = r[2];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_sexp_2Dmatch_3F;
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

case 61: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop lst k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 62: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r x pat k */
  if (bool_from_obj(r[1])) {
    r[0] = r[4];  
    r[1] = (cdr((r[3])));
    r[2] = (cdr((r[2])));
    goto gs_sexp_2Dmatch_3F;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 63: /* parse-program k lst */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k lst */
    hreserve(hbsz(1), 2); /* 2 live regs */
    *--hp = obj_from_void(0);
    r[2] = (hendblk(1));
    hreserve(hbsz(1), 3); /* 3 live regs */
    *--hp = obj_from_void(0);
    r[3] = (hendblk(1));
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(64);
    r[4] = (hendblk(1+1));
    (void)(objptr_from_obj(r[2])[0] = (mknull()));
    (void)(objptr_from_obj(r[3])[0] = (mknull()));
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(66);
    r[5] = (hendblk(2+1));
    r[6+0] = r[5];  
    r[6+1] = r[1];  
    r[6+2] = (mknull());
    r[6+3] = r[2];  
    r[6+4] = r[4];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_pprog;

case 64: /* clo k sym e */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k sym e global-e */
    r[4] = (cxs_lookupicate_23615((r[1]), (r[2])));
  if (bool_from_obj(r[4])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[5] = (objptr_from_obj(r[3])[0]);
    r[5] = (cxs_lookupicate_23615((r[1]), (r[5])));
  if (bool_from_obj(r[5])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = obj_from_case(65);
    r[6] = (hendblk(3+1));
    r[7+0] = (cx_timestamp);
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;
  }
  }

case 65: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k global-e sym */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = obj_from_bool(0);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = obj_from_fixnum(-fixnum_from_obj(r[1]));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("var")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[6] = (objptr_from_obj(r[3])[0]);
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    (void)(objptr_from_obj(r[3])[0] = (r[6]));
    r[6] = r[5];  
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 66: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r lifted-const-alist k */
    hreserve(hbsz(0+1), 4); /* 4 live regs */
    *--hp = obj_from_case(67);
    r[4] = (hendblk(0+1));
    r[5+0] = r[3];  
    r[5+1] = r[4];  
    r[5+2] = r[1];  
    r[5+3] = (objptr_from_obj(r[2])[0]);
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_reduce_2Dleft;

case 67: /* clo k c&exp&id exp */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k c&exp&id exp */
    r[3] = (cdr((r[1])));
    r[3] = (car((r[3])));
    r[4] = (cdr((r[1])));
    r[4] = (cdr((r[4])));
    r[4] = (car((r[4])));
    { /* vector */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("varassign-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[3] = (hendblk(3+1)); }
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = obj_from_case(68);
    r[4] = (hendblk(3+1));
    r[5+0] = (cx_timestamp);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 68: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k exp1 exp */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = obj_from_bool(0);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("_")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("var")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[7] = (hendblk(3+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[8] = (hendblk(3+1)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 69: /* clo k c */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* k c lifted-const-alist make-id */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(84);
    r[4] = (hendblk(2+1));
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(70);
    r[4] = (hendblk(4+1));
    r[5+0] = (cx_assoc);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = r[1];  
    r[5+3] = (objptr_from_obj(r[2])[0]);
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 70: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r psexp c lifted-const-alist k */
  if (bool_from_obj(r[1])) {
    r[6] = (cdr((r[1])));
    r[6] = (cdr((r[6])));
    r[6] = (car((r[6])));
    r[7+0] = r[5];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = obj_from_ktrap();
    r[7+2] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = r[2];  
    *--hp = obj_from_case(71);
    r[6] = (hendblk(4+1));
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[3];  
    *--hp = obj_from_case(75);
    r[6] = (hendblk(2+1));
    r[0] = r[6];  
    r[1] = (cx__23752);
    r[2] = r[3];  
    goto gs_sexp_2Dmatch_3F;
  }

case 71: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r psexp k c lifted-const-alist */
    hreserve(hbsz(5+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(72);
    r[6] = (hendblk(5+1));
    r[0] = (cx_timestamp);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 72: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r r psexp k c lifted-const-alist */
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(73);
    r[7] = (hendblk(5+1));
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = (cx__23711);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    r[9+0] = (cx_symbol_2A);
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[7];  
    r[9+2] = r[8];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 73: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r psexp k c lifted-const-alist r */
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("private")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = obj_from_fixnum(-fixnum_from_obj(r[6]));
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
    *--hp = (mksymbol(internsym("var")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    hreserve(hbsz(4+1), 8); /* 8 live regs */
    *--hp = r[5];  
    *--hp = r[7];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(74);
    r[8] = (hendblk(4+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = r[4];  
    r[3] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 74: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r k c r lifted-const-alist */
    r[6] = (objptr_from_obj(r[5])[0]);
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = r[4];  
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
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    (void)(objptr_from_obj(r[5])[0] = (r[6]));
    r[6] = r[4];  
    r[7+0] = r[2];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = obj_from_ktrap();
    r[7+2] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 75: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r c k */
  if (bool_from_obj(r[1])) {
    r[4] = (cdr((r[2])));
    r[4] = (cdr((r[4])));
    r[4] = (car((r[4])));
    r[0] = (cx_c_2Dundecorate_2Dalvar);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (cx__23746);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

s_pprog: /* k prog e lifted-const-alist make-id */
    hreserve(hbsz(1+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(81);
    r[5] = (hendblk(1+1));
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(84);
    r[6] = (hendblk(2+1));
    r[7+0] = r[5];  
    r[7+1] = r[1];  
    r[7+2] = (mknull());
    r[7+3] = r[6];  
    r[7+4] = r[4];  
    r[7+5] = r[2];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v4612;

case 76: /* clo k prog exps */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2];
    r[1+5] = p[3]; }
    r += 1; /* shift reg. wnd */
s_loop_v4612: /* k prog exps psexp make-id e */
  if ((isnull((r[1])))) {
    { fixnum_t v5180_tmp;
    { /* length */
    int n; obj l = r[2];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v5180_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v5180_tmp);
    hreserve(hbsz(3)*c, 6); /* 6 live regs */
    l = r[2];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[6] = (o); } }
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[6] = (car((r[1])));
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(76);
    r[7] = (hendblk(3+1));
    hreserve(hbsz(4+1), 8); /* 8 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[7];  
    *--hp = obj_from_case(77);
    r[7] = (hendblk(4+1));
    hreserve(hbsz(5+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(78);
    r[7] = (hendblk(5+1));
    r[0] = r[7];  
    r[1] = (cx__23778);
    r[2] = r[6];  
    goto gs_sexp_2Dmatch_3F;
  }

case 77: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop exps prog k */
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[7+0] = r[2];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[5];  
    r[7+2] = (cdr((r[4])));
    r[7+3] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 78: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r psexp make-id e x k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(79);
    r[7] = (hendblk(4+1));
    r[8] = (cdr((r[5])));
    r[8] = (cdr((r[8])));
    r[8] = (car((r[8])));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[8];  
    r[3] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[5];  
    r[3] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 79: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r make-id e x k */
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = obj_from_case(80);
    r[6] = (hendblk(2+1));
    r[7] = (cdr((r[4])));
    r[7] = (car((r[7])));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[7];  
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 80: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("varassign-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 81: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(82);
    r[3] = (hendblk(0+1));
    { /* vector */
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = (cx__23801);
    *--hp = (mksymbol(internsym("no-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(4+1)); }
    r[5+0] = r[2];  
    r[5+1] = r[3];  
    r[5+2] = r[4];  
    r[5+3] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_reduce_2Dright;

case 82: /* clo k exp1 exp2 */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k exp1 exp2 */
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(83);
    r[3] = (hendblk(3+1));
    r[4+0] = (cx_timestamp);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 83: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k exp1 exp2 */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = obj_from_bool(0);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("_")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("var")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[7] = (hendblk(3+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[8] = (hendblk(3+1)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 84: /* clo k x e */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* k x e lifted-const-alist make-id */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(69);
    r[5] = (hendblk(2+1));
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(84);
    r[6] = (hendblk(2+1));
    r[7+0] = r[0];  
    r[7+1] = r[1];  
    r[7+2] = r[5];  
    r[7+3] = r[6];  
    r[7+4] = r[4];  
    r[7+5] = r[2];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_p;

case 85: /* clo k x */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
s_p: /* k x liftc psexp make-id e */
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(85);
    r[6] = (hendblk(4+1));
    hreserve(hbsz(7+1), 7); /* 7 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(86);
    r[6] = (hendblk(7+1));
    r[7+0] = r[6];  
    r[7+1] = (cx__231327);
    r[7+2] = r[1];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_sexp_2Dmatch_3F;

case 86: /* clo ek r */
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
    /* ek r liftc psexp make-id e p x k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_case(87);
    r[9] = (hendblk(3+1));
    r[10] = (cdr((r[7])));
    r[10] = (cdr((r[10])));
    r[10] = (cdr((r[10])));
    r[10] = (car((r[10])));
    r[0] = r[6];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (r[10]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
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
    *--hp = obj_from_case(90);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    r[1] = (cx__231305);
    r[2] = r[7];  
    goto gs_sexp_2Dmatch_3F;
  }

case 87: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r p x k */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(88);
    r[5] = (hendblk(4+1));
    r[6] = (cdr((r[3])));
    r[6] = (cdr((r[6])));
    r[6] = (car((r[6])));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 88: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r p x k r */
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(89);
    r[6] = (hendblk(3+1));
    r[7] = (cdr((r[3])));
    r[7] = (car((r[7])));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 89: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r r */
    { /* vector */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("if-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[5] = (hendblk(4+1)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 90: /* clo ek r */
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
    /* ek r liftc psexp make-id e p x k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_case(91);
    r[9] = (hendblk(3+1));
    r[10] = (cdr((r[7])));
    r[10] = (cdr((r[10])));
    r[10] = (car((r[10])));
    r[0] = r[6];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (r[10]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
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
    *--hp = obj_from_case(93);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    r[1] = (cx__231287);
    r[2] = r[7];  
    goto gs_sexp_2Dmatch_3F;
  }

case 91: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r p x k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(92);
    r[5] = (hendblk(2+1));
    r[6] = (cdr((r[3])));
    r[6] = (car((r[6])));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 92: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* vector */
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = (cx__23801);
    *--hp = (mksymbol(internsym("no-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(4+1)); }
    { /* vector */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("if-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(4+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 93: /* clo ek r */
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
    /* ek r liftc psexp make-id e p x k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_case(94);
    r[9] = (hendblk(3+1));
    r[10] = (cdr((r[7])));
    r[10] = (car((r[10])));
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (r[10]);
    r[3] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[4];  
    *--hp = obj_from_case(96);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    r[1] = (cx__231270);
    r[2] = r[7];  
    goto gs_sexp_2Dmatch_3F;
  }

case 94: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r p x k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(95);
    r[5] = (hendblk(2+1));
    r[6] = (cdr((r[3])));
    r[6] = (cdr((r[6])));
    r[6] = (car((r[6])));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 95: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = (mksymbol(internsym("varassign-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 96: /* clo ek r */
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
    /* ek r make-id liftc psexp e p x k */
  if (bool_from_obj(r[1])) {
    r[9] = (cdr((r[7])));
    hreserve(hbsz(1+1), 10); /* 10 live regs */
    *--hp = r[8];  
    *--hp = obj_from_case(99);
    r[10] = (hendblk(1+1));
    r[0] = (r[10]);
    r[1] = r[9];  
    r[2] = r[6];  
    goto s_loop_v4505;
  } else {
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(102);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    r[1] = (cx__231248);
    r[2] = r[7];  
    goto gs_sexp_2Dmatch_3F;
  }

s_loop_v4505: /* k id p */
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
    *--hp = obj_from_case(97);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v4505;
  }

case 97: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r p id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(98);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 98: /* clo ek r */
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

case 99: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(100);
    r[3] = (hendblk(0+1));
    { /* vector */
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = (cx__23801);
    *--hp = (mksymbol(internsym("no-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(4+1)); }
    r[5+0] = r[2];  
    r[5+1] = r[3];  
    r[5+2] = r[4];  
    r[5+3] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_reduce_2Dright_2Fsingular_2Didentity;

case 100: /* clo k exp1 exp2 */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k exp1 exp2 */
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(101);
    r[3] = (hendblk(3+1));
    r[4+0] = (cx_timestamp);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 101: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k exp1 exp2 */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = obj_from_bool(0);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("_")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("var")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[7] = (hendblk(3+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[8] = (hendblk(3+1)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

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
    /* ek r make-id liftc p psexp e x k */
  if (bool_from_obj(r[1])) {
    r[9] = (cdr((r[7])));
    r[9] = (car((r[9])));
    hreserve(hbsz(4+1), 10); /* 10 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(105);
    r[10] = (hendblk(4+1));
    r[0] = (r[10]);
    r[1] = r[9];  
    goto s_loop_v4464;
  } else {
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[6];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(107);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    r[1] = (cx__231208);
    r[2] = r[7];  
    goto gs_sexp_2Dmatch_3F;
  }

s_loop_v4464: /* k id */
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
    *--hp = obj_from_case(103);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v4464;
  }

case 103: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(104);
    r[5] = (hendblk(3+1));
    r[0] = (cx_timestamp);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 104: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r sym */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = obj_from_bool(0);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("var")));
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

case 105: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r psexp e x k */
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = obj_from_case(106);
    r[6] = (hendblk(2+1));
    r[7] = (cdr((r[4])));
    r[7] = (cdr((r[7])));
    r[7] = (car((r[7])));
    { fixnum_t v5179_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v5179_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v5179_tmp);
    hreserve(hbsz(3)*c, 8); /* 8 live regs */
    l = r[1];   t = r[3];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[8] = (o); } }
    r[9+0] = r[2];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[6];  
    r[9+2] = r[7];  
    r[9+3] = r[8];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 106: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
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
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r make-id liftc p psexp x k e */
  if (bool_from_obj(r[1])) {
    r[9] = (cdr((r[6])));
    r[9] = (car((r[9])));
    hreserve(hbsz(4+1), 10); /* 10 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(109);
    r[10] = (hendblk(4+1));
    r[0] = (r[10]);
    r[1] = r[9];  
    goto s_loop_v4437;
  } else {
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[6];  
    *--hp = r[7];  
    *--hp = r[8];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(119);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    r[1] = (cx__231138);
    r[2] = r[6];  
    goto gs_sexp_2Dmatch_3F;
  }

s_loop_v4437: /* k id */
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
    *--hp = obj_from_case(108);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v4437;
  }

case 108: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    r[4] = (car((r[4])));
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

case 109: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r psexp x k e */
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(112);
    r[6] = (hendblk(4+1));
    r[0] = r[6];  
    /* r[1] */    
    goto s_loop_v4416;

s_loop_v4416: /* k id */
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
    *--hp = obj_from_case(110);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v4416;
  }

case 110: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(111);
    r[5] = (hendblk(3+1));
    r[0] = (cx_timestamp);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 111: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r sym */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = obj_from_bool(0);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("var")));
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

case 112: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r psexp x k e */
    { fixnum_t v5178_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v5178_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v5178_tmp);
    hreserve(hbsz(3)*c, 6); /* 6 live regs */
    l = r[1];   t = r[5];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[6] = (o); } }
    r[7] = (cdr((r[3])));
    r[7] = (car((r[7])));
    hreserve(hbsz(5+1), 8); /* 8 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[6];  
    *--hp = r[2];  
    *--hp = obj_from_case(114);
    r[8] = (hendblk(5+1));
    r[0] = r[8];  
    r[1] = r[7];  
    goto s_loop_v4401;

s_loop_v4401: /* k id */
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
    *--hp = obj_from_case(113);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v4401;
  }

case 113: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    r[5] = (cdr((r[4])));
    r[4] = (car((r[5])));
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

case 114: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r psexp e+ x r k */
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(117);
    r[7] = (hendblk(5+1));
    r[0] = r[7];  
    /* r[1] */    
    /* r[2] */    
    /* r[3] */    
    goto s_loop_v4380;

s_loop_v4380: /* k id psexp e+ */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[4] = (cdr((r[1])));
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(115);
    r[5] = (hendblk(4+1));
    r[0] = r[5];  
    r[1] = r[4];  
    /* r[2] */    
    /* r[3] */    
    goto s_loop_v4380;
  }

case 115: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r psexp e+ id k */
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = obj_from_case(116);
    r[6] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (car((r[4])));
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 116: /* clo ek r */
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

case 117: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r psexp e+ x r k */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_case(118);
    r[7] = (hendblk(3+1));
    r[8] = (cdr((r[4])));
    r[8] = (cdr((r[8])));
    r[8] = (car((r[8])));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[8];  
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 118: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r r r k */
    r[5+0] = (cx_letrec_2Dexp);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = r[3];  
    r[5+3] = r[2];  
    r[5+4] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
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
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r make-id liftc p psexp e k x */
  if (bool_from_obj(r[1])) {
    r[9] = (cdr((r[8])));
    r[9] = (car((r[9])));
    hreserve(hbsz(5+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[7];  
    *--hp = r[8];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(120);
    r[10] = (hendblk(5+1));
    r[0] = (cx_timestamp);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;
  } else {
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[7];  
    *--hp = r[8];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[6];  
    *--hp = r[2];  
    *--hp = obj_from_case(122);
    r[9] = (hendblk(6+1));
    r[0] = r[9];  
    r[1] = (cx__231113);
    r[2] = r[8];  
    goto gs_sexp_2Dmatch_3F;
  }

case 120: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r psexp e x k sym */
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = obj_from_bool(0);
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
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = (mksymbol(internsym("var")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    hreserve(hbsz(2+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = obj_from_case(121);
    r[8] = (hendblk(2+1));
    r[9] = (cdr((r[4])));
    r[9] = (cdr((r[9])));
    r[9] = (car((r[9])));
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = (mknull());
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { fixnum_t v5177_tmp;
    { /* length */
    int n; obj l = (r[10]);
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v5177_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v5177_tmp);
    hreserve(hbsz(3)*c, 11); /* 11 live regs */
    l = (r[10]); t = r[3];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[10] = (o); } }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = r[9];  
    r[3] = (r[10]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 121: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = (mksymbol(internsym("letcc-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 122: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r make-id e liftc p x k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(123);
    r[8] = (hendblk(3+1));
    r[9] = (cdr((r[6])));
    r[9] = (cdr((r[9])));
    r[9] = (car((r[9])));
    r[0] = r[5];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = r[9];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[7];  
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(125);
    r[8] = (hendblk(6+1));
    r[0] = r[8];  
    r[1] = (cx__231098);
    r[2] = r[6];  
    goto gs_sexp_2Dmatch_3F;
  }

case 123: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r p x k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(124);
    r[5] = (hendblk(2+1));
    r[6] = (cdr((r[3])));
    r[6] = (car((r[6])));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 124: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("withcc-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 125: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r make-id e p liftc k x */
  if (bool_from_obj(r[1])) {
    r[8] = (cdr((r[7])));
    r[8] = (car((r[8])));
    hreserve(hbsz(4+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(126);
    r[9] = (hendblk(4+1));
    r[0] = r[9];  
    r[1] = (cx__231093);
    r[2] = r[8];  
    goto gs_sexp_2Dmatch_3F;
  } else {
    hreserve(hbsz(5+1), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[7];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(129);
    r[8] = (hendblk(5+1));
    r[0] = r[8];  
    r[1] = (cx__231076);
    r[2] = r[7];  
    goto gs_sexp_2Dmatch_3F;
  }

case 126: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r p liftc k c */
  if (bool_from_obj(r[1])) {
    r[6] = (cdr((r[5])));
    r[6] = (car((r[6])));
    r[6] = obj_from_bool(isvector((r[6])));
  if (bool_from_obj(r[6])) {
    hreserve(hbsz(1+1), 6); /* 6 live regs */
    *--hp = r[4];  
    *--hp = obj_from_case(127);
    r[6] = (hendblk(1+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
    hreserve(hbsz(1+1), 6); /* 6 live regs */
    *--hp = r[4];  
    *--hp = obj_from_case(128);
    r[6] = (hendblk(1+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 127: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* vector */
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("var-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[3] = (hendblk(2+1)); }
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 128: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* vector */
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("var-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[3] = (hendblk(2+1)); }
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 129: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r make-id e p x k */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    r[0] = r[6];  
    r[1] = (cx__231075);
    r[2] = r[7];  
    goto gs_c_2Derror_2A;
  } else {
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(130);
    r[7] = (hendblk(5+1));
    r[0] = r[7];  
    r[1] = (cx__231064);
    r[2] = r[5];  
    goto gs_sexp_2Dmatch_3F;
  }

case 130: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r make-id e p x k */
  if (bool_from_obj(r[1])) {
    r[7] = (cdr((r[5])));
    r[7] = (cdr((r[7])));
    hreserve(hbsz(2+1), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(133);
    r[8] = (hendblk(2+1));
    r[0] = r[8];  
    r[1] = r[7];  
    r[2] = r[4];  
    goto s_loop_v4283;
  } else {
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(135);
    r[7] = (hendblk(5+1));
    r[0] = r[7];  
    r[1] = (cx__231036);
    r[2] = r[5];  
    goto gs_sexp_2Dmatch_3F;
  }

s_loop_v4283: /* k id p */
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
    *--hp = obj_from_case(131);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v4283;
  }

case 131: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r p id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(132);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 132: /* clo ek r */
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

case 133: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r x k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(134);
    r[4] = (hendblk(2+1));
    r[5] = (cdr((r[2])));
    r[5] = (car((r[5])));
    r[0] = r[4];  
    r[1] = r[5];  
    goto gs_string_2Dforest_2D_3Estring;

case 134: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* vector */
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("no-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(4+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 135: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r make-id e p x k */
  if (bool_from_obj(r[1])) {
    r[7] = (cdr((r[5])));
    r[7] = (cdr((r[7])));
    hreserve(hbsz(2+1), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(138);
    r[8] = (hendblk(2+1));
    r[0] = r[8];  
    r[1] = r[7];  
    r[2] = r[4];  
    goto s_loop_v4250;
  } else {
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(140);
    r[7] = (hendblk(5+1));
    r[0] = r[7];  
    r[1] = (cx__231007);
    r[2] = r[5];  
    goto gs_sexp_2Dmatch_3F;
  }

s_loop_v4250: /* k id p */
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
    *--hp = obj_from_case(136);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v4250;
  }

case 136: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r p id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(137);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 137: /* clo ek r */
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

case 138: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r x k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(139);
    r[4] = (hendblk(2+1));
    r[5] = (cdr((r[2])));
    r[5] = (car((r[5])));
    r[0] = r[4];  
    r[1] = r[5];  
    goto gs_string_2Dforest_2D_3Estring;

case 139: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* vector */
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("*-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(4+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
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
    /* ek r make-id e p x k */
  if (bool_from_obj(r[1])) {
    r[7] = (cdr((r[5])));
    r[7] = (cdr((r[7])));
    hreserve(hbsz(2+1), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(143);
    r[8] = (hendblk(2+1));
    r[0] = r[8];  
    r[1] = r[7];  
    r[2] = r[4];  
    goto s_loop_v4217;
  } else {
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(145);
    r[7] = (hendblk(5+1));
    r[0] = r[7];  
    r[1] = (cx__23978);
    r[2] = r[5];  
    goto gs_sexp_2Dmatch_3F;
  }

s_loop_v4217: /* k id p */
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
    *--hp = obj_from_case(141);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v4217;
  }

case 141: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r p id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(142);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 142: /* clo ek r */
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

case 143: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r x k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(144);
    r[4] = (hendblk(2+1));
    r[5] = (cdr((r[2])));
    r[5] = (car((r[5])));
    r[0] = r[4];  
    r[1] = r[5];  
    goto gs_string_2Dforest_2D_3Estring;

case 144: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* vector */
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("?-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(4+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 145: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r make-id e p x k */
  if (bool_from_obj(r[1])) {
    r[7] = (cdr((r[5])));
    r[7] = (cdr((r[7])));
    hreserve(hbsz(2+1), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(148);
    r[8] = (hendblk(2+1));
    r[0] = r[8];  
    r[1] = r[7];  
    r[2] = r[4];  
    goto s_loop_v4184;
  } else {
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(150);
    r[7] = (hendblk(5+1));
    r[0] = r[7];  
    r[1] = (cx__23949);
    r[2] = r[5];  
    goto gs_sexp_2Dmatch_3F;
  }

s_loop_v4184: /* k id p */
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
    *--hp = obj_from_case(146);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v4184;
  }

case 146: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r p id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(147);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 147: /* clo ek r */
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

case 148: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r x k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(149);
    r[4] = (hendblk(2+1));
    r[5] = (cdr((r[2])));
    r[5] = (car((r[5])));
    r[0] = r[4];  
    r[1] = r[5];  
    goto gs_string_2Dforest_2D_3Estring;

case 149: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* vector */
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("!-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(4+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
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
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r make-id e p x k */
  if (bool_from_obj(r[1])) {
    r[7] = (cdr((r[5])));
    r[7] = (cdr((r[7])));
    hreserve(hbsz(2+1), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(153);
    r[8] = (hendblk(2+1));
    r[0] = r[8];  
    r[1] = r[7];  
    r[2] = r[4];  
    goto s_loop_v4151;
  } else {
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(155);
    r[7] = (hendblk(5+1));
    r[0] = r[7];  
    r[1] = (cx__23920);
    r[2] = r[5];  
    goto gs_sexp_2Dmatch_3F;
  }

s_loop_v4151: /* k id p */
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
    *--hp = obj_from_case(151);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v4151;
  }

case 151: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r p id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(152);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 152: /* clo ek r */
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

case 153: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r x k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(154);
    r[4] = (hendblk(2+1));
    r[5] = (cdr((r[2])));
    r[5] = (car((r[5])));
    r[0] = r[4];  
    r[1] = r[5];  
    goto gs_string_2Dforest_2D_3Estring;

case 154: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* vector */
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("?!-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(4+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 155: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r make-id e p x k */
  if (bool_from_obj(r[1])) {
    r[7] = (cdr((r[5])));
    r[7] = (cdr((r[7])));
    hreserve(hbsz(2+1), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(158);
    r[8] = (hendblk(2+1));
    r[0] = r[8];  
    r[1] = r[7];  
    r[2] = r[4];  
    goto s_loop_v4118;
  } else {
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(160);
    r[7] = (hendblk(5+1));
    r[0] = r[7];  
    r[1] = (cx__23891);
    r[2] = r[5];  
    goto gs_sexp_2Dmatch_3F;
  }

s_loop_v4118: /* k id p */
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
    *--hp = obj_from_case(156);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v4118;
  }

case 156: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r p id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(157);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 157: /* clo ek r */
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

case 158: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r x k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(159);
    r[4] = (hendblk(2+1));
    r[5] = (cdr((r[2])));
    r[5] = (car((r[5])));
    r[0] = r[4];  
    r[1] = r[5];  
    goto gs_string_2Dforest_2D_3Estring;

case 159: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* vector */
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("*?-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(4+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 160: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r make-id e p x k */
  if (bool_from_obj(r[1])) {
    r[7] = (cdr((r[5])));
    r[7] = (cdr((r[7])));
    hreserve(hbsz(2+1), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(163);
    r[8] = (hendblk(2+1));
    r[0] = r[8];  
    r[1] = r[7];  
    r[2] = r[4];  
    goto s_loop_v4085;
  } else {
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(165);
    r[7] = (hendblk(5+1));
    r[0] = r[7];  
    r[1] = (cx__23862);
    r[2] = r[5];  
    goto gs_sexp_2Dmatch_3F;
  }

s_loop_v4085: /* k id p */
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
    *--hp = obj_from_case(161);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v4085;
  }

case 161: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r p id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(162);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 162: /* clo ek r */
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

case 163: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r x k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(164);
    r[4] = (hendblk(2+1));
    r[5] = (cdr((r[2])));
    r[5] = (car((r[5])));
    r[0] = r[4];  
    r[1] = r[5];  
    goto gs_string_2Dforest_2D_3Estring;

case 164: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* vector */
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("*!-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(4+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 165: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r make-id e p x k */
  if (bool_from_obj(r[1])) {
    r[7] = (cdr((r[5])));
    r[7] = (cdr((r[7])));
    hreserve(hbsz(2+1), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(168);
    r[8] = (hendblk(2+1));
    r[0] = r[8];  
    r[1] = r[7];  
    r[2] = r[4];  
    goto s_loop_v4052;
  } else {
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(170);
    r[7] = (hendblk(5+1));
    r[0] = r[7];  
    r[1] = (cx__23835);
    r[2] = r[5];  
    goto gs_sexp_2Dmatch_3F;
  }

s_loop_v4052: /* k id p */
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
    *--hp = obj_from_case(166);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v4052;
  }

case 166: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r p id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(167);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 167: /* clo ek r */
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

case 168: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r x k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(169);
    r[4] = (hendblk(2+1));
    r[5] = (cdr((r[2])));
    r[5] = (car((r[5])));
    r[0] = r[4];  
    r[1] = r[5];  
    goto gs_string_2Dforest_2D_3Estring;

case 169: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* vector */
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("*?!-effect")));
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(4+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 170: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r make-id e p x k */
  if (bool_from_obj(r[1])) {
    r[7] = (cdr((r[5])));
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(173);
    r[8] = (hendblk(3+1));
    r[0] = r[8];  
    r[1] = r[7];  
    r[2] = r[4];  
    goto s_loop_v4019;
  } else {
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(175);
    r[7] = (hendblk(4+1));
    r[0] = r[7];  
    r[1] = (mksymbol(internsym("<id>")));
    r[2] = (r[5]);
    goto gs_sexp_2Dmatch_3F;
  }

s_loop_v4019: /* k id p */
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
    *--hp = obj_from_case(171);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v4019;
  }

case 171: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r p id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(172);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 172: /* clo ek r */
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

case 173: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r p x k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(174);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 174: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 175: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r make-id e x k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(1+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = obj_from_case(176);
    r[6] = (hendblk(1+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[4];  
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[5];  
    r[1] = (cx__23819);
    r[2] = r[6];  
    goto gs_c_2Derror_2A;
  }

case 176: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* vector */
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("var-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[3] = (hendblk(2+1)); }
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 177: /* clo ek r */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* ek r */
    cx_exp_2Dvinfo = r[1];  
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(178);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(210);
    r[3] = (hendblk(0+1));
    r[0] = r[2];  
    r[1] = r[3];  
    goto gs_memoizate_2Dvar_2Dattribute;

case 178: /* clo ek r */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* ek r */
    cx_var_2Duses_2Din_2Dexp = r[1];  
    { static obj c[] = { obj_from_case(179) }; cx_var_2Dreferenced_2Din_2Dexp_3F = (obj)c; }
    { static obj c[] = { obj_from_case(181) }; cx_var_2Dassigned_2Din_2Dexp_3F = (obj)c; }
    { static obj c[] = { obj_from_case(183) }; cx_var_2Dreference_2Dcount = (obj)c; }
    { static obj c[] = { obj_from_case(185) }; cx_var_2Dassignment_2Dcount = (obj)c; }
    { static obj c[] = { obj_from_case(188) }; cx_var_2Donly_2Dapplied_2Din_2Dexp_3F = (obj)c; }
    { static obj c[] = { obj_from_case(190) }; cx_var_2Duse_2Dunboxed_2Dctype = (obj)c; }
    { static obj c[] = { obj_from_case(192) }; cx_var_2Dunboxed_2Dctype_2Din_2Dexp = (obj)c; }
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(196);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(207);
    r[3] = (hendblk(0+1));
    r[0] = r[2];  
    r[1] = r[3];  
    goto gs_memoizate_2Dexp_2Dattribute;

case 179: /* var-referenced-in-exp? k id exp */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k id exp */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(180);
    r[3] = (hendblk(1+1));
    r[4+0] = (cx_var_2Duses_2Din_2Dexp);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[1];  
    r[4+3] = r[2];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 180: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    r[3+0] = r[2];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(ispair((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 181: /* var-assigned-in-exp? k id exp */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k id exp */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(182);
    r[3] = (hendblk(1+1));
    r[4+0] = (cx_var_2Duses_2Din_2Dexp);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[1];  
    r[4+3] = r[2];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 182: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    r[0] = r[2];  
    /* r[1] */    
    goto s_loop_v3914;

s_loop_v3914: /* k id */
  if ((!(isnull((r[1]))))) {
    r[2] = (car((r[1])));
    r[2] = (vectorref((r[2]), (1)));
  if (bool_from_obj(r[2])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    /* r[0] */    
    r[1] = r[3];  
    goto s_loop_v3914;
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

case 183: /* var-reference-count k id exp */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k id exp */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(184);
    r[3] = (hendblk(1+1));
    r[4+0] = (cx_var_2Duses_2Din_2Dexp);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[1];  
    r[4+3] = r[2];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 184: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    r[3] = obj_from_fixnum(n); }
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 185: /* var-assignment-count k id exp */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k id exp */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(186);
    r[3] = (hendblk(1+1));
    r[4+0] = (cx_var_2Duses_2Din_2Dexp);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[1];  
    r[4+3] = r[2];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 186: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(187);
    r[3] = (hendblk(0+1));
    r[4+0] = r[2];  
    r[4+1] = r[3];  
    r[4+2] = obj_from_fixnum(0);
    r[4+3] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_reduce_2Dleft;

case 187: /* clo k vu n */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k vu n */
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = (bool_from_obj(vectorref((r[1]), (1))) ? obj_from_fixnum(fixnum_from_obj(r[2]) + (1)) : (r[2]));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 188: /* var-only-applied-in-exp? k id exp */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k id exp */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(189);
    r[3] = (hendblk(1+1));
    r[4+0] = (cx_var_2Duses_2Din_2Dexp);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[1];  
    r[4+3] = r[2];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 189: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    r[0] = r[2];  
    /* r[1] */    
    goto s_loop_v3859;

s_loop_v3859: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(isnull((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (car((r[1])));
    r[2] = (vectorref((r[2]), (2)));
  if (bool_from_obj(r[2])) {
    r[2] = (cdr((r[1])));
    /* r[0] */    
    r[1] = r[2];  
    goto s_loop_v3859;
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
  }

case 190: /* var-use-unboxed-ctype k vu */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_var_2Duse_2Dunboxed_2Dctype: /* k vu */
    r[2] = (vectorref((r[1]), (3)));
  if ((isstring((r[2])))) {
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = obj_from_case(191);
    r[3] = (hendblk(2+1));
    r[4+0] = (cx_member);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[2];  
    r[4+3] = (cx__231709);
    r += 4; /* shift reg wnd */
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

case 191: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k prt */
  if ((!bool_from_obj(r[1]))) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 192: /* var-unboxed-ctype-in-exp k id exp */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k id exp */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(193);
    r[3] = (hendblk(1+1));
    r[4+0] = (cx_var_2Duses_2Din_2Dexp);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[1];  
    r[4+3] = r[2];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 193: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    r[0] = r[2];  
    /* r[1] */    
    r[2] = obj_from_bool(0);
    goto s_loop_v3820;

case 194: /* clo k vu* ctype */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
s_loop_v3820: /* k vu* ctype */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(194);
    r[3] = (hendblk(0+1));
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(195);
    r[3] = (hendblk(4+1));
    r[0] = r[3];  
    r[1] = (car((r[1])));
    goto gs_var_2Duse_2Dunboxed_2Dctype;
  }

case 195: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop vu* k ctype */
  if (bool_from_obj(r[1])) {
  if (((!bool_from_obj(r[5])) || (strcmp(stringchars((r[1])), stringchars((r[5]))) == 0))) {
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[4];  
    r[6+2] = (cdr((r[3])));
    r[6+3] = r[1];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 196: /* clo ek r */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* ek r */
    cx_exp_2D_3Efree_2Dvars = r[1];  
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(197);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(204);
    r[3] = (hendblk(0+1));
    r[0] = r[2];  
    r[1] = r[3];  
    goto gs_memoizate_2Dexp_2Dattribute;

case 197: /* clo ek r */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* ek r */
    cx_exp_2D_3Efree_2Dlexvars = r[1];  
    { static obj c[] = { obj_from_case(198) }; cx_exp_2Dctype = (obj)c; }
    r[2] = obj_from_void(0);
    r[3+0] = r[0];
    pc = 0; /* exit from module init */
    r[3+1] = r[2];  
    r += 3; /* shift reg wnd */
    assert(rc = 2);
    goto jump;

case 198: /* exp-ctype k exp env */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_exp_2Dctype: /* k exp env */
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (2))) {
    r[3] = (vectorref((r[1]), (0)));
    r[3] = obj_from_bool((r[3]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    r[3] = (vectorref((r[1]), (1)));
    { /* assq */
    obj x = (r[3]), l = (r[2]), p = mknull();
    for (; l != mknull(); l = cdr(l)) { p = car(l); if (car(p) == x) break; }
    r[4] = (l == mknull() ? obj_from_bool(0) : p); }
    r[3] = (bool_from_obj(r[4]) ? (cdr((r[4]))) : obj_from_bool(0));
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[3] = (vectorref((r[1]), (0)));
    r[3] = obj_from_bool((r[3]) == (mksymbol(internsym("if-exp"))));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    r[3] = (vectorref((r[1]), (2)));
    r[4] = (vectorref((r[1]), (3)));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(199);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[4];  
    /* r[2] */    
    goto gs_exp_2Dctype;
  } else {
  if (bool_from_obj(cxs_let_2Dexp_3F_231827((r[1])))) {
    r[3] = (vectorref((r[1]), (1)));
    r[3] = (vectorref((r[3]), (1)));
    r[4] = (vectorref((r[1]), (2)));
    r[5] = (vectorref((r[1]), (1)));
    r[5] = (vectorref((r[5]), (2)));
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = obj_from_case(203);
    r[6] = (hendblk(3+1));
    r[7+0] = r[6];  
    r[7+1] = r[3];  
    r[7+2] = r[4];  
    r[7+3] = r[2];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v3771;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[3] = (vectorref((r[1]), (0)));
    r[3] = obj_from_bool((r[3]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    r[3] = (vectorref((r[1]), (2)));
    r[4+0] = (cx_prim_2Dctype);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[0];  
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
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
  }
  }
  }

case 199: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r env then-exp k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(200);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = r[3];  
    /* r[2] */    
    goto gs_exp_2Dctype;

case 200: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = (bool_from_obj(r[1]) ? (bool_from_obj(r[3]) ? ((strcmp(stringchars((r[1])), stringchars((r[3]))) == 0) ? (r[1]) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v3771: /* k id id env */
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
    *--hp = obj_from_case(201);
    r[6] = (hendblk(4+1));
    r[0] = r[6];  
    r[1] = r[4];  
    r[2] = r[5];  
    /* r[3] */    
    goto s_loop_v3771;
  }

case 201: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r env k id id */
    r[6] = (car((r[5])));
    r[7] = (car((r[4])));
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(202);
    r[8] = (hendblk(3+1));
    r[0] = r[8];  
    r[1] = r[7];  
    /* r[2] */    
    goto gs_exp_2Dctype;

case 202: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r id */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
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

case 203: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r body k env */
    { fixnum_t v5176_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v5176_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v5176_tmp);
    hreserve(hbsz(3)*c, 5); /* 5 live regs */
    l = r[1];   t = r[4];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[5] = (o); } }
    r[0] = r[3];  
    r[1] = r[2];  
    r[2] = r[5];  
    goto gs_exp_2Dctype;

case 204: /* clo k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k exp */
    hreserve(hbsz(1+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(205);
    r[2] = (hendblk(1+1));
    r[3+0] = (cx_exp_2Dvinfo);
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[2];  
    r[3+2] = r[1];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 205: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(206);
    r[3] = (hendblk(0+1));
    r[4+0] = r[2];  
    r[4+1] = r[3];  
    r[4+2] = (mknull());
    r[4+3] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_reduce_2Dleft;

case 206: /* clo k id&vu res */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k id&vu res */
    r[3] = (car((r[1])));
    r[4] = (cdr((r[3])));
    r[4] = (cdr((r[4])));
    r[4] = (car((r[4])));
    r[3] = obj_from_bool(fixnum_from_obj(r[4]) < (0));
  if (bool_from_obj(r[3])) {
    r[3] = r[2];  
  } else {
    r[3] = (car((r[1])));
    { /* memq */
    obj x = (r[3]), l = r[2];  
    for (; l != mknull(); l = cdr(l)) if (car(l) == x) break;
    r[4] = (l == mknull() ? obj_from_bool(0) : l); }
  if (bool_from_obj(r[4])) {
    r[3] = r[2];  
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
  }
  }
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 207: /* clo k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k exp */
    hreserve(hbsz(1+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(208);
    r[2] = (hendblk(1+1));
    r[3+0] = (cx_exp_2Dvinfo);
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[2];  
    r[3+2] = r[1];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 208: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(209);
    r[3] = (hendblk(0+1));
    r[4+0] = r[2];  
    r[4+1] = r[3];  
    r[4+2] = (mknull());
    r[4+3] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_reduce_2Dleft;

case 209: /* clo k id&vu res */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k id&vu res */
    r[3] = (car((r[1])));
    { /* memq */
    obj x = (r[3]), l = r[2];  
    for (; l != mknull(); l = cdr(l)) if (car(l) == x) break;
    r[4] = (l == mknull() ? obj_from_bool(0) : l); }
  if (bool_from_obj(r[4])) {
    r[3] = r[2];  
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
  }
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 210: /* clo k id exp */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k id exp */
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(211);
    r[3] = (hendblk(2+1));
    r[4+0] = (cx_exp_2Dvinfo);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[2];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 211: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k */
    r[4+0] = r[3];  
    r[4+1] = r[1];  
    r[4+2] = (mknull());
    r[4+3] = r[2];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v3700;

s_loop_v3700: /* k vinfo uses id */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[4] = (car((r[1])));
    r[4] = (car((r[4])));
    r[4] = obj_from_bool((r[3]) == (r[4]));
  if (bool_from_obj(r[4])) {
    r[4] = (cdr((r[1])));
    r[5] = (car((r[1])));
    r[5] = (cdr((r[5])));
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[2];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    /* r[0] */    
    r[1] = r[4];  
    r[2] = r[5];  
    /* r[3] */    
    goto s_loop_v3700;
  } else {
    r[4] = (cdr((r[1])));
    /* r[0] */    
    r[1] = r[4];  
    /* r[2] */    
    /* r[3] */    
    goto s_loop_v3700;
  }
  }

case 212: /* clo k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k exp */
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (2))) {
    r[2] = (vectorref((r[1]), (0)));
    r[2] = obj_from_bool((r[2]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[2] = obj_from_bool(0);
  }
  } else {
    r[2] = obj_from_bool(0);
  }
  if (bool_from_obj(r[2])) {
    r[2] = (vectorref((r[1]), (1)));
    { /* vector */
    hreserve(hbsz(4+1), 3); /* 3 live regs */
    *--hp = obj_from_bool(0);
    *--hp = obj_from_bool(0);
    *--hp = obj_from_bool(0);
    *--hp = (mksymbol(internsym("var-use")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[3] = (hendblk(4+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
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
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[2] = (vectorref((r[1]), (0)));
    r[2] = obj_from_bool((r[2]) == (mksymbol(internsym("varassign-exp"))));
  } else {
    r[2] = obj_from_bool(0);
  }
  } else {
    r[2] = obj_from_bool(0);
  }
  if (bool_from_obj(r[2])) {
    r[2] = (vectorref((r[1]), (1)));
    r[3] = (vectorref((r[1]), (2)));
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = obj_from_case(213);
    r[4] = (hendblk(2+1));
    r[5+0] = (cx_exp_2Dvinfo);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = r[3];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[2] = (vectorref((r[1]), (0)));
    r[2] = obj_from_bool((r[2]) == (mksymbol(internsym("if-exp"))));
  } else {
    r[2] = obj_from_bool(0);
  }
  } else {
    r[2] = obj_from_bool(0);
  }
  if (bool_from_obj(r[2])) {
    r[2] = (vectorref((r[1]), (1)));
    r[3] = (vectorref((r[1]), (2)));
    r[4] = (vectorref((r[1]), (3)));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_case(214);
    r[5] = (hendblk(3+1));
    r[6+0] = (cx_exp_2Dvinfo);
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[5];  
    r[6+2] = r[4];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
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
    r[2] = (vectorref((r[1]), (2)));
    r[3] = (vectorref((r[1]), (3)));
    r[4+0] = r[0];  
    r[4+1] = r[3];  
    r[4+2] = obj_from_fixnum(0);
    r[4+3] = (mknull());
    r[4+4] = r[2];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v3645;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[2] = (vectorref((r[1]), (0)));
    r[2] = obj_from_bool((r[2]) == (mksymbol(internsym("app-exp"))));
  } else {
    r[2] = obj_from_bool(0);
  }
  } else {
    r[2] = obj_from_bool(0);
  }
  if (bool_from_obj(r[2])) {
    r[2] = (vectorref((r[1]), (1)));
    r[3] = (vectorref((r[1]), (2)));
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[3];  
    *--hp = obj_from_case(220);
    r[4] = (hendblk(2+1));
  if ((isvector((r[2])))) {
  if (((vectorlen((r[2]))) == (2))) {
    r[5] = (vectorref((r[2]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[2]), (1)));
    { /* vector */
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = obj_from_bool(0);
    *--hp = obj_from_bool(1);
    *--hp = obj_from_bool(0);
    *--hp = (mksymbol(internsym("var-use")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[6] = (hendblk(4+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[6+0] = obj_from_ktrap();
    r[6+1] = r[5];  
    r[6+2] = r[3];  
    r[6+3] = r[0];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v3626;
  } else {
    r[0] = (cx_exp_2Dvinfo);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
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
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(223);
    r[4] = (hendblk(2+1));
    r[5+0] = (cx_exp_2Dvinfo);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = r[3];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[2] = (vectorref((r[1]), (0)));
    r[2] = obj_from_bool((r[2]) == (mksymbol(internsym("fix-exp"))));
  } else {
    r[2] = obj_from_bool(0);
  }
  } else {
    r[2] = obj_from_bool(0);
  }
  if (bool_from_obj(r[2])) {
    r[2] = (vectorref((r[1]), (1)));
    r[3] = (vectorref((r[1]), (2)));
    r[4] = (vectorref((r[1]), (3)));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_case(225);
    r[5] = (hendblk(3+1));
    r[6+0] = (cx_exp_2Dvinfo);
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[5];  
    r[6+2] = r[4];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[2] = (vectorref((r[1]), (0)));
    r[2] = obj_from_bool((r[2]) == (mksymbol(internsym("letcc-exp"))));
  } else {
    r[2] = obj_from_bool(0);
  }
  } else {
    r[2] = obj_from_bool(0);
  }
  if (bool_from_obj(r[2])) {
    r[2] = (vectorref((r[1]), (1)));
    r[3] = (vectorref((r[1]), (2)));
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(230);
    r[4] = (hendblk(2+1));
    r[5+0] = (cx_exp_2Dvinfo);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = r[3];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[2] = (vectorref((r[1]), (0)));
    r[2] = obj_from_bool((r[2]) == (mksymbol(internsym("withcc-exp"))));
  } else {
    r[2] = obj_from_bool(0);
  }
  } else {
    r[2] = obj_from_bool(0);
  }
  if (bool_from_obj(r[2])) {
    r[2] = (vectorref((r[1]), (1)));
    r[3] = (vectorref((r[1]), (2)));
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(232);
    r[4] = (hendblk(2+1));
    r[5+0] = (cx_exp_2Dvinfo);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = r[3];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cx__2Acurrent_2Derror_2Dport_2A);
    (void)(fputc('\n', oportdata((r[2]))));
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(234);
    r[3] = (hendblk(3+1));
  if (bool_from_obj(mksymbol(internsym("variant-case")))) {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("variant-case")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[3];  
    r[5+2] = r[2];  
    r[5+3] = (cx__231369);
    r[5+4] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[4+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (cx__231365);
    r[4+3] = obj_from_bool(1);
    r[4+4] = r[2];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }
  }
  }
  }
  }
  }
  }
  }
  }
  }

case 213: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    { /* vector */
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = obj_from_bool(0);
    *--hp = obj_from_bool(0);
    *--hp = obj_from_bool(1);
    *--hp = (mksymbol(internsym("var-use")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(4+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
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

case 214: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r then-exp test-exp k */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(215);
    r[5] = (hendblk(3+1));
    r[0] = (cx_exp_2Dvinfo);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 215: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r test-exp k r */
    { fixnum_t v5175_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v5175_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v5175_tmp);
    hreserve(hbsz(3)*c, 5); /* 5 live regs */
    l = r[1];   t = r[4];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[5] = (o); } }
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = obj_from_case(216);
    r[6] = (hendblk(2+1));
    r[0] = (cx_exp_2Dvinfo);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    /* r[2] */    
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
    { fixnum_t v5174_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v5174_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v5174_tmp);
    hreserve(hbsz(3)*c, 4); /* 4 live regs */
    l = r[1];   t = r[3];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[4] = (o); } }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 217: /* clo k rands n vi */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v3645: /* k rands n vi prim */
    hreserve(hbsz(1+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = obj_from_case(217);
    r[5] = (hendblk(1+1));
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[6] = (car((r[1])));
  if ((isvector((r[6])))) {
  if (((vectorlen((r[6]))) == (2))) {
    r[7] = (vectorref((r[6]), (0)));
    r[7] = obj_from_bool((r[7]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[7] = obj_from_bool(0);
  }
  } else {
    r[7] = obj_from_bool(0);
  }
  if (bool_from_obj(r[7])) {
    r[7] = (vectorref((r[6]), (1)));
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_case(218);
    r[8] = (hendblk(6+1));
    r[9+0] = (cx_c_2Dargref_2Dctype);
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[8];  
    r[9+2] = r[4];  
    r[9+3] = r[2];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[5];  
    *--hp = obj_from_case(219);
    r[7] = (hendblk(5+1));
    r[8+0] = (cx_exp_2Dvinfo);
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[7];  
    r[8+2] = r[6];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }

case 218: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop vi n rands k id */
    { /* vector */
    hreserve(hbsz(4+1), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = obj_from_bool(0);
    *--hp = obj_from_bool(0);
    *--hp = (mksymbol(internsym("var-use")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[8] = (hendblk(4+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[3];  
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[11+0] = r[2];  
    pc = objptr_from_obj(r[11+0])[0];
    r[11+1] = r[6];  
    r[11+2] = (cdr((r[5])));
    r[11+3] = obj_from_fixnum(fixnum_from_obj(r[4]) + (1));
    r[11+4] = (r[10]);
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 219: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop n rands k vi */
    { fixnum_t v5173_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v5173_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v5173_tmp);
    hreserve(hbsz(3)*c, 7); /* 7 live regs */
    l = r[1];   t = r[6];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[7] = (o); } }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (cdr((r[4])));
    r[3] = obj_from_fixnum(fixnum_from_obj(r[3]) + (1));
    r[4] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 220: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
s_l_v3626: /* ek r rands k */
    hreserve(hbsz(0+1), 4); /* 4 live regs */
    *--hp = obj_from_case(221);
    r[4] = (hendblk(0+1));
    r[5+0] = r[3];  
    r[5+1] = r[4];  
    r[5+2] = r[1];  
    r[5+3] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_reduce_2Dright;

case 221: /* clo k exp vi */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k exp vi */
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = obj_from_case(222);
    r[3] = (hendblk(2+1));
    r[4+0] = (cx_exp_2Dvinfo);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 222: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k vi */
    { fixnum_t v5172_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v5172_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v5172_tmp);
    hreserve(hbsz(3)*c, 4); /* 4 live regs */
    l = r[1];   t = r[3];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[4] = (o); } }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 223: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r ids k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(224);
    r[4] = (hendblk(1+1));
    r[5+0] = r[3];  
    r[5+1] = r[4];  
    r[5+2] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_keep_231406;

case 224: /* clo k id&vu */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k id&vu ids */
    r[3] = (car((r[1])));
    { /* memq */
    obj x = (r[3]), l = r[2];  
    for (; l != mknull(); l = cdr(l)) if (car(l) == x) break;
    r[3] = (l == mknull() ? obj_from_bool(0) : l); }
    r[3] = obj_from_bool(!bool_from_obj(r[3]));
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 225: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r lams ids k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(226);
    r[5] = (hendblk(2+1));
    hreserve(hbsz(0+1), 6); /* 6 live regs */
    *--hp = obj_from_case(228);
    r[6] = (hendblk(0+1));
    r[7+0] = r[5];  
    r[7+1] = r[6];  
    r[7+2] = r[1];  
    r[7+3] = r[2];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_reduce_2Dright;

case 226: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r ids k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(227);
    r[4] = (hendblk(1+1));
    r[5+0] = r[3];  
    r[5+1] = r[4];  
    r[5+2] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_keep_231406;

case 227: /* clo k id&vu */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k id&vu ids */
    r[3] = (car((r[1])));
    { /* memq */
    obj x = (r[3]), l = r[2];  
    for (; l != mknull(); l = cdr(l)) if (car(l) == x) break;
    r[3] = (l == mknull() ? obj_from_bool(0) : l); }
    r[3] = obj_from_bool(!bool_from_obj(r[3]));
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 228: /* clo k exp vi */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k exp vi */
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = obj_from_case(229);
    r[3] = (hendblk(2+1));
    r[4+0] = (cx_exp_2Dvinfo);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 229: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k vi */
    { fixnum_t v5171_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v5171_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v5171_tmp);
    hreserve(hbsz(3)*c, 4); /* 4 live regs */
    l = r[1];   t = r[3];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[4] = (o); } }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 230: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(231);
    r[4] = (hendblk(1+1));
    r[5+0] = r[3];  
    r[5+1] = r[4];  
    r[5+2] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_keep_231406;

case 231: /* clo k id&vu */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k id&vu id */
    { bool_t v5170_tmp;
    r[3] = (car((r[1])));
    v5170_tmp = ((r[3]) == (r[2]));
    r[3] = obj_from_bool(!(v5170_tmp)); }
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 232: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r cont-exp k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(233);
    r[4] = (hendblk(2+1));
    r[0] = (cx_exp_2Dvinfo);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 233: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { fixnum_t v5169_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v5169_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v5169_tmp);
    hreserve(hbsz(3)*c, 4); /* 4 live regs */
    l = r[1];   t = r[3];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[4] = (o); } }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 234: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  exp ep k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(235);
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
    r[7+3] = (cx__231363);
    r[7+4] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 235: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  ep k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(236);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cx__231360);
    r[5+3] = obj_from_bool(1);
    r[5+4] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 236: /* clo ek  */
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
