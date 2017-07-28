#ifndef _FSP_PROTO_H_
#define _FSP_PROTO_H_ 1


/* #ifdef UNDERSTANDS_PROTOTYPES */
#ifdef PROTOTYPES
#define PROTO0(a) a
#define PROTO1(a, b) (a b)
#define PROTO2(a, b, c, d) (a b, c d)
#define PROTO3(a, b, c, d, e, f) (a b, c d, e f)
#define PROTO4(a, b, c, d, e, f, g, h) (a b, c d, e f, g h)
#define PROTO5(a, b, c, d, e, f, g, h, i, j) (a b, c d, e f, g h, i j)
#define PROTO6(a, b, c, d, e, f, g, h, i, j, k, l) (a b, c d, e f, g h, i j, k l)
#define PROTO7(a, b, c, d, e, f, g, h, i, j, k, l, m, n) (a b, c d, e f, g h, i j, k l, m n)
#else
#define PROTO0(a) ()
#define PROTO1(a, b) (b) a b;
#define PROTO2(a, b, c, d) (b, d) a b; c d;
#define PROTO3(a, b, c, d, e, f) (b, d, f) a b; c d; e f;
#define PROTO4(a, b, c, d, e, f, g, h) (b, d, f, h) a b; c d; e f; g h;
#define PROTO5(a, b, c, d, e, f, g, h, i, j) (b, d, f, h, j) \
        a b; c d; e f; g h; i j;
#define PROTO6(a, b, c, d, e, f, g, h, i, j, k, l) (b, d, f, h, j, l) \
        a b; c d; e f; g h; i j; k l;
#define PROTO6(a, b, c, d, e, f, g, h, i, j, k, l, m, n) (b, d, f, h, j, l) \
        a b; c d; e f; g h; i j; k l;m n;
#endif /* UNDERSTANDS_PROTOTYPES */

#endif
