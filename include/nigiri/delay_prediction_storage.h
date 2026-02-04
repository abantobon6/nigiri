#pragma once


#include "nigiri/types.h"
#include "nigiri/hist_trip_times_storage.h"

namespace nigiri {

struct trip_delay_pred {
  // filter gain
  double filter_gain;
  // gain loop
  double gain_loop;
  // Filter Error
  double error;
  // predecessor trip_time_data
  const trip_time_data* predecessor;
  // historic trip_time_data
  vector<trip_time_data*> hist_trips_;
  // average historic stop times
  vector<duration_t> hist_avg_stop_durations_;
  // average historic segment times
  vector<duration_t> hist_avg_segment_durations_;
};

struct delay_prediction_storage {
  hash_map<key, trip_delay_pred> key_trip_delay_;

  trip_delay_pred get_or_create_kalman(key, unixtime_t, hist_trip_times_storage*);

  static duration_t get_avg_duration(vector<duration_t>);

  void print(std::ostream& out) const;

  friend std::ostream& operator<<(std::ostream& out, delay_prediction_storage const& tts);

};

}  // namespace nigiri