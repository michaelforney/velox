/* velox: velox/link.h
 *
 * Copyright (c) 2013 Michael Forney <mforney@mforney.org>
 *
 * This file is a part of velox.
 *
 * velox is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2, as published by the Free
 * Software Foundation.
 *
 * velox is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along
 * with velox.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VELOX_LINK_H
#define VELOX_LINK_H

#include <stdbool.h>

#define DEFAULT_LINK_MEMBER link

struct velox_link
{
    struct velox_link * prev, * next;
};

#ifndef offsetof
#   define offsetof(type, member) __builtin_offsetof(type, member)
#endif

#define __optional(dummy, value, ...) value
#define optional(default, ...) __optional(dummy, ##__VA_ARGS__, default)

#define link_entry(link, type, member...)                                   \
    ((type *) ((char *) link - offsetof(type,                               \
        optional(DEFAULT_LINK_MEMBER, ##member))))

#define __link_entry_next(entry, member)                                    \
    link_entry((entry)->member.next, typeof(*(entry)), member)
#define link_entry_next(entry, member...)                                   \
    __link_entry_next(entry, optional(DEFAULT_LINK_MEMBER, ##member))

#define __link_entry_prev(entry, member)                                    \
    link_entry((entry)->member.prev, typeof(*(entry)), member)
#define link_entry_prev(entry, member...)                                   \
    __link_entry_prev(entry, optional(DEFAULT_LINK_MEMBER, ##member))

static inline void __link_add(struct velox_link * link,
    struct velox_link * prev, struct velox_link * next)
{
    /* Forward link */
    next->prev = link;
    link->next = next;

    /* Backward link */
    link->prev = prev;
    prev->next = link;
}

static inline void __link_del(struct velox_link * prev, struct velox_link * next)
{
    next->prev = prev;
    prev->next = next;
}

static inline void link_del(struct velox_link * link)
{
    __link_del(link->prev, link->next);
}

static inline void link_add_after(struct velox_link * link,
    struct velox_link * new)
{
    __link_add(new, link, link->next);
}

static inline void link_add_before(struct velox_link * link,
    struct velox_link * new)
{
    __link_add(new, link->prev, link);
}

static inline void link_move_after(struct velox_link * link,
    struct velox_link * dest)
{
    __link_del(link->prev, link->next);
    link_add_after(dest, link);
}

static inline void link_move_before(struct velox_link * link,
    struct velox_link * dest)
{
    __link_del(link->prev, link->next);
    link_add_before(dest, link);
}

static inline void link_swap(struct velox_link * link1,
                             struct velox_link * link2)
{
    struct velox_link * tmp;

    tmp = link1->next;
    link1->next = link2->next == link1 ? link2 : link2->next;
    link2->next = tmp == link2 ? link1 : tmp;

    tmp = link1->prev;
    link1->prev = link2->prev == link1 ? link2 : link2->prev;
    link2->prev = tmp == link2 ? link1 : tmp;

    link1->next->prev = link1;
    link1->prev->next = link1;
    link2->next->prev = link2;
    link2->prev->next = link2;
}

static inline void link_cycle_next(struct velox_link ** link)
{
    *link = (*link)->next;
}

static inline void link_cycle_prev(struct velox_link ** link)
{
    *link = (*link)->prev;
}

static inline bool link_is_cyclic(struct velox_link * link)
{
    return link->prev == link && link->next == link;
}

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

