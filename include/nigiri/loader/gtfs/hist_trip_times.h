#pragma once

namespace nigiri::loader::gtfs {

struct key {
  string_idx_t trip_id_;
  date::sys_days start_date_;
};

struct hist_trip_times {

};

hash_map<std::string, hist_trip_times> read_hist_trip_times(std::string_view file_content);

}  // namespace nigiri::loader::gtfs