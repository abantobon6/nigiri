#include "nigiri/delay_prediction_storage.h"

namespace nigiri {

trip_delay_pred delay_prediction_storage::get_or_create_kalman(key k, unixtime_t start_time, hist_trip_times_storage* htts) {
  if (key_trip_delay_.contains(k)) {
    return key_trip_delay_[k];
  }

  trip_delay_pred tdp{};

  tdp.error = 14400;
  tdp.filter_gain = 0.66;
  tdp.gain_loop = 0.33;

  auto const coord_seq_idx = htts->cs_key_coord_seq_[k];

  // find predecessor and historic trips
  trip_time_data const* candidate = nullptr;
  vector<trip_time_data*> hist_trips_{};

  // TODO: Bisher wird nur Startzeit überprüft. Sollte man jede Arrival und Departurezeit vergleichen?

  for (auto const ttd_idx : htts->coord_seq_idx_ttd_[coord_seq_idx]) {
    auto& ttd = htts->ttd_idx_trip_time_data_[ttd_idx];
    if (ttd.start_timestamp < start_time && (candidate == nullptr || ttd.start_timestamp > candidate->start_timestamp)) {
      candidate = &ttd;
    }
    auto const time_difference_week = ((start_time - ttd.start_timestamp) % 10080).count();
    if (time_difference_week < 5 || time_difference_week > 10074) {
      hist_trips_.emplace_back(&ttd);
    }
  }
  tdp.predecessor = candidate;
  tdp.hist_trips_ = hist_trips_;

  // calculation of average historic stop/segment times
  if (!tdp.hist_trips_.empty()) {
    for (uint32_t i = 0; i < tdp.hist_trips_[0]->stop_durations_.size(); i++) {
      duration_t sum{0};
      for (auto const hist_trip : tdp.hist_trips_) {
        std::cout << "A" << std::endl;
        sum += hist_trip->stop_durations_[i];
      }
      tdp.hist_avg_stop_durations_.emplace_back(sum / tdp.hist_trips_.size());
    }
    for (uint32_t i = 0; i < tdp.hist_trips_[0]->segment_durations_.size(); i++) {
      duration_t sum{0};
      for (auto const hist_trip : tdp.hist_trips_) {
        sum += hist_trip->segment_durations_[i];
      }
      tdp.hist_avg_segment_durations_.emplace_back(sum / tdp.hist_trips_.size());
    }
  }

  key_trip_delay_.emplace(k, tdp);

  return tdp;
}

duration_t delay_prediction_storage::get_avg_duration(vector<duration_t> durations) {
  using rep = duration_t::rep;

  if (durations.empty()) {
    return duration_t{0};
  }

  std::int64_t sum = 0;
  for (auto const& d : durations) {
    sum += static_cast<std::int64_t>(d.count());
  }

  auto const n = static_cast<std::int64_t>(durations.size());
  std::int64_t avg = (sum >= 0 ? (sum + n/2) / n : -(((-sum) + n/2) / n));

  if (avg > static_cast<std::int64_t>(std::numeric_limits<rep>::max())) {
    avg = std::numeric_limits<rep>::max();
  }

  return duration_t{static_cast<rep>(avg)};
}

void delay_prediction_storage::print(std::ostream& out) const {
  out << "\ncs_key_coord_seq_:\n";
  for (const auto& [key, tdp] : key_trip_delay_) {
    out << "Key: Source: " << key.source_idx << " Trip: " << key.trip_idx
              << "\nTrip Delay Predicton: "
                 "\n  filter gain: " << tdp.filter_gain
              << "\n  gain loop" << tdp.gain_loop
              << "\n  error: " << tdp.error
              << "\n  predecessor: " << &tdp.predecessor << "\n"
              << "\n  hist trips: ";
    for (auto const hist_trip : tdp.hist_trips_) {
      out << &hist_trip << ", ";
    }
    out << "\n hist avg stop durations: ";
    for (auto const stop_dur : tdp.hist_avg_stop_durations_) {
      out << stop_dur.count() << ", ";
    }
    out << "\n hist avg segment durations: ";
    for (auto const seg_dur : tdp.hist_avg_segment_durations_) {
      out << seg_dur.count() << ", ";
    }

  }
}

std::ostream& operator<<(std::ostream& out, delay_prediction_storage const& dps) {
  dps.print(out);
  return out;
}

} // namespace nigiri
