/* SPDX-License-Identifier: LGPL-2.1-or-later */
#pragma once

#include <stdbool.h>

#include "sd-id128.h"

#include "hash-funcs.h"
#include "macro.h"


#if 0 /// UNNEEDED by elogind
bool id128_is_valid(const char *s) _pure_;
#endif // 0

typedef enum Id128Format {
        ID128_ANY,
        ID128_PLAIN,  /* formatted as 32 hex chars as-is */
        ID128_PLAIN_OR_UNINIT,  /* formatted as 32 hex chars as-is; allow special "uninitialized"
                                 * value when reading from file (id128_read() and id128_read_fd()).
                                 *
                                 * This format should be used when reading a machine-id file. */
        ID128_UUID,   /* formatted as 36 character uuid string */
        _ID128_FORMAT_MAX,
} Id128Format;

int id128_read_fd(int fd, Id128Format f, sd_id128_t *ret);
int id128_read(const char *p, Id128Format f, sd_id128_t *ret);

int id128_write_fd(int fd, Id128Format f, sd_id128_t id, bool do_sync);
int id128_write(const char *p, Id128Format f, sd_id128_t id, bool do_sync);
#if 0 /// UNNEEDED by elogind
#endif // 0

void id128_hash_func(const sd_id128_t *p, struct siphash *state);
int id128_compare_func(const sd_id128_t *a, const sd_id128_t *b) _pure_;
extern const struct hash_ops id128_hash_ops;

sd_id128_t id128_make_v4_uuid(sd_id128_t id);

int id128_get_product(sd_id128_t *ret);