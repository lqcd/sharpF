/* 2.sf */
#define MODULE module_2
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
extern obj cx__2Acurrent_2Doutput_2Dport_2A; /* *current-output-port* */
extern obj cx_assoc; /* assoc */
extern obj cx_equal_3F; /* equal? */
extern obj cx_fixnum_2D_3Estring; /* fixnum->string */
extern obj cx_flonum_2D_3Estring; /* flonum->string */
extern obj cx_fxexpt; /* fxexpt */
extern obj cx_list_3F; /* list? */
extern obj cx_member; /* member */
extern obj cx_read_2F1; /* read/1 */
extern obj cx_reset; /* reset */
extern obj cx_string_2D_3Efixnum; /* string->fixnum */
extern obj cx_string_2D_3Eflonum; /* string->flonum */
extern obj cx_wrap_2Dvecs; /* wrap-vecs */
extern obj cx_write_2F3; /* write/3 */
obj cx_acons; /* acons */
obj cx_append_2A; /* append* */
obj cx_apply_2Dexpander; /* apply-expander */
obj cx_apply_2Dmap_2Dlist; /* apply-map-list */
obj cx_apply_2Dsynrules; /* apply-synrules */
obj cx_builtin_2Dname; /* builtin-name */
obj cx_builtin_3F; /* builtin? */
obj cx_builtins_2Dstore; /* builtins-store */
obj cx_char_2Dsafe_3F; /* char-safe? */
obj cx_check_2Dsyntax_2Dbindings; /* check-syntax-bindings */
obj cx_code_2Doutput; /* code-output */
obj cx_compile_2Dsyntax_2Dlambda; /* compile-syntax-lambda */
obj cx_compile_2Dsyntax_2Drules; /* compile-syntax-rules */
obj cx_decompose_2Dliteral; /* decompose-literal */
obj cx_empty_2Denv; /* empty-env */
obj cx_empty_2Dstore; /* empty-store */
obj cx_expand_2Dany; /* expand-any */
obj cx_expand_2Dbody; /* expand-body */
obj cx_expand_2Dexpr; /* expand-expr */
obj cx_expand_2Dlambda; /* expand-lambda */
obj cx_expand_2Dletcc; /* expand-letcc */
obj cx_expand_2Dsyntax_2Dbindings; /* expand-syntax-bindings */
obj cx_expand_2Dtop_2Dlevel_2Dforms; /* expand-top-level-forms */
obj cx_expand_2Dtop_2Dlevel_2Dforms_21; /* expand-top-level-forms! */
obj cx_expand_2Dval; /* expand-val */
obj cx_expanded_2Dcode_3F; /* expanded-code? */
obj cx_expander_2Denv; /* expander-env */
obj cx_expander_2Dform; /* expander-form */
obj cx_expander_3F; /* expander? */
obj cx_extend_2Denv; /* extend-env */
obj cx_extend_2Dstore; /* extend-store */
obj cx_file_2Dexpand_2Dtop_2Dlevel_2Dforms_21; /* file-expand-top-level-forms! */
obj cx_fold_2Dmax; /* fold-max */
obj cx_integer_2Drepresentation_2Dwidth; /* integer-representation-width */
obj cx_intloc_2D_3Evar; /* intloc->var */
obj cx_list_2D_3Esvector; /* list->svector */
obj cx_list1_3F; /* list1? */
obj cx_list2_3F; /* list2? */
obj cx_loc_2D_3Evar; /* loc->var */
obj cx_lookup_2Dlocation; /* lookup-location */
obj cx_lookup_2Dsid; /* lookup-sid */
obj cx_lookup2; /* lookup2 */
obj cx_make_2Dbegin; /* make-begin */
obj cx_make_2Dbuiltin; /* make-builtin */
obj cx_make_2Dcode; /* make-code */
obj cx_make_2Dexpander; /* make-expander */
obj cx_make_2Dsid; /* make-sid */
obj cx_map_2Dvecs; /* map-vecs */
obj cx_null_2Dloc_2Dn; /* null-loc-n */
obj cx_null_2Dmstore; /* null-mstore */
obj cx_null_2Doutput; /* null-output */
obj cx_null_2Dprog; /* null-prog */
obj cx_null_2Dstore; /* null-store */
obj cx_null_2Dstuff; /* null-stuff */
obj cx_pairwise_2Dandmap; /* pairwise-andmap */
obj cx_pattern_2Dsbox_2D_3Esexp; /* pattern-sbox->sexp */
obj cx_pattern_2Dsbox_2D_3Etest; /* pattern-sbox->test */
obj cx_pattern_2Dsbox_3F; /* pattern-sbox? */
obj cx_renamed_2Dsid_3F; /* renamed-sid? */
obj cx_sbox_2D_3Esexp_2Dlist; /* sbox->sexp-list */
obj cx_sid_2Did; /* sid-id */
obj cx_sid_2Dlocation; /* sid-location */
obj cx_sid_2Dname; /* sid-name */
obj cx_sid_3F; /* sid? */
obj cx_substitute_2Din_2Dstore; /* substitute-in-store */
obj cx_svector_2D_3Elist; /* svector->list */
obj cx_svector_3F; /* svector? */
obj cx_symloc_2D_3Evar; /* symloc->var */
obj cx_syntax_3F; /* syntax? */
obj cx_template_2Dsbox_2D_3Econv; /* template-sbox->conv */
obj cx_template_2Dsbox_2D_3Esexp; /* template-sbox->sexp */
obj cx_template_2Dsbox_3F; /* template-sbox? */
obj cx_unsigned_2Drepresentation_2Dwidth; /* unsigned-representation-width */
obj cx_unwrap_2Dvec; /* unwrap-vec */
obj cx_unwrap_2Dvecs; /* unwrap-vecs */
obj cx_variable_3F; /* variable? */
obj cx_wrap_2Dvec; /* wrap-vec */
obj cx_wrap_2Dvecs; /* wrap-vecs */
obj cx_x_2Derror_2A; /* x-error* */
static obj cx__231042; /* constant #1042 */
static obj cx__231097; /* constant #1097 */
static obj cx__231104; /* constant #1104 */
static obj cx__231191; /* constant #1191 */
static obj cx__231196; /* constant #1196 */
static obj cx__231403; /* constant #1403 */
static obj cx__231452; /* constant #1452 */
static obj cx__23146; /* constant #146 */
static obj cx__231481; /* constant #1481 */
static obj cx__231494; /* constant #1494 */
static obj cx__231507; /* constant #1507 */
static obj cx__231522; /* constant #1522 */
static obj cx__231524; /* constant #1524 */
static obj cx__231526; /* constant #1526 */
static obj cx__231533; /* constant #1533 */
static obj cx__231536; /* constant #1536 */
static obj cx__231541; /* constant #1541 */
static obj cx__231542; /* constant #1542 */
static obj cx__231544; /* constant #1544 */
static obj cx__231563; /* constant #1563 */
static obj cx__231692; /* constant #1692 */
static obj cx__231712; /* constant #1712 */
static obj cx__231755; /* constant #1755 */
static obj cx__2318; /* constant #18 */
static obj cx__23187; /* constant #187 */
static obj cx__23188; /* constant #188 */
static obj cx__231988; /* constant #1988 */
static obj cx__232029; /* constant #2029 */
static obj cx__232176; /* constant #2176 */
static obj cx__232196; /* constant #2196 */
static obj cx__232212; /* constant #2212 */
static obj cx__232238; /* constant #2238 */
static obj cx__232264; /* constant #2264 */
static obj cx__232271; /* constant #2271 */
static obj cx__232289; /* constant #2289 */
static obj cx__232304; /* constant #2304 */
static obj cx__232322; /* constant #2322 */
static obj cx__232344; /* constant #2344 */
static obj cx__232349; /* constant #2349 */
static obj cx__232357; /* constant #2357 */
static obj cx__232411; /* constant #2411 */
static obj cx__232479; /* constant #2479 */
static obj cx__232486; /* constant #2486 */
static obj cx__232495; /* constant #2495 */
static obj cx__232500; /* constant #2500 */
static obj cx__232508; /* constant #2508 */
static obj cx__232561; /* constant #2561 */
static obj cx__232566; /* constant #2566 */
static obj cx__232600; /* constant #2600 */
static obj cx__232604; /* constant #2604 */
static obj cx__232606; /* constant #2606 */
static obj cx__232615; /* constant #2615 */
static obj cx__232622; /* constant #2622 */
static obj cx__232625; /* constant #2625 */
static obj cx__232627; /* constant #2627 */
static obj cx__232648; /* constant #2648 */
static obj cx__232653; /* constant #2653 */
static obj cx__232661; /* constant #2661 */
static obj cx__232670; /* constant #2670 */
static obj cx__232671; /* constant #2671 */
static obj cx__232817; /* constant #2817 */
static obj cx__232822; /* constant #2822 */
static obj cx__233084; /* constant #3084 */
static obj cx__233108; /* constant #3108 */
static obj cx__23471; /* constant #471 */
static obj cx__23565; /* constant #565 */
static obj cx__23566; /* constant #566 */
static obj cx__23608; /* constant #608 */
static obj cx__23847; /* constant #847 */
static obj cx__23852; /* constant #852 */
static obj cx__23862; /* constant #862 */
static obj cx__23917; /* constant #917 */
static obj cx__23943; /* constant #943 */
static obj cx__23949; /* constant #949 */
static obj cx__23964; /* constant #964 */
static obj cx__23973; /* constant #973 */

/* helper functions */
/* assv#530 */
static obj cxs_assv_23530(obj v533_x, obj v532_l)
{ 
  s_assv_23530:
  if ((isnull((v532_l)))) {
    return obj_from_bool(0);
  } else {
  { /* let */
    obj v535_p = (car((v532_l)));
  { /* let */
    obj v3532_tmp;
  { /* let */
    obj v538_y = (car((v535_p)));
    v3532_tmp = (((v533_x) == (v538_y)) ? obj_from_bool((v533_x) == (v538_y)) : ((is_flonum_obj(v533_x)) ? ((is_flonum_obj(v538_y)) ? obj_from_bool(flonum_from_obj(v533_x) == flonum_from_obj(v538_y)) : obj_from_bool(0)) : obj_from_bool(0)));
  }
  if (bool_from_obj(v3532_tmp)) {
    return (v535_p);
  } else {
  { /* let */
    obj v3534_tmp = (cdr((v532_l)));
    obj v3533_tmp = (v533_x);
    /* tail call */
    v533_x = (v3533_tmp);
    v532_l = (v3534_tmp);
    goto s_assv_23530;
  }
  }
  }
  }
  }
}

/* memv#2567 */
static obj cxs_memv_232567(obj v2570_x, obj v2569_l)
{ 
  { /* letrec */
    obj v2572_l;
    /* tail call */
    v2572_l = (v2569_l);
    goto s_memv;
  s_memv:
  if ((isnull((v2572_l)))) {
    return obj_from_bool(0);
  } else {
  { /* let */
    obj v3530_tmp;
  { /* let */
    obj v2575_y = (car((v2572_l)));
    v3530_tmp = (((v2570_x) == (v2575_y)) ? obj_from_bool((v2570_x) == (v2575_y)) : ((is_flonum_obj(v2570_x)) ? ((is_flonum_obj(v2575_y)) ? obj_from_bool(flonum_from_obj(v2570_x) == flonum_from_obj(v2575_y)) : obj_from_bool(0)) : obj_from_bool(0)));
  }
  if (bool_from_obj(v3530_tmp)) {
    return (v2572_l);
  } else {
  { /* let */
    obj v3531_tmp = (cdr((v2572_l)));
    /* tail call */
    v2572_l = (v3531_tmp);
    goto s_memv;
  }
  }
  }
  }
  }
}

/* char-safe? */
static obj cxs_char_2Dsafe_3F(obj v119_c)
{ 
    return ((char_from_obj(v119_c) <= (126)) ? ((char_from_obj(v119_c) >= (32)) ? ((!(char_from_obj(v119_c) == (92))) ? ((!(char_from_obj(v119_c) == (34))) ? obj_from_bool(!(char_from_obj(v119_c) == (36))) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0));
}

/* fold-max */
static obj cxs_fold_2Dmax(obj v135_base, obj v134_lst)
{ 
  s_fold_2Dmax:
  if ((isnull((v134_lst)))) {
    return (v135_base);
  } else {
  { /* let */
    obj v137_next = (car((v134_lst)));
  { /* let */
    obj v3529_tmp = (cdr((v134_lst)));
    obj v3528_tmp = (bool_from_obj(v135_base) ? (bool_from_obj(v137_next) ? ((fixnum_from_obj(v135_base) > fixnum_from_obj(v137_next)) ? (v135_base) : (v137_next)) : obj_from_bool(0)) : obj_from_bool(0));
    /* tail call */
    v135_base = (v3528_tmp);
    v134_lst = (v3529_tmp);
    goto s_fold_2Dmax;
  }
  }
  }
}

/* renamed-sid? */
static obj cxs_renamed_2Dsid_3F(obj v381_sexp)
{ 
    return ((isvector((v381_sexp))) ? obj_from_bool((1) < (vectorlen((v381_sexp)))) : obj_from_bool(0));
}

/* svector? */
static obj cxs_svector_3F(obj v384_sexp)
{ 
    return ((isvector((v384_sexp))) ? obj_from_bool((1) == (vectorlen((v384_sexp)))) : obj_from_bool(0));
}

/* sid-name */
static obj cxs_sid_2Dname(obj v400_sid)
{ 
    return ((issymbol((v400_sid))) ? (v400_sid) : (vectorref((v400_sid), (0))));
}

/* sid-id */
static obj cxs_sid_2Did(obj v405_sid)
{ 
    return ((issymbol((v405_sid))) ? (v405_sid) : (vectorref((v405_sid), (1))));
}

/* sid-location */
static obj cxs_sid_2Dlocation(obj v410_sid)
{ 
    return ((issymbol((v410_sid))) ? (v410_sid) : (vectorref((v410_sid), fixnum_from_obj(((2) == (vectorlen((v410_sid)))) ? obj_from_fixnum(0) : obj_from_fixnum(2)))));
}

/* list1? */
static obj cxs_list1_3F(obj v416_x)
{ 
    return ((ispair((v416_x))) ? obj_from_bool(isnull((cdr((v416_x))))) : obj_from_bool(0));
}

/* list2? */
static obj cxs_list2_3F(obj v421_x)
{ 
    return ((ispair((v421_x))) ? (cxs_list1_3F((cdr((v421_x))))) : obj_from_bool(0));
}

/* lookup-sid */
static obj cxs_lookup_2Dsid(obj v529_sid, obj v528_env)
{ 
  { /* let */
    obj v549_tmp = (cxs_assv_23530((cxs_sid_2Did((v529_sid))), (v528_env)));
    return (bool_from_obj(v549_tmp) ? (cdr((v549_tmp))) : (cxs_sid_2Dlocation((v529_sid))));
  }
}

/* pattern-sbox? */
static obj cxs_pattern_2Dsbox_3F(obj v2028_b)
{ 
  if ((isbox((v2028_b)))) {
  if (bool_from_obj(cxs_list2_3F((boxref((v2028_b)))))) {
    { /* memq */
    obj x = (car((boxref((v2028_b))))), l = (cx__232029);
    for (; l != mknull(); l = cdr(l)) if (car(l) == x) break;
    return (l == mknull() ? obj_from_bool(0) : l); };
  } else {
    return obj_from_bool(0);
  }
  } else {
    return obj_from_bool(0);
  }
}

/* gc roots */
static obj *globv[] = {
  &cx_builtins_2Dstore,
  &cx_empty_2Denv,
  &cx_empty_2Dstore,
  &cx_null_2Dloc_2Dn,
  &cx_null_2Doutput,
  &cx_null_2Dprog,
  &cx_null_2Dstore,
  &cx_null_2Dstuff,
  &cx__231042,
  &cx__231097,
  &cx__231104,
  &cx__231191,
  &cx__231196,
  &cx__231403,
  &cx__231452,
  &cx__23146,
  &cx__231481,
  &cx__231494,
  &cx__231507,
  &cx__231522,
  &cx__231524,
  &cx__231526,
  &cx__231533,
  &cx__231536,
  &cx__231541,
  &cx__231542,
  &cx__231544,
  &cx__231563,
  &cx__231692,
  &cx__231712,
  &cx__231755,
  &cx__2318,
  &cx__23187,
  &cx__23188,
  &cx__231988,
  &cx__232029,
  &cx__232176,
  &cx__232196,
  &cx__232212,
  &cx__232238,
  &cx__232264,
  &cx__232271,
  &cx__232289,
  &cx__232304,
  &cx__232322,
  &cx__232344,
  &cx__232349,
  &cx__232357,
  &cx__232411,
  &cx__232479,
  &cx__232486,
  &cx__232495,
  &cx__232500,
  &cx__232508,
  &cx__232561,
  &cx__232566,
  &cx__232600,
  &cx__232604,
  &cx__232606,
  &cx__232615,
  &cx__232622,
  &cx__232625,
  &cx__232627,
  &cx__232648,
  &cx__232653,
  &cx__232661,
  &cx__232670,
  &cx__232671,
  &cx__232817,
  &cx__232822,
  &cx__233084,
  &cx__233108,
  &cx__23471,
  &cx__23565,
  &cx__23566,
  &cx__23608,
  &cx__23847,
  &cx__23852,
  &cx__23862,
  &cx__23917,
  &cx__23943,
  &cx__23949,
  &cx__23964,
  &cx__23973,
};

static cxroot_t root = {
  sizeof(globv)/sizeof(obj *), globv, NULL
};

/* entry points */
static obj host(obj);
static obj cases[469] = {
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
  (obj)host,  (obj)host,  (obj)host,  (obj)host,
};

/* host procedure */
#define MAX_LIVEREGS 38
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
    cx__2318 = (hpushstr(0, newstring("Syntax error: ")));
    cx__23146 = (hpushstr(0, newstring("unsupported literal: ")));
    cx__23187 = (hpushstr(0, newstring("0")));
    cx__23188 = (hpushstr(0, newstring("1")));
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("builtin")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23471 = (hendblk(3)); }
    cx__23565 = (hpushstr(0, newstring(" (or an internal define-syntax): ")));
    cx__23566 = (hpushstr(0, newstring("premature use of keyword bound by letrec-syntax")));
    cx__23608 = (hpushstr(0, newstring("_")));
    cx__23847 = (hpushstr(0, newstring(" in lambda formals: ")));
    cx__23852 = (hpushstr(0, newstring("duplicate variable: ")));
    cx__23862 = (hpushstr(0, newstring("non-identifier: ")));
    cx__23917 = (hpushstr(0, newstring(" used as letcc formal")));
    cx__23943 = (hpushstr(0, newstring(" and: ")));
    cx__23949 = (hpushstr(0, newstring("duplicate bindings for a keyword: ")));
    cx__23964 = (hpushstr(0, newstring("malformed syntax binding: ")));
    cx__23973 = (hpushstr(0, newstring("non-list syntax bindings list: ")));
    cx__231042 = (hpushstr(0, newstring("invalid expander: ")));
    cx__231097 = (hpushstr(0, newstring("incorrect number of arguments to syntax lambda: ")));
    cx__231104 = (hpushstr(0, newstring("syntax lambda applied in bad context: ")));
    cx__231191 = (hpushstr(0, newstring(" expression: ")));
    cx__231196 = (hpushstr(0, newstring("malformed ")));
    cx__231403 = (hpushstr(0, newstring("attempt to set a keyword: ")));
    cx__231452 = (hpushstr(0, newstring("malformed definition: ")));
    cx__231481 = (hpushstr(0, newstring("empty begin expression: ")));
    cx__231494 = (hpushstr(0, newstring("malformed syntax-lambda form: ")));
    cx__231507 = (hpushstr(0, newstring("empty syntax-rules form: ")));
    cx__231522 = (hpushstr(0, newstring("definition")));
    cx__231524 = (hpushstr(0, newstring("syntax")));
    cx__231526 = (hpushstr(0, newstring("expression")));
    cx__231533 = (hpushstr(0, newstring(" used in bad context: ")));
    cx__231536 = (hpushstr(0, newstring(" used as an expression, syntax, or definition.")));
    cx__231541 = (hpushstr(0, newstring("non-s-expression: ")));
    cx__231542 = (hpushstr(0, newstring("vector: ")));
    cx__231544 = (hpushstr(0, newstring("improper list: ")));
    cx__231563 = (hpushstr(0, newstring("null used as an expression or syntax: ")));
    cx__231692 = (hpushstr(0, newstring("duplicate internal definitions: ")));
    cx__231712 = (hpushstr(0, newstring("variable definition in a syntax body: ")));
    cx__231755 = (hpushstr(0, newstring("non-syntax definition in a syntax body: ")));
    cx__231988 = (hpushstr(0, newstring(" in syntax-lambda formals: ")));
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("id?")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("string?")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("number?")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__232029 = (hendblk(3)); }
    cx__232176 = (hpushstr(0, newstring("string->id: not a string: ")));
    cx__232196 = (hpushstr(0, newstring("id->string: not an id: ")));
    cx__232212 = (hpushstr(0, newstring("*: not a number: ")));
    cx__232238 = (hpushstr(0, newstring("+: not a number: ")));
    cx__232264 = (hpushstr(0, newstring("string-append: not a string: ")));
    cx__232271 = (hpushstr(0, newstring("")));
    cx__232289 = (hpushstr(0, newstring("length: not a list: ")));
    cx__232304 = (hpushstr(0, newstring("string->number: not a string: ")));
    cx__232322 = (hpushstr(0, newstring("number->string: not a number: ")));
    cx__232344 = (hpushstr(0, newstring(" in literals list of: ")));
    cx__232349 = (hpushstr(0, newstring("ellipsis ")));
    cx__232357 = (hpushstr(0, newstring("non-id: ")));
    cx__232411 = (hpushstr(0, newstring("pattern literals list is not a list: ")));
    cx__232479 = (hpushstr(0, newstring("malformed pattern: ")));
    cx__232486 = (hpushstr(0, newstring("malformed syntax rule: ")));
    cx__232495 = (hpushstr(0, newstring(" in template: ")));
    cx__232500 = (hpushstr(0, newstring("template ellipsis closes no variables: ")));
    cx__232508 = (hpushstr(0, newstring("pattern var used at bad depth: ")));
    cx__232561 = (hpushstr(0, newstring(" in pattern: ")));
    cx__232566 = (hpushstr(0, newstring("duplicate pattern var: ")));
    cx__232600 = (hpushstr(0, newstring("ellipsis following ")));
    cx__232604 = (hpushstr(0, newstring("improper list pattern with an ellipsis")));
    cx__232606 = (hpushstr(0, newstring("list or vector pattern with multiple ellipses")));
    cx__232615 = (hpushstr(0, newstring(": ")));
    cx__232622 = (hpushstr(0, newstring("malformed box")));
    cx__232625 = (hpushstr(0, newstring("a '#&(op'")));
    cx__232627 = (hpushstr(0, newstring("a '#('")));
    cx__232648 = (hpushstr(0, newstring("an ellipsis")));
    cx__232653 = (hpushstr(0, newstring("a '.'")));
    cx__232661 = (hpushstr(0, newstring("a '('")));
    cx__232670 = (hpushstr(0, newstring("the pattern keyword")));
    cx__232671 = (hpushstr(0, newstring("nothing")));
    cx__232817 = (hpushstr(0, newstring(" in macro call: ")));
    cx__232822 = (hpushstr(0, newstring("unequal sequence lengths for pattern vars: ")));
    cx__233084 = (hpushstr(0, newstring("no matching rule for macro use: ")));
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
    *--hp = (mksymbol(internsym("%prim*?!")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%prim*?")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%prim*!")));
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
    *--hp = (mksymbol(internsym("%prim!")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%prim?")));
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
    *--hp = (mksymbol(internsym("syntax-lambda")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("syntax-rules")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("set!")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("quote")));
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
    *--hp = (mksymbol(internsym("define-syntax")));
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
    cx__233108 = (hendblk(3)); }
    { static obj c[] = { obj_from_case(1) }; cx_x_2Derror_2A = (obj)c; }
    { static obj c[] = { obj_from_case(8) }; cx_append_2A = (obj)c; }
    { static obj c[] = { obj_from_case(10) }; cx_apply_2Dmap_2Dlist = (obj)c; }
    { static obj c[] = { obj_from_case(17) }; cx_pairwise_2Dandmap = (obj)c; }
    { static obj c[] = { obj_from_case(19) }; cx_integer_2Drepresentation_2Dwidth = (obj)c; }
    { static obj c[] = { obj_from_case(23) }; cx_unsigned_2Drepresentation_2Dwidth = (obj)c; }
    { static obj c[] = { obj_from_case(26) }; cx_char_2Dsafe_3F = (obj)c; }
    { static obj c[] = { obj_from_case(27) }; cx_fold_2Dmax = (obj)c; }
    { static obj c[] = { obj_from_case(28) }; cx_decompose_2Dliteral = (obj)c; }
    { static obj c[] = { obj_from_case(62) }; cx_sid_3F = (obj)c; }
    { static obj c[] = { obj_from_case(63) }; cx_renamed_2Dsid_3F = (obj)c; }
    { static obj c[] = { obj_from_case(64) }; cx_svector_3F = (obj)c; }
    { static obj c[] = { obj_from_case(65) }; cx_svector_2D_3Elist = (obj)c; }
    { static obj c[] = { obj_from_case(66) }; cx_list_2D_3Esvector = (obj)c; }
    { static obj c[] = { obj_from_case(67) }; cx_make_2Dsid = (obj)c; }
    { static obj c[] = { obj_from_case(68) }; cx_sid_2Dname = (obj)c; }
    { static obj c[] = { obj_from_case(69) }; cx_sid_2Did = (obj)c; }
    { static obj c[] = { obj_from_case(70) }; cx_sid_2Dlocation = (obj)c; }
    { static obj c[] = { obj_from_case(71) }; cx_list1_3F = (obj)c; }
    { static obj c[] = { obj_from_case(72) }; cx_list2_3F = (obj)c; }
    { static obj c[] = { obj_from_case(73) }; cx_map_2Dvecs = (obj)c; }
    { static obj c[] = { obj_from_case(80) }; cx_wrap_2Dvec = (obj)c; }
    { static obj c[] = { obj_from_case(82) }; cx_wrap_2Dvecs = (obj)c; }
    { static obj c[] = { obj_from_case(85) }; cx_unwrap_2Dvec = (obj)c; }
    { static obj c[] = { obj_from_case(87) }; cx_unwrap_2Dvecs = (obj)c; }
    { static obj c[] = { obj_from_case(88) }; cx_make_2Dcode = (obj)c; }
    { static obj c[] = { obj_from_case(89) }; cx_make_2Dbuiltin = (obj)c; }
    { static obj c[] = { obj_from_case(90) }; cx_variable_3F = (obj)c; }
    { static obj c[] = { obj_from_case(91) }; cx_expanded_2Dcode_3F = (obj)c; }
    { static obj c[] = { obj_from_case(92) }; cx_code_2Doutput = (obj)c; }
    { static obj c[] = { obj_from_case(93) }; cx_syntax_3F = (obj)c; }
    { static obj c[] = { obj_from_case(94) }; cx_builtin_3F = (obj)c; }
    { static obj c[] = { obj_from_case(95) }; cx_builtin_2Dname = (obj)c; }
    { static obj c[] = { obj_from_case(96) }; cx_expander_3F = (obj)c; }
    { static obj c[] = { obj_from_case(97) }; cx_make_2Dexpander = (obj)c; }
    { static obj c[] = { obj_from_case(98) }; cx_expander_2Dform = (obj)c; }
    { static obj c[] = { obj_from_case(99) }; cx_expander_2Denv = (obj)c; }
    { static obj c[] = { obj_from_case(100) }; cx_acons = (obj)c; }
    cx_empty_2Denv = (mknull());
    cx_empty_2Dstore = (mknull());
    { static obj c[] = { obj_from_case(101) }; cx_lookup_2Dsid = (obj)c; }
    { static obj c[] = { obj_from_case(102) }; cx_lookup_2Dlocation = (obj)c; }
    { static obj c[] = { obj_from_case(103) }; cx_lookup2 = (obj)c; }
    { static obj c[] = { obj_from_case(105) }; cx_extend_2Denv = (obj)c; }
    { static obj c[] = { obj_from_case(106) }; cx_extend_2Dstore = (obj)c; }
    { static obj c[] = { obj_from_case(107) }; cx_substitute_2Din_2Dstore = (obj)c; }
    { static obj c[] = { obj_from_case(112) }; cx_symloc_2D_3Evar = (obj)c; }
    { static obj c[] = { obj_from_case(113) }; cx_intloc_2D_3Evar = (obj)c; }
    { static obj c[] = { obj_from_case(115) }; cx_loc_2D_3Evar = (obj)c; }
    { static obj c[] = { obj_from_case(116) }; cx_make_2Dbegin = (obj)c; }
    { static obj c[] = { obj_from_case(117) }; cx_expand_2Dlambda = (obj)c; }
    { static obj c[] = { obj_from_case(135) }; cx_expand_2Dletcc = (obj)c; }
    { static obj c[] = { obj_from_case(139) }; cx_check_2Dsyntax_2Dbindings = (obj)c; }
    { static obj c[] = { obj_from_case(149) }; cx_expand_2Dsyntax_2Dbindings = (obj)c; }
    { static obj c[] = { obj_from_case(152) }; cx_apply_2Dexpander = (obj)c; }
    { static obj c[] = { obj_from_case(159) }; cx_expand_2Dany = (obj)c; }
    { static obj c[] = { obj_from_case(222) }; cx_expand_2Dval = (obj)c; }
    { static obj c[] = { obj_from_case(225) }; cx_expand_2Dexpr = (obj)c; }
    { static obj c[] = { obj_from_case(227) }; cx_expand_2Dbody = (obj)c; }
    { static obj c[] = { obj_from_case(266) }; cx_expand_2Dtop_2Dlevel_2Dforms = (obj)c; }
    { static obj c[] = { obj_from_case(280) }; cx_compile_2Dsyntax_2Dlambda = (obj)c; }
    { static obj c[] = { obj_from_case(288) }; cx_sbox_2D_3Esexp_2Dlist = (obj)c; }
    { static obj c[] = { obj_from_case(289) }; cx_pattern_2Dsbox_2D_3Esexp = (obj)c; }
    { static obj c[] = { obj_from_case(290) }; cx_pattern_2Dsbox_3F = (obj)c; }
    { static obj c[] = { obj_from_case(291) }; cx_pattern_2Dsbox_2D_3Etest = (obj)c; }
    { static obj c[] = { obj_from_case(295) }; cx_template_2Dsbox_2D_3Esexp = (obj)c; }
    { static obj c[] = { obj_from_case(296) }; cx_template_2Dsbox_3F = (obj)c; }
    { static obj c[] = { obj_from_case(297) }; cx_template_2Dsbox_2D_3Econv = (obj)c; }
    { static obj c[] = { obj_from_case(312) }; cx_compile_2Dsyntax_2Drules = (obj)c; }
    { static obj c[] = { obj_from_case(388) }; cx_apply_2Dsynrules = (obj)c; }
    hreserve(hbsz(0+1), 0); /* 0 live regs */
    *--hp = obj_from_case(459);
    r[0] = (hendblk(0+1));
    r[1+0] = r[0];  
    r[1+1] = (cx__233108);
    r[1+2] = (mknull());
    r += 1; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v5871;

case 1: /* x-error* k reason args */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_x_2Derror_2A: /* k reason args */
    (void)(fputc('\n', oportdata((cx__2Acurrent_2Doutput_2Dport_2A))));
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(2);
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

case 2: /* clo ek  */
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
    *--hp = obj_from_case(3);
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

case 3: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  args k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(7);
    r[4] = (hendblk(1+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto s_loop;

case 4: /* clo k id */
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
    *--hp = obj_from_case(4);
    r[3] = (hendblk(0+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(5);
    r[3] = (hendblk(3+1));
  if ((isstring((r[2])))) {
    r[4+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[2];  
    r[4+3] = obj_from_bool(1);
    r[4+4] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(6);
    r[4] = (hendblk(1+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto gs_unwrap_2Dvecs;
  }
  }

case 5: /* clo ek  */
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

case 6: /* clo ek r */
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

case 7: /* clo ek  */
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

case 8: /* append* k lst */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_append_2A: /* k lst */
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
    r[2] = obj_from_bool(isnull((r[2])));
  if (bool_from_obj(r[2])) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (car((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(2+1), 2); /* 2 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(9);
    r[2] = (hendblk(2+1));
    r[0] = r[2];  
    r[1] = (cdr((r[1])));
    goto gs_append_2A;
  }
  }

case 9: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k lst */
    r[4] = (car((r[3])));
    { fixnum_t v9261_tmp;
    { /* length */
    int n; obj l = r[4];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v9261_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v9261_tmp);
    hreserve(hbsz(3)*c, 5); /* 5 live regs */
    l = r[4];   t = r[1];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[5] = (o); } }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 10: /* apply-map-list k lst */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_apply_2Dmap_2Dlist: /* k lst */
    r[2+0] = r[0];  
    r[2+1] = r[1];  
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v9123;

case 11: /* clo k lst res */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
s_loop_v9123: /* k lst res */
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(11);
    r[3] = (hendblk(0+1));
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_case(12);
    r[3] = (hendblk(4+1));
    r[0] = r[3];  
    /* r[1] */    
    goto s_loop_v9164;

s_loop_v9164: /* k id */
  if ((!(isnull((r[1]))))) {
    r[2] = (car((r[1])));
    r[2] = obj_from_bool(isnull((r[2])));
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
    goto s_loop_v9164;
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

case 12: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r lst loop k res */
  if (bool_from_obj(r[1])) {
    { fixnum_t v9260_tmp;
    { /* length */
    int n; obj l = r[5];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v9260_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v9260_tmp);
    hreserve(hbsz(3)*c, 6); /* 6 live regs */
    l = r[5];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[6] = (o); } }
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(14);
    r[6] = (hendblk(4+1));
    r[0] = r[6];  
    r[1] = r[2];  
    goto s_loop_v9149;
  }

s_loop_v9149: /* k id */
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
    *--hp = obj_from_case(13);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v9149;
  }

case 13: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    r[4] = (cdr((r[4])));
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

case 14: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r lst loop k res */
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(16);
    r[6] = (hendblk(4+1));
    r[0] = r[6];  
    r[1] = r[2];  
    goto s_loop_v9134;

s_loop_v9134: /* k id */
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
    *--hp = obj_from_case(15);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v9134;
  }

case 15: /* clo ek r */
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

case 16: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop r k res */
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = r[3];  
    r[3] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 17: /* pairwise-andmap k pred? lst */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_pairwise_2Dandmap: /* k pred? lst */
  if ((isnull((r[2])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(isnull((r[2])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (car((r[2])));
    r[4] = (cdr((r[2])));
  if ((isnull((r[4])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(isnull((r[4])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(18);
    r[5] = (hendblk(3+1));
    r[6+0] = r[1];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[5];  
    r[6+2] = r[3];  
    r[6+3] = (car((r[4])));
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }
  }

case 18: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r r pred? k */
  if (bool_from_obj(r[1])) {
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto gs_pairwise_2Dandmap;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 19: /* integer-representation-width k x */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_integer_2Drepresentation_2Dwidth: /* k x */
    r[2+0] = r[0];  
    r[2+1] = obj_from_fixnum(8);
    r[2+2] = r[1];  
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v9086;

case 20: /* clo k rb */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v9086: /* k rb x */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(20);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[3];  
    *--hp = obj_from_case(21);
    r[3] = (hendblk(4+1));
    r[4+0] = (cx_fxexpt);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = obj_from_fixnum(2);
    r[4+3] = obj_from_fixnum(fixnum_from_obj(r[1]) - (1));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 21: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop k rb x */
    hreserve(hbsz(5+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(22);
    r[6] = (hendblk(5+1));
    r[0] = (cx_fxexpt);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = obj_from_fixnum(2);
    r[3] = obj_from_fixnum(fixnum_from_obj(r[4]) - (1));
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 22: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop k rb r x */
    { const fixnum_t v9259_x = fixnum_from_obj(r[6]);
  if ((((-fixnum_from_obj(r[1])) <= (v9259_x)) && ((v9259_x) <= (fixnum_from_obj(r[5]) - (1))))) {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((fixnum_from_obj(r[4]) < (128))) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = obj_from_fixnum(fixnum_from_obj(r[4]) + (8));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } } 

case 23: /* unsigned-representation-width k x */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_unsigned_2Drepresentation_2Dwidth: /* k x */
  if ((!(fixnum_from_obj(r[1]) < (0)))) {
    r[2+0] = r[0];  
    r[2+1] = obj_from_fixnum(8);
    r[2+2] = r[1];  
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v9069;
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

case 24: /* clo k rb */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v9069: /* k rb x */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(24);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[3];  
    *--hp = obj_from_case(25);
    r[3] = (hendblk(4+1));
    r[4+0] = (cx_fxexpt);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = obj_from_fixnum(2);
    r[4+3] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 25: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop k rb x */
  if ((fixnum_from_obj(r[5]) < fixnum_from_obj(r[1]))) {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((fixnum_from_obj(r[4]) < (128))) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = obj_from_fixnum(fixnum_from_obj(r[4]) + (8));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
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

case 26: /* char-safe? k c */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k c */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cxs_char_2Dsafe_3F((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 27: /* fold-max k base lst */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k base lst */
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = (cxs_fold_2Dmax((r[1]), (r[2])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 28: /* decompose-literal k x */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_decompose_2Dliteral: /* k x */
  if ((isnull((r[1])))) {
    { /* cons */ 
    hreserve(hbsz(3), 2); /* 2 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("null")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = (mksymbol(internsym("%const")));
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
    hreserve(hbsz(2+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(29);
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

case 29: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r x k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(32);
    r[4] = (hendblk(1+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto s_loop_v9028;
  } else {
  if ((ispair((r[2])))) {
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(33);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = (cdr((r[2])));
    goto gs_decompose_2Dliteral;
  } else {
  if ((isbox((r[2])))) {
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(35);
    r[4] = (hendblk(1+1));
    r[0] = r[4];  
    r[1] = (boxref((r[2])));
    goto gs_decompose_2Dliteral;
  } else {
  if ((isvector((r[2])))) {
    { /* vector->list */
    obj v, l = mknull(); int c = (vectorlen((r[2])));
    hreserve(hbsz(3)*c, 4); /* 4 live regs */
    v = r[2];   /* gc-safe */
    while (c-- > 0) { *--hp = l; *--hp = hblkref(v, 1+c);
    *--hp = obj_from_size(PAIR_BTAG); l = hendblk(3); }
    r[4] = (l); }
    hreserve(hbsz(1+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(38);
    r[5] = (hendblk(1+1));
    r[0] = r[5];  
    r[1] = r[4];  
    goto s_loop_v8987;
  } else {
  if ((is_char_obj(r[2]))) {
    r[4] = obj_from_fixnum((fixnum_t)char_from_obj(r[2]));
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = obj_from_case(39);
    r[5] = (hendblk(2+1));
    r[0] = (cx_fixnum_2D_3Estring);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[4];  
    r[3] = obj_from_fixnum(10);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
  if ((isstring((r[2])))) {
    { /* string->list */
    int c = (stringlen((r[2])));
    char *s; obj l = mknull();
    hreserve(hbsz(3)*c, 4); /* 4 live regs */
    s = stringchars((r[2])); /* gc-safe */
    while (c-- > 0) { *--hp = l; *--hp = obj_from_char(s[c]);
    *--hp = obj_from_size(PAIR_BTAG); l = hendblk(3); }
    r[4] = (l); }
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = obj_from_case(42);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[4];  
    goto s_loop_v8958;
  } else {
  if ((issymbol((r[2])))) {
    r[4] = (hpushstr(4, newstring(symbolname(getsymbol((r[2]))))));
    { /* string->list */
    int c = (stringlen((r[4])));
    char *s; obj l = mknull();
    hreserve(hbsz(3)*c, 5); /* 5 live regs */
    s = stringchars((r[4])); /* gc-safe */
    while (c-- > 0) { *--hp = l; *--hp = obj_from_char(s[c]);
    *--hp = obj_from_size(PAIR_BTAG); l = hendblk(3); }
    r[5] = (l); }
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_case(51);
    r[6] = (hendblk(3+1));
    r[0] = r[6];  
    r[1] = r[5];  
    goto s_loop_v8884;
  } else {
  if ((is_bool_obj(r[2]))) {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = (bool_from_obj(r[2]) ? (cx__23188) : (cx__23187));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("boolean")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("%const")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (((is_fixnum_obj(r[2])) && (is_fixnum_obj(r[2])))) {
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_case(59);
    r[4] = (hendblk(2+1));
    r[0] = (cx_fixnum_2D_3Estring);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = obj_from_fixnum(labs(fixnum_from_obj(r[2])));
    r[3] = obj_from_fixnum(10);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
  if ((((is_fixnum_obj(r[2])) || (is_flonum_obj(r[2]))) && (is_flonum_obj(r[2])))) {
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(61);
    r[4] = (hendblk(1+1));
  if ((is_fixnum_obj(r[2]))) {
    r[0] = (cx_fixnum_2D_3Estring);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    /* r[2] */    
    r[3] = obj_from_fixnum(10);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = (cx_flonum_2D_3Estring);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = r[3];  
    r[1] = (cx__23146);
    r[2] = r[4];  
    goto gs_x_2Derror_2A;
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

s_loop_v9028: /* k id */
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
    *--hp = obj_from_case(30);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v9028;
  }

case 30: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(31);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = (car((r[2])));
    goto gs_decompose_2Dliteral;

case 31: /* clo ek r */
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

case 32: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* list* */
    obj p;
    hreserve(hbsz(3)*3, 3); /* 3 live regs */
    p = r[1];   /* gc-safe */
    *--hp = p; *--hp = (mksymbol(internsym("list")));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (mksymbol(internsym("%const")));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    r[3] = (p); }
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 33: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r x k */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(34);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = (car((r[2])));
    goto gs_decompose_2Dliteral;

case 34: /* clo ek r */
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
    *--hp = (mksymbol(internsym("pair")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("%const")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 35: /* clo ek r */
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
    *--hp = (mksymbol(internsym("box")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("%const")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v8987: /* k id */
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
    *--hp = obj_from_case(36);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v8987;
  }

case 36: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(37);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = (car((r[2])));
    goto gs_decompose_2Dliteral;

case 37: /* clo ek r */
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

case 38: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* list* */
    obj p;
    hreserve(hbsz(3)*3, 3); /* 3 live regs */
    p = r[1];   /* gc-safe */
    *--hp = p; *--hp = (mksymbol(internsym("vector")));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (mksymbol(internsym("%const")));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    r[3] = (p); }
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 39: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r x k */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(40);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = r[2];  
    goto gs_unsigned_2Drepresentation_2Dwidth;

case 40: /* clo ek r */
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
    *--hp = (mksymbol(internsym("char")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("%const")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v8958: /* k id */
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
    *--hp = obj_from_case(41);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v8958;
  }

case 41: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    r[4] = obj_from_fixnum((fixnum_t)char_from_obj(r[4]));
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

case 42: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r c* k x */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_case(43);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[2];  
    goto s_loop_v8949;

s_loop_v8949: /* k id */
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
    r[2] = (cxs_char_2Dsafe_3F((r[2])));
  if (bool_from_obj(r[2])) {
    r[2] = (cdr((r[1])));
    /* r[0] */    
    r[1] = r[2];  
    goto s_loop_v8949;
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

case 43: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r r k x */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("string")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("%const")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(46);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = r[2];  
    goto s_loop_v8928;
  }

s_loop_v8928: /* k id */
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
    *--hp = obj_from_case(44);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v8928;
  }

case 44: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(45);
    r[4] = (hendblk(2+1));
    r[5] = (car((r[2])));
    r[5] = obj_from_bool(is_fixnum_obj(r[5]));
  if (bool_from_obj(r[5])) {
    r[0] = (cx_fixnum_2D_3Estring);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (car((r[2])));
    r[3] = obj_from_fixnum(10);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = (cx_flonum_2D_3Estring);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (car((r[2])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 45: /* clo ek r */
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

case 46: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r r k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(49);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto s_loop_v8907;

s_loop_v8907: /* k id */
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
    *--hp = obj_from_case(47);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v8907;
  }

case 47: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(48);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = (car((r[2])));
    goto gs_unsigned_2Drepresentation_2Dwidth;

case 48: /* clo ek r */
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

case 49: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    r[4] = (cxs_fold_2Dmax(obj_from_fixnum(8), (r[1])));
    { /* list* */
    obj p;
    hreserve(hbsz(3)*4, 5); /* 5 live regs */
    p = r[3];   /* gc-safe */
    *--hp = p; *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (mksymbol(internsym("string")));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (mksymbol(internsym("%const")));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    r[5] = (p); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v8884: /* k id */
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
    *--hp = obj_from_case(50);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v8884;
  }

case 50: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    r[4] = obj_from_fixnum((fixnum_t)char_from_obj(r[4]));
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

case 51: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r c* k s */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_case(52);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[2];  
    goto s_loop_v8875;

s_loop_v8875: /* k id */
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
    r[2] = (cxs_char_2Dsafe_3F((r[2])));
  if (bool_from_obj(r[2])) {
    r[2] = (cdr((r[1])));
    /* r[0] */    
    r[1] = r[2];  
    goto s_loop_v8875;
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

case 52: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r r k s */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("symbol")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("%const")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(55);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = r[2];  
    goto s_loop_v8854;
  }

s_loop_v8854: /* k id */
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
    goto s_loop_v8854;
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
    r[5] = (car((r[2])));
    r[5] = obj_from_bool(is_fixnum_obj(r[5]));
  if (bool_from_obj(r[5])) {
    r[0] = (cx_fixnum_2D_3Estring);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (car((r[2])));
    r[3] = obj_from_fixnum(10);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = (cx_flonum_2D_3Estring);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (car((r[2])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

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
    /* ek r r k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(58);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto s_loop_v8833;

s_loop_v8833: /* k id */
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
    *--hp = obj_from_case(56);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v8833;
  }

case 56: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(57);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = (car((r[2])));
    goto gs_unsigned_2Drepresentation_2Dwidth;

case 57: /* clo ek r */
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

case 58: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    r[4] = (cxs_fold_2Dmax(obj_from_fixnum(8), (r[1])));
    { /* list* */
    obj p;
    hreserve(hbsz(3)*4, 5); /* 5 live regs */
    p = r[3];   /* gc-safe */
    *--hp = p; *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (mksymbol(internsym("symbol")));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (mksymbol(internsym("%const")));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    r[5] = (p); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 59: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k x */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = ((fixnum_from_obj(r[3]) < (0)) ? (mksymbol(internsym("-"))) : (mksymbol(internsym("+"))));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[2];  
    *--hp = obj_from_case(60);
    r[6] = (hendblk(2+1));
    r[0] = r[6];  
    r[1] = r[3];  
    goto gs_integer_2Drepresentation_2Dwidth;

case 60: /* clo ek r */
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
    *--hp = (mksymbol(internsym("exact-integer")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("%const")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 61: /* clo ek r */
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
    *--hp = (mksymbol(internsym("inexact-real")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("%const")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 62: /* sid? k sexp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_sid_3F: /* k sexp */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = ((issymbol((r[1]))) ? obj_from_bool(issymbol((r[1]))) : (cxs_renamed_2Dsid_3F((r[1]))));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 63: /* renamed-sid? k sexp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k sexp */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cxs_renamed_2Dsid_3F((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 64: /* svector? k sexp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k sexp */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cxs_svector_3F((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 65: /* svector->list k sexp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k sexp */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (vectorref((r[1]), (0)));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 66: /* list->svector k l */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k l */
    { /* vector */
    hreserve(hbsz(1+1), 2); /* 2 live regs */
    *--hp = r[1];  
    *--hp = obj_from_size(VECTOR_BTAG);
    r[2] = (hendblk(1+1)); }
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 67: /* make-sid k name renamed-id location */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
gs_make_2Dsid: /* k name renamed-id location */
  if (((r[1]) == (r[3]))) {
    { /* vector */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(2+1)); }
  } else {
    { /* vector */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(3+1)); }
  }
    r[5+0] = r[0];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 68: /* sid-name k sid */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k sid */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cxs_sid_2Dname((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 69: /* sid-id k sid */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k sid */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cxs_sid_2Did((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 70: /* sid-location k sid */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k sid */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cxs_sid_2Dlocation((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 71: /* list1? k x */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k x */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cxs_list1_3F((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 72: /* list2? k x */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k x */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cxs_list2_3F((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 73: /* map-vecs k f x */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_map_2Dvecs: /* k f x */
    r[3+0] = r[0];  
    r[3+1] = r[2];  
    r[3+2] = r[1];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_mv;

case 74: /* clo k x */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_mv: /* k x f */
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(75);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    /* r[1] */    
    /* r[2] */    
    goto s_mv2;

case 75: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k x */
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = (bool_from_obj(r[1]) ? (r[1]) : (r[3]));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 76: /* clo k x */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_mv2: /* k x f */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(76);
    r[3] = (hendblk(1+1));
  if ((isvector((r[1])))) {
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[0];  
    r[4+2] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((ispair((r[1])))) {
    r[4] = (car((r[1])));
    r[5] = (cdr((r[1])));
    hreserve(hbsz(1+1), 6); /* 6 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(74);
    r[6] = (hendblk(1+1));
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(77);
    r[6] = (hendblk(5+1));
    r[0] = r[6];  
    r[1] = r[4];  
    /* r[2] */    
    goto s_mv2;
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

case 77: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r mv2 a mv b k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = obj_from_case(78);
    r[7] = (hendblk(2+1));
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[3];  
    *--hp = r[6];  
    *--hp = obj_from_case(79);
    r[7] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 78: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
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

case 79: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k a */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
  } else {
    r[4] = obj_from_bool(0);
  }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 80: /* wrap-vec k v */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k v */
    hreserve(hbsz(1+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(81);
    r[2] = (hendblk(1+1));
    { /* vector->list */
    obj v, l = mknull(); int c = (vectorlen((r[1])));
    hreserve(hbsz(3)*c, 3); /* 3 live regs */
    v = r[1];   /* gc-safe */
    while (c-- > 0) { *--hp = l; *--hp = hblkref(v, 1+c);
    *--hp = obj_from_size(PAIR_BTAG); l = hendblk(3); }
    r[3] = (l); }
    r[0] = r[2];  
    r[1] = r[3];  
    goto gs_wrap_2Dvecs;

case 81: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* vector */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = obj_from_size(VECTOR_BTAG);
    r[3] = (hendblk(1+1)); }
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 82: /* wrap-vecs k input */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_wrap_2Dvecs: /* k input */
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(83);
    r[2] = (hendblk(0+1));
    r[3+0] = r[0];  
    r[3+1] = r[2];  
    r[3+2] = r[1];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_map_2Dvecs;

case 83: /* clo k v */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k v */
    hreserve(hbsz(1+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(84);
    r[2] = (hendblk(1+1));
    { /* vector->list */
    obj v, l = mknull(); int c = (vectorlen((r[1])));
    hreserve(hbsz(3)*c, 3); /* 3 live regs */
    v = r[1];   /* gc-safe */
    while (c-- > 0) { *--hp = l; *--hp = hblkref(v, 1+c);
    *--hp = obj_from_size(PAIR_BTAG); l = hendblk(3); }
    r[3] = (l); }
    r[4+0] = (cx_wrap_2Dvecs);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[2];  
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 84: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* vector */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = obj_from_size(VECTOR_BTAG);
    r[3] = (hendblk(1+1)); }
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 85: /* unwrap-vec k v-sexp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k v-sexp */
  if (((1) == (vectorlen((r[1]))))) {
    hreserve(hbsz(1+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(86);
    r[2] = (hendblk(1+1));
    r[0] = r[2];  
    r[1] = (vectorref((r[1]), (0)));
    goto gs_unwrap_2Dvecs;
  } else {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (vectorref((r[1]), (0)));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 86: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { fixnum_t v9258_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v9258_tmp = (n); }
    { /* list->vector */
    obj l; int i, c = (v9258_tmp);
    hreserve(hbsz(c+1), 3); /* 3 live regs */
    l = r[1];   /* gc-safe */
    for (i = 0; i < c; ++i, l = cdr(l)) hp[i-c] = car(l);
    hp -= c; *--hp = obj_from_size(VECTOR_BTAG);
    r[3] = (hendblk(c+1)); } }
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 87: /* unwrap-vecs k sexp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_unwrap_2Dvecs: /* k sexp */
    r[2+0] = r[0];  
    r[2+1] = (cx_unwrap_2Dvec);
    r[2+2] = r[1];  
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_map_2Dvecs;

case 88: /* make-code k output */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k output */
    { /* cons */ 
    hreserve(hbsz(3), 2); /* 2 live regs */
    *--hp = (mknull());
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

case 89: /* make-builtin k name */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k name */
    { /* cons */ 
    hreserve(hbsz(3), 2); /* 2 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = (cx__23471);
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

case 90: /* variable? k val */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k val */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(issymbol((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 91: /* expanded-code? k val */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k val */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cxs_list1_3F((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 92: /* code-output k code */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k code */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (car((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 93: /* syntax? k val */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k val */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cxs_list2_3F((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 94: /* builtin? k syntax */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k syntax */
    r[2] = (car((r[1])));
    r[2] = (car((r[2])));
    r[2] = obj_from_bool((mksymbol(internsym("builtin"))) == (r[2]));
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 95: /* builtin-name k builtin */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k builtin */
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

case 96: /* expander? k syntax */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k syntax */
    { bool_t v9257_tmp;
    r[2] = (car((r[1])));
    r[2] = (car((r[2])));
    v9257_tmp = ((mksymbol(internsym("builtin"))) == (r[2]));
    r[2] = obj_from_bool(!(v9257_tmp)); }
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 97: /* make-expander k form env */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k form env */
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 98: /* expander-form k expd */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k expd */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (car((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 99: /* expander-env k expd */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k expd */
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

case 100: /* acons k key val alist */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
    /* k key val alist */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[0];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 101: /* lookup-sid k sid env */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k sid env */
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = (cxs_lookup_2Dsid((r[1]), (r[2])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 102: /* lookup-location k location store */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_lookup_2Dlocation: /* k location store */
    r[3] = (cxs_assv_23530((r[1]), (r[2])));
  if (bool_from_obj(r[3])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((issymbol((r[1])))) {
    /* r[0] */    
    /* r[1] */    
    goto gs_symloc_2D_3Evar;
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

case 103: /* lookup2 k sid env store */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
gs_lookup2: /* k sid env store */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(104);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = (cxs_lookup_2Dsid((r[1]), (r[2])));
    r[2] = r[3];  
    goto gs_lookup_2Dlocation;

case 104: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r sid k */
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
    { /* string-append */
    int *d = stringcat(stringdata((cx__23566)), stringdata((cx__23565)));
    r[4] = (hpushstr(4, d)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[3];  
    r[1] = r[4];  
    r[2] = r[5];  
    goto gs_x_2Derror_2A;
  }

case 105: /* extend-env k env id location */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
    /* k env id location */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[0];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 106: /* extend-store k store loc val */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
    /* k store loc val */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[0];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 107: /* substitute-in-store k store loc val */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
gs_substitute_2Din_2Dstore: /* k store loc val */
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_case(108);
    r[4] = (hendblk(3+1));
  if (bool_from_obj(cxs_assv_23530((r[2]), (r[1])))) {
    r[0] = r[4];  
    /* r[1] */    
    /* r[2] */    
    goto s_loop_v8462;
  } else {
    r[5+0] = obj_from_ktrap();
    r[5+1] = r[1];  
    r[5+2] = r[3];  
    r[5+3] = r[2];  
    r[5+4] = r[0];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v8474;
  }

case 108: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
s_l_v8474: /* ek r val loc k */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(109);
    r[5] = (hendblk(4+1));
  if ((issymbol((r[3])))) {
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[2];  
    *--hp = r[5];  
    *--hp = obj_from_case(110);
    r[6] = (hendblk(2+1));
    r[0] = r[6];  
    r[1] = r[3];  
    goto gs_symloc_2D_3Evar;
  } else {
    r[6+0] = obj_from_ktrap();
    r[6+1] = obj_from_bool(0);
    r[6+2] = r[2];  
    r[6+3] = r[3];  
    r[6+4] = r[4];  
    r[6+5] = r[1];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v8482;
  }

case 109: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
s_l_v8482: /* ek r val loc k r */
  if (bool_from_obj(r[1])) {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 110: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k val */
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = obj_from_bool((r[3]) == (r[1]));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v8462: /* k store loc */
    r[3] = (car((r[1])));
    r[4] = (car((r[3])));
    r[4] = obj_from_bool(((r[2]) == (r[4])) || ((is_flonum_obj(r[2])) && ((is_flonum_obj(r[4])) && (flonum_from_obj(r[2]) == flonum_from_obj(r[4])))));
  if (bool_from_obj(r[4])) {
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = (cdr((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[4] = (cdr((r[1])));
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = obj_from_case(111);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = r[4];  
    /* r[2] */    
    goto s_loop_v8462;
  }

case 111: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k p */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
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

case 112: /* symloc->var k sym */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_symloc_2D_3Evar: /* k sym */
    hreserve(hbsz(1), 2); /* 2 live regs */
    *--hp = obj_from_void(0);
    r[2] = (hendblk(1));
    r[3] = (hpushstr(3, newstring(symbolname(getsymbol((r[1]))))));
    (void)(objptr_from_obj(r[2])[0] = (r[3]));
  if (((((r[1]) == (mksymbol(internsym("begin")))) || ((is_flonum_obj(r[1])) && ((is_flonum_obj(mksymbol(internsym("begin")))) && (flonum_from_obj(r[1]) == flonum_from_obj(mksymbol(internsym("begin"))))))) ? (1) : ((((r[1]) == (mksymbol(internsym("define")))) || ((is_flonum_obj(r[1])) && ((is_flonum_obj(mksymbol(internsym("define")))) && (flonum_from_obj(r[1]) == flonum_from_obj(mksymbol(internsym("define"))))))) ? (1) : ((((r[1]) == (mksymbol(internsym("if")))) || ((is_flonum_obj(r[1])) && ((is_flonum_obj(mksymbol(internsym("if")))) && (flonum_from_obj(r[1]) == flonum_from_obj(mksymbol(internsym("if"))))))) ? (1) : ((((r[1]) == (mksymbol(internsym("lambda")))) || ((is_flonum_obj(r[1])) && ((is_flonum_obj(mksymbol(internsym("lambda")))) && (flonum_from_obj(r[1]) == flonum_from_obj(mksymbol(internsym("lambda"))))))) ? (1) : ((((r[1]) == (mksymbol(internsym("letrec")))) || ((is_flonum_obj(r[1])) && ((is_flonum_obj(mksymbol(internsym("letrec")))) && (flonum_from_obj(r[1]) == flonum_from_obj(mksymbol(internsym("letrec"))))))) ? (1) : ((((r[1]) == (mksymbol(internsym("quote")))) || ((is_flonum_obj(r[1])) && ((is_flonum_obj(mksymbol(internsym("quote")))) && (flonum_from_obj(r[1]) == flonum_from_obj(mksymbol(internsym("quote"))))))) ? (1) : ((((r[1]) == (mksymbol(internsym("set!")))) || ((is_flonum_obj(r[1])) && ((is_flonum_obj(mksymbol(internsym("set!")))) && (flonum_from_obj(r[1]) == flonum_from_obj(mksymbol(internsym("set!"))))))) ? (1) : ((((r[1]) == (mksymbol(internsym("letcc")))) || ((is_flonum_obj(r[1])) && ((is_flonum_obj(mksymbol(internsym("letcc")))) && (flonum_from_obj(r[1]) == flonum_from_obj(mksymbol(internsym("letcc"))))))) ? (1) : ((((r[1]) == (mksymbol(internsym("withcc")))) || ((is_flonum_obj(r[1])) && ((is_flonum_obj(mksymbol(internsym("withcc")))) && (flonum_from_obj(r[1]) == flonum_from_obj(mksymbol(internsym("withcc"))))))) && (1))))))))))) {
    /* r[0] */    
    r[1] = r[2];  
    goto s_l_v8453;
  } else {
  if (((((r[1]) == (mksymbol(internsym("%prim")))) || ((is_flonum_obj(r[1])) && ((is_flonum_obj(mksymbol(internsym("%prim")))) && (flonum_from_obj(r[1]) == flonum_from_obj(mksymbol(internsym("%prim"))))))) ? (1) : ((((r[1]) == (mksymbol(internsym("%prim?")))) || ((is_flonum_obj(r[1])) && ((is_flonum_obj(mksymbol(internsym("%prim?")))) && (flonum_from_obj(r[1]) == flonum_from_obj(mksymbol(internsym("%prim?"))))))) ? (1) : ((((r[1]) == (mksymbol(internsym("%prim!")))) || ((is_flonum_obj(r[1])) && ((is_flonum_obj(mksymbol(internsym("%prim!")))) && (flonum_from_obj(r[1]) == flonum_from_obj(mksymbol(internsym("%prim!"))))))) ? (1) : ((((r[1]) == (mksymbol(internsym("%prim?!")))) || ((is_flonum_obj(r[1])) && ((is_flonum_obj(mksymbol(internsym("%prim?!")))) && (flonum_from_obj(r[1]) == flonum_from_obj(mksymbol(internsym("%prim?!"))))))) ? (1) : ((((r[1]) == (mksymbol(internsym("%prim*")))) || ((is_flonum_obj(r[1])) && ((is_flonum_obj(mksymbol(internsym("%prim*")))) && (flonum_from_obj(r[1]) == flonum_from_obj(mksymbol(internsym("%prim*"))))))) ? (1) : ((((r[1]) == (mksymbol(internsym("%prim*!")))) || ((is_flonum_obj(r[1])) && ((is_flonum_obj(mksymbol(internsym("%prim*!")))) && (flonum_from_obj(r[1]) == flonum_from_obj(mksymbol(internsym("%prim*!"))))))) ? (1) : ((((r[1]) == (mksymbol(internsym("%prim*?")))) || ((is_flonum_obj(r[1])) && ((is_flonum_obj(mksymbol(internsym("%prim*?")))) && (flonum_from_obj(r[1]) == flonum_from_obj(mksymbol(internsym("%prim*?"))))))) ? (1) : ((((r[1]) == (mksymbol(internsym("%prim*?!")))) || ((is_flonum_obj(r[1])) && ((is_flonum_obj(mksymbol(internsym("%prim*?!")))) && (flonum_from_obj(r[1]) == flonum_from_obj(mksymbol(internsym("%prim*?!"))))))) && (1)))))))))) {
    /* r[0] */    
    r[1] = r[2];  
    goto s_l_v8453;
  } else {
  if (((((r[1]) == (mksymbol(internsym("%quote")))) || ((is_flonum_obj(r[1])) && ((is_flonum_obj(mksymbol(internsym("%quote")))) && (flonum_from_obj(r[1]) == flonum_from_obj(mksymbol(internsym("%quote"))))))) ? (1) : ((((r[1]) == (mksymbol(internsym("%const")))) || ((is_flonum_obj(r[1])) && ((is_flonum_obj(mksymbol(internsym("%const")))) && (flonum_from_obj(r[1]) == flonum_from_obj(mksymbol(internsym("%const"))))))) ? (1) : ((((r[1]) == (mksymbol(internsym("%definition")))) || ((is_flonum_obj(r[1])) && ((is_flonum_obj(mksymbol(internsym("%definition")))) && (flonum_from_obj(r[1]) == flonum_from_obj(mksymbol(internsym("%definition"))))))) ? (1) : ((((r[1]) == (mksymbol(internsym("%localdef")))) || ((is_flonum_obj(r[1])) && ((is_flonum_obj(mksymbol(internsym("%localdef")))) && (flonum_from_obj(r[1]) == flonum_from_obj(mksymbol(internsym("%localdef"))))))) && (1)))))) {
    /* r[0] */    
    r[1] = r[2];  
    goto s_l_v8453;
  } else {
    { fixnum_t v9256_tmp;
    r[3] = (objptr_from_obj(r[2])[0]);
    v9256_tmp = (stringlen((r[3])));
    r[3] = obj_from_bool((v9256_tmp) > (0)); }
  if (bool_from_obj(r[3])) {
    { char_t v9255_tmp;
    r[3] = (objptr_from_obj(r[2])[0]);
    v9255_tmp = (*stringref((r[3]), (0)));
    r[3] = obj_from_bool((95) == (v9255_tmp)); }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    /* r[0] */    
    r[1] = r[2];  
    goto s_l_v8453;
  } else {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[1];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }
  }
  }

s_l_v8453: /* k str */
    r[2] = (objptr_from_obj(r[1])[0]);
    { /* string-append */
    int *d = stringcat(stringdata((r[2])), stringdata((cx__23608)));
    r[2] = (hpushstr(3, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__23608)), stringdata((r[2])));
    r[2] = (hpushstr(3, d)); }
    r[2] = (mksymbol(internsym(stringchars((r[2])))));
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 113: /* intloc->var k intloc sid */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_intloc_2D_3Evar: /* k intloc sid */
    r[3] = (cxs_sid_2Dname((r[2])));
    r[3] = (hpushstr(4, newstring(symbolname(getsymbol((r[3]))))));
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = obj_from_case(114);
    r[4] = (hendblk(2+1));
  if ((is_fixnum_obj(r[1]))) {
    r[5+0] = (cx_fixnum_2D_3Estring);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = r[1];  
    r[5+3] = obj_from_fixnum(10);
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[5+0] = (cx_flonum_2D_3Estring);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 114: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k str */
    { /* string-append */
    int *d = stringcat(stringdata((cx__23608)), stringdata((r[1])));
    r[4] = (hpushstr(4, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((r[3])), stringdata((r[4])));
    r[5] = (hpushstr(5, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__23608)), stringdata((r[5])));
    r[6] = (hpushstr(6, d)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mksymbol(internsym(stringchars((r[6])))));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 115: /* loc->var k loc sid */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_loc_2D_3Evar: /* k loc sid */
  if ((issymbol((r[1])))) {
    /* r[0] */    
    /* r[1] */    
    goto gs_symloc_2D_3Evar;
  } else {
    /* r[0] */    
    /* r[1] */    
    /* r[2] */    
    goto gs_intloc_2D_3Evar;
  }

case 116: /* make-begin k outputs */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_make_2Dbegin: /* k outputs */
  if (bool_from_obj(cxs_list1_3F((r[1])))) {
    r[2] = (car((r[1])));
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 2); /* 2 live regs */
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("begin")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
  }
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 117: /* expand-lambda k formals expr id-n env store loc-n */
    assert(rc == 8);
    r += 1; /* shift reg. wnd */
gs_expand_2Dlambda: /* k formals expr id-n env store loc-n */
    hreserve(hbsz(0+1), 7); /* 7 live regs */
    *--hp = obj_from_case(131);
    r[7] = (hendblk(0+1));
    hreserve(hbsz(0+1), 8); /* 8 live regs */
    *--hp = obj_from_case(133);
    r[8] = (hendblk(0+1));
    hreserve(hbsz(9+1), 9); /* 9 live regs */
    *--hp = r[0];  
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = r[7];  
    *--hp = r[1];  
    *--hp = r[8];  
    *--hp = obj_from_case(118);
    r[7] = (hendblk(9+1));
    r[8+0] = (cx_list_3F);
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[7];  
    r[8+2] = r[1];  
    r += 8; /* shift reg wnd */
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
    r[1+9] = p[8];
    r[1+10] = p[9]; }
    r += 1; /* shift reg. wnd */
    /* ek r flatten-dotted formals dot-flattened id-n expr loc-n store env k */
    r[11] = obj_from_bool(!bool_from_obj(r[1]));
    hreserve(hbsz(9+1), 12); /* 12 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = (r[11]);
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(119);
    r[12] = (hendblk(9+1));
  if (bool_from_obj(r[11])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[12]);
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = obj_from_ktrap();
    r[1] = r[3];  
    r[2] = r[3];  
    r[3] = r[4];  
    r[4] = (r[11]);
    /* r[5] */    
    /* r[6] */    
    /* r[7] */    
    /* r[8] */    
    /* r[9] */    
    r[10] = (r[10]);
    goto s_l_v8323;
  }

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
    r[1+9] = p[8];
    r[1+10] = p[9]; }
    r += 1; /* shift reg. wnd */
s_l_v8323: /* ek r formals dot-flattened dotted? id-n expr loc-n store env k */
    hreserve(hbsz(9+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[1];  
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(126);
    r[11] = (hendblk(9+1));
    r[12+0] = (r[11]);
    r[12+1] = r[1];  
    r[12+2] = r[1];  
    r[12+3] = r[2];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v8361;

case 120: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
s_loop_v8361: /* k id r formals */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[4] = (car((r[1])));
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(120);
    r[5] = (hendblk(2+1));
    hreserve(hbsz(6+1), 6); /* 6 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[2];  
    *--hp = obj_from_case(121);
    r[5] = (hendblk(6+1));
    r[0] = r[5];  
    r[1] = r[4];  
    goto gs_sid_3F;
  }

case 121: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r r x formals loop id k */
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(122);
    r[8] = (hendblk(6+1));
  if (bool_from_obj(r[1])) {
    r[0] = obj_from_ktrap();
    /* r[1] */    
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    /* r[7] */    
    goto s_l_v8365;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = (mknull());
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = (cx__23847);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[0] = r[8];  
    r[1] = (cx__23862);
    r[2] = r[9];  
    goto gs_x_2Derror_2A;
  }

case 122: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
s_l_v8365: /* ek  r x formals loop id k */
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(123);
    r[8] = (hendblk(3+1));
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(124);
    r[8] = (hendblk(3+1));
    r[9+0] = (cx_member);
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[8];  
    r[9+2] = r[3];  
    r[9+3] = r[2];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 123: /* clo ek  */
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

case 124: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r x formals k */
    r[5] = (cdr((r[1])));
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(125);
    r[6] = (hendblk(3+1));
    r[0] = (cx_member);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    /* r[2] */    
    r[3] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 125: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r x formals k */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (cx__23847);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[4];  
    r[1] = (cx__23852);
    r[2] = r[5];  
    goto gs_x_2Derror_2A;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 126: /* clo ek  */
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
    /* ek  dot-flattened dotted? id-n expr loc-n store env r k */
    r[11+0] = (r[10]);
    r[11+1] = r[9];  
    r[11+2] = (mknull());
    r[11+3] = r[8];  
    r[11+4] = r[7];  
    r[11+5] = r[6];  
    r[11+6] = r[2];  
    r[11+7] = r[3];  
    r[11+8] = r[4];  
    r[11+9] = r[5];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v8326;

case 127: /* clo k formals rvars env store loc-n */
    assert(rc == 7);
    { obj* p = objptr_from_obj(r[0]);
    r[1+6] = p[1];
    r[1+7] = p[2];
    r[1+8] = p[3];
    r[1+9] = p[4]; }
    r += 1; /* shift reg. wnd */
s_loop_v8326: /* k formals rvars env store loc-n dot-flattened dotted? id-n expr */
  if ((!(isnull((r[1]))))) {
    hreserve(hbsz(4+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_case(127);
    r[10] = (hendblk(4+1));
    hreserve(hbsz(7+1), 11); /* 11 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = (r[10]);
    *--hp = obj_from_case(128);
    r[10] = (hendblk(7+1));
    r[11+0] = (r[10]);
    r[11+1] = r[5];  
    r[11+2] = (car((r[1])));
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_intloc_2D_3Evar;
  } else {
    { fixnum_t v9254_tmp;
    { /* length */
    int n; obj l = r[2];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v9254_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v9254_tmp);
    hreserve(hbsz(3)*c, 10); /* 10 live regs */
    l = r[2];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[10] = (o); } }
    hreserve(hbsz(6+1), 11); /* 11 live regs */
    *--hp = r[0];  
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = obj_from_case(129);
    r[11] = (hendblk(6+1));
  if (bool_from_obj(r[7])) {
    r[0] = r[6];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[11]);
    r[2] = (r[10]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[12+0] = obj_from_ktrap();
    r[12+1] = (r[10]);
    r[12+2] = r[5];  
    r[12+3] = r[4];  
    r[12+4] = r[3];  
    r[12+5] = r[8];  
    r[12+6] = r[9];  
    r[12+7] = r[0];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v8328;
  }
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
    /* ek r loop rvars k store env loc-n formals */
    r[9] = (car((r[8])));
    r[9] = (cxs_sid_2Did((r[9])));
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[7];  
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[6];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[1];  
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[5];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    r[12+0] = r[2];  
    pc = objptr_from_obj(r[12+0])[0];
    r[12+1] = r[4];  
    r[12+2] = (cdr((r[8])));
    r[12+3] = (r[11]);
    r[12+4] = r[9];  
    r[12+5] = (r[10]);
    r[12+6] = obj_from_fixnum((1) + fixnum_from_obj(r[7]));
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 7);
    goto jump;

case 129: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
s_l_v8328: /* ek r loc-n store env id-n expr k */
    hreserve(hbsz(2+1), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = r[7];  
    *--hp = obj_from_case(130);
    r[8] = (hendblk(2+1));
    r[9+0] = r[8];  
    r[9+1] = r[6];  
    r[9+2] = r[5];  
    r[9+3] = r[4];  
    r[9+4] = r[3];  
    r[9+5] = r[2];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dexpr;

case 130: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 131: /* clo k x */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
s_dot_2Dflattened: /* k x */
    r[2] = (cdr((r[1])));
    r[2] = obj_from_bool(isnull((r[2])));
  if (bool_from_obj(r[2])) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (car((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(132);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_dot_2Dflattened;
  }

case 132: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k x */
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

case 133: /* clo k x */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
s_flatten_2Ddotted: /* k x */
  if ((ispair((r[1])))) {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(134);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_flatten_2Ddotted;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 2); /* 2 live regs */
    *--hp = (mknull());
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
  }

case 134: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k x */
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

case 135: /* expand-letcc k formal expr id-n env store loc-n */
    assert(rc == 8);
    r += 1; /* shift reg. wnd */
gs_expand_2Dletcc: /* k formal expr id-n env store loc-n */
    hreserve(hbsz(7+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_case(136);
    r[7] = (hendblk(7+1));
    r[0] = r[7];  
    /* r[1] */    
    goto gs_sid_3F;

case 136: /* clo ek r */
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
    /* ek r id-n expr k store env loc-n formal */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(137);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    r[1] = r[7];  
    r[2] = r[8];  
    goto gs_intloc_2D_3Evar;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = (mknull());
    *--hp = (cx__23917);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[0] = r[4];  
    r[1] = (cx__23862);
    r[2] = r[9];  
    goto gs_x_2Derror_2A;
  }

case 137: /* clo ek r */
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
    /* ek r id-n expr k store env loc-n formal */
    r[9] = (cxs_sid_2Did((r[8])));
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[7];  
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[6];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[1];  
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[5];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    hreserve(hbsz(2+1), 11); /* 11 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(138);
    r[11] = (hendblk(2+1));
    r[0] = (r[11]);
    r[1] = r[3];  
    /* r[2] */    
    r[3] = r[9];  
    r[4] = (r[10]);
    r[5] = obj_from_fixnum((1) + fixnum_from_obj(r[7]));
    goto gs_expand_2Dexpr;

case 138: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 139: /* check-syntax-bindings k bindings */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k bindings */
    hreserve(hbsz(2+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(140);
    r[2] = (hendblk(2+1));
    r[3+0] = (cx_list_3F);
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[2];  
    r[3+2] = r[1];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 140: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r bindings k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(141);
    r[4] = (hendblk(2+1));
  if (bool_from_obj(r[1])) {
    r[0] = obj_from_ktrap();
    /* r[1] */    
    /* r[2] */    
    /* r[3] */    
    goto s_l_v8209;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[4];  
    r[1] = (cx__23973);
    r[2] = r[5];  
    goto gs_x_2Derror_2A;
  }

case 141: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
s_l_v8209: /* ek  bindings k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(145);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto s_loop_v8233;

case 142: /* clo k id */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
s_loop_v8233: /* k id */
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
    *--hp = obj_from_case(142);
    r[3] = (hendblk(0+1));
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(143);
    r[3] = (hendblk(4+1));
  if (bool_from_obj(cxs_list2_3F((r[2])))) {
    r[0] = r[3];  
    r[1] = (car((r[2])));
    goto gs_sid_3F;
  } else {
    hreserve(hbsz(0+1), 4); /* 4 live regs */
    *--hp = obj_from_case(142);
    r[4] = (hendblk(0+1));
    r[5+0] = obj_from_ktrap();
    r[5+1] = obj_from_bool(0);
    r[5+2] = r[2];  
    r[5+3] = r[4];  
    r[5+4] = r[1];  
    r[5+5] = r[0];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v8235;
  }
  }

case 143: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
s_l_v8235: /* ek r b loop id k */
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(144);
    r[6] = (hendblk(3+1));
  if (bool_from_obj(r[1])) {
    r[0] = obj_from_ktrap();
    /* r[1] */    
    r[2] = r[3];  
    r[3] = r[4];  
    r[4] = r[5];  
    goto s_l_v8237;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    r[0] = r[6];  
    r[1] = (cx__23964);
    r[2] = r[7];  
    goto gs_x_2Derror_2A;
  }

case 144: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
s_l_v8237: /* ek  loop id k */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 145: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  bindings k */
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v8212;

case 146: /* clo k bs */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
s_loop_v8212: /* k bs */
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
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(146);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(147);
    r[2] = (hendblk(3+1));
    r[3] = (car((r[1])));
    r[3] = (car((r[3])));
    r[4+0] = (cx_assoc);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[2];  
    r[4+2] = r[3];  
    r[4+3] = (cdr((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 147: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop bs k */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(148);
    r[5] = (hendblk(3+1));
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = (cx__23943);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[7] = (car((r[3])));
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[7+0] = r[5];  
    r[7+1] = (cx__23949);
    r[7+2] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_x_2Derror_2A;
  } else {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_void(0);
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    goto s_l_v8216;
  }

case 148: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
s_l_v8216: /* ek  loop bs k */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 149: /* expand-syntax-bindings k bindings id-n syntax-env ienv store loc-n k */
    assert(rc == 9);
    r += 1; /* shift reg. wnd */
gs_expand_2Dsyntax_2Dbindings: /* k bindings id-n syntax-env ienv store loc-n k */
    r[8+0] = r[0];  
    r[8+1] = r[1];  
    r[8+2] = (mknull());
    r[8+3] = r[5];  
    r[8+4] = r[6];  
    r[8+5] = r[7];  
    r[8+6] = r[4];  
    r[8+7] = r[1];  
    r[8+8] = r[3];  
    r[8+9] = r[2];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v8158;

case 150: /* clo k bs vals store loc-n */
    assert(rc == 6);
    { obj* p = objptr_from_obj(r[0]);
    r[1+5] = p[1];
    r[1+6] = p[2];
    r[1+7] = p[3];
    r[1+8] = p[4];
    r[1+9] = p[5]; }
    r += 1; /* shift reg. wnd */
s_loop_v8158: /* k bs vals store loc-n k ienv bindings syntax-env id-n */
  if ((!(isnull((r[1]))))) {
    r[10] = (car((r[1])));
    r[10] = (cdr((r[10])));
    r[10] = (car((r[10])));
    hreserve(hbsz(5+1), 11); /* 11 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(150);
    r[11] = (hendblk(5+1));
    hreserve(hbsz(3+1), 12); /* 12 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = (r[11]);
    *--hp = obj_from_case(151);
    r[11] = (hendblk(3+1));
    r[12+0] = r[0];  
    r[12+1] = (r[10]);
    r[12+2] = r[9];  
    r[12+3] = r[8];  
    r[12+4] = r[3];  
    r[12+5] = r[4];  
    r[12+6] = (r[11]);
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dval;
  } else {
    { fixnum_t v9253_tmp;
    { /* length */
    int n; obj l = r[2];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v9253_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v9253_tmp);
    hreserve(hbsz(3)*c, 10); /* 10 live regs */
    l = r[2];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[10] = (o); } }
    r[11+0] = r[0];  
    r[11+1] = r[3];  
    r[11+2] = (r[10]);
    r[11+3] = r[7];  
    r[11+4] = r[5];  
    r[11+5] = r[4];  
    r[11+6] = r[6];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v8159;
  }

case 151: /* clo k val store loc-n */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1];
    r[1+5] = p[2];
    r[1+6] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* k val store loc-n loop vals bs */
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    r[8+0] = r[4];  
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[0];  
    r[8+2] = (cdr((r[6])));
    r[8+3] = r[7];  
    r[8+4] = r[2];  
    r[8+5] = r[3];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;

s_loop_v8159: /* k store vals bs k loc-n ienv */
  if ((!(isnull((r[2]))))) {
    r[7] = (car((r[3])));
    r[7] = (car((r[7])));
    r[7] = (cxs_lookup_2Dsid((r[7]), (r[6])));
    r[8] = (car((r[2])));
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[1];  
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    r[9] = (cdr((r[2])));
    r[10] = (cdr((r[3])));
    /* r[0] */    
    r[1] = r[8];  
    r[2] = r[9];  
    r[3] = (r[10]);
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    goto s_loop_v8159;
  } else {
    r[7+0] = r[4];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[0];  
    r[7+2] = r[1];  
    r[7+3] = r[5];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 152: /* apply-expander k syntax sexp id-n env store loc-n lsd? ek sk dk bk */
    assert(rc == 13);
    r += 1; /* shift reg. wnd */
gs_apply_2Dexpander: /* k syntax sexp id-n env store loc-n lsd? ek sk dk bk */
    r[12] = (car((r[1])));
    r[12] = (car((r[12])));
    r[12] = (cxs_sid_2Dname((r[12])));
  if (((((r[12]) == (mksymbol(internsym("syntax-rules")))) || ((is_flonum_obj(r[12])) && ((is_flonum_obj(mksymbol(internsym("syntax-rules")))) && (flonum_from_obj(r[12]) == flonum_from_obj(mksymbol(internsym("syntax-rules"))))))) && (1))) {
    hreserve(hbsz(8+1), 13); /* 13 live regs */
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = r[7];  
    *--hp = r[8];  
    *--hp = r[9];  
    *--hp = (r[10]);
    *--hp = (r[11]);
    *--hp = obj_from_case(153);
    r[13] = (hendblk(8+1));
    r[14+0] = r[0];  
    r[14+1] = r[1];  
    r[14+2] = r[2];  
    r[14+3] = r[3];  
    r[14+4] = r[4];  
    r[14+5] = (r[13]);
    r += 14; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_apply_2Dsynrules;
  } else {
  if (((((r[12]) == (mksymbol(internsym("syntax-lambda")))) || ((is_flonum_obj(r[12])) && ((is_flonum_obj(mksymbol(internsym("syntax-lambda")))) && (flonum_from_obj(r[12]) == flonum_from_obj(mksymbol(internsym("syntax-lambda"))))))) && (1))) {
    hreserve(hbsz(12+1), 13); /* 13 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[6];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = r[7];  
    *--hp = r[8];  
    *--hp = r[9];  
    *--hp = (r[10]);
    *--hp = (r[11]);
    *--hp = obj_from_case(154);
    r[13] = (hendblk(12+1));
  if (bool_from_obj(r[8])) {
    r[14+0] = obj_from_ktrap();
    r[14+1] = r[8];  
    r[14+2] = (r[11]);
    r[14+3] = (r[10]);
    r[14+4] = r[9];  
    r[14+5] = r[8];  
    r[14+6] = r[7];  
    r[14+7] = r[5];  
    r[14+8] = r[4];  
    r[14+9] = r[3];  
    r[14+10] = r[6];  
    r[14+11] = r[0];  
    r[14+12] = r[2];  
    r[14+13] = r[1];  
    r += 14; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v8077;
  } else {
  if (bool_from_obj(r[9])) {
    r[14+0] = obj_from_ktrap();
    r[14+1] = r[9];  
    r[14+2] = (r[11]);
    r[14+3] = (r[10]);
    r[14+4] = r[9];  
    r[14+5] = r[8];  
    r[14+6] = r[7];  
    r[14+7] = r[5];  
    r[14+8] = r[4];  
    r[14+9] = r[3];  
    r[14+10] = r[6];  
    r[14+11] = r[0];  
    r[14+12] = r[2];  
    r[14+13] = r[1];  
    r += 14; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v8077;
  } else {
  if (bool_from_obj(r[7])) {
    r[14+0] = obj_from_ktrap();
    r[14+1] = r[7];  
    r[14+2] = (r[11]);
    r[14+3] = (r[10]);
    r[14+4] = r[9];  
    r[14+5] = r[8];  
    r[14+6] = r[7];  
    r[14+7] = r[5];  
    r[14+8] = r[4];  
    r[14+9] = r[3];  
    r[14+10] = r[6];  
    r[14+11] = r[0];  
    r[14+12] = r[2];  
    r[14+13] = r[1];  
    r += 14; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v8077;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[14] = (hendblk(3)); }
    r[0] = (r[13]);
    r[1] = (cx__231104);
    r[2] = (r[14]);
    goto gs_x_2Derror_2A;
  }
  }
  }
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    r[14+0] = r[0];  
    r[14+1] = (cx__231042);
    r[14+2] = (r[13]);
    r += 14; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_x_2Derror_2A;
  }
  }

case 153: /* clo k sexp id-n */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2];
    r[1+5] = p[3];
    r[1+6] = p[4];
    r[1+7] = p[5];
    r[1+8] = p[6];
    r[1+9] = p[7];
    r[1+10] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* k sexp id-n bk dk sk ek lsd? loc-n store env */
    r[11+0] = r[0];  
    r[11+1] = r[1];  
    r[11+2] = r[2];  
    r[11+3] = (r[10]);
    r[11+4] = r[9];  
    r[11+5] = r[8];  
    r[11+6] = r[7];  
    r[11+7] = r[6];  
    r[11+8] = r[5];  
    r[11+9] = r[4];  
    r[11+10] = r[3];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dany;

case 154: /* clo ek  */
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
    r[1+12] = p[11];
    r[1+13] = p[12]; }
    r += 1; /* shift reg. wnd */
s_l_v8077: /* ek  bk dk sk ek lsd? store env id-n loc-n k sexp syntax */
    r[14] = (car((r[13])));
    r[14] = (cdr((r[14])));
    r[14] = (car((r[14])));
    r[15] = (cdr((r[12])));
    r[16] = (car((r[13])));
    r[16] = (cdr((r[16])));
    r[16] = (cdr((r[16])));
    r[17] = (cdr((r[13])));
    r[17] = (car((r[17])));
    { fixnum_t v9252_tmp;
    { fixnum_t v9251_tmp;
    { /* length */
    int n; obj l = (r[14]);
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v9252_tmp = (n); }
    { /* length */
    int n; obj l = (r[15]);
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v9251_tmp = (n); }
    r[18] = obj_from_bool((v9252_tmp) == (v9251_tmp)); } }
    hreserve(hbsz(14+1), 19); /* 19 live regs */
    *--hp = (r[11]);
    *--hp = (r[14]);
    *--hp = (r[10]);
    *--hp = (r[17]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = (r[16]);
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = (r[15]);
    *--hp = obj_from_case(155);
    r[19] = (hendblk(14+1));
  if (bool_from_obj(r[18])) {
    r[20+0] = obj_from_ktrap();
    r[20+1] = (r[18]);
    r[20+2] = (r[15]);
    r[20+3] = r[2];  
    r[20+4] = r[3];  
    r[20+5] = r[4];  
    r[20+6] = r[5];  
    r[20+7] = r[6];  
    r[20+8] = (r[16]);
    r[20+9] = r[7];  
    r[20+10] = r[8];  
    r[20+11] = r[9];  
    r[20+12] = (r[17]);
    r[20+13] = (r[10]);
    r[20+14] = (r[14]);
    r[20+15] = (r[11]);
    r += 20; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v8079;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 20); /* 20 live regs */
    *--hp = (mknull());
    *--hp = (r[12]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[20] = (hendblk(3)); }
    r[0] = (r[19]);
    r[1] = (cx__231097);
    r[2] = (r[20]);
    goto gs_x_2Derror_2A;
  }

case 155: /* clo ek  */
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
    r[1+12] = p[11];
    r[1+13] = p[12];
    r[1+14] = p[13];
    r[1+15] = p[14]; }
    r += 1; /* shift reg. wnd */
s_l_v8079: /* ek  sexps bk dk sk ek lsd? body store env id-n denv loc-n formals k */
    r[16+0] = (r[15]);
    r[16+1] = (r[14]);
    r[16+2] = (r[13]);
    r[16+3] = (r[12]);
    r[16+4] = r[2];  
    r[16+5] = (r[14]);
    r[16+6] = r[3];  
    r[16+7] = r[4];  
    r[16+8] = r[5];  
    r[16+9] = r[6];  
    r[16+10] = r[7];  
    r[16+11] = r[8];  
    r[16+12] = r[9];  
    r[16+13] = (r[10]);
    r[16+14] = (r[11]);
    r += 16; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v8080;

s_loop_v8080: /* k ids loc-n ienv sexps formals bk dk sk ek lsd? body store env id-n */
  if ((!(isnull((r[1]))))) {
    r[15] = (cdr((r[1])));
    r[16] = (car((r[1])));
    r[16] = (cxs_sid_2Did((r[16])));
    { /* cons */ 
    hreserve(hbsz(3), 17); /* 17 live regs */
    *--hp = r[2];  
    *--hp = (r[16]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[17] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 18); /* 18 live regs */
    *--hp = r[3];  
    *--hp = (r[17]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[16] = (hendblk(3)); }
    /* r[0] */    
    r[1] = (r[15]);
    r[2] = obj_from_fixnum(fixnum_from_obj(r[2]) + (1));
    r[3] = (r[16]);
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    /* r[7] */    
    /* r[8] */    
    /* r[9] */    
    r[10] = (r[10]);
    r[11] = (r[11]);
    r[12] = (r[12]);
    r[13] = (r[13]);
    r[14] = (r[14]);
    goto s_loop_v8080;
  } else {
    hreserve(hbsz(12+1), 15); /* 15 live regs */
    *--hp = r[0];  
    *--hp = (r[14]);
    *--hp = (r[13]);
    *--hp = r[3];  
    *--hp = (r[12]);
    *--hp = r[2];  
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_case(157);
    r[15] = (hendblk(12+1));
    r[0] = (r[15]);
    r[1] = r[5];  
    r[2] = r[4];  
    goto s_loop_v8095;
  }

s_loop_v8095: /* k id id */
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
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(156);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[3];  
    r[2] = r[4];  
    goto s_loop_v8095;
  }

case 156: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id id */
    r[5] = (car((r[4])));
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[6] = (car((r[3])));
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 157: /* clo ek r */
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
    r[1+12] = p[11];
    r[1+13] = p[12]; }
    r += 1; /* shift reg. wnd */
    /* ek r bk dk sk ek lsd? body loc-n store ienv env id-n k */
    hreserve(hbsz(8+1), 14); /* 14 live regs */
    *--hp = r[7];  
    *--hp = (r[12]);
    *--hp = (r[10]);
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(158);
    r[14] = (hendblk(8+1));
    r[15+0] = (r[13]);
    r[15+1] = r[1];  
    r[15+2] = (r[12]);
    r[15+3] = (r[11]);
    r[15+4] = (r[10]);
    r[15+5] = r[9];  
    r[15+6] = r[8];  
    r[15+7] = (r[14]);
    r += 15; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dsyntax_2Dbindings;

case 158: /* clo k store loc-n */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2];
    r[1+5] = p[3];
    r[1+6] = p[4];
    r[1+7] = p[5];
    r[1+8] = p[6];
    r[1+9] = p[7];
    r[1+10] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* k store loc-n bk dk sk ek lsd? ienv id-n body */
    r[11+0] = r[0];  
    r[11+1] = (r[10]);
    r[11+2] = r[9];  
    r[11+3] = r[8];  
    r[11+4] = r[1];  
    r[11+5] = r[2];  
    r[11+6] = r[7];  
    r[11+7] = r[6];  
    r[11+8] = r[5];  
    r[11+9] = (bool_from_obj(r[7]) ? (r[4]) : obj_from_bool(0));
    r[11+10] = (bool_from_obj(r[7]) ? (r[3]) : obj_from_bool(0));
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dbody;

case 159: /* expand-any k sexp id-n env store loc-n lsd? ek sk dk bk */
    assert(rc == 12);
    r += 1; /* shift reg. wnd */
gs_expand_2Dany: /* k sexp id-n env store loc-n lsd? ek sk dk bk */
    hreserve(hbsz(0+1), 11); /* 11 live regs */
    *--hp = obj_from_case(160);
    r[11] = (hendblk(0+1));
    hreserve(hbsz(12+1), 12); /* 12 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = r[8];  
    *--hp = (r[11]);
    *--hp = r[7];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[6];  
    *--hp = r[9];  
    *--hp = (r[10]);
    *--hp = obj_from_case(161);
    r[11] = (hendblk(12+1));
    r[0] = (r[11]);
    /* r[1] */    
    goto gs_sid_3F;

case 160: /* clo k k sexp name */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
    /* k k sexp name */
  if (bool_from_obj(r[1])) {
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    { /* string-append */
    int *d = stringcat(stringdata((r[3])), stringdata((cx__231533)));
    r[4] = (hpushstr(4, d)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    /* r[0] */    
    r[1] = r[4];  
    r[2] = r[5];  
    goto gs_x_2Derror_2A;
  }

case 161: /* clo ek r */
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
    r[1+12] = p[11];
    r[1+13] = p[12]; }
    r += 1; /* shift reg. wnd */
    /* ek r bk dk lsd? id-n env ek get-k sk loc-n store sexp k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(7+1), 14); /* 14 live regs */
    *--hp = (r[13]);
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = obj_from_case(162);
    r[14] = (hendblk(7+1));
    r[0] = (r[14]);
    r[1] = (r[12]);
    r[2] = r[6];  
    r[3] = (r[11]);
    goto gs_lookup2;
  } else {
  if ((isnull((r[12])))) {
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = (mknull());
    *--hp = (r[12]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[14] = (hendblk(3)); }
    r[0] = (r[13]);
    r[1] = (cx__231563);
    r[2] = (r[14]);
    goto gs_x_2Derror_2A;
  } else {
    hreserve(hbsz(12+1), 14); /* 14 live regs */
    *--hp = (r[13]);
    *--hp = (r[12]);
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[7];  
    *--hp = r[4];  
    *--hp = r[9];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[8];  
    *--hp = obj_from_case(165);
    r[14] = (hendblk(12+1));
    r[0] = (cx_list_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[14]);
    r[2] = (r[12]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }

case 162: /* clo ek r */
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
    /* ek r ek get-k sk loc-n store sexp k */
  if (bool_from_obj(cxs_list2_3F((r[1])))) {
    hreserve(hbsz(5+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[1];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(163);
    r[9] = (hendblk(5+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = r[4];  
    r[3] = r[7];  
    r[4] = (cx__231524);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    hreserve(hbsz(2+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[1];  
    *--hp = obj_from_case(164);
    r[9] = (hendblk(2+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    /* r[2] */    
    r[3] = r[7];  
    r[4] = (cx__231526);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }

case 163: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r loc-n store sexp r k */
    r[7+0] = r[1];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[6];  
    r[7+2] = r[5];  
    r[7+3] = r[4];  
    r[7+4] = r[3];  
    r[7+5] = r[2];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;

case 164: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r r k */
    r[0] = r[1];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = (bool_from_obj(cxs_list1_3F((r[2]))) ? (car((r[2]))) : (r[2]));
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
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9];
    r[1+11] = p[10];
    r[1+12] = p[11];
    r[1+13] = p[12]; }
    r += 1; /* shift reg. wnd */
    /* ek r get-k bk dk sk lsd? ek loc-n store env id-n sexp k */
  if (bool_from_obj(r[1])) {
  if (bool_from_obj(r[7])) {
    hreserve(hbsz(6+1), 14); /* 14 live regs */
    *--hp = (r[12]);
    *--hp = r[7];  
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = obj_from_case(166);
    r[14] = (hendblk(6+1));
  } else {
    r[14] = obj_from_bool(0);
  }
    hreserve(hbsz(11+1), 15); /* 15 live regs */
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[6];  
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = obj_from_case(170);
    r[15] = (hendblk(11+1));
    r[16+0] = (r[13]);
    r[16+1] = (car((r[12])));
    r[16+2] = (r[11]);
    r[16+3] = (r[10]);
    r[16+4] = r[9];  
    r[16+5] = r[8];  
    r[16+6] = obj_from_bool(0);
    r[16+7] = (r[14]);
    r[16+8] = (r[15]);
    r[16+9] = obj_from_bool(0);
    r[16+10] = obj_from_bool(0);
    r += 16; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dany;
  } else {
    r[14] = obj_from_bool((is_fixnum_obj(r[12])) || (is_flonum_obj(r[12])));
    r[14] = (bool_from_obj(r[14]) ? (r[14]) : obj_from_bool((is_bool_obj(r[12])) || ((isstring((r[12]))) || (is_char_obj(r[12])))));
  if (bool_from_obj(r[14])) {
    hreserve(hbsz(9+1), 14); /* 14 live regs */
    *--hp = (r[13]);
    *--hp = (r[11]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[6];  
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(220);
    r[14] = (hendblk(9+1));
    r[0] = (r[14]);
    r[1] = (r[12]);
    goto gs_decompose_2Dliteral;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = (mknull());
    *--hp = (cx__231536);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[14] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 15); /* 15 live regs */
    *--hp = (r[14]);
    *--hp = (r[12]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[14] = (hendblk(3)); }
    r[0] = (r[13]);
    r[1] = ((ispair((r[12]))) ? (cx__231544) : ((isvector((r[12]))) ? (cx__231542) : (cx__231541)));
    r[2] = (r[14]);
    goto gs_x_2Derror_2A;
  }
  }

case 166: /* clo k output */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* k output loc-n store env id-n ek sexp */
  if ((ispair((r[1])))) {
    r[8] = (car((r[1])));
    r[8] = obj_from_bool((mksymbol(internsym("lambda"))) == (r[8]));
  if (bool_from_obj(r[8])) {
    r[8] = (cdr((r[1])));
    r[8] = (car((r[8])));
    r[8] = obj_from_bool(isnull((r[8])));
  if (bool_from_obj(r[8])) {
    r[8] = (cdr((r[7])));
    r[8] = obj_from_bool(isnull((r[8])));
  } else {
    r[8] = obj_from_bool(0);
  }
  } else {
    r[8] = obj_from_bool(0);
  }
  } else {
    r[8] = obj_from_bool(0);
  }
  if (bool_from_obj(r[8])) {
    r[8] = (cdr((r[1])));
    r[8] = (cdr((r[8])));
    r[8] = (car((r[8])));
    r[9+0] = r[6];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[0];  
    r[9+2] = r[8];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[8] = (cdr((r[7])));
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[6];  
    *--hp = obj_from_case(169);
    r[9] = (hendblk(3+1));
    r[0] = r[9];  
    r[1] = r[8];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    goto s_loop_v7995;
  }

s_loop_v7995: /* k id loc-n store env id-n */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[6] = (cdr((r[1])));
    hreserve(hbsz(6+1), 7); /* 7 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(167);
    r[7] = (hendblk(6+1));
    r[0] = r[7];  
    r[1] = r[6];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    goto s_loop_v7995;
  }

case 167: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r loc-n store env id-n id k */
    hreserve(hbsz(2+1), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = r[7];  
    *--hp = obj_from_case(168);
    r[8] = (hendblk(2+1));
    r[9+0] = r[8];  
    r[9+1] = (car((r[6])));
    r[9+2] = r[5];  
    r[9+3] = r[4];  
    r[9+4] = r[3];  
    r[9+5] = r[2];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dexpr;

case 168: /* clo ek r */
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

case 169: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r ek k output */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 170: /* clo k syntax head store loc-n */
    assert(rc == 6);
    { obj* p = objptr_from_obj(r[0]);
    r[1+5] = p[1];
    r[1+6] = p[2];
    r[1+7] = p[3];
    r[1+8] = p[4];
    r[1+9] = p[5];
    r[1+10] = p[6];
    r[1+11] = p[7];
    r[1+12] = p[8];
    r[1+13] = p[9];
    r[1+14] = p[10];
    r[1+15] = p[11]; }
    r += 1; /* shift reg. wnd */
    /* k syntax head store loc-n loc-n store get-k bk dk sk ek lsd? env id-n sexp */
    r[16] = (cdr((r[15])));
    { /* cons */ 
    hreserve(hbsz(3), 17); /* 17 live regs */
    *--hp = (r[16]);
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[17] = (hendblk(3)); }
    { bool_t v9250_tmp;
    r[18] = (car((r[1])));
    r[18] = (car((r[18])));
    v9250_tmp = ((mksymbol(internsym("builtin"))) == (r[18]));
    r[18] = obj_from_bool(!(v9250_tmp)); }
  if (bool_from_obj(r[18])) {
    r[18+0] = r[0];  
    r[18+1] = r[1];  
    r[18+2] = (r[17]);
    r[18+3] = (r[14]);
    r[18+4] = (r[13]);
    r[18+5] = r[3];  
    r[18+6] = r[4];  
    r[18+7] = (r[12]);
    r[18+8] = (r[11]);
    r[18+9] = (r[10]);
    r[18+10] = r[9];  
    r[18+11] = r[8];  
    r += 18; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_apply_2Dexpander;
  } else {
    r[18] = (cdr((r[1])));
    r[18] = (car((r[18])));
    { /* length */
    int n; obj l = (r[16]);
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    r[19] = obj_from_fixnum(n); }
  if (((((r[18]) == (mksymbol(internsym("syntax-rules")))) || ((is_flonum_obj(r[18])) && ((is_flonum_obj(mksymbol(internsym("syntax-rules")))) && (flonum_from_obj(r[18]) == flonum_from_obj(mksymbol(internsym("syntax-rules"))))))) && (1))) {
    hreserve(hbsz(7+1), 20); /* 20 live regs */
    *--hp = r[0];  
    *--hp = (r[17]);
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = (r[10]);
    *--hp = r[7];  
    *--hp = (r[13]);
    *--hp = obj_from_case(171);
    r[20] = (hendblk(7+1));
  if ((fixnum_from_obj(r[19]) < (1))) {
    { /* cons */ 
    hreserve(hbsz(3), 21); /* 21 live regs */
    *--hp = (mknull());
    *--hp = (r[17]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[21] = (hendblk(3)); }
    r[0] = (r[20]);
    r[1] = (cx__231507);
    r[2] = (r[21]);
    goto gs_x_2Derror_2A;
  } else {
    r[21+0] = obj_from_ktrap();
    r[21+1] = obj_from_void(0);
    r[21+2] = (r[13]);
    r[21+3] = r[7];  
    r[21+4] = (r[10]);
    r[21+5] = r[4];  
    r[21+6] = r[3];  
    r[21+7] = (r[17]);
    r[21+8] = r[0];  
    r += 21; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v7960;
  }
  } else {
  if (((((r[18]) == (mksymbol(internsym("syntax-lambda")))) || ((is_flonum_obj(r[18])) && ((is_flonum_obj(mksymbol(internsym("syntax-lambda")))) && (flonum_from_obj(r[18]) == flonum_from_obj(mksymbol(internsym("syntax-lambda"))))))) && (1))) {
    hreserve(hbsz(7+1), 20); /* 20 live regs */
    *--hp = r[0];  
    *--hp = (r[17]);
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = (r[10]);
    *--hp = r[7];  
    *--hp = (r[13]);
    *--hp = obj_from_case(174);
    r[20] = (hendblk(7+1));
  if ((fixnum_from_obj(r[19]) < (2))) {
    { /* cons */ 
    hreserve(hbsz(3), 21); /* 21 live regs */
    *--hp = (mknull());
    *--hp = (r[17]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[21] = (hendblk(3)); }
    r[0] = (r[20]);
    r[1] = (cx__231494);
    r[2] = (r[21]);
    goto gs_x_2Derror_2A;
  } else {
    r[21+0] = obj_from_ktrap();
    r[21+1] = obj_from_void(0);
    r[21+2] = (r[13]);
    r[21+3] = r[7];  
    r[21+4] = (r[10]);
    r[21+5] = r[4];  
    r[21+6] = r[3];  
    r[21+7] = (r[17]);
    r[21+8] = r[0];  
    r += 21; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v7942;
  }
  } else {
  if (((((r[18]) == (mksymbol(internsym("begin")))) || ((is_flonum_obj(r[18])) && ((is_flonum_obj(mksymbol(internsym("begin")))) && (flonum_from_obj(r[18]) == flonum_from_obj(mksymbol(internsym("begin"))))))) && (1))) {
  if (bool_from_obj(r[8])) {
    r[20+0] = r[8];  
    pc = objptr_from_obj(r[20+0])[0];
    r[20+1] = r[0];  
    r[20+2] = (r[17]);
    r[20+3] = (r[14]);
    r[20+4] = (r[13]);
    r[20+5] = r[3];  
    r[20+6] = r[4];  
    r += 20; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 7);
    goto jump;
  } else {
  if ((isnull((r[16])))) {
    { /* cons */ 
    hreserve(hbsz(3), 20); /* 20 live regs */
    *--hp = (mknull());
    *--hp = (r[17]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[20] = (hendblk(3)); }
    /* r[0] */    
    r[1] = (cx__231481);
    r[2] = (r[20]);
    goto gs_x_2Derror_2A;
  } else {
    hreserve(hbsz(1+1), 20); /* 20 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(179);
    r[20] = (hendblk(1+1));
    r[0] = (r[20]);
    r[1] = (r[16]);
    r[2] = r[5];  
    r[3] = r[6];  
    r[4] = (r[13]);
    r[5] = (r[14]);
    goto s_loop_v7924;
  }
  }
  } else {
  if (((((r[18]) == (mksymbol(internsym("define")))) || ((is_flonum_obj(r[18])) && ((is_flonum_obj(mksymbol(internsym("define")))) && (flonum_from_obj(r[18]) == flonum_from_obj(mksymbol(internsym("define"))))))) ? (1) : ((((r[18]) == (mksymbol(internsym("define-syntax")))) || ((is_flonum_obj(r[18])) && ((is_flonum_obj(mksymbol(internsym("define-syntax")))) && (flonum_from_obj(r[18]) == flonum_from_obj(mksymbol(internsym("define-syntax"))))))) && (1)))) {
    hreserve(hbsz(10+1), 20); /* 20 live regs */
    *--hp = r[0];  
    *--hp = (r[18]);
    *--hp = (r[17]);
    *--hp = (r[14]);
    *--hp = (r[13]);
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[9];  
    *--hp = r[7];  
    *--hp = (r[19]);
    *--hp = obj_from_case(180);
    r[20] = (hendblk(10+1));
  if (((2) == fixnum_from_obj(r[19]))) {
    r[0] = (r[20]);
    r[1] = (car((r[16])));
    goto gs_sid_3F;
  } else {
    r[21+0] = obj_from_ktrap();
    r[21+1] = obj_from_bool(0);
    r[21+2] = (r[19]);
    r[21+3] = r[7];  
    r[21+4] = r[9];  
    r[21+5] = r[4];  
    r[21+6] = r[3];  
    r[21+7] = (r[13]);
    r[21+8] = (r[14]);
    r[21+9] = (r[17]);
    r[21+10] = (r[18]);
    r[21+11] = r[0];  
    r += 21; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v7901;
  }
  } else {
    hreserve(hbsz(11+1), 20); /* 20 live regs */
    *--hp = (r[17]);
    *--hp = (r[18]);
    *--hp = r[0];  
    *--hp = (r[16]);
    *--hp = (r[14]);
    *--hp = (r[13]);
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = (r[19]);
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(183);
    r[20] = (hendblk(11+1));
    r[21+0] = r[7];  
    pc = objptr_from_obj(r[21+0])[0];
    r[21+1] = (r[20]);
    r[21+2] = (r[11]);
    r[21+3] = (r[17]);
    r[21+4] = (cx__231526);
    r += 21; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }
  }
  }
  }
  }

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
s_l_v7960: /* ek  env get-k sk loc-n store sexp k */
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(172);
    r[9] = (hendblk(6+1));
    r[0] = r[9];  
    r[1] = r[7];  
    /* r[2] */    
    goto gs_compile_2Dsyntax_2Drules;

case 172: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r get-k sk loc-n store sexp k */
    hreserve(hbsz(5+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(173);
    r[8] = (hendblk(5+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = r[3];  
    r[3] = r[6];  
    r[4] = (cx__231524);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 173: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r loc-n store sexp r k */
    r[7+0] = r[1];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[6];  
    r[7+2] = r[5];  
    r[7+3] = r[4];  
    r[7+4] = r[3];  
    r[7+5] = r[2];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;

case 174: /* clo ek  */
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
s_l_v7942: /* ek  env get-k sk loc-n store sexp k */
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(175);
    r[9] = (hendblk(6+1));
    r[0] = r[9];  
    r[1] = r[7];  
    /* r[2] */    
    goto gs_compile_2Dsyntax_2Dlambda;

case 175: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r get-k sk loc-n store sexp k */
    hreserve(hbsz(5+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(176);
    r[8] = (hendblk(5+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = r[3];  
    r[3] = r[6];  
    r[4] = (cx__231524);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 176: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r loc-n store sexp r k */
    r[7+0] = r[1];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[6];  
    r[7+2] = r[5];  
    r[7+3] = r[4];  
    r[7+4] = r[3];  
    r[7+5] = r[2];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;

s_loop_v7924: /* k id loc-n store env id-n */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[6] = (cdr((r[1])));
    hreserve(hbsz(6+1), 7); /* 7 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(177);
    r[7] = (hendblk(6+1));
    r[0] = r[7];  
    r[1] = r[6];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    goto s_loop_v7924;
  }

case 177: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r loc-n store env id-n id k */
    hreserve(hbsz(2+1), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = r[7];  
    *--hp = obj_from_case(178);
    r[8] = (hendblk(2+1));
    r[9+0] = r[8];  
    r[9+1] = (car((r[6])));
    r[9+2] = r[5];  
    r[9+3] = r[4];  
    r[9+4] = r[3];  
    r[9+5] = r[2];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dexpr;

case 178: /* clo ek r */
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

case 179: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    r[0] = r[2];  
    /* r[1] */    
    goto gs_make_2Dbegin;

case 180: /* clo ek r */
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
s_l_v7901: /* ek r len get-k dk loc-n store env id-n sexp builtin k */
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
    *--hp = obj_from_case(181);
    r[12] = (hendblk(9+1));
  if (bool_from_obj(r[1])) {
    r[0] = obj_from_ktrap();
    /* r[1] */    
    r[2] = r[3];  
    r[3] = r[4];  
    r[4] = r[5];  
    r[5] = r[6];  
    r[6] = r[7];  
    r[7] = r[8];  
    r[8] = r[9];  
    r[9] = (r[10]);
    r[10] = (r[11]);
    goto s_l_v7903;
  } else {
    r[13] = obj_from_bool(((1) == fixnum_from_obj(r[2])) && ((r[10]) == (mksymbol(internsym("define")))));
  if (bool_from_obj(r[13])) {
    r[0] = obj_from_ktrap();
    r[1] = (r[13]);
    r[2] = r[3];  
    r[3] = r[4];  
    r[4] = r[5];  
    r[5] = r[6];  
    r[6] = r[7];  
    r[7] = r[8];  
    r[8] = r[9];  
    r[9] = (r[10]);
    r[10] = (r[11]);
    goto s_l_v7903;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = (mknull());
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[14] = (hendblk(3)); }
    r[0] = (r[12]);
    r[1] = (cx__231452);
    r[2] = (r[14]);
    goto gs_x_2Derror_2A;
  }
  }

case 181: /* clo ek  */
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
s_l_v7903: /* ek  get-k dk loc-n store env id-n sexp builtin k */
    hreserve(hbsz(7+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(182);
    r[11] = (hendblk(7+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[11]);
    r[2] = r[3];  
    r[3] = r[8];  
    r[4] = (cx__231522);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 182: /* clo ek r */
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
    /* ek r loc-n store env id-n sexp builtin k */
    r[9+0] = r[1];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[8];  
    r[9+2] = r[7];  
    r[9+3] = r[6];  
    r[9+4] = r[5];  
    r[9+5] = r[4];  
    r[9+6] = r[3];  
    r[9+7] = r[2];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 8);
    goto jump;

case 183: /* clo ek r */
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
    /* ek r loc-n store len loc-n store env id-n tail k builtin sexp */
    { const fixnum_t v9249_len = fixnum_from_obj(r[4]);
    hreserve(hbsz(2+1), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = obj_from_case(184);
    r[13] = (hendblk(2+1));
  if (((((r[11]) == (mksymbol(internsym("lambda")))) || ((is_flonum_obj(r[11])) && ((is_flonum_obj(mksymbol(internsym("lambda")))) && (flonum_from_obj(r[11]) == flonum_from_obj(mksymbol(internsym("lambda"))))))) && (1))) {
    hreserve(hbsz(7+1), 14); /* 14 live regs */
    *--hp = (r[10]);
    *--hp = r[1];  
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(185);
    r[14] = (hendblk(7+1));
    r[0] = (r[14]);
    r[1] = obj_from_bool((v9249_len) == (2));
    r[2] = (r[11]);
    r[3] = (r[12]);
    goto s_l_v7891;
  } else {
  if (((((r[11]) == (mksymbol(internsym("quote")))) || ((is_flonum_obj(r[11])) && ((is_flonum_obj(mksymbol(internsym("quote")))) && (flonum_from_obj(r[11]) == flonum_from_obj(mksymbol(internsym("quote"))))))) && (1))) {
    hreserve(hbsz(6+1), 14); /* 14 live regs */
    *--hp = (r[10]);
    *--hp = r[1];  
    *--hp = r[8];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[9];  
    *--hp = obj_from_case(187);
    r[14] = (hendblk(6+1));
    r[0] = (r[14]);
    r[1] = obj_from_bool((v9249_len) == (1));
    r[2] = (r[11]);
    r[3] = (r[12]);
    goto s_l_v7891;
  } else {
  if (((((r[11]) == (mksymbol(internsym("set!")))) || ((is_flonum_obj(r[11])) && ((is_flonum_obj(mksymbol(internsym("set!")))) && (flonum_from_obj(r[11]) == flonum_from_obj(mksymbol(internsym("set!"))))))) && (1))) {
    hreserve(hbsz(10+1), 14); /* 14 live regs */
    *--hp = (r[10]);
    *--hp = r[1];  
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = (r[12]);
    *--hp = r[6];  
    *--hp = (r[13]);
    *--hp = obj_from_case(191);
    r[14] = (hendblk(10+1));
  if (((v9249_len) == (2))) {
    r[0] = (r[14]);
    r[1] = (car((r[9])));
    goto gs_sid_3F;
  } else {
    r[15+0] = obj_from_ktrap();
    r[15+1] = obj_from_bool(0);
    r[15+2] = (r[13]);
    r[15+3] = r[6];  
    r[15+4] = (r[12]);
    r[15+5] = r[2];  
    r[15+6] = r[3];  
    r[15+7] = r[7];  
    r[15+8] = r[8];  
    r[15+9] = r[9];  
    r[15+10] = r[1];  
    r[15+11] = (r[10]);
    r += 15; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v7825;
  }
  } else {
  if (((((r[11]) == (mksymbol(internsym("if")))) || ((is_flonum_obj(r[11])) && ((is_flonum_obj(mksymbol(internsym("if")))) && (flonum_from_obj(r[11]) == flonum_from_obj(mksymbol(internsym("if"))))))) && (1))) {
    hreserve(hbsz(7+1), 14); /* 14 live regs */
    *--hp = (r[10]);
    *--hp = r[1];  
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(196);
    r[14] = (hendblk(7+1));
    r[0] = (r[14]);
    r[1] = obj_from_bool(((2) <= (v9249_len)) && ((v9249_len) <= (3)));
    r[2] = (r[11]);
    r[3] = (r[12]);
    goto s_l_v7891;
  } else {
  if (((((r[11]) == (mksymbol(internsym("letcc")))) || ((is_flonum_obj(r[11])) && ((is_flonum_obj(mksymbol(internsym("letcc")))) && (flonum_from_obj(r[11]) == flonum_from_obj(mksymbol(internsym("letcc"))))))) && (1))) {
    hreserve(hbsz(7+1), 14); /* 14 live regs */
    *--hp = (r[10]);
    *--hp = r[1];  
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(200);
    r[14] = (hendblk(7+1));
    r[0] = (r[14]);
    r[1] = obj_from_bool((v9249_len) == (2));
    r[2] = (r[11]);
    r[3] = (r[12]);
    goto s_l_v7891;
  } else {
  if (((((r[11]) == (mksymbol(internsym("withcc")))) || ((is_flonum_obj(r[11])) && ((is_flonum_obj(mksymbol(internsym("withcc")))) && (flonum_from_obj(r[11]) == flonum_from_obj(mksymbol(internsym("withcc"))))))) && (1))) {
    hreserve(hbsz(7+1), 14); /* 14 live regs */
    *--hp = (r[10]);
    *--hp = r[1];  
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(202);
    r[14] = (hendblk(7+1));
    r[0] = (r[14]);
    r[1] = obj_from_bool((v9249_len) == (2));
    r[2] = (r[11]);
    r[3] = (r[12]);
    goto s_l_v7891;
  } else {
  if (((((r[11]) == (mksymbol(internsym("%quote")))) || ((is_flonum_obj(r[11])) && ((is_flonum_obj(mksymbol(internsym("%quote")))) && (flonum_from_obj(r[11]) == flonum_from_obj(mksymbol(internsym("%quote"))))))) && (1))) {
    hreserve(hbsz(7+1), 14); /* 14 live regs */
    *--hp = (r[10]);
    *--hp = r[1];  
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(206);
    r[14] = (hendblk(7+1));
    r[0] = (r[14]);
    r[1] = obj_from_bool((v9249_len) == (1));
    r[2] = (r[11]);
    r[3] = (r[12]);
    goto s_l_v7891;
  } else {
  if (((((r[11]) == (mksymbol(internsym("%const")))) || ((is_flonum_obj(r[11])) && ((is_flonum_obj(mksymbol(internsym("%const")))) && (flonum_from_obj(r[11]) == flonum_from_obj(mksymbol(internsym("%const"))))))) && (1))) {
    hreserve(hbsz(3+1), 14); /* 14 live regs */
    *--hp = r[9];  
    *--hp = (r[10]);
    *--hp = r[1];  
    *--hp = obj_from_case(208);
    r[14] = (hendblk(3+1));
    r[0] = (r[14]);
    r[1] = obj_from_bool((v9249_len) >= (1));
    r[2] = (r[11]);
    r[3] = (r[12]);
    goto s_l_v7891;
  } else {
  if (((((r[11]) == (mksymbol(internsym("%definition")))) || ((is_flonum_obj(r[11])) && ((is_flonum_obj(mksymbol(internsym("%definition")))) && (flonum_from_obj(r[11]) == flonum_from_obj(mksymbol(internsym("%definition"))))))) ? (1) : ((((r[11]) == (mksymbol(internsym("%localdef")))) || ((is_flonum_obj(r[11])) && ((is_flonum_obj(mksymbol(internsym("%localdef")))) && (flonum_from_obj(r[11]) == flonum_from_obj(mksymbol(internsym("%localdef"))))))) && (1)))) {
  if (((v9249_len) == (1))) {
    r[14] = (car((r[9])));
    r[14] = obj_from_bool(isstring((r[14])));
  } else {
    r[14] = obj_from_bool(0);
  }
    hreserve(hbsz(4+1), 15); /* 15 live regs */
    *--hp = (r[11]);
    *--hp = r[9];  
    *--hp = (r[10]);
    *--hp = r[1];  
    *--hp = obj_from_case(209);
    r[15] = (hendblk(4+1));
    r[0] = (r[15]);
    r[1] = (r[14]);
    r[2] = (r[11]);
    r[3] = (r[12]);
    goto s_l_v7891;
  } else {
  if (((((r[11]) == (mksymbol(internsym("%prim")))) || ((is_flonum_obj(r[11])) && ((is_flonum_obj(mksymbol(internsym("%prim")))) && (flonum_from_obj(r[11]) == flonum_from_obj(mksymbol(internsym("%prim"))))))) ? (1) : ((((r[11]) == (mksymbol(internsym("%prim?")))) || ((is_flonum_obj(r[11])) && ((is_flonum_obj(mksymbol(internsym("%prim?")))) && (flonum_from_obj(r[11]) == flonum_from_obj(mksymbol(internsym("%prim?"))))))) ? (1) : ((((r[11]) == (mksymbol(internsym("%prim!")))) || ((is_flonum_obj(r[11])) && ((is_flonum_obj(mksymbol(internsym("%prim!")))) && (flonum_from_obj(r[11]) == flonum_from_obj(mksymbol(internsym("%prim!"))))))) ? (1) : ((((r[11]) == (mksymbol(internsym("%prim?!")))) || ((is_flonum_obj(r[11])) && ((is_flonum_obj(mksymbol(internsym("%prim?!")))) && (flonum_from_obj(r[11]) == flonum_from_obj(mksymbol(internsym("%prim?!"))))))) ? (1) : ((((r[11]) == (mksymbol(internsym("%prim*")))) || ((is_flonum_obj(r[11])) && ((is_flonum_obj(mksymbol(internsym("%prim*")))) && (flonum_from_obj(r[11]) == flonum_from_obj(mksymbol(internsym("%prim*"))))))) ? (1) : ((((r[11]) == (mksymbol(internsym("%prim*!")))) || ((is_flonum_obj(r[11])) && ((is_flonum_obj(mksymbol(internsym("%prim*!")))) && (flonum_from_obj(r[11]) == flonum_from_obj(mksymbol(internsym("%prim*!"))))))) ? (1) : ((((r[11]) == (mksymbol(internsym("%prim*?")))) || ((is_flonum_obj(r[11])) && ((is_flonum_obj(mksymbol(internsym("%prim*?")))) && (flonum_from_obj(r[11]) == flonum_from_obj(mksymbol(internsym("%prim*?"))))))) ? (1) : ((((r[11]) == (mksymbol(internsym("%prim*?!")))) || ((is_flonum_obj(r[11])) && ((is_flonum_obj(mksymbol(internsym("%prim*?!")))) && (flonum_from_obj(r[11]) == flonum_from_obj(mksymbol(internsym("%prim*?!"))))))) && (1)))))))))) {
    hreserve(hbsz(9+1), 14); /* 14 live regs */
    *--hp = r[9];  
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[1];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = (r[13]);
    *--hp = obj_from_case(210);
    r[14] = (hendblk(9+1));
  if (((v9249_len) >= (1))) {
    r[15] = (car((r[9])));
    r[0] = (r[14]);
    r[1] = (r[15]);
    goto s_ok_3F;
  } else {
    r[15+0] = obj_from_ktrap();
    r[15+1] = obj_from_bool(0);
    r[15+2] = (r[13]);
    r[15+3] = r[2];  
    r[15+4] = r[3];  
    r[15+5] = r[7];  
    r[15+6] = r[8];  
    r[15+7] = r[1];  
    r[15+8] = (r[10]);
    r[15+9] = (r[11]);
    r[15+10] = r[9];  
    r += 15; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v7696;
  }
  } else {
    r[0] = r[1];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
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
  } } 

case 184: /* clo k test */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
s_l_v7891: /* k test builtin sexp */
  if (bool_from_obj(r[1])) {
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (cx__231191);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    /* r[0] */    
    r[1] = (cx__231196);
    r[2] = r[4];  
    goto gs_x_2Derror_2A;
  }

case 185: /* clo ek  */
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
    /* ek  loc-n store env id-n tail r k */
    hreserve(hbsz(2+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = obj_from_case(186);
    r[9] = (hendblk(2+1));
    r[10] = (cdr((r[6])));
    r[10] = (car((r[10])));
    r[11+0] = r[9];  
    r[11+1] = (car((r[6])));
    r[11+2] = (r[10]);
    r[11+3] = r[5];  
    r[11+4] = r[4];  
    r[11+5] = r[3];  
    r[11+6] = r[2];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dlambda;

case 186: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r r k */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("lambda")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = r[4];  
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
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek  tail loc-n store id-n r k */
    hreserve(hbsz(5+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(188);
    r[8] = (hendblk(5+1));
    r[0] = r[8];  
    r[1] = (car((r[2])));
    goto gs_unwrap_2Dvecs;

case 188: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r loc-n store id-n r k */
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(189);
    r[7] = (hendblk(5+1));
    r[0] = r[7];  
    /* r[1] */    
    goto gs_decompose_2Dliteral;

case 189: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r loc-n store id-n r k */
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(190);
    r[7] = (hendblk(2+1));
    r[8+0] = r[7];  
    r[8+1] = r[1];  
    r[8+2] = r[4];  
    r[8+3] = (mknull());
    r[8+4] = r[3];  
    r[8+5] = r[2];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dexpr;

case 190: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r r k */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("%quote")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 191: /* clo ek r */
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
s_l_v7825: /* ek r expr-assert store sexp loc-n store env id-n tail r k */
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
    r[13+0] = r[2];  
    pc = objptr_from_obj(r[13+0])[0];
    r[13+1] = (r[12]);
    r[13+2] = r[1];  
    r += 13; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

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
    /* ek  store sexp loc-n store env id-n tail r k */
    hreserve(hbsz(8+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(193);
    r[11] = (hendblk(8+1));
    r[12+0] = (r[11]);
    r[12+1] = (car((r[8])));
    r[12+2] = r[6];  
    r[12+3] = r[2];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_lookup2;

case 193: /* clo ek r */
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
    /* ek r sexp loc-n store env id-n tail r k */
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[1];  
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(194);
    r[10] = (hendblk(8+1));
  if ((issymbol((r[1])))) {
    r[11+0] = obj_from_ktrap();
    r[11+1] = obj_from_bool(issymbol((r[1])));
    r[11+2] = r[3];  
    r[11+3] = r[4];  
    r[11+4] = r[5];  
    r[11+5] = r[6];  
    r[11+6] = r[7];  
    r[11+7] = r[8];  
    r[11+8] = r[9];  
    r[11+9] = r[1];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v7831;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    r[0] = (r[10]);
    r[1] = (cx__231403);
    r[2] = (r[11]);
    goto gs_x_2Derror_2A;
  }

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
s_l_v7831: /* ek  loc-n store env id-n tail r k r */
    hreserve(hbsz(3+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = obj_from_case(195);
    r[10] = (hendblk(3+1));
    r[11] = (cdr((r[6])));
    r[11] = (car((r[11])));
    r[12+0] = (r[10]);
    r[12+1] = (r[11]);
    r[12+2] = r[5];  
    r[12+3] = r[4];  
    r[12+4] = r[3];  
    r[12+5] = r[2];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dexpr;

case 195: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r r k r */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = (mksymbol(internsym("set!")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = r[7];  
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
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  loc-n store env id-n tail r k */
    hreserve(hbsz(2+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = obj_from_case(199);
    r[9] = (hendblk(2+1));
    r[0] = r[9];  
    r[1] = r[6];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    goto s_loop_v7803;

s_loop_v7803: /* k id loc-n store env id-n */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[6] = (cdr((r[1])));
    hreserve(hbsz(6+1), 7); /* 7 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(197);
    r[7] = (hendblk(6+1));
    r[0] = r[7];  
    r[1] = r[6];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    goto s_loop_v7803;
  }

case 197: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r loc-n store env id-n id k */
    hreserve(hbsz(2+1), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = r[7];  
    *--hp = obj_from_case(198);
    r[8] = (hendblk(2+1));
    r[9+0] = r[8];  
    r[9+1] = (car((r[6])));
    r[9+2] = r[5];  
    r[9+3] = r[4];  
    r[9+4] = r[3];  
    r[9+5] = r[2];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dexpr;

case 198: /* clo ek r */
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

case 199: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r r k */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("if")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 200: /* clo ek  */
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
    /* ek  loc-n store env id-n tail r k */
    hreserve(hbsz(2+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = obj_from_case(201);
    r[9] = (hendblk(2+1));
    r[10] = (cdr((r[6])));
    r[10] = (car((r[10])));
    r[11+0] = r[9];  
    r[11+1] = (car((r[6])));
    r[11+2] = (r[10]);
    r[11+3] = r[5];  
    r[11+4] = r[4];  
    r[11+5] = r[3];  
    r[11+6] = r[2];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dletcc;

case 201: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r r k */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("letcc")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 202: /* clo ek  */
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
    /* ek  loc-n store env id-n tail r k */
    hreserve(hbsz(2+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = obj_from_case(205);
    r[9] = (hendblk(2+1));
    r[0] = r[9];  
    r[1] = r[6];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    goto s_loop_v7762;

s_loop_v7762: /* k id loc-n store env id-n */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[6] = (cdr((r[1])));
    hreserve(hbsz(6+1), 7); /* 7 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(203);
    r[7] = (hendblk(6+1));
    r[0] = r[7];  
    r[1] = r[6];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    goto s_loop_v7762;
  }

case 203: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r loc-n store env id-n id k */
    hreserve(hbsz(2+1), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = r[7];  
    *--hp = obj_from_case(204);
    r[8] = (hendblk(2+1));
    r[9+0] = r[8];  
    r[9+1] = (car((r[6])));
    r[9+2] = r[5];  
    r[9+3] = r[4];  
    r[9+4] = r[3];  
    r[9+5] = r[2];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dexpr;

case 204: /* clo ek r */
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

case 205: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r r k */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = (mksymbol(internsym("withcc")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 206: /* clo ek  */
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
    /* ek  loc-n store env id-n tail r k */
    hreserve(hbsz(2+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = obj_from_case(207);
    r[9] = (hendblk(2+1));
    r[10+0] = r[9];  
    r[10+1] = (car((r[6])));
    r[10+2] = r[5];  
    r[10+3] = r[4];  
    r[10+4] = r[3];  
    r[10+5] = r[2];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dexpr;

case 207: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r r k */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("%quote")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 208: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  r k tail */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("%const")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 209: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek  r k tail builtin */
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 210: /* clo ek r */
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
s_l_v7696: /* ek r expr-assert loc-n store env id-n r k builtin tail */
    hreserve(hbsz(8+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(211);
    r[11] = (hendblk(8+1));
    r[12+0] = r[2];  
    pc = objptr_from_obj(r[12+0])[0];
    r[12+1] = (r[11]);
    r[12+2] = r[1];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 211: /* clo ek  */
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
    /* ek  loc-n store env id-n r k builtin tail */
    r[10] = (cdr((r[9])));
    hreserve(hbsz(4+1), 11); /* 11 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_case(214);
    r[11] = (hendblk(4+1));
    r[0] = (r[11]);
    r[1] = (r[10]);
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    goto s_loop_v7705;

s_loop_v7705: /* k id loc-n store env id-n */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[6] = (cdr((r[1])));
    hreserve(hbsz(6+1), 7); /* 7 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(212);
    r[7] = (hendblk(6+1));
    r[0] = r[7];  
    r[1] = r[6];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    goto s_loop_v7705;
  }

case 212: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r loc-n store env id-n id k */
    hreserve(hbsz(2+1), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = r[7];  
    *--hp = obj_from_case(213);
    r[8] = (hendblk(2+1));
    r[9+0] = r[8];  
    r[9+1] = (car((r[6])));
    r[9+2] = r[5];  
    r[9+3] = r[4];  
    r[9+4] = r[3];  
    r[9+5] = r[2];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dexpr;

case 213: /* clo ek r */
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

case 214: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r r k builtin tail */
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = (car((r[5])));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 215: /* clo k sf */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
s_ok_3F: /* k sf */
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(215);
    r[2] = (hendblk(0+1));
  if ((isstring((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isstring((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(216);
    r[3] = (hendblk(3+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(217);
    r[3] = (hendblk(3+1));
    r[4+0] = (cx_list_3F);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 216: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r ok? sf k */
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
  if ((isvector((r[3])))) {
    { /* vector->list */
    obj v, l = mknull(); int c = (vectorlen((r[3])));
    hreserve(hbsz(3)*c, 5); /* 5 live regs */
    v = r[3];   /* gc-safe */
    while (c-- > 0) { *--hp = l; *--hp = hblkref(v, 1+c);
    *--hp = obj_from_size(PAIR_BTAG); l = hendblk(3); }
    r[5] = (l); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = r[5];  
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
  }

case 217: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r ok? sf k */
  if (bool_from_obj(r[1])) {
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v7670;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 218: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7670: /* k id ok? */
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
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(218);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(219);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 219: /* clo ek r */
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

case 220: /* clo ek r */
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
    /* ek r bk dk sk ek lsd? loc-n store id-n k */
    hreserve(hbsz(1+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = obj_from_case(221);
    r[11] = (hendblk(1+1));
    r[12+0] = (r[11]);
    r[12+1] = r[1];  
    r[12+2] = r[9];  
    r[12+3] = (mknull());
    r[12+4] = r[8];  
    r[12+5] = r[7];  
    r[12+6] = r[6];  
    r[12+7] = r[5];  
    r[12+8] = r[4];  
    r[12+9] = r[3];  
    r[12+10] = r[2];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dany;

case 221: /* clo ek r */
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
    *--hp = (mksymbol(internsym("%quote")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 222: /* expand-val k sexp id-n env store loc-n k */
    assert(rc == 8);
    r += 1; /* shift reg. wnd */
gs_expand_2Dval: /* k sexp id-n env store loc-n k */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = obj_from_case(223);
    r[7] = (hendblk(3+1));
    hreserve(hbsz(1+1), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = obj_from_case(224);
    r[8] = (hendblk(1+1));
    r[9+0] = r[0];  
    r[9+1] = r[1];  
    r[9+2] = r[2];  
    r[9+3] = r[3];  
    r[9+4] = r[4];  
    r[9+5] = r[5];  
    r[9+6] = obj_from_bool(0);
    r[9+7] = r[7];  
    r[9+8] = r[8];  
    r[9+9] = obj_from_bool(0);
    r[9+10] = obj_from_bool(0);
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dany;

case 223: /* clo k output */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* k output k loc-n store */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[0];  
    r[6+2] = r[5];  
    r[6+3] = r[4];  
    r[6+4] = r[3];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 224: /* clo k syn error-sexp store loc-n */
    assert(rc == 6);
    { obj* p = objptr_from_obj(r[0]);
    r[1+5] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k syn error-sexp store loc-n k */
    r[6+0] = r[5];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[0];  
    r[6+2] = r[1];  
    r[6+3] = r[3];  
    r[6+4] = r[4];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 225: /* expand-expr k sexp id-n env store loc-n */
    assert(rc == 7);
    r += 1; /* shift reg. wnd */
gs_expand_2Dexpr: /* k sexp id-n env store loc-n */
    hreserve(hbsz(0+1), 6); /* 6 live regs */
    *--hp = obj_from_case(226);
    r[6] = (hendblk(0+1));
    r[7+0] = r[0];  
    r[7+1] = r[1];  
    r[7+2] = r[2];  
    r[7+3] = r[3];  
    r[7+4] = r[4];  
    r[7+5] = r[5];  
    r[7+6] = obj_from_bool(0);
    r[7+7] = r[6];  
    r[7+8] = obj_from_bool(0);
    r[7+9] = obj_from_bool(0);
    r[7+10] = obj_from_bool(0);
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dany;

case 226: /* clo k x */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k x */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = r[1];  
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 227: /* expand-body k sexps id-n env store loc-n lsd? ek sk dk bk */
    assert(rc == 12);
    r += 1; /* shift reg. wnd */
gs_expand_2Dbody: /* k sexps id-n env store loc-n lsd? ek sk dk bk */
    r[11] = (car((r[1])));
    r[12] = (cdr((r[1])));
    hreserve(hbsz(0+1), 13); /* 13 live regs */
    *--hp = obj_from_case(247);
    r[13] = (hendblk(0+1));
    r[14+0] = r[0];  
    r[14+1] = (r[11]);
    r[14+2] = (r[12]);
    r[14+3] = (mknull());
    r[14+4] = (mknull());
    r[14+5] = (bool_from_obj(r[7]) ? (mknull()) : obj_from_bool(0));
    r[14+6] = r[2];  
    r[14+7] = r[3];  
    r[14+8] = r[4];  
    r[14+9] = r[5];  
    r[14+10] = (r[13]);
    r[14+11] = (r[10]);
    r[14+12] = r[9];  
    r[14+13] = r[8];  
    r[14+14] = r[7];  
    r[14+15] = r[6];  
    r += 14; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v7395;

case 228: /* clo k first rest vds sds exprs id-n env store loc-n */
    assert(rc == 11);
    { obj* p = objptr_from_obj(r[0]);
    r[1+10] = p[1];
    r[1+11] = p[2];
    r[1+12] = p[3];
    r[1+13] = p[4];
    r[1+14] = p[5];
    r[1+15] = p[6]; }
    r += 1; /* shift reg. wnd */
s_loop_v7395: /* k first rest vds sds exprs id-n env store loc-n expand-def bk dk sk ek lsd? */
    hreserve(hbsz(14+1), 16); /* 16 live regs */
    *--hp = r[6];  
    *--hp = r[7];  
    *--hp = r[8];  
    *--hp = r[9];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = (r[15]);
    *--hp = (r[14]);
    *--hp = (r[13]);
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = r[4];  
    *--hp = obj_from_case(229);
    r[16] = (hendblk(14+1));
  if ((isnull((r[2])))) {
    r[17+0] = r[0];  
    r[17+1] = obj_from_bool(0);
    r[17+2] = r[4];  
    r[17+3] = (r[11]);
    r[17+4] = (r[12]);
    r[17+5] = (r[13]);
    r[17+6] = (r[14]);
    r[17+7] = (r[15]);
    r[17+8] = r[1];  
    r[17+9] = r[5];  
    r[17+10] = r[3];  
    r[17+11] = r[2];  
    r[17+12] = r[9];  
    r[17+13] = r[8];  
    r[17+14] = r[7];  
    r[17+15] = r[6];  
    r += 17; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v7415;
  } else {
    hreserve(hbsz(6+1), 17); /* 17 live regs */
    *--hp = (r[15]);
    *--hp = (r[14]);
    *--hp = (r[13]);
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = obj_from_case(228);
    r[17] = (hendblk(6+1));
    hreserve(hbsz(2+1), 18); /* 18 live regs */
    *--hp = r[2];  
    *--hp = (r[17]);
    *--hp = obj_from_case(246);
    r[17] = (hendblk(2+1));
    r[18+0] = (r[10]);
    pc = objptr_from_obj(r[18+0])[0];
    r[18+1] = r[0];  
    r[18+2] = r[1];  
    r[18+3] = r[3];  
    r[18+4] = r[4];  
    r[18+5] = r[5];  
    r[18+6] = r[6];  
    r[18+7] = r[7];  
    r[18+8] = r[8];  
    r[18+9] = r[9];  
    r[18+10] = (r[17]);
    r[18+11] = (bool_from_obj(r[14]) ? (r[16]) : obj_from_bool(0));
    r += 18; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 12);
    goto jump;
  }

case 229: /* clo k boundary-exp-output */
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
    r[1+12] = p[11];
    r[1+13] = p[12];
    r[1+14] = p[13];
    r[1+15] = p[14]; }
    r += 1; /* shift reg. wnd */
s_l_v7415: /* k boundary-exp-output sds bk dk sk ek lsd? first exprs vds rest loc-n store env id-n */
    hreserve(hbsz(15+1), 16); /* 16 live regs */
    *--hp = r[0];  
    *--hp = (r[15]);
    *--hp = (r[14]);
    *--hp = (r[13]);
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
    *--hp = r[1];  
    *--hp = obj_from_case(231);
    r[16] = (hendblk(15+1));
    r[0] = (r[16]);
    r[1] = r[2];  
    goto s_loop_v7523;

s_loop_v7523: /* k id */
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
    *--hp = obj_from_case(230);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v7523;
  }

case 230: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    r[4] = (cdr((r[4])));
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

case 231: /* clo ek r */
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
    r[1+12] = p[11];
    r[1+13] = p[12];
    r[1+14] = p[13];
    r[1+15] = p[14];
    r[1+16] = p[15]; }
    r += 1; /* shift reg. wnd */
    /* ek r boundary-exp-output bk dk sk ek lsd? first exprs vds rest loc-n store env id-n k */
    hreserve(hbsz(12+1), 17); /* 17 live regs */
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = (r[15]);
    *--hp = (r[14]);
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(232);
    r[17] = (hendblk(12+1));
    r[18+0] = (r[16]);
    r[18+1] = r[1];  
    r[18+2] = (r[15]);
    r[18+3] = (r[14]);
    r[18+4] = (r[14]);
    r[18+5] = (r[13]);
    r[18+6] = (r[12]);
    r[18+7] = (r[17]);
    r += 18; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dsyntax_2Dbindings;

case 232: /* clo k store loc-n */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2];
    r[1+5] = p[3];
    r[1+6] = p[4];
    r[1+7] = p[5];
    r[1+8] = p[6];
    r[1+9] = p[7];
    r[1+10] = p[8];
    r[1+11] = p[9];
    r[1+12] = p[10];
    r[1+13] = p[11];
    r[1+14] = p[12]; }
    r += 1; /* shift reg. wnd */
    /* k store loc-n boundary-exp-output bk dk sk ek lsd? env id-n first exprs vds rest */
  if (((isnull((r[14]))) && ((isnull((r[13]))) && (!(ispair((r[12]))))))) {
    r[15+0] = r[0];  
    r[15+1] = (r[11]);
    r[15+2] = (r[10]);
    r[15+3] = r[9];  
    r[15+4] = r[1];  
    r[15+5] = r[2];  
    r[15+6] = r[8];  
    r[15+7] = r[7];  
    r[15+8] = r[6];  
    r[15+9] = r[5];  
    r[15+10] = r[4];  
    r += 15; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dany;
  } else {
    hreserve(hbsz(8+1), 15); /* 15 live regs */
    *--hp = r[0];  
    *--hp = r[7];  
    *--hp = (r[13]);
    *--hp = r[9];  
    *--hp = r[1];  
    *--hp = (r[10]);
    *--hp = r[2];  
    *--hp = (r[12]);
    *--hp = obj_from_case(233);
    r[15] = (hendblk(8+1));
  if ((isnull((r[14])))) {
    hreserve(hbsz(1+1), 16); /* 16 live regs */
    *--hp = (r[15]);
    *--hp = obj_from_case(242);
    r[16] = (hendblk(1+1));
    r[17+0] = (r[16]);
    r[17+1] = (r[11]);
    r[17+2] = (r[10]);
    r[17+3] = r[9];  
    r[17+4] = r[1];  
    r[17+5] = r[2];  
    r += 17; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dexpr;
  } else {
    hreserve(hbsz(2+1), 16); /* 16 live regs */
    *--hp = r[3];  
    *--hp = (r[15]);
    *--hp = obj_from_case(245);
    r[16] = (hendblk(2+1));
    r[17+0] = (r[16]);
    r[17+1] = (r[14]);
    r[17+2] = r[2];  
    r[17+3] = r[1];  
    r[17+4] = r[9];  
    r[17+5] = (r[10]);
    r += 17; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v7426;
  }
  }

case 233: /* clo ek r */
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
    /* ek r exprs loc-n id-n store env vds ek k */
    { fixnum_t v9248_tmp;
    { /* length */
    int n; obj l = r[2];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v9248_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v9248_tmp);
    hreserve(hbsz(3)*c, 10); /* 10 live regs */
    l = r[2];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[10] = (o); } }
    hreserve(hbsz(8+1), 11); /* 11 live regs */
    *--hp = r[1];  
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(236);
    r[11] = (hendblk(8+1));
    r[12+0] = (r[11]);
    r[12+1] = (r[10]);
    r[12+2] = r[3];  
    r[12+3] = r[5];  
    r[12+4] = r[6];  
    r[12+5] = r[4];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v7492;

s_loop_v7492: /* k id loc-n store env id-n */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[6] = (cdr((r[1])));
    hreserve(hbsz(6+1), 7); /* 7 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(234);
    r[7] = (hendblk(6+1));
    r[0] = r[7];  
    r[1] = r[6];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    goto s_loop_v7492;
  }

case 234: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r loc-n store env id-n id k */
    hreserve(hbsz(2+1), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = r[7];  
    *--hp = obj_from_case(235);
    r[8] = (hendblk(2+1));
    r[9+0] = r[8];  
    r[9+1] = (car((r[6])));
    r[9+2] = r[5];  
    r[9+3] = r[4];  
    r[9+4] = r[3];  
    r[9+5] = r[2];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dexpr;

case 235: /* clo ek r */
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

case 236: /* clo ek r */
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
    /* ek r loc-n id-n store env vds ek k r */
    { fixnum_t v9247_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v9247_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v9247_tmp);
    hreserve(hbsz(3)*c, 10); /* 10 live regs */
    l = r[1];   t = r[9];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[10] = (o); } }
    hreserve(hbsz(7+1), 11); /* 11 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(237);
    r[11] = (hendblk(7+1));
    r[0] = (r[11]);
    r[1] = (r[10]);
    goto gs_make_2Dbegin;

case 237: /* clo ek r */
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
    /* ek r loc-n id-n store env vds ek k */
    { fixnum_t v9246_tmp;
    { /* length */
    int n; obj l = r[6];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v9246_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v9246_tmp);
    hreserve(hbsz(3)*c, 9); /* 9 live regs */
    l = r[6];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[9] = (o); } }
    hreserve(hbsz(3+1), 10); /* 10 live regs */
    *--hp = r[1];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = obj_from_case(241);
    r[10] = (hendblk(3+1));
    r[0] = (r[10]);
    r[1] = r[9];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    goto s_loop_v7461;

s_loop_v7461: /* k id loc-n id-n store env */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[6] = (cdr((r[1])));
    hreserve(hbsz(6+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(238);
    r[7] = (hendblk(6+1));
    r[0] = r[7];  
    r[1] = r[6];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    goto s_loop_v7461;
  }

case 238: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r loc-n id-n store env k id */
    r[8] = (car((r[7])));
    hreserve(hbsz(5+1), 9); /* 9 live regs */
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[8];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(239);
    r[9] = (hendblk(5+1));
    r[10] = (cdr((r[8])));
    r[10] = (cdr((r[10])));
    r[10] = (car((r[10])));
    r[11+0] = r[9];  
    r[11+1] = (r[10]);
    r[11+2] = r[3];  
    r[11+3] = r[5];  
    r[11+4] = r[4];  
    r[11+5] = r[2];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dexpr;

case 239: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r store env vd k r */
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(240);
    r[8] = (hendblk(3+1));
    r[9] = (cdr((r[4])));
    r[9] = (car((r[9])));
    r[10+0] = r[8];  
    r[10+1] = r[9];  
    r[10+2] = r[3];  
    r[10+3] = r[2];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_lookup2;

case 240: /* clo ek r */
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

case 241: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r ek k r */
  if ((isnull((r[1])))) {
    r[5] = r[4];  
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[4];  
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
    *--hp = (mksymbol(internsym("letrec")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
  }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 242: /* clo ek r */
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

s_loop_v7426: /* k id loc-n store env id-n */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[6] = (cdr((r[1])));
    hreserve(hbsz(6+1), 7); /* 7 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(243);
    r[7] = (hendblk(6+1));
    r[0] = r[7];  
    r[1] = r[6];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    goto s_loop_v7426;
  }

case 243: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r loc-n store env id-n id k */
    hreserve(hbsz(2+1), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = r[7];  
    *--hp = obj_from_case(244);
    r[8] = (hendblk(2+1));
    r[9+0] = r[8];  
    r[9+1] = (car((r[6])));
    r[9+2] = r[5];  
    r[9+3] = r[4];  
    r[9+4] = r[3];  
    r[9+5] = r[2];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dexpr;

case 244: /* clo ek r */
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

case 245: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k boundary-exp-output */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
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

case 246: /* clo k vds sds exprs id-n env store loc-n */
    assert(rc == 9);
    { obj* p = objptr_from_obj(r[0]);
    r[1+8] = p[1];
    r[1+9] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* k vds sds exprs id-n env store loc-n loop rest */
    r[10+0] = r[8];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[0];  
    r[10+2] = (car((r[9])));
    r[10+3] = (cdr((r[9])));
    r[10+4] = r[1];  
    r[10+5] = r[2];  
    r[10+6] = r[3];  
    r[10+7] = r[4];  
    r[10+8] = r[5];  
    r[10+9] = r[6];  
    r[10+10] = r[7];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 11);
    goto jump;

case 247: /* clo k sexp vds sds exprs id-n env store loc-n k ek */
    assert(rc == 12);
    r += 1; /* shift reg. wnd */
    /* k sexp vds sds exprs id-n env store loc-n k ek */
    hreserve(hbsz(4+1), 11); /* 11 live regs */
    *--hp = r[4];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = r[9];  
    *--hp = obj_from_case(248);
    r[11] = (hendblk(4+1));
    hreserve(hbsz(0+1), 12); /* 12 live regs */
    *--hp = obj_from_case(247);
    r[12] = (hendblk(0+1));
    hreserve(hbsz(6+1), 13); /* 13 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = (r[12]);
    *--hp = obj_from_case(258);
    r[12] = (hendblk(6+1));
    r[13+0] = r[0];  
    r[13+1] = r[1];  
    r[13+2] = r[5];  
    r[13+3] = r[6];  
    r[13+4] = r[7];  
    r[13+5] = r[8];  
    r[13+6] = obj_from_bool(0);
    r[13+7] = (r[10]);
    r[13+8] = obj_from_bool(0);
    r[13+9] = (r[11]);
    r[13+10] = (r[12]);
    r += 13; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dany;

case 248: /* clo k builtin sexp id-n env store loc-n */
    assert(rc == 8);
    { obj* p = objptr_from_obj(r[0]);
    r[1+7] = p[1];
    r[1+8] = p[2];
    r[1+9] = p[3];
    r[1+10] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* k builtin sexp id-n env store loc-n k sds vds exprs */
  if (bool_from_obj(cxs_list2_3F((r[2])))) {
  if (bool_from_obj(r[10])) {
    r[11] = (cdr((r[2])));
    r[11] = (car((r[11])));
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (r[10]);
    *--hp = (r[11]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    r[12+0] = r[7];  
    pc = objptr_from_obj(r[12+0])[0];
    r[12+1] = r[0];  
    r[12+2] = r[9];  
    r[12+3] = r[8];  
    r[12+4] = (r[11]);
    r[12+5] = r[3];  
    r[12+6] = r[4];  
    r[12+7] = r[5];  
    r[12+8] = r[6];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 9);
    goto jump;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    /* r[0] */    
    r[1] = (cx__231755);
    r[2] = (r[11]);
    goto gs_x_2Derror_2A;
  }
  } else {
    r[11] = (cdr((r[2])));
    r[11] = (car((r[11])));
    r[12] = (cxs_sid_2Did((r[11])));
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = r[6];  
    *--hp = (r[12]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = r[4];  
    *--hp = (r[13]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    hreserve(hbsz(2+1), 14); /* 14 live regs */
    *--hp = (r[12]);
    *--hp = r[2];  
    *--hp = obj_from_case(249);
    r[14] = (hendblk(2+1));
    hreserve(hbsz(13+1), 15); /* 15 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[9];  
    *--hp = r[2];  
    *--hp = r[8];  
    *--hp = (r[10]);
    *--hp = r[3];  
    *--hp = (r[13]);
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = r[7];  
    *--hp = (r[11]);
    *--hp = (r[14]);
    *--hp = obj_from_case(252);
    r[15] = (hendblk(13+1));
    r[0] = (r[15]);
    r[1] = r[8];  
    r[2] = (r[14]);
    goto s_loop_v7353;
  }

case 249: /* clo k def */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* k def sexp id */
    r[4] = (cdr((r[1])));
    r[4] = (car((r[4])));
    r[4] = (cxs_sid_2Did((r[4])));
    r[4] = obj_from_bool(((r[3]) == (r[4])) || ((is_flonum_obj(r[3])) && ((is_flonum_obj(r[4])) && (flonum_from_obj(r[3]) == flonum_from_obj(r[4])))));
  if (bool_from_obj(r[4])) {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (cx__23943);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[0];  
    r[5+1] = (cx__231692);
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_x_2Derror_2A;
  } else {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 250: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7353: /* k id check */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(250);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(251);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 251: /* clo ek  */
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

case 252: /* clo ek  */
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
    r[1+12] = p[11];
    r[1+13] = p[12];
    r[1+14] = p[13]; }
    r += 1; /* shift reg. wnd */
    /* ek  check sid k loc-n store env id-n exprs sds sexp vds k builtin */
    hreserve(hbsz(12+1), 15); /* 15 live regs */
    *--hp = (r[14]);
    *--hp = (r[13]);
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
    *--hp = obj_from_case(255);
    r[15] = (hendblk(12+1));
    r[0] = (r[15]);
    r[1] = (r[12]);
    /* r[2] */    
    goto s_loop_v7338;

case 253: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7338: /* k id check */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(253);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(254);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 254: /* clo ek  */
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

case 255: /* clo ek  */
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
    r[1+12] = p[11];
    r[1+13] = p[12]; }
    r += 1; /* shift reg. wnd */
    /* ek  sid k loc-n store env id-n exprs sds sexp vds k builtin */
  if (((((r[13]) == (mksymbol(internsym("define-syntax")))) || ((is_flonum_obj(r[13])) && ((is_flonum_obj(mksymbol(internsym("define-syntax")))) && (flonum_from_obj(r[13]) == flonum_from_obj(mksymbol(internsym("define-syntax"))))))) && (1))) {
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = r[9];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[14] = (hendblk(3)); }
    r[15+0] = r[3];  
    pc = objptr_from_obj(r[15+0])[0];
    r[15+1] = (r[12]);
    r[15+2] = (r[11]);
    r[15+3] = (r[14]);
    r[15+4] = r[8];  
    r[15+5] = r[7];  
    r[15+6] = r[6];  
    r[15+7] = r[5];  
    r[15+8] = obj_from_fixnum(fixnum_from_obj(r[4]) + (1));
    r += 15; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 9);
    goto jump;
  } else {
  if (((((r[13]) == (mksymbol(internsym("define")))) || ((is_flonum_obj(r[13])) && ((is_flonum_obj(mksymbol(internsym("define")))) && (flonum_from_obj(r[13]) == flonum_from_obj(mksymbol(internsym("define"))))))) && (1))) {
    hreserve(hbsz(11+1), 14); /* 14 live regs */
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = (r[12]);
    *--hp = (r[10]);
    *--hp = (r[11]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(256);
    r[14] = (hendblk(11+1));
  if (bool_from_obj(r[8])) {
    r[15+0] = obj_from_ktrap();
    r[15+1] = r[8];  
    r[15+2] = r[2];  
    r[15+3] = r[3];  
    r[15+4] = r[6];  
    r[15+5] = r[7];  
    r[15+6] = r[8];  
    r[15+7] = r[9];  
    r[15+8] = (r[11]);
    r[15+9] = (r[10]);
    r[15+10] = (r[12]);
    r[15+11] = r[5];  
    r[15+12] = r[4];  
    r += 15; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v7323;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 15); /* 15 live regs */
    *--hp = (mknull());
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[15] = (hendblk(3)); }
    r[0] = (r[14]);
    r[1] = (cx__231712);
    r[2] = (r[15]);
    goto gs_x_2Derror_2A;
  }
  } else {
    r[0] = (r[12]);
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }

case 256: /* clo ek  */
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
s_l_v7323: /* ek  sid k env id-n exprs sds vds sexp k store loc-n */
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
    *--hp = obj_from_case(257);
    r[13] = (hendblk(10+1));
    r[0] = (r[13]);
    r[1] = (r[12]);
    /* r[2] */    
    goto gs_intloc_2D_3Evar;

case 257: /* clo ek r */
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
    /* ek r k env id-n exprs sds vds sexp k store loc-n */
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = r[1];  
    *--hp = (r[11]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = (r[10]);
    *--hp = (r[12]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = r[7];  
    *--hp = r[8];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    r[14+0] = r[2];  
    pc = objptr_from_obj(r[14+0])[0];
    r[14+1] = r[9];  
    r[14+2] = (r[13]);
    r[14+3] = r[6];  
    r[14+4] = r[5];  
    r[14+5] = r[4];  
    r[14+6] = r[3];  
    r[14+7] = (r[12]);
    r[14+8] = obj_from_fixnum(fixnum_from_obj(r[11]) + (1));
    r += 14; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 9);
    goto jump;

case 258: /* clo k sexp id-n env store loc-n */
    assert(rc == 7);
    { obj* p = objptr_from_obj(r[0]);
    r[1+6] = p[1];
    r[1+7] = p[2];
    r[1+8] = p[3];
    r[1+9] = p[4];
    r[1+10] = p[5];
    r[1+11] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* k sexp id-n env store loc-n expand-def k ek exprs sds vds */
    r[12] = (cdr((r[1])));
    r[13+0] = r[0];  
    r[13+1] = (r[12]);
    r[13+2] = (r[11]);
    r[13+3] = (r[10]);
    r[13+4] = r[9];  
    r[13+5] = r[2];  
    r[13+6] = r[3];  
    r[13+7] = r[4];  
    r[13+8] = r[5];  
    r[13+9] = r[8];  
    r[13+10] = r[6];  
    r[13+11] = r[7];  
    r += 13; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v7230;

case 259: /* clo k sexps vds sds exprs id-n env store loc-n ek */
    assert(rc == 11);
    { obj* p = objptr_from_obj(r[0]);
    r[1+10] = p[1];
    r[1+11] = p[2]; }
    r += 1; /* shift reg. wnd */
s_loop_v7230: /* k sexps vds sds exprs id-n env store loc-n ek expand-def k */
  if ((isnull((r[1])))) {
    r[12+0] = (r[11]);
    pc = objptr_from_obj(r[12+0])[0];
    r[12+1] = r[0];  
    r[12+2] = r[2];  
    r[12+3] = r[3];  
    r[12+4] = r[4];  
    r[12+5] = r[5];  
    r[12+6] = r[6];  
    r[12+7] = r[7];  
    r[12+8] = r[8];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 9);
    goto jump;
  } else {
    hreserve(hbsz(2+1), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = obj_from_case(259);
    r[12] = (hendblk(2+1));
    hreserve(hbsz(2+1), 13); /* 13 live regs */
    *--hp = r[1];  
    *--hp = (r[12]);
    *--hp = obj_from_case(260);
    r[12] = (hendblk(2+1));
  if (bool_from_obj(r[9])) {
    hreserve(hbsz(6+1), 13); /* 13 live regs */
    *--hp = r[9];  
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = r[7];  
    *--hp = r[8];  
    *--hp = obj_from_case(261);
    r[13] = (hendblk(6+1));
  } else {
    r[13] = obj_from_bool(0);
  }
    r[14+0] = (r[10]);
    pc = objptr_from_obj(r[14+0])[0];
    r[14+1] = r[0];  
    r[14+2] = (car((r[1])));
    r[14+3] = r[2];  
    r[14+4] = r[3];  
    r[14+5] = r[4];  
    r[14+6] = r[5];  
    r[14+7] = r[6];  
    r[14+8] = r[7];  
    r[14+9] = r[8];  
    r[14+10] = (r[12]);
    r[14+11] = (r[13]);
    r += 14; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 12);
    goto jump;
  }

case 260: /* clo k vds sds exprs id-n env store loc-n */
    assert(rc == 9);
    { obj* p = objptr_from_obj(r[0]);
    r[1+8] = p[1];
    r[1+9] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* k vds sds exprs id-n env store loc-n loop sexps */
    r[10+0] = r[8];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[0];  
    r[10+2] = (cdr((r[9])));
    r[10+3] = r[1];  
    r[10+4] = r[2];  
    r[10+5] = r[3];  
    r[10+6] = r[4];  
    r[10+7] = r[5];  
    r[10+8] = r[6];  
    r[10+9] = r[7];  
    r[10+10] = obj_from_bool(0);
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 11);
    goto jump;

case 261: /* clo k out */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* k out loc-n store env id-n sexps ek */
    r[8] = (cdr((r[6])));
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[0];  
    *--hp = r[7];  
    *--hp = r[1];  
    *--hp = obj_from_case(264);
    r[9] = (hendblk(3+1));
    r[0] = r[9];  
    r[1] = r[8];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    goto s_loop_v7245;

s_loop_v7245: /* k id loc-n store env id-n */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[6] = (cdr((r[1])));
    hreserve(hbsz(6+1), 7); /* 7 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(262);
    r[7] = (hendblk(6+1));
    r[0] = r[7];  
    r[1] = r[6];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    goto s_loop_v7245;
  }

case 262: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r loc-n store env id-n id k */
    hreserve(hbsz(2+1), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = r[7];  
    *--hp = obj_from_case(263);
    r[8] = (hendblk(2+1));
    r[9+0] = r[8];  
    r[9+1] = (car((r[6])));
    r[9+2] = r[5];  
    r[9+3] = r[4];  
    r[9+4] = r[3];  
    r[9+5] = r[2];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dexpr;

case 263: /* clo ek r */
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

case 264: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r out ek k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(265);
    r[5] = (hendblk(2+1));
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[5];  
    r[1] = r[6];  
    goto gs_make_2Dbegin;

case 265: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r ek k */
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 266: /* expand-top-level-forms k forms store loc-n k */
    assert(rc == 6);
    r += 1; /* shift reg. wnd */
gs_expand_2Dtop_2Dlevel_2Dforms: /* k forms store loc-n k */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = obj_from_case(267);
    r[5] = (hendblk(4+1));
    r[0] = r[5];  
    /* r[1] */    
    goto gs_wrap_2Dvecs;

case 267: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r k loc-n store k */
    hreserve(hbsz(1+1), 6); /* 6 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(279);
    r[6] = (hendblk(1+1));
    r[7+0] = r[5];  
    r[7+1] = r[1];  
    r[7+2] = obj_from_fixnum(0);
    r[7+3] = (mknull());
    r[7+4] = r[4];  
    r[7+5] = r[3];  
    r[7+6] = (mknull());
    r[7+7] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_expand;

case 268: /* clo k sexps id-n env store loc-n acc k */
    assert(rc == 9);
    r += 1; /* shift reg. wnd */
s_expand: /* k sexps id-n env store loc-n acc k */
    hreserve(hbsz(0+1), 8); /* 8 live regs */
    *--hp = obj_from_case(268);
    r[8] = (hendblk(0+1));
  if ((isnull((r[1])))) {
    r[9+0] = r[7];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[0];  
    r[9+2] = r[4];  
    r[9+3] = r[5];  
    r[9+4] = r[6];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[9] = (cdr((r[1])));
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = r[7];  
    *--hp = r[8];  
    *--hp = obj_from_case(269);
    r[10] = (hendblk(8+1));
    hreserve(hbsz(7+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[6];  
    *--hp = r[9];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = r[7];  
    *--hp = r[8];  
    *--hp = obj_from_case(270);
    r[11] = (hendblk(7+1));
    hreserve(hbsz(6+1), 12); /* 12 live regs */
    *--hp = r[6];  
    *--hp = r[9];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = r[7];  
    *--hp = r[8];  
    *--hp = obj_from_case(277);
    r[12] = (hendblk(6+1));
    r[13+0] = r[0];  
    r[13+1] = (car((r[1])));
    r[13+2] = r[2];  
    r[13+3] = r[3];  
    r[13+4] = r[4];  
    r[13+5] = r[5];  
    r[13+6] = obj_from_bool(1);
    r[13+7] = (r[10]);
    r[13+8] = obj_from_bool(0);
    r[13+9] = (r[11]);
    r[13+10] = (r[12]);
    r += 13; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dany;
  }

case 269: /* clo k output */
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
    /* k output expand k acc loc-n store env id-n rest */
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[11+0] = r[2];  
    pc = objptr_from_obj(r[11+0])[0];
    r[11+1] = r[0];  
    r[11+2] = r[9];  
    r[11+3] = r[8];  
    r[11+4] = r[7];  
    r[11+5] = r[6];  
    r[11+6] = r[5];  
    r[11+7] = (r[10]);
    r[11+8] = r[3];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 9);
    goto jump;

case 270: /* clo k builtin sexp id-n* env* store loc-n */
    assert(rc == 8);
    { obj* p = objptr_from_obj(r[0]);
    r[1+7] = p[1];
    r[1+8] = p[2];
    r[1+9] = p[3];
    r[1+10] = p[4];
    r[1+11] = p[5];
    r[1+12] = p[6];
    r[1+13] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* k builtin sexp id-n* env* store loc-n expand k env id-n rest acc ek */
  if (bool_from_obj(cxs_list2_3F((r[2])))) {
    hreserve(hbsz(2+1), 14); /* 14 live regs */
    *--hp = r[0];  
    *--hp = (r[13]);
    *--hp = obj_from_case(271);
    r[14] = (hendblk(2+1));
    r[15] = (cdr((r[2])));
    r[15] = (car((r[15])));
    r[0] = (r[14]);
    r[1] = (r[15]);
    r[2] = r[3];  
    r[3] = r[4];  
    r[4] = r[5];  
    r[5] = r[6];  
    goto gs_expand_2Dexpr;
  } else {
    r[14] = (cdr((r[2])));
    r[15] = (car((r[14])));
    r[16] = (cxs_sid_2Dlocation((r[15])));
    r[17] = (cdr((r[14])));
    r[17] = (car((r[17])));
  if (((r[1]) == (mksymbol(internsym("define"))))) {
    hreserve(hbsz(11+1), 18); /* 18 live regs */
    *--hp = (r[12]);
    *--hp = r[0];  
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[6];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = (r[16]);
    *--hp = (r[15]);
    *--hp = obj_from_case(272);
    r[18] = (hendblk(11+1));
    r[0] = (r[18]);
    r[1] = (r[17]);
    r[2] = r[3];  
    r[3] = r[4];  
    r[4] = r[5];  
    r[5] = r[6];  
    goto gs_expand_2Dexpr;
  } else {
    hreserve(hbsz(7+1), 18); /* 18 live regs */
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = (r[12]);
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = (r[16]);
    *--hp = obj_from_case(275);
    r[18] = (hendblk(7+1));
    /* r[0] */    
    r[1] = (r[17]);
    r[2] = r[3];  
    r[3] = r[4];  
    r[4] = r[5];  
    r[5] = r[6];  
    r[6] = (r[18]);
    goto gs_expand_2Dval;
  }
  }

case 271: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r ek k */
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 272: /* clo ek r */
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
    /* ek r sid loc store expand k loc-n env id-n rest k acc */
    hreserve(hbsz(11+1), 13); /* 13 live regs */
    *--hp = r[1];  
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
    *--hp = obj_from_case(273);
    r[13] = (hendblk(11+1));
    r[0] = (r[13]);
    r[1] = r[3];  
    /* r[2] */    
    goto gs_loc_2D_3Evar;

case 273: /* clo ek r */
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
    /* ek r loc store expand k loc-n env id-n rest k acc r */
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = (mknull());
    *--hp = (r[12]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = (r[13]);
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = (r[13]);
    *--hp = (mksymbol(internsym("define")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = (r[11]);
    *--hp = (r[13]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    hreserve(hbsz(8+1), 14); /* 14 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = (r[13]);
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(274);
    r[14] = (hendblk(8+1));
    r[15+0] = (r[14]);
    r[15+1] = r[3];  
    r[15+2] = r[2];  
    r[15+3] = r[1];  
    r += 15; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_substitute_2Din_2Dstore;

case 274: /* clo ek r */
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
    /* ek r expand k acc loc-n env id-n rest k */
    r[10+0] = r[2];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[9];  
    r[10+2] = r[8];  
    r[10+3] = r[7];  
    r[10+4] = r[6];  
    r[10+5] = r[1];  
    r[10+6] = r[5];  
    r[10+7] = r[4];  
    r[10+8] = r[3];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 9);
    goto jump;

case 275: /* clo k val store loc-n */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1];
    r[1+5] = p[2];
    r[1+6] = p[3];
    r[1+7] = p[4];
    r[1+8] = p[5];
    r[1+9] = p[6];
    r[1+10] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* k val store loc-n loc expand k acc env id-n rest */
    hreserve(hbsz(8+1), 11); /* 11 live regs */
    *--hp = r[0];  
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[3];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(276);
    r[11] = (hendblk(8+1));
    r[12+0] = (r[11]);
    r[12+1] = r[2];  
    r[12+2] = r[4];  
    r[12+3] = r[1];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_substitute_2Din_2Dstore;

case 276: /* clo ek r */
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
    /* ek r expand k acc loc-n env id-n rest k */
    r[10+0] = r[2];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[9];  
    r[10+2] = r[8];  
    r[10+3] = r[7];  
    r[10+4] = r[6];  
    r[10+5] = r[1];  
    r[10+6] = r[5];  
    r[10+7] = r[4];  
    r[10+8] = r[3];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 9);
    goto jump;

case 277: /* clo k sexp id-n* env* store loc-n */
    assert(rc == 7);
    { obj* p = objptr_from_obj(r[0]);
    r[1+6] = p[1];
    r[1+7] = p[2];
    r[1+8] = p[3];
    r[1+9] = p[4];
    r[1+10] = p[5];
    r[1+11] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* k sexp id-n* env* store loc-n expand k env id-n rest acc */
    hreserve(hbsz(5+1), 12); /* 12 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_case(278);
    r[12] = (hendblk(5+1));
    r[13+0] = r[6];  
    pc = objptr_from_obj(r[13+0])[0];
    r[13+1] = r[0];  
    r[13+2] = (cdr((r[1])));
    r[13+3] = r[2];  
    r[13+4] = r[3];  
    r[13+5] = r[4];  
    r[13+6] = r[5];  
    r[13+7] = (r[11]);
    r[13+8] = (r[12]);
    r += 13; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 9);
    goto jump;

case 278: /* clo k store loc-n acc */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1];
    r[1+5] = p[2];
    r[1+6] = p[3];
    r[1+7] = p[4];
    r[1+8] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* k store loc-n acc expand k env id-n rest */
    r[9+0] = r[4];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[0];  
    r[9+2] = r[8];  
    r[9+3] = r[7];  
    r[9+4] = r[6];  
    r[9+5] = r[1];  
    r[9+6] = r[2];  
    r[9+7] = r[3];  
    r[9+8] = r[5];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 9);
    goto jump;

case 279: /* clo k store loc-n acc */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k store loc-n acc k */
    { fixnum_t v9245_tmp;
    { /* length */
    int n; obj l = r[3];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v9245_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v9245_tmp);
    hreserve(hbsz(3)*c, 5); /* 5 live regs */
    l = r[3];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[5] = (o); } }
    r[6+0] = r[4];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[0];  
    r[6+2] = r[5];  
    r[6+3] = r[1];  
    r[6+4] = r[2];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 280: /* compile-syntax-lambda k synlambda env */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_compile_2Dsyntax_2Dlambda: /* k synlambda env */
    r[3] = (cdr((r[1])));
    r[3] = (car((r[3])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = obj_from_case(287);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    r[2] = r[3];  
    goto s_loop_v7053;

case 281: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v7053: /* k id formals */
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
    *--hp = obj_from_case(281);
    r[4] = (hendblk(1+1));
    hreserve(hbsz(5+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_case(282);
    r[4] = (hendblk(5+1));
    r[0] = r[4];  
    r[1] = r[3];  
    goto gs_sid_3F;
  }

case 282: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r x formals loop id k */
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(283);
    r[7] = (hendblk(5+1));
  if (bool_from_obj(r[1])) {
    r[0] = obj_from_ktrap();
    /* r[1] */    
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    goto s_l_v7057;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = (cx__231988);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    r[0] = r[7];  
    r[1] = (cx__23862);
    r[2] = r[8];  
    goto gs_x_2Derror_2A;
  }

case 283: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
s_l_v7057: /* ek  x formals loop id k */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(284);
    r[7] = (hendblk(3+1));
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(285);
    r[7] = (hendblk(3+1));
    r[0] = (cx_member);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    /* r[2] */    
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 284: /* clo ek  */
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

case 285: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r x formals k */
    r[5] = (cdr((r[1])));
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(286);
    r[6] = (hendblk(3+1));
    r[0] = (cx_member);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    /* r[2] */    
    r[3] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 286: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r x formals k */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (cx__231988);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[4];  
    r[1] = (cx__23852);
    r[2] = r[5];  
    goto gs_x_2Derror_2A;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 287: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  k env synlambda */
    r[5] = (cdr((r[4])));
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (mksymbol(internsym("syntax-lambda")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 288: /* sbox->sexp-list k b */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k b */
    r[2] = (boxref((r[1])));
    r[2] = (cdr((r[2])));
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 289: /* pattern-sbox->sexp k b */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k b */
    r[2] = (boxref((r[1])));
    r[3] = (cdr((r[2])));
    r[2] = (car((r[3])));
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 290: /* pattern-sbox? k b */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k b */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cxs_pattern_2Dsbox_3F((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 291: /* pattern-sbox->test k b */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_pattern_2Dsbox_2D_3Etest: /* k b */
    r[2] = (boxref((r[1])));
    r[2] = (car((r[2])));
  if (((((r[2]) == (mksymbol(internsym("number?")))) || ((is_flonum_obj(r[2])) && ((is_flonum_obj(mksymbol(internsym("number?")))) && (flonum_from_obj(r[2]) == flonum_from_obj(mksymbol(internsym("number?"))))))) && (1))) {
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(292);
    r[2] = (hendblk(0+1));
  } else {
  if (((((r[2]) == (mksymbol(internsym("string?")))) || ((is_flonum_obj(r[2])) && ((is_flonum_obj(mksymbol(internsym("string?")))) && (flonum_from_obj(r[2]) == flonum_from_obj(mksymbol(internsym("string?"))))))) && (1))) {
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(293);
    r[2] = (hendblk(0+1));
  } else {
  if (((((r[2]) == (mksymbol(internsym("id?")))) || ((is_flonum_obj(r[2])) && ((is_flonum_obj(mksymbol(internsym("id?")))) && (flonum_from_obj(r[2]) == flonum_from_obj(mksymbol(internsym("id?"))))))) && (1))) {
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(294);
    r[2] = (hendblk(0+1));
  } else {
    r[2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
  }
  }
  }
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 292: /* clo k sexp env */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k sexp env */
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool((is_fixnum_obj(r[1])) || (is_flonum_obj(r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 293: /* clo k sexp env */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k sexp env */
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isstring((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 294: /* clo k sexp env */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k sexp env */
    /* r[0] */    
    /* r[1] */    
    goto gs_sid_3F;

case 295: /* template-sbox->sexp k b */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k b */
    r[2] = (boxref((r[1])));
    r[2] = (cdr((r[2])));
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 296: /* template-sbox? k b */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_template_2Dsbox_3F: /* k b */
  if ((isbox((r[1])))) {
    r[2] = (boxref((r[1])));
    r[2] = obj_from_bool(ispair((r[2])));
  if (bool_from_obj(r[2])) {
    r[2] = (boxref((r[1])));
    r[3] = (car((r[2])));
  if (((((r[3]) == (mksymbol(internsym("number->string")))) || ((is_flonum_obj(r[3])) && ((is_flonum_obj(mksymbol(internsym("number->string")))) && (flonum_from_obj(r[3]) == flonum_from_obj(mksymbol(internsym("number->string"))))))) && (1))) {
    r[4] = (cdr((r[2])));
    r[4] = (cxs_list1_3F((r[4])));
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (((((r[3]) == (mksymbol(internsym("string->number")))) || ((is_flonum_obj(r[3])) && ((is_flonum_obj(mksymbol(internsym("string->number")))) && (flonum_from_obj(r[3]) == flonum_from_obj(mksymbol(internsym("string->number"))))))) && (1))) {
    r[4] = (cdr((r[2])));
    r[4] = (cxs_list1_3F((r[4])));
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (((((r[3]) == (mksymbol(internsym("length")))) || ((is_flonum_obj(r[3])) && ((is_flonum_obj(mksymbol(internsym("length")))) && (flonum_from_obj(r[3]) == flonum_from_obj(mksymbol(internsym("length"))))))) && (1))) {
    r[4] = (cdr((r[2])));
    r[4] = (cxs_list1_3F((r[4])));
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (((((r[3]) == (mksymbol(internsym("string-append")))) || ((is_flonum_obj(r[3])) && ((is_flonum_obj(mksymbol(internsym("string-append")))) && (flonum_from_obj(r[3]) == flonum_from_obj(mksymbol(internsym("string-append"))))))) && (1))) {
    r[4+0] = (cx_list_3F);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[0];  
    r[4+2] = (cdr((r[2])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (((((r[3]) == (mksymbol(internsym("+")))) || ((is_flonum_obj(r[3])) && ((is_flonum_obj(mksymbol(internsym("+")))) && (flonum_from_obj(r[3]) == flonum_from_obj(mksymbol(internsym("+"))))))) ? (1) : ((((r[3]) == (mksymbol(internsym("*")))) || ((is_flonum_obj(r[3])) && ((is_flonum_obj(mksymbol(internsym("*")))) && (flonum_from_obj(r[3]) == flonum_from_obj(mksymbol(internsym("*"))))))) && (1)))) {
    r[4+0] = (cx_list_3F);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[0];  
    r[4+2] = (cdr((r[2])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (((((r[3]) == (mksymbol(internsym("id->string")))) || ((is_flonum_obj(r[3])) && ((is_flonum_obj(mksymbol(internsym("id->string")))) && (flonum_from_obj(r[3]) == flonum_from_obj(mksymbol(internsym("id->string"))))))) && (1))) {
    r[4] = (cdr((r[2])));
    r[4] = (cxs_list1_3F((r[4])));
  } else {
  if (((((r[3]) == (mksymbol(internsym("string->id")))) || ((is_flonum_obj(r[3])) && ((is_flonum_obj(mksymbol(internsym("string->id")))) && (flonum_from_obj(r[3]) == flonum_from_obj(mksymbol(internsym("string->id"))))))) && (1))) {
    r[4] = (cdr((r[2])));
    r[4] = (cxs_list1_3F((r[4])));
  } else {
    r[4] = obj_from_bool(0);
  }
  }
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }
  }
  }
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

case 297: /* template-sbox->conv k b */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_template_2Dsbox_2D_3Econv: /* k b */
    r[2] = (boxref((r[1])));
    r[2] = (car((r[2])));
  if (((((r[2]) == (mksymbol(internsym("number->string")))) || ((is_flonum_obj(r[2])) && ((is_flonum_obj(mksymbol(internsym("number->string")))) && (flonum_from_obj(r[2]) == flonum_from_obj(mksymbol(internsym("number->string"))))))) && (1))) {
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(298);
    r[2] = (hendblk(0+1));
  } else {
  if (((((r[2]) == (mksymbol(internsym("string->number")))) || ((is_flonum_obj(r[2])) && ((is_flonum_obj(mksymbol(internsym("string->number")))) && (flonum_from_obj(r[2]) == flonum_from_obj(mksymbol(internsym("string->number"))))))) && (1))) {
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(299);
    r[2] = (hendblk(0+1));
  } else {
  if (((((r[2]) == (mksymbol(internsym("length")))) || ((is_flonum_obj(r[2])) && ((is_flonum_obj(mksymbol(internsym("length")))) && (flonum_from_obj(r[2]) == flonum_from_obj(mksymbol(internsym("length"))))))) && (1))) {
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(301);
    r[2] = (hendblk(0+1));
  } else {
  if (((((r[2]) == (mksymbol(internsym("string-append")))) || ((is_flonum_obj(r[2])) && ((is_flonum_obj(mksymbol(internsym("string-append")))) && (flonum_from_obj(r[2]) == flonum_from_obj(mksymbol(internsym("string-append"))))))) && (1))) {
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(303);
    r[2] = (hendblk(0+1));
  } else {
  if (((((r[2]) == (mksymbol(internsym("+")))) || ((is_flonum_obj(r[2])) && ((is_flonum_obj(mksymbol(internsym("+")))) && (flonum_from_obj(r[2]) == flonum_from_obj(mksymbol(internsym("+"))))))) && (1))) {
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(305);
    r[2] = (hendblk(0+1));
  } else {
  if (((((r[2]) == (mksymbol(internsym("*")))) || ((is_flonum_obj(r[2])) && ((is_flonum_obj(mksymbol(internsym("*")))) && (flonum_from_obj(r[2]) == flonum_from_obj(mksymbol(internsym("*"))))))) && (1))) {
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(307);
    r[2] = (hendblk(0+1));
  } else {
  if (((((r[2]) == (mksymbol(internsym("id->string")))) || ((is_flonum_obj(r[2])) && ((is_flonum_obj(mksymbol(internsym("id->string")))) && (flonum_from_obj(r[2]) == flonum_from_obj(mksymbol(internsym("id->string"))))))) && (1))) {
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(309);
    r[2] = (hendblk(0+1));
  } else {
  if (((((r[2]) == (mksymbol(internsym("string->id")))) || ((is_flonum_obj(r[2])) && ((is_flonum_obj(mksymbol(internsym("string->id")))) && (flonum_from_obj(r[2]) == flonum_from_obj(mksymbol(internsym("string->id"))))))) && (1))) {
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(311);
    r[2] = (hendblk(0+1));
  } else {
    r[2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
  }
  }
  }
  }
  }
  }
  }
  }
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 298: /* clo k sexps env */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k sexps env */
    r[3] = (car((r[1])));
  if (((is_fixnum_obj(r[3])) || (is_flonum_obj(r[3])))) {
  if ((is_fixnum_obj(r[3]))) {
    r[4+0] = (cx_fixnum_2D_3Estring);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[0];  
    r[4+2] = r[3];  
    r[4+3] = obj_from_fixnum(10);
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[4+0] = (cx_flonum_2D_3Estring);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[0];  
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    /* r[0] */    
    r[1] = (cx__232322);
    r[2] = r[4];  
    goto gs_x_2Derror_2A;
  }

case 299: /* clo k sexps env */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k sexps env */
    r[3] = (car((r[1])));
  if ((isstring((r[3])))) {
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[3];  
    *--hp = obj_from_case(300);
    r[4] = (hendblk(2+1));
    r[5+0] = (cx_string_2D_3Efixnum);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = r[3];  
    r[5+3] = obj_from_fixnum(10);
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    /* r[0] */    
    r[1] = (cx__232304);
    r[2] = r[4];  
    goto gs_x_2Derror_2A;
  }

case 300: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r sexp k */
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
    r[0] = (cx_string_2D_3Eflonum);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 301: /* clo k sexps env */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k sexps env */
    r[3] = (car((r[1])));
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = obj_from_case(302);
    r[4] = (hendblk(2+1));
    r[5+0] = (cx_list_3F);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = r[3];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 302: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k sexp */
  if (bool_from_obj(r[1])) {
    { /* length */
    int n; obj l = r[3];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    r[4] = obj_from_fixnum(n); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = r[2];  
    r[1] = (cx__232289);
    r[2] = r[4];  
    goto gs_x_2Derror_2A;
  }

case 303: /* clo k sexps env */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k sexps env */
    /* r[0] */    
    /* r[1] */    
    goto s_loop_v6928;

s_loop_v6928: /* k sexps */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cx__232271);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (car((r[1])));
    r[2] = obj_from_bool(isstring((r[2])));
  if (bool_from_obj(r[2])) {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(304);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v6928;
  } else {
    r[2] = (car((r[1])));
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    r[3+0] = r[0];  
    r[3+1] = (cx__232264);
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_x_2Derror_2A;
  }
  }

case 304: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k sexps */
    { /* string-append */
    int *d = stringcat(stringdata((car((r[3])))), stringdata((r[1])));
    r[4] = (hpushstr(4, d)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 305: /* clo k sexps env */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k sexps env */
    /* r[0] */    
    /* r[1] */    
    goto s_loop_v6909;

s_loop_v6909: /* k sexps */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_fixnum(0);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (car((r[1])));
    r[2] = obj_from_bool((is_fixnum_obj(r[2])) || (is_flonum_obj(r[2])));
  if (bool_from_obj(r[2])) {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(306);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v6909;
  } else {
    r[2] = (car((r[1])));
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    r[3+0] = r[0];  
    r[3+1] = (cx__232238);
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_x_2Derror_2A;
  }
  }

case 306: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k sexps */
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = obj_from_fixnum(fixnum_from_obj(car((r[3]))) + fixnum_from_obj(r[1]));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 307: /* clo k sexps env */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k sexps env */
    /* r[0] */    
    /* r[1] */    
    goto s_loop_v6890;

s_loop_v6890: /* k sexps */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_fixnum(1);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (car((r[1])));
    r[2] = obj_from_bool((is_fixnum_obj(r[2])) || (is_flonum_obj(r[2])));
  if (bool_from_obj(r[2])) {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(308);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v6890;
  } else {
    r[2] = (car((r[1])));
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    r[3+0] = r[0];  
    r[3+1] = (cx__232212);
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_x_2Derror_2A;
  }
  }

case 308: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k sexps */
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = obj_from_fixnum(fixnum_from_obj(car((r[3]))) * fixnum_from_obj(r[1]));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 309: /* clo k sexps env */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k sexps env */
    r[3] = (car((r[1])));
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = obj_from_case(310);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = r[3];  
    goto gs_sid_3F;

case 310: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k sexp */
  if (bool_from_obj(r[1])) {
    r[4] = (cxs_sid_2Dname((r[3])));
    r[4] = (hpushstr(5, newstring(symbolname(getsymbol((r[4]))))));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = r[2];  
    r[1] = (cx__232196);
    r[2] = r[4];  
    goto gs_x_2Derror_2A;
  }

case 311: /* clo k sexps env */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k sexps env */
    r[3] = (car((r[1])));
  if ((isstring((r[3])))) {
    r[4] = (mksymbol(internsym(stringchars((r[3])))));
    r[5] = (cxs_assv_23530((r[4]), (r[2])));
    r[5] = (bool_from_obj(r[5]) ? (cdr((r[5]))) : (r[4]));
    r[6+0] = r[0];  
    r[6+1] = r[4];  
    r[6+2] = r[4];  
    r[6+3] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_make_2Dsid;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    /* r[0] */    
    r[1] = (cx__232176);
    r[2] = r[4];  
    goto gs_x_2Derror_2A;
  }

case 312: /* compile-syntax-rules k synrules env */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_compile_2Dsyntax_2Drules: /* k synrules env */
    hreserve(hbsz(1), 3); /* 3 live regs */
    *--hp = obj_from_void(0);
    r[3] = (hendblk(1));
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(313);
    r[4] = (hendblk(2+1));
    hreserve(hbsz(5+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(315);
    r[4] = (hendblk(5+1));
    r[5] = (cdr((r[1])));
    r[5] = (cdr((r[5])));
    r[5] = obj_from_bool(ispair((r[5])));
  if (bool_from_obj(r[5])) {
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(387);
    r[5] = (hendblk(2+1));
    r[6] = (cdr((r[1])));
    r[6] = (car((r[6])));
    r[0] = r[5];  
    r[1] = r[6];  
    goto gs_sid_3F;
  } else {
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(313);
    r[5] = (hendblk(2+1));
    r[6+0] = obj_from_ktrap();
    r[6+1] = obj_from_bool(0);
    r[6+2] = r[2];  
    r[6+3] = r[0];  
    r[6+4] = r[5];  
    r[6+5] = r[1];  
    r[6+6] = r[3];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v6389;
  }

case 313: /* clo k x */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* k x env ellipsis-id */
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = obj_from_case(314);
    r[4] = (hendblk(4+1));
    r[0] = r[4];  
    /* r[1] */    
    goto gs_sid_3F;

case 314: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r k env x ellipsis-id */
  if (bool_from_obj(r[1])) {
  if (bool_from_obj(objptr_from_obj(r[5])[0])) {
    r[6] = (objptr_from_obj(r[5])[0]);
    r[7] = (cxs_sid_2Did((r[4])));
    r[6] = obj_from_bool(((r[6]) == (r[7])) || ((is_flonum_obj(r[6])) && ((is_flonum_obj(r[7])) && (flonum_from_obj(r[6]) == flonum_from_obj(r[7])))));
  } else {
    r[6] = (cxs_lookup_2Dsid((r[4]), (r[3])));
    r[6] = obj_from_bool((mksymbol(internsym("..."))) == (r[6]));
  }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
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

case 315: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
s_l_v6389: /* ek r env k ellipsis? synrules ellipsis-id */
    (void)(objptr_from_obj(r[6])[0] = (r[1]));
  if (bool_from_obj(objptr_from_obj(r[6])[0])) {
    r[7] = (cdr((r[5])));
    r[7] = (cdr((r[7])));
  } else {
    r[7] = (cdr((r[5])));
  }
    r[8] = (car((r[7])));
    r[9] = (cdr((r[7])));
    hreserve(hbsz(7+1), 10); /* 10 live regs */
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[6];  
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = obj_from_case(316);
    r[10] = (hendblk(7+1));
    r[0] = (cx_list_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 316: /* clo ek r */
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
    /* ek r pat-literal-sids rules ellipsis-id env k synrules ellipsis? */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(317);
    r[9] = (hendblk(7+1));
  if (bool_from_obj(r[1])) {
    r[0] = obj_from_ktrap();
    /* r[1] */    
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    /* r[7] */    
    /* r[8] */    
    goto s_l_v6393;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[0] = r[9];  
    r[1] = (cx__232411);
    r[2] = (r[10]);
    goto gs_x_2Derror_2A;
  }

case 317: /* clo ek  */
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
s_l_v6393: /* ek  pat-literal-sids rules ellipsis-id env k synrules ellipsis? */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(323);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    r[1] = r[2];  
    r[2] = r[8];  
    r[3] = r[7];  
    goto s_loop_v6805;

case 318: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
s_loop_v6805: /* k id ellipsis? synrules */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[4] = (car((r[1])));
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(318);
    r[5] = (hendblk(2+1));
    hreserve(hbsz(6+1), 6); /* 6 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[2];  
    *--hp = obj_from_case(319);
    r[5] = (hendblk(6+1));
    r[0] = r[5];  
    r[1] = r[4];  
    goto gs_sid_3F;
  }

case 319: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r ellipsis? lit synrules loop id k */
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(320);
    r[8] = (hendblk(6+1));
  if (bool_from_obj(r[1])) {
    r[0] = obj_from_ktrap();
    /* r[1] */    
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    /* r[7] */    
    goto s_l_v6809;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = (mknull());
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = (cx__232344);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[0] = r[8];  
    r[1] = (cx__232357);
    r[2] = r[9];  
    goto gs_x_2Derror_2A;
  }

case 320: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
s_l_v6809: /* ek  ellipsis? lit synrules loop id k */
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(321);
    r[8] = (hendblk(3+1));
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(322);
    r[8] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 321: /* clo ek  */
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

case 322: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r lit synrules k */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (cx__232344);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[4];  
    r[1] = (cx__232349);
    r[2] = r[5];  
    goto gs_x_2Derror_2A;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 323: /* clo ek  */
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
    /* ek  pat-literal-sids rules ellipsis-id env k synrules ellipsis? */
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(325);
    r[9] = (hendblk(6+1));
    r[0] = r[9];  
    r[1] = r[2];  
    goto s_loop_v6790;

s_loop_v6790: /* k id */
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
    *--hp = obj_from_case(324);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v6790;
  }

case 324: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    r[4] = (cxs_sid_2Did((r[4])));
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

case 325: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r rules ellipsis-id env k synrules ellipsis? */
    hreserve(hbsz(1+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = obj_from_case(326);
    r[8] = (hendblk(1+1));
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(380);
    r[9] = (hendblk(6+1));
    hreserve(hbsz(2+1), 10); /* 10 live regs */
    *--hp = r[7];  
    *--hp = r[8];  
    *--hp = obj_from_case(327);
    r[10] = (hendblk(2+1));
    r[11+0] = r[9];  
    r[11+1] = r[2];  
    r[11+2] = (r[10]);
    r[11+3] = r[1];  
    r[11+4] = r[8];  
    r[11+5] = r[7];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v6448;

case 326: /* clo k x */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k x ellipsis? */
  if ((ispair((r[1])))) {
    r[3+0] = r[2];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[0];  
    r[3+2] = (car((r[1])));
    r += 3; /* shift reg wnd */
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

case 327: /* clo k pat/tmpl in-template? */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* k pat/tmpl in-template? ellipsis-pair? ellipsis? */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(328);
    r[5] = (hendblk(2+1));
    hreserve(hbsz(1+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = obj_from_case(329);
    r[6] = (hendblk(1+1));
    r[7] = (bool_from_obj(r[2]) ? (r[1]) : (cdr((r[1]))));
    hreserve(hbsz(7+1), 8); /* 8 live regs */
    *--hp = r[0];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[2];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(330);
    r[8] = (hendblk(7+1));
  if (bool_from_obj(r[2])) {
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_case(349);
    r[9] = (hendblk(3+1));
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(5+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[7];  
    *--hp = r[3];  
    *--hp = obj_from_case(350);
    r[9] = (hendblk(5+1));
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 328: /* clo k x reason */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* k x reason in-template? pat/tmpl */
    { /* string-append */
    int *d = stringcat(stringdata((r[2])), stringdata((cx__232615)));
    r[5] = (hpushstr(5, d)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = (bool_from_obj(r[3]) ? (cx__232495) : (cx__232561));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[7+0] = r[0];  
    r[7+1] = r[5];  
    r[7+2] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_x_2Derror_2A;

case 329: /* clo k x thing */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k x thing bad-ellipsis */
    { /* string-append */
    int *d = stringcat(stringdata((cx__232600)), stringdata((r[2])));
    r[4] = (hpushstr(4, d)); }
    r[5+0] = r[3];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[0];  
    r[5+2] = r[1];  
    r[5+3] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 330: /* clo ek  */
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
    /* ek  ellipsis-pair? ellipsis? bad-ellipsis in-template? ellipsis-follows x k */
    r[0] = r[8];  
    r[1] = r[7];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    goto s_check;

case 331: /* clo k x */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
s_check: /* k x ellipsis-pair? ellipsis? bad-ellipsis in-template? ellipsis-follows */
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(331);
    r[7] = (hendblk(5+1));
  if ((ispair((r[1])))) {
    hreserve(hbsz(8+1), 8); /* 8 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[7];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(332);
    r[8] = (hendblk(8+1));
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = obj_from_case(345);
    r[8] = (hendblk(3+1));
    r[9+0] = r[3];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[8];  
    r[9+2] = (car((r[1])));
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (bool_from_obj(cxs_svector_3F((r[1])))) {
    r[8] = (vectorref((r[1]), (0)));
    hreserve(hbsz(5+1), 9); /* 9 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = obj_from_case(346);
    r[9] = (hendblk(5+1));
    r[10+0] = r[2];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[9];  
    r[10+2] = r[8];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[6];  
    *--hp = r[7];  
    *--hp = r[2];  
    *--hp = r[4];  
    *--hp = obj_from_case(347);
    r[8] = (hendblk(6+1));
  if (bool_from_obj(r[5])) {
    r[0] = r[8];  
    /* r[1] */    
    goto gs_template_2Dsbox_3F;
  } else {
    r[9] = (cxs_pattern_2Dsbox_3F((r[1])));
    r[10+0] = obj_from_ktrap();
    r[10+1] = r[9];  
    r[10+2] = r[4];  
    r[10+3] = r[2];  
    r[10+4] = r[7];  
    r[10+5] = r[6];  
    r[10+6] = r[0];  
    r[10+7] = r[1];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v6650;
  }
  }
  }

case 332: /* clo ek  */
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
    /* ek  ellipsis-pair? ellipsis? bad-ellipsis check in-template? ellipsis-follows x k */
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(333);
    r[10] = (hendblk(8+1));
    r[0] = r[5];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (car((r[8])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 333: /* clo ek  */
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
    /* ek  ellipsis-pair? ellipsis? bad-ellipsis check in-template? ellipsis-follows x k */
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(334);
    r[10] = (hendblk(8+1));
    hreserve(hbsz(3+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = obj_from_case(344);
    r[10] = (hendblk(3+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cdr((r[8])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 334: /* clo ek  */
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
    /* ek  ellipsis-pair? ellipsis? bad-ellipsis check in-template? ellipsis-follows x k */
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(335);
    r[10] = (hendblk(8+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cdr((r[8])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 335: /* clo ek r */
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
    /* ek r ellipsis-pair? ellipsis? bad-ellipsis check in-template? ellipsis-follows x k */
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
    *--hp = obj_from_case(336);
    r[10] = (hendblk(8+1));
    r[11] = (cdr((r[8])));
    r[11] = (cdr((r[11])));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[5];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (cdr((r[8])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 336: /* clo ek r */
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
    /* ek r ellipsis-pair? ellipsis? bad-ellipsis check in-template? ellipsis-follows x k */
  if (bool_from_obj(r[1])) {
    r[0] = r[7];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (cdr((r[8])));
    r[3] = (cx__232653);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    hreserve(hbsz(7+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(337);
    r[10] = (hendblk(7+1));
    r[11] = (cdr((r[8])));
    r[11] = (cdr((r[11])));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 337: /* clo ek r */
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
    /* ek r ellipsis? bad-ellipsis check in-template? ellipsis-follows x k */
  if (bool_from_obj(r[1])) {
    r[0] = r[6];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = (cdr((r[7])));
    r[3] = (cx__232648);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
  if (bool_from_obj(r[5])) {
    r[9] = (cdr((r[7])));
    r[9] = (cdr((r[9])));
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = r[9];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(5+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(338);
    r[9] = (hendblk(5+1));
    r[0] = (cx_list_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }

case 338: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r ellipsis? bad-ellipsis check x k */
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(339);
    r[7] = (hendblk(5+1));
  if (bool_from_obj(r[1])) {
    r[0] = obj_from_ktrap();
    /* r[1] */    
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    goto s_l_v6694;
  } else {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[5];  
    r[3] = (cx__232604);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 339: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
s_l_v6694: /* ek  ellipsis? bad-ellipsis check x k */
    r[7] = (cdr((r[5])));
    r[7] = (cdr((r[7])));
    r[8+0] = r[6];  
    r[8+1] = r[7];  
    r[8+2] = r[2];  
    r[8+3] = r[3];  
    r[8+4] = r[5];  
    r[8+5] = r[4];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v6695;

case 340: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
s_loop_v6695: /* k id ellipsis? bad-ellipsis x check */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[6] = (car((r[1])));
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(340);
    r[7] = (hendblk(4+1));
    hreserve(hbsz(5+1), 8); /* 8 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(341);
    r[7] = (hendblk(5+1));
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(343);
    r[7] = (hendblk(3+1));
    r[8+0] = r[2];  
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[7];  
    r[8+2] = r[6];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 341: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek  check y loop id k */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(342);
    r[7] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 342: /* clo ek  */
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

case 343: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r bad-ellipsis x k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = r[3];  
    r[3] = (cx__232606);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 344: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r ellipsis-follows x k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = r[3];  
    r[3] = (cx__232653);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 345: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r ellipsis-follows x k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = r[3];  
    r[3] = (cx__232661);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 346: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r check elts ellipsis-follows x k */
  if (bool_from_obj(r[1])) {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[5];  
    r[3] = (cx__232627);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 347: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
s_l_v6650: /* ek r bad-ellipsis ellipsis-pair? check ellipsis-follows k x */
  if (bool_from_obj(r[1])) {
    r[8] = (boxref((r[7])));
    r[8] = (cdr((r[8])));
    hreserve(hbsz(5+1), 9); /* 9 live regs */
    *--hp = r[6];  
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = r[8];  
    *--hp = r[4];  
    *--hp = obj_from_case(348);
    r[9] = (hendblk(5+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isbox((r[7])))) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[7];  
    r[3] = (cx__232622);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[6];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }

case 348: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r check elts ellipsis-follows x k */
  if (bool_from_obj(r[1])) {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[5];  
    r[3] = (cx__232625);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 349: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r ellipsis-follows x k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = r[3];  
    r[3] = (cx__232671);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 350: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r ellipsis-pair? x ellipsis-follows pat/tmpl k */
  if (bool_from_obj(r[1])) {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[5];  
    r[3] = (cx__232653);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(351);
    r[7] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 351: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r ellipsis-follows pat/tmpl k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = r[3];  
    r[3] = (cx__232670);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

s_loop_v6448: /* k id check-ellipses r ellipsis-pair? ellipsis? */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[6] = (cdr((r[1])));
    hreserve(hbsz(6+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(352);
    r[7] = (hendblk(6+1));
    r[0] = r[7];  
    r[1] = r[6];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    goto s_loop_v6448;
  }

case 352: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r check-ellipses r ellipsis-pair? ellipsis? k id */
    r[8] = (car((r[7])));
    r[9] = (cxs_list2_3F((r[8])));
    hreserve(hbsz(7+1), 10); /* 10 live regs */
    *--hp = r[8];  
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(353);
    r[10] = (hendblk(7+1));
  if (bool_from_obj(r[9])) {
    r[11+0] = obj_from_ktrap();
    r[11+1] = r[9];  
    r[11+2] = r[2];  
    r[11+3] = r[3];  
    r[11+4] = r[4];  
    r[11+5] = r[5];  
    r[11+6] = r[6];  
    r[11+7] = r[1];  
    r[11+8] = r[8];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v6452;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (mknull());
    *--hp = r[8];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    r[0] = (r[10]);
    r[1] = (cx__232486);
    r[2] = (r[11]);
    goto gs_x_2Derror_2A;
  }

case 353: /* clo ek  */
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
s_l_v6452: /* ek  check-ellipses r ellipsis-pair? ellipsis? k r rule */
    r[9] = (cdr((r[8])));
    r[9] = (car((r[9])));
    r[10] = (car((r[8])));
    hreserve(hbsz(8+1), 11); /* 11 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[9];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = (r[10]);
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(354);
    r[11] = (hendblk(8+1));
  if ((ispair((r[10])))) {
    r[0] = (r[11]);
    r[1] = (car((r[10])));
    goto gs_sid_3F;
  } else {
    r[12+0] = obj_from_ktrap();
    r[12+1] = obj_from_bool(0);
    r[12+2] = r[2];  
    r[12+3] = r[3];  
    r[12+4] = (r[10]);
    r[12+5] = r[4];  
    r[12+6] = r[5];  
    r[12+7] = r[9];  
    r[12+8] = r[6];  
    r[12+9] = r[7];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v6454;
  }

case 354: /* clo ek r */
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
s_l_v6454: /* ek r check-ellipses r pat ellipsis-pair? ellipsis? tmpl k r */
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(355);
    r[10] = (hendblk(8+1));
  if (bool_from_obj(r[1])) {
    r[0] = obj_from_ktrap();
    /* r[1] */    
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    /* r[7] */    
    /* r[8] */    
    /* r[9] */    
    goto s_l_v6456;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (mknull());
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    r[0] = (r[10]);
    r[1] = (cx__232479);
    r[2] = (r[11]);
    goto gs_x_2Derror_2A;
  }

case 355: /* clo ek  */
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
s_l_v6456: /* ek  check-ellipses r pat ellipsis-pair? ellipsis? tmpl k r */
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(356);
    r[10] = (hendblk(8+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = r[4];  
    r[3] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 356: /* clo ek  */
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
    /* ek  check-ellipses r pat ellipsis-pair? ellipsis? tmpl k r */
    hreserve(hbsz(7+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(357);
    r[10] = (hendblk(7+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = r[7];  
    r[3] = obj_from_bool(1);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 357: /* clo ek  */
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
    /* ek  r pat ellipsis-pair? ellipsis? tmpl k r */
    r[9] = (cdr((r[3])));
    hreserve(hbsz(5+1), 10); /* 10 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(363);
    r[10] = (hendblk(5+1));
    r[11+0] = (r[10]);
    r[11+1] = r[9];  
    r[11+2] = obj_from_fixnum(0);
    r[11+3] = (mknull());
    r[11+4] = r[4];  
    r[11+5] = r[3];  
    r[11+6] = r[2];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_collect;

case 358: /* clo k x depth l */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1];
    r[1+5] = p[2];
    r[1+6] = p[3]; }
    r += 1; /* shift reg. wnd */
s_collect: /* k x depth l ellipsis-pair? pat r */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(358);
    r[7] = (hendblk(3+1));
    hreserve(hbsz(8+1), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = r[2];  
    *--hp = r[7];  
    *--hp = r[4];  
    *--hp = obj_from_case(359);
    r[7] = (hendblk(8+1));
    r[0] = r[7];  
    /* r[1] */    
    goto gs_sid_3F;

case 359: /* clo ek r */
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
    /* ek r ellipsis-pair? collect depth pat k l r x */
  if (bool_from_obj(r[1])) {
    r[10] = (cxs_sid_2Did((r[9])));
  if (bool_from_obj(cxs_memv_232567((r[10]), (r[8])))) {
    r[0] = r[6];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (bool_from_obj(cxs_assv_23530((r[10]), (r[7])))) {
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = (cx__232561);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    r[0] = r[6];  
    r[1] = (cx__232566);
    r[2] = (r[11]);
    goto gs_x_2Derror_2A;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[4];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = r[7];  
    *--hp = (r[11]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    r[0] = r[6];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }
  } else {
  if ((isvector((r[9])))) {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (vectorref((r[9]), (0)));
    r[3] = r[4];  
    r[4] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
  if ((isbox((r[9])))) {
    r[10] = (boxref((r[9])));
    r[11] = (cdr((r[10])));
    r[10] = (car((r[11])));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (r[10]);
    r[3] = r[4];  
    r[4] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
  if ((ispair((r[9])))) {
    hreserve(hbsz(5+1), 10); /* 10 live regs */
    *--hp = r[6];  
    *--hp = r[9];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[7];  
    *--hp = obj_from_case(360);
    r[10] = (hendblk(5+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cdr((r[9])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[6];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }
  }
  }

case 360: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r l collect depth x k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(361);
    r[7] = (hendblk(4+1));
    r[8] = (cdr((r[5])));
    r[8] = (cdr((r[8])));
    r[9+0] = r[3];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[7];  
    r[9+2] = r[8];  
    r[9+3] = r[4];  
    r[9+4] = r[2];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(362);
    r[7] = (hendblk(4+1));
    r[8+0] = r[3];  
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[7];  
    r[8+2] = (cdr((r[5])));
    r[8+3] = r[4];  
    r[8+4] = r[2];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }

case 361: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r collect depth x k */
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[5];  
    r[6+2] = (car((r[4])));
    r[6+3] = obj_from_fixnum((1) + fixnum_from_obj(r[3]));
    r[6+4] = r[1];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 362: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r collect depth x k */
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[5];  
    r[6+2] = (car((r[4])));
    r[6+3] = r[3];  
    r[6+4] = r[1];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 363: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r ellipsis-pair? ellipsis? tmpl k r */
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(374);
    r[7] = (hendblk(5+1));
    r[8+0] = r[7];  
    r[8+1] = r[4];  
    r[8+2] = obj_from_fixnum(0);
    r[8+3] = r[2];  
    r[8+4] = r[4];  
    r[8+5] = r[1];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_collect_v6500;

case 364: /* clo k x depth */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2];
    r[1+5] = p[3]; }
    r += 1; /* shift reg. wnd */
s_collect_v6500: /* k x depth ellipsis-pair? tmpl r */
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(364);
    r[6] = (hendblk(3+1));
    hreserve(hbsz(7+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = r[3];  
    *--hp = obj_from_case(365);
    r[6] = (hendblk(7+1));
    r[0] = r[6];  
    /* r[1] */    
    goto gs_sid_3F;

case 365: /* clo ek r */
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
    /* ek r ellipsis-pair? collect tmpl k depth r x */
  if (bool_from_obj(r[1])) {
    r[9] = (cxs_sid_2Did((r[8])));
    r[9] = (cxs_assv_23530((r[9]), (r[7])));
  if (bool_from_obj(r[9])) {
    r[10] = (cdr((r[9])));
    hreserve(hbsz(3+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(366);
    r[11] = (hendblk(3+1));
  if (((fixnum_from_obj(r[10]) > (0)) && (!(fixnum_from_obj(r[6]) == fixnum_from_obj(r[10]))))) {
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (mknull());
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = (cx__232495);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = r[8];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    r[0] = (r[11]);
    r[1] = (cx__232508);
    r[2] = (r[12]);
    goto gs_x_2Derror_2A;
  } else {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_void(0);
    r[2] = r[5];  
    r[3] = r[6];  
    r[4] = (r[10]);
    goto s_l_v6546;
  }
  } else {
    r[0] = r[5];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
  if ((isvector((r[8])))) {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (vectorref((r[8]), (0)));
    r[3] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
  if ((isbox((r[8])))) {
    r[9] = (boxref((r[8])));
    r[9] = (cdr((r[9])));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[9];  
    r[3] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
  if ((ispair((r[8])))) {
    hreserve(hbsz(5+1), 9); /* 9 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[8];  
    *--hp = r[6];  
    *--hp = r[3];  
    *--hp = obj_from_case(367);
    r[9] = (hendblk(5+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (cdr((r[8])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[5];  
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

case 366: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
s_l_v6546: /* ek  k depth pat-depth */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(fixnum_from_obj(r[3]) == fixnum_from_obj(r[4]));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 367: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r collect depth x tmpl k */
    hreserve(hbsz(6+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(368);
    r[7] = (hendblk(6+1));
    r[8+0] = r[2];  
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[7];  
    r[8+2] = (car((r[4])));
    r[8+3] = (bool_from_obj(r[1]) ? obj_from_fixnum((1) + fixnum_from_obj(r[3])) : (r[3]));
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 368: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r collect depth x tmpl r k */
  if (bool_from_obj(r[6])) {
    hreserve(hbsz(0+1), 8); /* 8 live regs */
    *--hp = obj_from_case(369);
    r[8] = (hendblk(0+1));
  } else {
    hreserve(hbsz(0+1), 8); /* 8 live regs */
    *--hp = obj_from_case(370);
    r[8] = (hendblk(0+1));
  }
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[1];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(371);
    r[9] = (hendblk(7+1));
    r[0] = r[8];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 369: /* clo k x */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k x */
    r[2] = (cdr((r[1])));
    r[2] = (cdr((r[2])));
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 370: /* clo k p */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k p */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cdr((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 371: /* clo ek r */
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
    /* ek r collect depth x tmpl r k r */
    hreserve(hbsz(5+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(372);
    r[9] = (hendblk(5+1));
    r[10+0] = r[2];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[9];  
    r[10+2] = r[1];  
    r[10+3] = r[3];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 372: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r x tmpl r k r */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = obj_from_case(373);
    r[7] = (hendblk(3+1));
  if (bool_from_obj(r[4])) {
  if ((!bool_from_obj(r[6]))) {
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = (cx__232495);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    r[0] = r[7];  
    r[1] = (cx__232500);
    r[2] = r[8];  
    goto gs_x_2Derror_2A;
  } else {
    r[8+0] = obj_from_ktrap();
    r[8+1] = obj_from_bool(0);
    r[8+2] = r[5];  
    r[8+3] = r[1];  
    r[8+4] = r[6];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v6512;
  }
  } else {
    r[8+0] = obj_from_ktrap();
    r[8+1] = obj_from_bool(0);
    r[8+2] = r[5];  
    r[8+3] = r[1];  
    r[8+4] = r[6];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v6512;
  }

case 373: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
s_l_v6512: /* ek  k r r */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (bool_from_obj(r[4]) ? (r[4]) : (r[3]));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 374: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek  ellipsis? r tmpl k r */
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(379);
    r[7] = (hendblk(2+1));
    r[8+0] = r[7];  
    r[8+1] = r[4];  
    r[8+2] = (mknull());
    r[8+3] = r[2];  
    r[8+4] = r[3];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_collect_v6471;

case 375: /* clo k x lits */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2]; }
    r += 1; /* shift reg. wnd */
s_collect_v6471: /* k x lits ellipsis? r */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(375);
    r[5] = (hendblk(2+1));
    hreserve(hbsz(5+1), 6); /* 6 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = obj_from_case(376);
    r[5] = (hendblk(5+1));
    r[6+0] = r[3];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[5];  
    r[6+2] = r[1];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 376: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r collect r x k lits */
  if (bool_from_obj(r[1])) {
    r[0] = r[5];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[2];  
    *--hp = obj_from_case(377);
    r[7] = (hendblk(5+1));
    r[0] = r[7];  
    r[1] = r[4];  
    goto gs_sid_3F;
  }

case 377: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r collect k lits r x */
  if (bool_from_obj(r[1])) {
    r[7] = (cxs_sid_2Did((r[6])));
    r[7] = (cxs_assv_23530((r[7]), (r[5])));
  if (bool_from_obj(r[7])) {
    r[7] = r[4];  
  } else {
    r[7] = (cxs_sid_2Did((r[6])));
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[4];  
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
  }
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[6])))) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = (vectorref((r[6]), (0)));
    r[3] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
  if ((isbox((r[6])))) {
    r[7] = (boxref((r[6])));
    r[7] = (cdr((r[7])));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = r[7];  
    r[3] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
  if ((ispair((r[6])))) {
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[3];  
    *--hp = r[6];  
    *--hp = r[2];  
    *--hp = obj_from_case(378);
    r[7] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = (cdr((r[6])));
    r[3] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }
  }
  }

case 378: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r collect x k */
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (car((r[3])));
    r[5+3] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 379: /* clo ek r */
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

case 380: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r rules ellipsis-id env k synrules r */
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    hreserve(hbsz(5+1), 9); /* 9 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(381);
    r[9] = (hendblk(5+1));
    r[0] = r[9];  
    r[1] = r[8];  
    goto gs_append_2A;

case 381: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r rules ellipsis-id env k synrules */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(382);
    r[7] = (hendblk(3+1));
  if (bool_from_obj(objptr_from_obj(r[3])[0])) {
    r[0] = obj_from_ktrap();
    /* r[1] */    
    r[2] = r[4];  
    r[3] = r[5];  
    r[4] = r[6];  
    goto s_l_v6419;
  } else {
    r[8+0] = r[7];  
    r[8+1] = r[2];  
    r[8+2] = r[1];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_list_2Ddots_2Dids;
  }

case 382: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
s_l_v6419: /* ek r env k synrules */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(383);
    r[5] = (hendblk(2+1));
    r[6+0] = r[5];  
    r[6+1] = r[1];  
    r[6+2] = (mknull());
    r[6+3] = r[2];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v6426;

s_loop_v6426: /* k ids reduced-env env */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[4] = (cdr((r[1])));
    r[5] = (car((r[1])));
    r[6] = (cxs_assv_23530((r[5]), (r[2])));
    r[6] = obj_from_bool(!bool_from_obj(r[6]));
  if (bool_from_obj(r[6])) {
    r[6] = (cxs_assv_23530((r[5]), (r[3])));
  } else {
    r[6] = obj_from_bool(0);
  }
  if (bool_from_obj(r[6])) {
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[2];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
  } else {
    r[5] = r[2];  
  }
    /* r[0] */    
    r[1] = r[4];  
    r[2] = r[5];  
    /* r[3] */    
    goto s_loop_v6426;
  }

case 383: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k synrules */
    r[4] = (cdr((r[3])));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("syntax-rules")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
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

case 384: /* clo k x ids */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
s_list_2Ddots_2Dids: /* k x ids */
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(384);
    r[3] = (hendblk(0+1));
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[3];  
    *--hp = obj_from_case(385);
    r[3] = (hendblk(4+1));
    r[0] = r[3];  
    /* r[1] */    
    goto gs_sid_3F;

case 385: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r list-dots-ids k ids x */
  if (bool_from_obj(r[1])) {
    r[6] = (cxs_sid_2Dlocation((r[5])));
    r[6] = obj_from_bool((mksymbol(internsym("..."))) == (r[6]));
  if (bool_from_obj(r[6])) {
    r[6] = (cxs_sid_2Did((r[5])));
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
  } else {
    r[6] = r[4];  
  }
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[5])))) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = (vectorref((r[5]), (0)));
    r[3] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
  if ((isbox((r[5])))) {
    r[6] = (boxref((r[5])));
    r[6] = (cdr((r[6])));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = r[6];  
    r[3] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
  if ((ispair((r[5])))) {
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = r[2];  
    *--hp = obj_from_case(386);
    r[6] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (cdr((r[5])));
    r[3] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }
  }
  }

case 386: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r list-dots-ids x k */
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (car((r[3])));
    r[5+3] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 387: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k synrules */
  if (bool_from_obj(r[1])) {
    r[4] = (cdr((r[3])));
    r[4] = (car((r[4])));
    r[4] = (cxs_sid_2Did((r[4])));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
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

case 388: /* apply-synrules k transformer sexp id-n env k */
    assert(rc == 7);
    r += 1; /* shift reg. wnd */
gs_apply_2Dsynrules: /* k transformer sexp id-n env k */
    r[6] = (car((r[1])));
    r[7] = (cdr((r[1])));
    r[7] = (car((r[7])));
    hreserve(hbsz(7+1), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[7];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[2];  
    *--hp = r[5];  
    *--hp = r[0];  
    *--hp = obj_from_case(389);
    r[8] = (hendblk(7+1));
    hreserve(hbsz(2+1), 9); /* 9 live regs */
    *--hp = r[6];  
    *--hp = r[8];  
    *--hp = obj_from_case(458);
    r[8] = (hendblk(2+1));
    r[9] = (cdr((r[6])));
    r[9] = (car((r[9])));
    r[0] = r[8];  
    r[1] = r[9];  
    goto gs_sid_3F;

case 389: /* clo ek r */
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
    /* ek r k k sexp env id-n mac-env synrules */
  if (bool_from_obj(r[1])) {
    r[9] = (cdr((r[8])));
    r[9] = (cdr((r[9])));
  } else {
    r[9] = (cdr((r[8])));
  }
    r[10] = (car((r[9])));
    hreserve(hbsz(8+1), 11); /* 11 live regs */
    *--hp = r[9];  
    *--hp = r[1];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(391);
    r[11] = (hendblk(8+1));
    r[0] = (r[11]);
    r[1] = (r[10]);
    goto s_loop_v6353;

s_loop_v6353: /* k id */
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
    *--hp = obj_from_case(390);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v6353;
  }

case 390: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    r[4] = (cxs_sid_2Did((r[4])));
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

case 391: /* clo ek r */
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
    /* ek r k k sexp env id-n mac-env r rest */
    r[10] = (cdr((r[9])));
    hreserve(hbsz(1+1), 11); /* 11 live regs */
    *--hp = r[1];  
    *--hp = obj_from_case(392);
    r[11] = (hendblk(1+1));
    hreserve(hbsz(2+1), 12); /* 12 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = obj_from_case(393);
    r[12] = (hendblk(2+1));
    hreserve(hbsz(1+1), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = obj_from_case(395);
    r[13] = (hendblk(1+1));
    hreserve(hbsz(8+1), 14); /* 14 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = (r[13]);
    *--hp = (r[12]);
    *--hp = r[3];  
    *--hp = (r[11]);
    *--hp = obj_from_case(402);
    r[14] = (hendblk(8+1));
    r[15+0] = r[2];  
    r[15+1] = (r[10]);
    r[15+2] = r[7];  
    r[15+3] = r[5];  
    r[15+4] = r[1];  
    r[15+5] = (r[12]);
    r[15+6] = (r[13]);
    r[15+7] = (r[11]);
    r[15+8] = (r[14]);
    r[15+9] = r[4];  
    r += 15; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v5890;

case 392: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k id r */
    r[3] = (cxs_memv_232567((r[1]), (r[2])));
    r[3] = obj_from_bool(!bool_from_obj(r[3]));
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 393: /* clo k x */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* k x mac-env r */
  if ((ispair((r[1])))) {
    r[4] = (car((r[1])));
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = obj_from_case(394);
    r[5] = (hendblk(4+1));
    r[0] = r[5];  
    r[1] = r[4];  
    goto gs_sid_3F;
  } else {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 394: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r k mac-env x r */
  if (bool_from_obj(r[1])) {
  if (bool_from_obj(r[5])) {
    r[6] = (cxs_sid_2Did((r[4])));
    r[6] = obj_from_bool(((r[5]) == (r[6])) || ((is_flonum_obj(r[5])) && ((is_flonum_obj(r[6])) && (flonum_from_obj(r[5]) == flonum_from_obj(r[6])))));
  } else {
    r[6] = (cxs_lookup_2Dsid((r[4]), (r[3])));
    r[6] = obj_from_bool((mksymbol(internsym("..."))) == (r[6]));
  }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
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

case 395: /* clo k x include-scalars pred? */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k x include-scalars pred? ellipsis-pair? */
    r[5+0] = r[0];  
    r[5+1] = r[1];  
    r[5+2] = r[2];  
    r[5+3] = (mknull());
    r[5+4] = r[4];  
    r[5+5] = r[3];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_collect_v6284;

case 396: /* clo k x inc l */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1];
    r[1+5] = p[2]; }
    r += 1; /* shift reg. wnd */
s_collect_v6284: /* k x inc l ellipsis-pair? pred? */
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(396);
    r[6] = (hendblk(2+1));
    hreserve(hbsz(7+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = obj_from_case(397);
    r[6] = (hendblk(7+1));
    r[0] = r[6];  
    /* r[1] */    
    goto gs_sid_3F;

case 397: /* clo ek r */
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
    /* ek r ellipsis-pair? collect pred? inc k l x */
  if (bool_from_obj(r[1])) {
    r[9] = (cxs_sid_2Did((r[8])));
    hreserve(hbsz(3+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_case(398);
    r[10] = (hendblk(3+1));
  if (bool_from_obj(r[5])) {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = r[9];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_bool(0);
    r[2] = r[6];  
    r[3] = r[7];  
    r[4] = r[9];  
    goto s_l_v6306;
  }
  } else {
  if ((isvector((r[8])))) {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (vectorref((r[8]), (0)));
    r[3] = r[5];  
    r[4] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
  if ((isbox((r[8])))) {
    r[9] = (boxref((r[8])));
    r[9] = (cdr((r[9])));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[9];  
    r[3] = r[5];  
    r[4] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
  if ((ispair((r[8])))) {
    hreserve(hbsz(5+1), 9); /* 9 live regs */
    *--hp = r[6];  
    *--hp = r[8];  
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = r[7];  
    *--hp = obj_from_case(399);
    r[9] = (hendblk(5+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (cdr((r[8])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[6];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }
  }
  }

case 398: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
s_l_v6306: /* ek r k l id */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 399: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r l inc collect x k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(400);
    r[7] = (hendblk(3+1));
    r[8] = (cdr((r[5])));
    r[8] = (cdr((r[8])));
    r[9+0] = r[4];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[7];  
    r[9+2] = r[8];  
    r[9+3] = r[3];  
    r[9+4] = r[2];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = obj_from_case(401);
    r[7] = (hendblk(4+1));
    r[8+0] = r[4];  
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[7];  
    r[8+2] = (cdr((r[5])));
    r[8+3] = r[3];  
    r[8+4] = r[2];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }

case 400: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r collect x k */
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (car((r[3])));
    r[5+3] = obj_from_bool(1);
    r[5+4] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 401: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r collect inc x k */
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[5];  
    r[6+2] = (car((r[4])));
    r[6+3] = r[3];  
    r[6+4] = r[1];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 402: /* clo k pat tmpl top-bindings */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1];
    r[1+5] = p[2];
    r[1+6] = p[3];
    r[1+7] = p[4];
    r[1+8] = p[5];
    r[1+9] = p[6];
    r[1+10] = p[7];
    r[1+11] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* k pat tmpl top-bindings not-pat-literal? k ellipsis-pair? list-ids sexp env id-n mac-env */
    hreserve(hbsz(1), 12); /* 12 live regs */
    *--hp = obj_from_void(0);
    r[12] = (hendblk(1));
    hreserve(hbsz(1), 13); /* 13 live regs */
    *--hp = obj_from_void(0);
    r[13] = (hendblk(1));
    hreserve(hbsz(9+1), 14); /* 14 live regs */
    *--hp = r[3];  
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = (r[13]);
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_case(408);
    r[14] = (hendblk(9+1));
    hreserve(hbsz(9+1), 15); /* 15 live regs */
    *--hp = (r[12]);
    *--hp = (r[13]);
    *--hp = r[0];  
    *--hp = (r[10]);
    *--hp = r[5];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = (r[14]);
    *--hp = r[7];  
    *--hp = obj_from_case(403);
    r[14] = (hendblk(9+1));
    r[15+0] = r[7];  
    pc = objptr_from_obj(r[15+0])[0];
    r[15+1] = (r[14]);
    r[15+2] = r[1];  
    r[15+3] = obj_from_bool(0);
    r[15+4] = r[4];  
    r += 15; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 403: /* clo ek r */
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
    /* ek r list-ids expand top-bindings tmpl k id-n k ellipsis-vars tmpl-literals */
    hreserve(hbsz(9+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[1];  
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(404);
    r[11] = (hendblk(9+1));
    hreserve(hbsz(1+1), 12); /* 12 live regs */
    *--hp = r[4];  
    *--hp = obj_from_case(407);
    r[12] = (hendblk(1+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[11]);
    r[2] = r[5];  
    r[3] = obj_from_bool(1);
    r[4] = (r[12]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 404: /* clo ek r */
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
    /* ek r expand top-bindings tmpl k id-n k ellipsis-vars r tmpl-literals */
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
    *--hp = obj_from_case(405);
    r[11] = (hendblk(9+1));
    r[0] = (r[11]);
    /* r[1] */    
    r[2] = (mknull());
    goto s_loop_v6256;

s_loop_v6256: /* k l result */
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
    r[4] = (car((r[1])));
  if (bool_from_obj(cxs_memv_232567((r[4]), (r[2])))) {
    r[4] = r[2];  
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[2];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
  }
    /* r[0] */    
    r[1] = r[3];  
    r[2] = r[4];  
    goto s_loop_v6256;
  }

case 405: /* clo ek r */
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
    /* ek r expand top-bindings tmpl k id-n k ellipsis-vars r tmpl-literals */
    (void)(objptr_from_obj(r[10])[0] = (r[1]));
    (void)(objptr_from_obj(r[8])[0] = (r[9]));
    hreserve(hbsz(4+1), 11); /* 11 live regs */
    *--hp = r[7];  
    *--hp = (r[10]);
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(406);
    r[11] = (hendblk(4+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[11]);
    r[2] = r[4];  
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 406: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id-n tmpl-literals k */
    { fixnum_t v9244_tmp;
    r[6] = (objptr_from_obj(r[4])[0]);
    { /* length */
    int n; obj l = r[6];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v9244_tmp = (n); }
    r[6] = obj_from_fixnum(fixnum_from_obj(r[3]) + (v9244_tmp)); }
    r[7+0] = r[2];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[5];  
    r[7+2] = r[1];  
    r[7+3] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 407: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k id top-bindings */
    r[3] = (cxs_assv_23530((r[1]), (r[2])));
    r[3] = obj_from_bool(!bool_from_obj(r[3]));
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 408: /* clo k tmpl bindings */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2];
    r[1+5] = p[3];
    r[1+6] = p[4];
    r[1+7] = p[5];
    r[1+8] = p[6];
    r[1+9] = p[7];
    r[1+10] = p[8];
    r[1+11] = p[9]; }
    r += 1; /* shift reg. wnd */
    /* k tmpl bindings ellipsis-pair? list-ids ellipsis-vars sexp env id-n mac-env tmpl-literals top-bindings */
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
    *--hp = obj_from_case(408);
    r[12] = (hendblk(9+1));
    r[13+0] = r[0];  
    r[13+1] = r[1];  
    r[13+2] = r[3];  
    r[13+3] = r[4];  
    r[13+4] = r[5];  
    r[13+5] = r[6];  
    r[13+6] = (r[12]);
    r[13+7] = r[7];  
    r[13+8] = r[8];  
    r[13+9] = r[9];  
    r[13+10] = (r[10]);
    r[13+11] = (r[11]);
    r[13+12] = r[2];  
    r += 13; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_expand_2Dpart;

case 409: /* clo k tmpl */
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
s_expand_2Dpart: /* k tmpl ellipsis-pair? list-ids ellipsis-vars sexp expand env id-n mac-env tmpl-literals top-bindings bindings */
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
    *--hp = obj_from_case(409);
    r[13] = (hendblk(11+1));
    hreserve(hbsz(14+1), 14); /* 14 live regs */
    *--hp = r[1];  
    *--hp = (r[12]);
    *--hp = r[0];  
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = (r[13]);
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(410);
    r[13] = (hendblk(14+1));
    r[0] = (r[13]);
    /* r[1] */    
    goto gs_sid_3F;

case 410: /* clo ek r */
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
    r[1+12] = p[11];
    r[1+13] = p[12];
    r[1+14] = p[13];
    r[1+15] = p[14]; }
    r += 1; /* shift reg. wnd */
    /* ek r ellipsis-pair? list-ids ellipsis-vars sexp expand env expand-part id-n mac-env tmpl-literals top-bindings k bindings tmpl */
  if (bool_from_obj(r[1])) {
    r[16] = (cxs_sid_2Did((r[15])));
    r[17] = (cxs_assv_23530((r[16]), (r[14])));
  if (bool_from_obj(r[17])) {
    r[0] = (r[13]);
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (cdr((r[17])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[18] = (cxs_assv_23530((r[16]), (r[12])));
  if (bool_from_obj(r[18])) {
    r[0] = (r[13]);
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (cdr((r[18])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    { fixnum_t v9243_index;
    { fixnum_t v9242_tmp;
    r[19] = (objptr_from_obj(r[11])[0]);
    r[19] = (cxs_memv_232567((r[16]), (r[19])));
    { /* length */
    int n; obj l = (r[19]);
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v9242_tmp = (n); }
    v9243_index = ((-1) + (v9242_tmp)); }
    r[19] = (cxs_lookup_2Dsid((r[15]), (r[10])));
    r[0] = (r[13]);
    r[1] = (cxs_sid_2Dname((r[15])));
    r[2] = obj_from_fixnum(fixnum_from_obj(r[9]) + (v9243_index));
    r[3] = (r[19]);
    goto gs_make_2Dsid; }
  }
  }
  } else {
  if ((isvector((r[15])))) {
    hreserve(hbsz(1+1), 16); /* 16 live regs */
    *--hp = (r[13]);
    *--hp = obj_from_case(411);
    r[16] = (hendblk(1+1));
    r[0] = r[8];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[16]);
    r[2] = (vectorref((r[15]), (0)));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isbox((r[15])))) {
    hreserve(hbsz(4+1), 16); /* 16 live regs */
    *--hp = (r[13]);
    *--hp = r[7];  
    *--hp = (r[15]);
    *--hp = r[8];  
    *--hp = obj_from_case(412);
    r[16] = (hendblk(4+1));
    r[0] = (r[16]);
    r[1] = (r[15]);
    goto gs_template_2Dsbox_2D_3Econv;
  } else {
  if ((ispair((r[15])))) {
    hreserve(hbsz(8+1), 16); /* 16 live regs */
    *--hp = (r[13]);
    *--hp = (r[15]);
    *--hp = r[6];  
    *--hp = r[8];  
    *--hp = r[5];  
    *--hp = (r[14]);
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(414);
    r[16] = (hendblk(8+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[16]);
    r[2] = (cdr((r[15])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = (r[13]);
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (r[15]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }
  }
  }

case 411: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* vector */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = obj_from_size(VECTOR_BTAG);
    r[3] = (hendblk(1+1)); }
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 412: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r expand-part tmpl env k */
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_case(413);
    r[6] = (hendblk(3+1));
    r[7] = (boxref((r[3])));
    r[7] = (cdr((r[7])));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 413: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r r env k */
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = r[1];  
    r[5+3] = r[3];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 414: /* clo ek r */
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
    /* ek r list-ids ellipsis-vars bindings sexp expand-part expand tmpl k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(6+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(415);
    r[10] = (hendblk(6+1));
    hreserve(hbsz(1+1), 11); /* 11 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(429);
    r[11] = (hendblk(1+1));
    r[12+0] = r[2];  
    pc = objptr_from_obj(r[12+0])[0];
    r[12+1] = (r[10]);
    r[12+2] = (car((r[8])));
    r[12+3] = obj_from_bool(1);
    r[12+4] = (r[11]);
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[6];  
    *--hp = obj_from_case(430);
    r[10] = (hendblk(3+1));
    r[0] = r[6];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cdr((r[8])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 415: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r bindings sexp expand-part expand tmpl k */
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(417);
    r[8] = (hendblk(6+1));
    r[0] = r[8];  
    /* r[1] */    
    /* r[2] */    
    goto s_loop_v6186;

s_loop_v6186: /* k id bindings */
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
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = obj_from_case(416);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v6186;
  }

case 416: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k bindings id */
    r[5] = (car((r[4])));
    r[6] = (cxs_assv_23530((r[5]), (r[3])));
    r[5] = (cdr((r[6])));
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 417: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r sexp expand-part r expand tmpl k */
    r[8] = (cdr((r[1])));
    r[8] = obj_from_bool(isnull((r[8])));
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(418);
    r[9] = (hendblk(7+1));
  if (bool_from_obj(r[8])) {
    r[10+0] = obj_from_ktrap();
    r[10+1] = r[8];  
    r[10+2] = r[2];  
    r[10+3] = r[3];  
    r[10+4] = r[1];  
    r[10+5] = r[4];  
    r[10+6] = r[5];  
    r[10+7] = r[6];  
    r[10+8] = r[7];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v6123;
  } else {
    hreserve(hbsz(1+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = obj_from_case(427);
    r[10] = (hendblk(1+1));
    r[0] = (r[10]);
    /* r[1] */    
    goto s_loop_v6111;
  }

case 418: /* clo ek r */
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
s_l_v6123: /* ek r sexp expand-part r r expand tmpl k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(5+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(419);
    r[9] = (hendblk(5+1));
    r[10] = (cdr((r[7])));
    r[10] = (cdr((r[10])));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (r[10]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = (cx__232817);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[0] = r[8];  
    r[1] = (cx__232822);
    r[2] = r[9];  
    goto gs_x_2Derror_2A;
  }

case 419: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r r r expand tmpl k */
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(420);
    r[7] = (hendblk(5+1));
    r[0] = r[7];  
    r[1] = r[2];  
    goto gs_apply_2Dmap_2Dlist;

case 420: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r r expand tmpl k r */
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(425);
    r[7] = (hendblk(2+1));
    r[0] = r[7];  
    /* r[1] */    
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    goto s_loop_v6134;

s_loop_v6134: /* k id r expand tmpl */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[5] = (cdr((r[1])));
    hreserve(hbsz(5+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(421);
    r[6] = (hendblk(5+1));
    r[0] = r[6];  
    r[1] = r[5];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    goto s_loop_v6134;
  }

case 421: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r r expand tmpl k id */
    r[7] = (car((r[6])));
    hreserve(hbsz(4+1), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(423);
    r[8] = (hendblk(4+1));
    r[0] = r[8];  
    r[1] = r[2];  
    r[2] = r[7];  
    goto s_loop_v6149;

s_loop_v6149: /* k id id */
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
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(422);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[3];  
    r[2] = r[4];  
    goto s_loop_v6149;
  }

case 422: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id id */
    r[5] = (car((r[4])));
    r[6] = (car((r[3])));
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 423: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r expand tmpl k r */
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(424);
    r[6] = (hendblk(2+1));
    r[7+0] = r[2];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[6];  
    r[7+2] = (car((r[3])));
    r[7+3] = r[1];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 424: /* clo ek r */
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

case 425: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { fixnum_t v9241_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v9241_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v9241_tmp);
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

s_loop_v6111: /* k id */
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
    *--hp = obj_from_case(426);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v6111;
  }

case 426: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    { /* length */
    int n; obj l = r[4];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    r[4] = obj_from_fixnum(n); }
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

case 427: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(428);
    r[3] = (hendblk(0+1));
    r[4+0] = r[2];  
    r[4+1] = r[3];  
    r[4+2] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_pairwise_2Dandmap;

case 428: /* clo k x y */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k x y */
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(fixnum_from_obj(r[1]) == fixnum_from_obj(r[2]));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 429: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k id ellipsis-vars */
    r[3] = (objptr_from_obj(r[2])[0]);
    r[3] = (cxs_memv_232567((r[1]), (r[3])));
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 430: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r expand-part tmpl k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(431);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 431: /* clo ek r */
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

case 432: /* clo k rules */
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
s_loop_v5890: /* k rules mac-env env r ellipsis-pair? list-ids not-pat-literal? expand-template sexp */
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(432);
    r[10] = (hendblk(8+1));
  if ((isnull((r[1])))) {
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (mknull());
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    /* r[0] */    
    r[1] = (cx__233084);
    r[2] = (r[11]);
    goto gs_x_2Derror_2A;
  } else {
    r[11] = (car((r[1])));
    r[12] = (car((r[11])));
    r[13] = (cdr((r[11])));
    r[13] = (car((r[13])));
  if ((ispair((r[12])))) {
    hreserve(hbsz(11+1), 14); /* 14 live regs */
    *--hp = (r[12]);
    *--hp = r[0];  
    *--hp = (r[13]);
    *--hp = r[8];  
    *--hp = r[9];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = (r[10]);
    *--hp = obj_from_case(433);
    r[14] = (hendblk(11+1));
  if ((ispair((r[9])))) {
    r[15] = (cdr((r[12])));
    r[16] = (cdr((r[9])));
    r[17+0] = (r[14]);
    r[17+1] = (r[15]);
    r[17+2] = (r[16]);
    r[17+3] = r[5];  
    r[17+4] = r[2];  
    r[17+5] = r[3];  
    r[17+6] = r[4];  
    r += 17; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_match;
  } else {
    r[15+0] = obj_from_ktrap();
    r[15+1] = obj_from_bool(0);
    r[15+2] = (r[10]);
    r[15+3] = r[1];  
    r[15+4] = r[4];  
    r[15+5] = r[5];  
    r[15+6] = r[6];  
    r[15+7] = r[7];  
    r[15+8] = r[9];  
    r[15+9] = r[8];  
    r[15+10] = (r[13]);
    r[15+11] = r[0];  
    r[15+12] = (r[12]);
    r += 15; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v5961;
  }
  } else {
    hreserve(hbsz(5+1), 14); /* 14 live regs */
    *--hp = r[0];  
    *--hp = (r[13]);
    *--hp = r[8];  
    *--hp = r[1];  
    *--hp = (r[10]);
    *--hp = obj_from_case(457);
    r[14] = (hendblk(5+1));
    r[0] = (r[14]);
    r[1] = r[9];  
    goto gs_sid_3F;
  }
  }

case 433: /* clo ek r */
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
s_l_v5961: /* ek r loop rules r ellipsis-pair? list-ids not-pat-literal? sexp expand-template tmpl k pat */
  if (bool_from_obj(r[1])) {
    r[13] = (cdr((r[12])));
    r[14] = (cdr((r[8])));
    hreserve(hbsz(4+1), 15); /* 15 live regs */
    *--hp = (r[11]);
    *--hp = (r[12]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = obj_from_case(447);
    r[15] = (hendblk(4+1));
    r[0] = (r[15]);
    r[1] = (r[13]);
    r[2] = (r[14]);
    r[3] = (mknull());
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    /* r[7] */    
    goto s_collect_v5968;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[11]);
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 434: /* clo k pat sexp bindings */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1];
    r[1+5] = p[2];
    r[1+6] = p[3];
    r[1+7] = p[4]; }
    r += 1; /* shift reg. wnd */
s_collect_v5968: /* k pat sexp bindings r ellipsis-pair? list-ids not-pat-literal? */
    hreserve(hbsz(4+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(434);
    r[8] = (hendblk(4+1));
    hreserve(hbsz(8+1), 9); /* 9 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(435);
    r[8] = (hendblk(8+1));
    hreserve(hbsz(3+1), 9); /* 9 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[8];  
    *--hp = obj_from_case(446);
    r[8] = (hendblk(3+1));
    r[0] = r[8];  
    /* r[1] */    
    goto gs_sid_3F;

case 435: /* clo ek r */
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
    /* ek r ellipsis-pair? list-ids not-pat-literal? collect k bindings sexp pat */
  if (bool_from_obj(r[1])) {
    r[10] = (cxs_sid_2Did((r[9])));
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[8];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = r[7];  
    *--hp = (r[11]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[0] = r[6];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (r[10]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (bool_from_obj(cxs_svector_3F((r[9])))) {
    r[0] = r[5];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (vectorref((r[9]), (0)));
    r[3] = (vectorref((r[8]), (0)));
    r[4] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
  if ((isbox((r[9])))) {
    r[10] = (boxref((r[9])));
    r[11] = (cdr((r[10])));
    r[10] = (car((r[11])));
    r[0] = r[5];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (r[10]);
    r[3] = r[8];  
    r[4] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
  if ((!(ispair((r[9]))))) {
    r[0] = r[6];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(7+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[6];  
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(436);
    r[10] = (hendblk(7+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cdr((r[9])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }
  }
  }

case 436: /* clo ek r */
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
    /* ek r list-ids not-pat-literal? collect bindings k sexp pat */
  if (bool_from_obj(r[1])) {
    { fixnum_t v9236_tail_2Dlen;
    r[9] = (cdr((r[8])));
    r[9] = (cdr((r[9])));
    { /* length */
    int n; obj l = r[9];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v9236_tail_2Dlen = (n); }
    { fixnum_t v9238_tmp;
    { fixnum_t v9237_tmp;
    { /* length */
    int n; obj l = r[7];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v9237_tmp = (n); }
    v9238_tmp = ((v9237_tmp) - (v9236_tail_2Dlen)); }
    { /* list-tail */
    obj l = r[7];   int c = (v9238_tmp);
    while (c-- > 0) l = cdr(l);
    r[9] = (l); } }
    { fixnum_t v9239_tmp;
    { /* length */
    int n; obj l = r[7];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v9239_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v9239_tmp);
    hreserve(hbsz(3)*c, 10); /* 10 live regs */
    l = r[7];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[10] = (o); } }
    { /* list-tail */
    obj l = (r[10]); int c = (v9236_tail_2Dlen);
    while (c-- > 0) l = cdr(l);
    r[10] = (l); }
    { fixnum_t v9240_tmp;
    { /* length */
    int n; obj l = (r[10]);
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v9240_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v9240_tmp);
    hreserve(hbsz(3)*c, 11); /* 11 live regs */
    l = (r[10]); /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[10] = (o); } }
    hreserve(hbsz(6+1), 11); /* 11 live regs */
    *--hp = r[6];  
    *--hp = r[8];  
    *--hp = r[9];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = (r[10]);
    *--hp = obj_from_case(437);
    r[11] = (hendblk(6+1));
    r[12+0] = r[2];  
    pc = objptr_from_obj(r[12+0])[0];
    r[12+1] = (r[11]);
    r[12+2] = (car((r[8])));
    r[12+3] = obj_from_bool(1);
    r[12+4] = r[3];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump; }
  } else {
    hreserve(hbsz(4+1), 9); /* 9 live regs */
    *--hp = r[6];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[4];  
    *--hp = obj_from_case(445);
    r[9] = (hendblk(4+1));
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (cdr((r[8])));
    r[3] = (cdr((r[7])));
    r[4] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }

case 437: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r matches collect bindings tail pat k */
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(442);
    r[8] = (hendblk(6+1));
    r[0] = r[8];  
    r[1] = r[2];  
    r[2] = r[3];  
    r[3] = r[6];  
    goto s_loop_v6005;

s_loop_v6005: /* k id collect pat */
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
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(438);
    r[5] = (hendblk(4+1));
    r[0] = r[5];  
    r[1] = r[4];  
    /* r[2] */    
    /* r[3] */    
    goto s_loop_v6005;
  }

case 438: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r collect pat k id */
    r[6] = (car((r[5])));
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(439);
    r[7] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = (car((r[3])));
    r[3] = r[6];  
    r[4] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 439: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(441);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    /* r[1] */    
    goto s_loop_v6016;

s_loop_v6016: /* k id */
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
    *--hp = obj_from_case(440);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v6016;
  }

case 440: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    r[4] = (cdr((r[4])));
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

case 441: /* clo ek r */
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

case 442: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r collect bindings tail pat k r */
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    hreserve(hbsz(5+1), 9); /* 9 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(443);
    r[9] = (hendblk(5+1));
    r[0] = r[9];  
    r[1] = r[8];  
    goto gs_apply_2Dmap_2Dlist;

case 443: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r collect bindings tail pat k */
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = obj_from_case(444);
    r[7] = (hendblk(2+1));
    r[8] = (cdr((r[5])));
    r[8] = (cdr((r[8])));
    r[9+0] = r[2];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[7];  
    r[9+2] = r[8];  
    r[9+3] = r[4];  
    r[9+4] = r[3];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 444: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { fixnum_t v9235_tmp;
    { /* length */
    int n; obj l = r[3];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v9235_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v9235_tmp);
    hreserve(hbsz(3)*c, 4); /* 4 live regs */
    l = r[3];   t = r[1];   /* gc-safe */
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

case 445: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r collect sexp pat k */
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[5];  
    r[6+2] = (car((r[4])));
    r[6+3] = (car((r[3])));
    r[6+4] = r[1];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 446: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r pat */
  if (bool_from_obj(r[1])) {
    r[5] = (cxs_sid_2Did((r[4])));
    r[5] = (cxs_memv_232567((r[5]), (r[3])));
    r[5] = obj_from_bool(!bool_from_obj(r[5]));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
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

case 447: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r expand-template tmpl pat k */
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[5];  
    r[6+2] = (cdr((r[4])));
    r[6+3] = r[3];  
    r[6+4] = r[1];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 448: /* clo k pat sexp */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2];
    r[1+5] = p[3];
    r[1+6] = p[4]; }
    r += 1; /* shift reg. wnd */
s_match: /* k pat sexp ellipsis-pair? mac-env env r */
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(448);
    r[7] = (hendblk(4+1));
    hreserve(hbsz(8+1), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = r[2];  
    *--hp = r[4];  
    *--hp = r[7];  
    *--hp = r[3];  
    *--hp = obj_from_case(449);
    r[7] = (hendblk(8+1));
    r[0] = r[7];  
    /* r[1] */    
    goto gs_sid_3F;

case 449: /* clo ek r */
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
    /* ek r ellipsis-pair? match mac-env sexp env k r pat */
  if (bool_from_obj(r[1])) {
    r[10] = (cxs_sid_2Did((r[9])));
    r[10] = (cxs_memv_232567((r[10]), (r[8])));
    r[10] = obj_from_bool(!bool_from_obj(r[10]));
  if (bool_from_obj(r[10])) {
    r[0] = r[7];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (r[10]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(5+1), 11); /* 11 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[9];  
    *--hp = r[7];  
    *--hp = obj_from_case(450);
    r[11] = (hendblk(5+1));
    r[0] = (r[11]);
    r[1] = r[5];  
    goto gs_sid_3F;
  }
  } else {
  if (bool_from_obj(cxs_svector_3F((r[9])))) {
  if (bool_from_obj(cxs_svector_3F((r[5])))) {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = (vectorref((r[9]), (0)));
    r[3] = (vectorref((r[5]), (0)));
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[7];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
  if (bool_from_obj(cxs_pattern_2Dsbox_3F((r[9])))) {
    hreserve(hbsz(5+1), 10); /* 10 live regs */
    *--hp = r[7];  
    *--hp = r[9];  
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = r[6];  
    *--hp = obj_from_case(451);
    r[10] = (hendblk(5+1));
    r[0] = (r[10]);
    r[1] = r[9];  
    goto gs_pattern_2Dsbox_2D_3Etest;
  } else {
  if ((!(ispair((r[9]))))) {
    r[0] = (cx_equal_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[9];  
    r[3] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    hreserve(hbsz(4+1), 10); /* 10 live regs */
    *--hp = r[7];  
    *--hp = r[9];  
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = obj_from_case(453);
    r[10] = (hendblk(4+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cdr((r[9])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }
  }
  }

case 450: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r k pat mac-env sexp env */
  if (bool_from_obj(r[1])) {
    r[7] = (cxs_lookup_2Dsid((r[5]), (r[6])));
    r[8] = (cxs_lookup_2Dsid((r[3]), (r[4])));
    r[7] = obj_from_bool(((r[8]) == (r[7])) || ((is_flonum_obj(r[8])) && ((is_flonum_obj(r[7])) && (flonum_from_obj(r[8]) == flonum_from_obj(r[7])))));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[7];  
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

case 451: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r env match sexp pat k */
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(452);
    r[7] = (hendblk(4+1));
    r[8+0] = r[1];  
    pc = objptr_from_obj(r[8+0])[0];
    r[8+1] = r[7];  
    r[8+2] = r[4];  
    r[8+3] = r[2];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 452: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r match sexp pat k */
  if (bool_from_obj(r[1])) {
    r[6] = (boxref((r[4])));
    r[7] = (cdr((r[6])));
    r[6] = (car((r[7])));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[6];  
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[5];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 453: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r match sexp pat k */
  if (bool_from_obj(r[1])) {
    r[6] = (cdr((r[4])));
    r[6] = (cdr((r[6])));
    r[7+0] = r[5];  
    r[7+1] = r[6];  
    r[7+2] = r[3];  
    r[7+3] = r[2];  
    r[7+4] = r[4];  
    r[7+5] = r[3];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_skip;
  } else {
  if ((ispair((r[3])))) {
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(456);
    r[6] = (hendblk(4+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (car((r[4])));
    r[3] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[5];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }

s_skip: /* k p s match pat sexp */
  if ((ispair((r[1])))) {
  if ((ispair((r[2])))) {
    r[6] = (cdr((r[1])));
    r[7] = (cdr((r[2])));
    /* r[0] */    
    r[1] = r[6];  
    r[2] = r[7];  
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    goto s_skip;
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
    r[1] = r[5];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    goto s_match_2Dcars;
  }

case 454: /* clo k sexp s */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2]; }
    r += 1; /* shift reg. wnd */
s_match_2Dcars: /* k sexp s match pat */
  if ((ispair((r[2])))) {
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(454);
    r[5] = (hendblk(2+1));
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[5];  
    *--hp = obj_from_case(455);
    r[5] = (hendblk(4+1));
    r[6+0] = r[3];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[5];  
    r[6+2] = (car((r[4])));
    r[6+3] = (car((r[1])));
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[5] = (cdr((r[4])));
    r[5] = (cdr((r[5])));
    r[6+0] = r[3];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[0];  
    r[6+2] = r[5];  
    r[6+3] = r[1];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 455: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r match-cars s sexp k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (cdr((r[4])));
    r[3] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[5];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 456: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r match sexp pat k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (cdr((r[4])));
    r[3] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[5];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 457: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop rules expand-template tmpl k */
  if (bool_from_obj(r[1])) {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (mknull());
    r[3] = r[5];  
    r[4] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 458: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k synrules */
  if (bool_from_obj(r[1])) {
    r[4] = (cdr((r[3])));
    r[4] = (car((r[4])));
    r[4] = (cxs_sid_2Did((r[4])));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
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

s_loop_v5871: /* k bs store */
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
    r[4] = (car((r[1])));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (cx__23471);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5] = (car((r[1])));
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[2];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    /* r[0] */    
    r[1] = r[3];  
    r[2] = r[4];  
    goto s_loop_v5871;
  }

case 459: /* clo ek r */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* ek r */
    cx_builtins_2Dstore = r[1];  
    cx_null_2Dprog = (mknull());
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(460);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(468);
    r[3] = (hendblk(0+1));
    r[4+0] = r[2];  
    r[4+1] = (mknull());
    r[4+2] = (cx_builtins_2Dstore);
    r[4+3] = obj_from_fixnum(0);
    r[4+4] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dtop_2Dlevel_2Dforms;

case 460: /* clo ek r */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* ek r */
    cx_null_2Dstuff = r[1];  
    cx_null_2Doutput = (car((cx_null_2Dstuff)));
    r[2] = (cdr((cx_null_2Dstuff)));
    cx_null_2Dstore = (car((r[2])));
    r[2] = (cdr((cx_null_2Dstuff)));
    r[2] = (cdr((r[2])));
    cx_null_2Dloc_2Dn = (car((r[2])));
    { static obj c[] = { obj_from_case(461) }; cx_null_2Dmstore = (obj)c; }
    { static obj c[] = { obj_from_case(462) }; cx_expand_2Dtop_2Dlevel_2Dforms_21 = (obj)c; }
    { static obj c[] = { obj_from_case(464) }; cx_file_2Dexpand_2Dtop_2Dlevel_2Dforms_21 = (obj)c; }
    r[2] = obj_from_void(0);
    r[3+0] = r[0];
    pc = 0; /* exit from module init */
    r[3+1] = r[2];  
    r += 3; /* shift reg wnd */
    assert(rc = 2);
    goto jump;

case 461: /* null-mstore k */
    assert(rc == 2);
    r += 1; /* shift reg. wnd */
    /* k */
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = (cx_null_2Dloc_2Dn);
    *--hp = (cx_null_2Dstore);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[1] = (hendblk(3)); }
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = r[1];  
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 462: /* expand-top-level-forms! k forms mstore */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_expand_2Dtop_2Dlevel_2Dforms_21: /* k forms mstore */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(463);
    r[3] = (hendblk(1+1));
    r[4+0] = r[0];  
    r[4+1] = r[1];  
    r[4+2] = (car((r[2])));
    r[4+3] = (cdr((r[2])));
    r[4+4] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_expand_2Dtop_2Dlevel_2Dforms;

case 463: /* clo k outputs store loc-n */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k outputs store loc-n mstore */
    (void)(car((r[4])) = (r[2]));
    (void)(cdr((r[4])) = (r[3]));
    r[5] = r[1];  
    r[6+0] = r[0];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 464: /* file-expand-top-level-forms! k filename mstore */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k filename mstore */
    r[3] = (mkiport(3, cxm_cknull(fopen(stringchars((r[1])), "r"), "fopen")));
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = obj_from_case(467);
    r[4] = (hendblk(2+1));
    r[5+0] = r[4];  
    r[5+1] = (mknull());
    r[5+2] = r[3];  
    r[5+3] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v5824;

case 465: /* clo k forms */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
s_loop_v5824: /* k forms p mstore */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(465);
    r[4] = (hendblk(2+1));
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = obj_from_case(466);
    r[4] = (hendblk(4+1));
    r[5+0] = (cx_read_2F1);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 466: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop mstore forms k */
  if ((iseof((r[1])))) {
    { fixnum_t v9234_tmp;
    { /* length */
    int n; obj l = r[4];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v9234_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v9234_tmp);
    hreserve(hbsz(3)*c, 6); /* 6 live regs */
    l = r[4];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[6] = (o); } }
    r[0] = r[5];  
    r[1] = r[6];  
    r[2] = r[3];  
    goto gs_expand_2Dtop_2Dlevel_2Dforms_21;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[7+0] = r[2];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[5];  
    r[7+2] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 467: /* clo ek r */
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

case 468: /* clo k acc store loc-n */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
    /* k acc store loc-n */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[0];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
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
