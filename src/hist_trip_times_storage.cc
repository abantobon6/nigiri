
#include "nigiri/hist_trip_times_storage.h"

#include <float.h>

#include "nigiri/for_each_meta.h"
#include "nigiri/types.h"

namespace nigiri {

// check if key already exists
// if not: check if similar enough coord_seq exists (find_duplicates())
// if not: create new index and add entries data structures
coord_seq_idx_t hist_trip_times_storage::match_trip_to_coord_seq(timetable const& tt, key k, vector<location_idx_t> coord_seq) {

  if (cs_key_coord_seq_.contains(k)) {
    // key already exists
    return cs_key_coord_seq_[k];
  }

  for (coord_seq_idx_t idx{0}; idx < coord_seq_idx_coord_seq_.size(); ++idx) {
    auto const& bucket = coord_seq_idx_coord_seq_[idx];

    if (bucket.size() != coord_seq.size()) {
      continue;
    }

    for (ulong i = 0; i < bucket.size(); ++i) {
      if (!routing::matches(tt, routing::location_match_mode::kEquivalent, bucket[i], coord_seq[i])) {
        break;
      }
      if (i == bucket.size() - 1) {
        // coord_seq already exists
        // create new [key, coord_seq_idx] entry
        cs_key_coord_seq_.emplace(k, idx);
        return idx;
      }
    }
  }
  // create new coord_seq_idx and new coord_seq and entries
  coord_seq_idx_coord_seq_.emplace_back(coord_seq);
  coord_seq_idx_t new_idx{coord_seq_idx_coord_seq_.size() - 1};
  cs_key_coord_seq_.emplace(k, new_idx);
  coord_seq_idx_ttd_.emplace_back(vector<trip_time_data_idx_t>{});
  return new_idx;
}

std::pair<segment_idx_t, double> hist_trip_times_storage::get_segment_progress(timetable const& tt, geo::latlng vehicle_position, coord_seq_idx_t coord_seq_idx) {

  auto const app_dist_lng_deg_vp = geo::approx_distance_lng_degrees(vehicle_position);
  std::pair closest = {geo::latlng{0, 0}, DBL_MAX};

  auto segment_from = coord_seq_idx_coord_seq_[coord_seq_idx].begin();
  for (auto segment_to = coord_seq_idx_coord_seq_[coord_seq_idx].begin() + 1;
          segment_to != coord_seq_idx_coord_seq_[coord_seq_idx].end(); ++segment_to) {

    auto const segment_to_test = geo::approx_closest_on_segment(vehicle_position,
      tt.locations_.coordinates_[*segment_from],
       tt.locations_.coordinates_[*segment_to],
                 app_dist_lng_deg_vp);

    if (closest.second < segment_to_test.second) {
      break;
    }

    closest = segment_to_test;
    ++segment_from;
  }

  auto const adld = geo::approx_distance_lng_degrees(closest.first);

  return {segment_idx_t{segment_from->v_},
            geo::approx_squared_distance(closest.first, tt.locations_.coordinates_[*segment_from-1], adld)
            / geo::approx_squared_distance(tt.locations_.coordinates_[*segment_from], tt.locations_.coordinates_[*(segment_from-1)], adld)};
}

duration_t hist_trip_times_storage::get_remaining_time_till_next_stop(trip_seg_data const* tsg,
                                              trip_time_data const* ttd) {
  auto last_tsd_before_stop = std::find_if(ttd->seg_data_.rbegin(), ttd->seg_data_.rend(),
    [tsg](auto const check_if_last_tsd){return tsg->seg_idx == check_if_last_tsd.seg_idx;});

  return last_tsd_before_stop->timestamp - tsg->timestamp;
}

void hist_trip_times_storage::print(std::ostream& out) const {
  out << "\ncs_key_coord_seq_:\n";
  for (const auto& [key, coord_seq_idx] : cs_key_coord_seq_) {
    out << "Key: Source: " << key.source_idx << " Trip: " << key.trip_idx
              << "\nCoord_seq_Idx: " << coord_seq_idx << "\n";
  }

  out << "\ncoord_seq_idx_coord_seq_:";
  for (coord_seq_idx_t idx{0}; idx < coord_seq_idx_coord_seq_.size(); ++idx) {
    out << "\nCoord_seq_Idx: " << idx << "\nLocation_Sequence: ";
    for (auto loc_idx : coord_seq_idx_coord_seq_[idx]) {
      out << loc_idx << ",";
    }
  }

  out << "\n\ncoord_seq_idx_ttd_:";
  for (coord_seq_idx_t idx{0}; idx < coord_seq_idx_ttd_.size(); ++idx) {
    out << "\nCoord_seq_Idx: " << idx << "\nTrip_Time_Data_Idxs: ";
    for (auto loc_idx : coord_seq_idx_ttd_[idx]) {
      out << loc_idx << ",";
    }
  }

  out << "\n\nttd_idx_trip_time_data_:\n";
  for (trip_time_data_idx_t idx{0}; idx < ttd_idx_trip_time_data_.size(); ++idx) {
    out << "Trip_Time_Data_Idx: " << idx << " Start_Time: " << ttd_idx_trip_time_data_[idx].start_timestamp << "\n";
    for (auto tsd : ttd_idx_trip_time_data_[idx].seg_data_) {
      out << "Segment: " << tsd.seg_idx
                << " Progress: " << tsd.progress
                << " Timestamp: " << tsd.timestamp
                << "\n";
    }
  }
}

std::ostream& operator<<(std::ostream& out, hist_trip_times_storage const& tts) {
  tts.print(out);
  return out;
}

} // namespace nigiri