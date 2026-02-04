#pragma once

#include "boost/date_time/gregorian/gregorian_types.hpp"

#include "nigiri/types.h"
#include "timetable.h"

namespace nigiri {

using coord_seq_idx_t = cista::strong<std::uint32_t, struct _coord_seq_idx>;
using trip_time_data_idx_t = cista::strong<std::uint64_t, struct _trip_time_data_idx>;
using segment_idx_t = cista::strong<std::uint32_t, struct _part_idx>;

struct key {
  trip_idx_t trip_idx;
  source_idx_t source_idx;
};

struct trip_seg_data {
  segment_idx_t seg_idx;
  double progress;
  unixtime_t timestamp;
  geo::latlng position;

  bool operator== (const trip_seg_data& tsd) const {
    return seg_idx == tsd.seg_idx
            && progress == tsd.progress
            && timestamp == tsd.timestamp
            && position == tsd.position;
  }
};

struct trip_time_data {
  unixtime_t start_timestamp;
  vector<trip_seg_data> seg_data_;
  vector<duration_t> stop_durations_;
  vector<duration_t> segment_durations_;
};

struct hist_trip_times_storage {
  hash_map<key, coord_seq_idx_t> cs_key_coord_seq_;
  paged_vecvec<coord_seq_idx_t, location_idx_t> coord_seq_idx_coord_seq_;

  mm_paged_vecvec<coord_seq_idx_t, trip_time_data_idx_t> coord_seq_idx_ttd_;
  vector_map<trip_time_data_idx_t, trip_time_data> ttd_idx_trip_time_data_;

  // check if key already exists
  // if not: check if similar enough coord_seq exists (find_duplicates())
  // if not: create new index and add entries data structures
  coord_seq_idx_t match_trip_to_coord_seq(timetable const&, key, vector<location_idx_t>);

  std::pair<segment_idx_t, double> get_segment_progress(timetable const&, geo::latlng, coord_seq_idx_t);

  duration_t get_remaining_time_till_next_stop(trip_seg_data const*, trip_time_data const*);

  void print(std::ostream& out) const;

  friend std::ostream& operator<<(std::ostream& out, hist_trip_times_storage const& tts);

};

}  // namespace nigiri