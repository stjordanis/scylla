/*
 * Copyright (C) 2019 ScyllaDB
 *
 */

/*
 * This file is part of Scylla.
 *
 * Scylla is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Scylla is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scylla.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <seastar/core/shared_ptr.hh>
#include <seastar/core/sharded.hh>

#include "disk-error-handler.hh"
#include "gc_clock.hh"
#include "sstables/shareable_components.hh"
#include "sstables/shared_sstable.hh"
#include "sstables/version.hh"
#include "sstables/component_type.hh"
#include "db/large_data_handler.hh"

namespace sstables {

using schema_ptr = lw_shared_ptr<const schema>;
using shareable_components_ptr = lw_shared_ptr<shareable_components>;

// contains data for loading a sstable using components shared by a single shard;
// can be moved across shards
struct foreign_sstable_open_info {
    foreign_ptr<shareable_components_ptr> components;
    std::vector<shard_id> owners;
    seastar::file_handle data;
    seastar::file_handle index;
    uint64_t generation;
    sstable_version_types version;
    sstable_format_types format;
};

// can only be used locally
struct sstable_open_info {
    shareable_components_ptr components;
    std::vector<shard_id> owners;
    file data;
    file index;
};

struct entry_descriptor {
    sstring sstdir;
    sstring ks;
    sstring cf;
    int64_t generation;
    sstable_version_types version;
    sstable_format_types format;
    component_type component;

    static entry_descriptor make_descriptor(sstring sstdir, sstring fname);

    entry_descriptor(sstring sstdir, sstring ks, sstring cf, int64_t generation,
            sstable_version_types version, sstable_format_types format,
            component_type component)
        : sstdir(sstdir)
        , ks(ks)
        , cf(cf)
        , generation(generation)
        , version(version)
        , format(format)
        , component(component)
    {}
};

static constexpr inline size_t default_sstable_buffer_size() {
    return 128 * 1024;
}

class sstables_manager {
public:
    sstables_manager() = default;

    // Constructs a shared sstable
    shared_sstable make_sstable(schema_ptr schema, sstring dir, int64_t generation, sstable_version_types v, sstable_format_types f,
            const db::large_data_handler& large_data_handler,
            gc_clock::time_point now = gc_clock::now(),
            io_error_handler_gen error_handler_gen = default_io_error_handler_gen(),
            size_t buffer_size = default_sstable_buffer_size());
};

}   // namespace sstables
