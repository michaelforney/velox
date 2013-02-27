/* velox: velox/list.h
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

#ifndef VELOX_LIST_H
#define VELOX_LIST_H

#include <velox/link.h>

struct velox_list
{
    struct velox_link head;
};

#define VELOX_LIST(name) struct velox_list name = {                         \
    .head = { .next = &name.head, .prev = &name.head }                      \
}

static inline void list_init(struct velox_list * list)
{
    list->head.next = &list->head;
    list->head.prev = &list->head;
};

static inline struct velox_link * list_first_link(struct velox_list * list)
{
    return list->head.next;
}

static inline struct velox_link * list_last_link(struct velox_list * list)
{
    return list->head.prev;
}

#define list_append(list, entry, member...)                                 \
    link_add_before(&(list)->head,                                          \
        &(entry)->optional(DEFAULT_LINK_MEMBER, ##member))

#define list_prepend(list, entry, member...)                                \
    link_add_after(&(list)->head,                                           \
        &(entry)->optional(DEFAULT_LINK_MEMBER, ##member))

#define list_del(entry, member...) link_del(&(entry)                        \
    ->optional(DEFAULT_LINK_MEMBER, ##member))

#define list_first(list, type, member...)                                   \
    link_entry(list_first_link(list), type, ##member)

#define list_last(list, type, member...)                                    \
    link_entry(list_last_link(list), type, ##member)

static inline struct velox_link * list_next(struct velox_list * list,
    struct velox_link * link)
{
    return link->next == &list->head ? link->next->next : link->next;
}

static inline struct velox_link * list_prev(struct velox_list * list,
    struct velox_link * link)
{
    return link->prev == &list->head ? link->prev->prev : link->prev;
}

static inline bool list_is_empty(struct velox_list * list)
{
    return link_is_cyclic(&list->head);
}

static inline bool list_is_singular(struct velox_list * list)
{
    return !list_is_empty(list) && list->head.next == list->head.prev;
}

static inline bool list_is_trivial(struct velox_list * list)
{
    return list->head.next == list->head.prev;
}

/* List iteration */
#define __list_for_each_entry(list, entry, member)                          \
    for (entry = link_entry((list)->head.next, typeof(*entry), member);     \
        &entry->member != &(list)->head;                                    \
        entry = link_entry(entry->member.next, typeof(*entry), member))
#define list_for_each_entry(list, entry, member...)                         \
    __list_for_each_entry(list, entry, optional(DEFAULT_LINK_MEMBER, ##member))

#define __list_for_each_entry_reverse(list, entry, member)                  \
    for (entry = link_entry((list)->head.prev, typeof(*entry), member);     \
        &entry->member != &(list)->head;                                    \
        entry = link_entry(entry->member.prev, typeof(*entry), member))
#define list_for_each_entry_reverse(list, entry, member...)                 \
    __list_for_each_entry_reverse(list, entry,                              \
        optional(DEFAULT_LINK_MEMBER, ##member))

#define __list_for_each_entry_safe(list, entry, tmp, member)                \
    for (entry = link_entry((list)->head.next, typeof(*entry), member),     \
            tmp = entry->member.next;                                       \
        &entry->member != &(list)->head;                                    \
        entry = link_entry(tmp, typeof(*entry), member),                    \
            tmp = tmp->next)
#define list_for_each_entry_safe(list, entry, tmp, member...)               \
    __list_for_each_entry_safe(list, entry, tmp,                            \
        optional(DEFAULT_LINK_MEMBER, ##member))

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

