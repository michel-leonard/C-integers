#ifndef CINT_MASTER
#define CINT_MASTER

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef int_fast64_t h_cint_t;

static const h_cint_t cint_exponent = 4 * sizeof(h_cint_t) - 1;
static const h_cint_t cint_base = (h_cint_t) (1ULL << cint_exponent);
static const h_cint_t cint_mask = (h_cint_t) (cint_base - 1);
static const char *cint_alpha = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

typedef struct {
    h_cint_t *mem; // data pointer
    h_cint_t *end;
    h_cint_t nat; // -1 = negative, +1 = positive, zero is positive
    size_t size;
} cint;

static cint h_cint_tmp[7];

static size_t cint_count_bits(const cint *num) {
    size_t res = 0;
    if (num->end != num->mem) {
        for (; *(num->end - 1) >> ++res;);
        res += (num->end - num->mem - 1) * cint_exponent;
    }
    return res;
}

static inline h_cint_t h_cint_compare(const cint *lhs, const cint *rhs) {
    h_cint_t res = (h_cint_t) ((lhs->end - lhs->mem) - (rhs->end - rhs->mem));
    if (res == 0 && rhs->end != rhs->mem)
        for (const h_cint_t *l = lhs->end, *r = rhs->end; !(res = *--l - *--r) && l != lhs->mem;);
    return res;
}

static inline h_cint_t cint_compare(const cint *lhs, const cint *rhs) {
    h_cint_t res = lhs->nat - rhs->nat;
    res || (res = lhs->nat * h_cint_compare(lhs, rhs));
    return res;
}

static void cint_init(cint *num, size_t size, h_cint_t value) {
    num->size = size;
    num->end = num->mem = calloc(size + 1, sizeof(*num->mem));
    (num->nat = 1 - ((value < 0) << 1)) > 0 || (value = -value);
    for (; value; *num->end = value % cint_base, value /= cint_base, ++num->end);
}

static void cint_reinit(cint *num, h_cint_t value) {
    num->end = memset(num->mem, 0, (num->end - num->mem) * sizeof(h_cint_t));
    (num->nat = 1 - ((value < 0) << 1)) > 0 || (value = -value);
    for (; value; *num->end = value % cint_base, value /= cint_base, ++num->end);
}

static void cint_reinit_by_string(cint *num, const char *str, const int base) {
    cint_reinit(num, 0);
    for (; *str && memchr(cint_alpha, *str, base) == 0; num->nat *= 1 - ((*str++ == '-') << 1));
    for (h_cint_t *p; *str; *num->mem += (h_cint_t) ((char *) memchr(cint_alpha, *str++, base) - cint_alpha), num->end += *num->end != 0)
        for (p = num->end; --p >= num->mem; *(p + 1) += (*p *= base) >> cint_exponent, *p &= cint_mask);
    for (h_cint_t *p = num->mem; *p & cint_base; *(p + 1) += *p >> cint_exponent, *p++ &= cint_mask);
    num->end += *num->end != 0, num->mem != num->end || (num->nat = 1);
}

static char *cint_to_string(const cint *num, const int base_out) {
    h_cint_t a, b, *c = num->end;
    size_t d, e = 1;
    char *s = malloc(2);
    strcpy(s, "0");
    for (; --c >= num->mem;) {
        for (a = *c, d = e; d;) {
            b = (h_cint_t) ((char *) memchr(cint_alpha, s[--d], base_out) - cint_alpha), b = b * cint_base + a;
            s[d] = cint_alpha[b % base_out];
            a = b / base_out;
        }
        for (; a; s = realloc(s, ++e + 1), memmove(s + 1, s, e), *s = cint_alpha[a % base_out], a /= base_out);
    }
    if (num->nat < 0)
        s = realloc(s, e + 2), memmove(s + 1, s, e + 1), *s = '-';
    return s;
}

static inline void cint_init_by_string(cint *num, const size_t size, const char *str, const int base) { cint_init(num, size, 0), cint_reinit_by_string(num, str, base); }

static void cint_reinit_by_double(cint *num, const double value) {
    cint_reinit(num, 0);
    uint64_t memory;
    memcpy(&memory, &value, sizeof(value));
    uint64_t ex = (memory << 1 >> 53) - 1023, m_1 = 1ULL << 52;
    if (ex < 1024) {
        h_cint_t m_2 = 1 << ex % cint_exponent;
        num->nat *= 1 - ((value < 0) << 1);
        num->end = 1 + num->mem + ex / cint_exponent;
        h_cint_t *n = num->end;
        for (*(n - 1) |= m_2; --n >= num->mem; m_2 = cint_base)
            for (; m_2 >>= 1;)
                if (m_1 >>= 1)
                    *n |= m_2 * ((memory & m_1) != 0);
                else return;
    }
}

static double cint_to_double(const cint *num) {
    uint64_t memory = cint_count_bits(num) + 1022, m_write = 1ULL << 52, m_read = 1 << memory % cint_exponent;
    double res = 0;
    memory <<= 52;
    for (h_cint_t *n = num->end; --n >= num->mem; m_read = 1LL << cint_exponent)
        for (; m_read >>= 1;)
            if (m_write >>= 1)
                memory |= m_write * ((*n & m_read) != 0);
            else
                n = num->mem, m_read = 0;
    memcpy(&res, &memory, sizeof(memory));
    return (double) num->nat * res;
}

static inline void cint_init_by_double(cint *num, const size_t size, const double value) { cint_init(num, size, 0), cint_reinit_by_double(num, value); }

static void cint_dup(cint *to, const cint *from) {
    const size_t b = from->end - from->mem, a = to->end - to->mem;
    memcpy(to->mem, from->mem, b * sizeof(*from->mem));
    to->end = to->mem + b;
    to->nat = from->nat;
    b < a && memset(to->end, 0, (a - b) * sizeof(*to->mem));
}

static void cint_rescale(cint *num, size_t size) {
    const size_t off = num->end - num->mem;
    num->mem = realloc(num->mem, sizeof(*num->mem) * (num->size = size));
    memset(num->end = num->mem + off, 0, (size - off) * sizeof(*num->mem));
}

static inline void h_cint_begin() { for (int i = 0; i < 7; ++i) cint_init(h_cint_tmp + i, 100, 0); }

static inline void h_cint_clears() { for (int i = 0; i < 7; ++i) free(h_cint_tmp[i].mem); }

static inline cint *h_cint_get_tmp(const size_t nth, const cint *about) {
    size_t required = about->end - about->mem + 32;
    return h_cint_tmp + (h_cint_tmp[nth].size < required ? cint_rescale(h_cint_tmp + nth, required), nth : nth);
}

static inline cint *cint_immediate(const h_cint_t value) {
    static size_t i = 0; ++i;
    cint_reinit(h_cint_tmp + 5 + (i & 1), value);
    return h_cint_tmp + 5 + (i & 1);
}

static void h_cint_addi(cint *lhs, const cint *rhs) {
    h_cint_t *l = lhs->mem;
    for (const h_cint_t *r = rhs->mem; r < rhs->end;)
        *l += *r++, *(l + 1) += *l >> cint_exponent, *l++ &= cint_mask;
    for (; *l & cint_base; *(l + 1) += *l >> cint_exponent, *l++ &= cint_mask);
    if (rhs->end - rhs->mem > lhs->end - lhs->mem)
        lhs->end = lhs->mem + (rhs->end - rhs->mem);
    lhs->end += *lhs->end != 0;
}

static void h_cint_subi(cint *lhs, const cint *rhs) {
    h_cint_t a = 0, cmp, *l, *r, *e, *o;
    if (lhs->mem == lhs->end)
        cint_dup(lhs, rhs);
    else if (rhs->mem != rhs->end) {
        cmp = h_cint_compare(lhs, rhs);
        if (cmp) {
            if (cmp < 0) l = lhs->mem, r = rhs->mem, e = rhs->end, lhs->nat = -lhs->nat;
            else l = rhs->mem, r = lhs->mem, e = lhs->end;
            for (o = lhs->mem; r < e; *o = *r++ - *l++ - a, a = (*o & cint_base) != 0, *o++ &= cint_mask);
            for (*o &= cint_mask, o += a; --o > lhs->mem && !*o;);
            lhs->end = 1 + o;
        } else cint_reinit(lhs, 0);
    }
}

static inline void cint_addi(cint *lhs, const cint *rhs) { lhs->nat == rhs->nat ? h_cint_addi(lhs, rhs) : h_cint_subi(lhs, rhs); }

static inline void cint_subi(cint *lhs, const cint *rhs) { lhs->nat == rhs->nat ? lhs->nat = -lhs->nat, h_cint_subi(lhs, rhs), lhs->mem == lhs->end || (lhs->nat = -lhs->nat), (void) 0 : h_cint_addi(lhs, rhs); }

static void cint_left_shifti(cint *num, const size_t bits) {
    if (num->end != num->mem) {
        const size_t a = bits / cint_exponent, b = bits % cint_exponent, c = cint_exponent - b;
        if (a) {
            memmove(num->mem + a, num->mem, (num->end - num->mem + 1) * sizeof(h_cint_t));
            memset(num->mem, 0, a * sizeof(h_cint_t));
            num->end += a;
        }
        if (b) for (h_cint_t *l = num->end, *e = num->mem + a; --l >= e; *(l + 1) |= *l >> c, *l = *l << b & cint_mask);
        num->end += *(num->end) != 0;
    }
}

static void cint_right_shifti(cint *num, const size_t bits) {
    size_t a = bits / cint_exponent, b = bits % cint_exponent, c = cint_exponent - b;
    if (num->end - a > num->mem) {
        if (a) {
            num->mem + a > num->end && (a = num->end - num->mem);
            memmove(num->mem, num->mem + a, (num->end - num->mem) * sizeof(h_cint_t));
            memset(num->end -= a, 0, a * sizeof(h_cint_t));
        }
        if (b) for (h_cint_t *l = num->mem; l < num->end; *l = (*l >> b | *(l + 1) << c) & cint_mask, ++l);
        num->end == num->mem || (num->end -= *(num->end - 1) == 0), num->end == num->mem && (num->nat = 1);
    } else cint_reinit(num, 0);
}

static void cint_mul(const cint *lhs, const cint *rhs, cint *res) {
    h_cint_t *l, *r, *o, *p;
    cint_reinit(res, 0);
    if (lhs->mem != lhs->end && rhs->mem != rhs->end) {
        res->nat = lhs->nat * rhs->nat, res->end += (lhs->end - lhs->mem) + (rhs->end - rhs->mem) - 1;
        for (l = lhs->mem, p = res->mem; l < lhs->end; ++l)
            for (r = rhs->mem, o = p++; r < rhs->end; *(o + 1) += (*o += *l * *r++) >> cint_exponent, *o++ &= cint_mask);
        res->end += *res->end != 0;
    }
}

static void cint_powi(cint *lhs, size_t power) {
    if (power == 0) cint_reinit(lhs, 1);
    else if (lhs->mem != lhs->end) {
        cint *a = h_cint_get_tmp(0, lhs), *b = h_cint_get_tmp(1, lhs), *c = lhs, *d;
        for (cint_reinit(a, 1); power > 1; cint_mul(c, c, b), d = c, c = b, b = d)
            power & 1 ? cint_mul(a, c, b), d = a, a = b, b = d, power = (power - 1) >> 1 : (power >>= 1);
        cint_mul(a, c, b), lhs == b ? (void) 0 : cint_dup(lhs, b);
    }
}

static void cint_div(const cint *lhs, const cint *rhs, cint *q, cint *r) {
    cint_reinit(r, 0);
    if (rhs->end == rhs->mem)
        for (q->nat = lhs->nat * rhs->nat, q->end = q->mem; q->end < q->mem + q->size; *q->end++ = cint_mask); // DBZ
    else {
        h_cint_t a = h_cint_compare(lhs, rhs);
        if (a) {
            cint_reinit(q, 0);
            if (a > 0) {
                h_cint_t *l = lhs->end, *k, *qq = q->mem + (lhs->end - lhs->mem);
                for (; --qq, --l >= lhs->mem;)
                    for (a = cint_base; a >>= 1;) {
                        for (k = r->end; --k >= r->mem; *(k + 1) |= (*k <<= 1) >> cint_exponent, *k &= cint_mask);
                        *r->mem += (a & *l) != 0, r->end += *r->end != 0;
                        h_cint_compare(r, rhs) >= 0 ? h_cint_subi(r, rhs), *qq |= a : 0;
                    }
                q->end += (lhs->end - lhs->mem) - (rhs->end - rhs->mem), q->end += *q->end != 0;
                q->nat = rhs->nat * lhs->nat, (r->end == r->mem) || (r->nat = lhs->nat); // lhs = q * rhs + r
            } else cint_dup(r, lhs);
        } else cint_reinit(q, rhs->nat * lhs->nat);
    }
}

static void h_cint_div_approx(const cint *lhs, const cint *rhs, cint *res) {
    h_cint_t x, bits = h_cint_compare(lhs, rhs), *o = rhs->end, *p;
    if (bits == 0)
        cint_reinit(res, 1), res->nat = lhs->nat * rhs->nat;
    else if (bits < 0)
        cint_reinit(res, 0);
    else {
        cint_dup(res, lhs);
        res->nat *= rhs->nat;
        x = *--o, --o < rhs->mem || (x = x << cint_exponent | *o);
        for (bits = 0; cint_mask < x; x >>= 1, ++bits);
        cint_right_shifti(res, (rhs->end - rhs->mem - 1) * cint_exponent + (bits > 0) * (bits - cint_exponent));
        p = res->end - 3 > res->mem ? res->end - 3 : res->mem;
        for (o = res->end; --o > p; *(o - 1) += (*o % x) << cint_exponent, *o /= x);
        *o /= x;
        res->end -= *(res->end - 1) == 0;
    }
}

static void h_cint_div_experimental(const cint *lhs, const cint *rhs, cint *q, cint *r) {
    if (rhs->mem == rhs->end)
        cint_div(lhs, rhs, q, r);
    else {
        cint *tmp_1 = h_cint_get_tmp(0, lhs), *tmp_2 = h_cint_get_tmp(1, lhs);
        cint_reinit(q, 0);
        cint_dup(r, lhs);
        for (; h_cint_div_approx(r, rhs, tmp_2), tmp_2->mem != tmp_2->end;) {
            cint_addi(q, tmp_2);
            cint_mul(tmp_2, rhs, tmp_1);
            h_cint_subi(r, tmp_1);
        }
        if (r->end != r->mem && r->nat != lhs->nat) // lhs = q * rhs + r
            cint_reinit(tmp_2, q->nat), h_cint_subi(q, tmp_2), h_cint_subi(r, rhs);
    }
}

static size_t cint_remove(cint *lhs, const cint *rhs) {
    size_t res = 0;
    if (rhs->end == rhs->mem + 1 && *(rhs->mem) == 1)
        res = 1, lhs->nat *= rhs->nat;
    else if (lhs->end != lhs->mem && rhs->end != rhs->mem) {
        cint *tmp_1 = h_cint_get_tmp(0, lhs), *tmp_2 = h_cint_get_tmp(1, lhs);
        for (cint *tmp; cint_div(lhs, rhs, tmp_1, tmp_2), tmp_2->mem == tmp_2->end; tmp = lhs, lhs = tmp_1, tmp_1 = tmp, ++res);
        if (res & 1) cint_dup(tmp_1, lhs);
    }
    return res;
}

static void cint_sqrt(const cint *num, cint *res, cint *rem) {
    cint_reinit(res, 0), cint_dup(rem, num); // res ** 2 + rem = num
    if (num->nat > 0 && num->end != num->mem) {
        cint *a = h_cint_get_tmp(0, num), *b = h_cint_get_tmp(1, num);
        cint_reinit(a, 1);
        cint_left_shifti(a, cint_count_bits(num) & ~1);
        for (; a->mem != a->end;) {
            cint_dup(b, res);
            h_cint_addi(b, a);
            cint_right_shifti(res, 1);
            if (h_cint_compare(rem, b) >= 0)
                h_cint_subi(rem, b), h_cint_addi(res, a);
            cint_right_shifti(a, 2);
        }
    }
}

static void cint_cbrt(const cint *num, cint *res, cint *rem) {
    cint_reinit(res, 0), cint_dup(rem, num); // res ** 3 + rem = num
    if (num->mem != num->end) {
        cint *a = h_cint_get_tmp(0, num), *b = h_cint_get_tmp(1, num);
        for (size_t c = cint_count_bits(num) / 3 * 3; c < -1U; c -= 3) {
            cint_left_shifti(res, 1);
            cint_dup(a, res);
            cint_left_shifti(a, 1);
            h_cint_addi(a, res);
            cint_mul(a, res, b);
            ++*b->mem;
            h_cint_addi(b, a);
            cint_dup(a, rem);
            cint_right_shifti(a, c);
            if (h_cint_compare(a, b) >= 0)
                cint_left_shifti(b, c), h_cint_subi(rem, b), cint_reinit(b, 1), h_cint_addi(res, b);
        }
        res->nat = num->nat;
    }
}

static void cint_nth_root(const cint *num, const unsigned nth, cint *res) {
    if (nth) {
        if (num->end > num->mem + 1 || *num->mem > 1) {
            cint *a = h_cint_get_tmp(0, num), *b = h_cint_get_tmp(1, num), *c = h_cint_get_tmp(2, num), *d = h_cint_get_tmp(3, num), *r = res, *tmp;
            cint_reinit(a, 1), cint_reinit(d, 1);
            cint_left_shifti(a, (cint_count_bits(num) + nth - 1) / nth);
            h_cint_addi(r, d);
            do {
                tmp = a, a = r, r = tmp, cint_dup(a, num);
                for (unsigned count = nth; --count && (cint_div(a, r, b, c), tmp = a, a = b, b = tmp, a->mem != a->end););
                *d->mem = nth - 1, cint_mul(r, d, b);
                h_cint_addi(b, a);
                *d->mem = nth, cint_div(b, d, a, c);
            } while (h_cint_compare(a, r) < 0);
            r == res ? (void) 0 : cint_dup(res, tmp == a ? a : r);
            res->nat = nth & 1 ? num->nat : 1;
        } else cint_dup(res, num);
    } else cint_reinit(res, num->end == num->mem + 1 && *num->mem == 1);

}

static void cint_random_bits(cint *num, size_t bits) {
    size_t i = 0;
    cint_reinit(num, 0);
    for (; bits; ++num->end)
        for (i = 0; bits && i < cint_exponent; ++i, --bits)
            *num->end = *num->end << 1 | (rand() & 1);
    if (i) *(num->end - 1) |= 1 << (i - 1);
}

#endif
