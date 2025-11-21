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
  trip_time_data* predecessor;
  // historic trip_time_data
  vector<trip_time_data*> hist_trips_;
};

struct delay_prediction_storage {
  mm_vec_map<key, trip_delay_pred> key_trip_delay_;
};

}  // namespace nigiri