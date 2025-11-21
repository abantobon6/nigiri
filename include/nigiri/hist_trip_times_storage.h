#pragma once

#include "boost/date_time/gregorian/gregorian_types.hpp"

#include "nigiri/types.h"

namespace nigiri {

using coord_seq_idx_t = cista::strong<std::uint32_t, struct _coord_seq_idx>;
using trip_time_data_idx_t = cista::strong<std::uint64_t, struct _trip_time_data_idx>;

struct key {
  trip_idx_t trip_idx;
  source_idx_t source_idx;
};

struct trip_seg_data {
  segment_idx_t seg_idx;
  double progress;
  unixtime_t timestamp;
};

struct trip_time_data {
  unixtime_t start_timestamp;
  vector<trip_seg_data> seg_data_;
};

struct hist_trip_times_storage {
  mm_vec_map<key, coord_seq_idx_t> cs_key_coord_seq_;
  mm_paged_vecvec<coord_seq_idx_t, geo::latlng> coord_seq_idx_coord_seq_;

  mm_paged_vecvec<coord_seq_idx_t, trip_time_data_idx_t> coord_seq_idx_ttd_;
  vector_map<trip_time_data_idx_t, trip_time_data> test;

  // check if key already exists
  // if not: check if similar enough coord_seq exists (find_duplicates())
  // if not: create new index and add entries data structures
  coord_seq_idx_t match_trip_to_coord_seq(key, interval<location_idx_t>);
};

}  // namespace nigiri