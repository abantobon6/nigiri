#include "gtest/gtest.h"

#include "nigiri/loader/gtfs/files.h"
#include "nigiri/loader/gtfs/load_timetable.h"
#include "nigiri/loader/init_finish.h"
#include "nigiri/rt/create_rt_timetable.h"
#include "nigiri/rt/frun.h"
#include "nigiri/rt/gtfsrt_resolve_run.h"
#include "nigiri/rt/gtfsrt_update.h"
#include "nigiri/rt/util.h"
#include "nigiri/timetable.h"

#include "./util.h"

using namespace nigiri;
using namespace nigiri::loader;
using namespace nigiri::loader::gtfs;
using namespace nigiri::rt;
using namespace date;
using namespace std::chrono_literals;
using namespace std::string_literals;
using namespace std::string_view_literals;

namespace {
mem_dir test_files() {
  return mem_dir::read(R"(
     "(
# agency.txt
agency_name,agency_url,agency_timezone,agency_lang,agency_phone,agency_fare_url,agency_id
"grt",https://grt.ca,America/New_York,en,519-585-7555,http://www.grt.ca/en/fares/FarePrices.asp,grt

# stops.txt
stop_id,stop_code,stop_name,stop_desc,stop_lat,stop_lon,zone_id,stop_url,location_type,parent_station,wheelchair_boarding,platform_code
2351,2351,Block Line Station,,  43.422095, -80.462740,,
1033,1033,Block Line / Hanover,,  43.419023, -80.466600,,,0,,1,
2086,2086,Block Line / Kingswood,,  43.417796, -80.473666,,,0,,1,
2885,2885,Block Line / Strasburg,,  43.415733, -80.480340,,,0,,1,

# calendar_dates.txt
service_id,date,exception_type
201-Weekday-66-23SUMM-1111100,20230703,1
201-Weekday-66-23SUMM-1111100,20230704,1
201-Weekday-66-23SUMM-1111100,20230705,1
201-Weekday-66-23SUMM-1111100,20230706,1
201-Weekday-66-23SUMM-1111100,20230707,1
201-Weekday-66-23SUMM-1111100,20230710,1
201-Weekday-66-23SUMM-1111100,20230711,1
201-Weekday-66-23SUMM-1111100,20230712,1
201-Weekday-66-23SUMM-1111100,20230713,1
201-Weekday-66-23SUMM-1111100,20230714,1
201-Weekday-66-23SUMM-1111100,20230717,1
201-Weekday-66-23SUMM-1111100,20230718,1
201-Weekday-66-23SUMM-1111100,20230719,1
201-Weekday-66-23SUMM-1111100,20230720,1
201-Weekday-66-23SUMM-1111100,20230721,1
201-Weekday-66-23SUMM-1111100,20230724,1
201-Weekday-66-23SUMM-1111100,20230725,1
201-Weekday-66-23SUMM-1111100,20230726,1
201-Weekday-66-23SUMM-1111100,20230727,1
201-Weekday-66-23SUMM-1111100,20230728,1
201-Weekday-66-23SUMM-1111100,20230731,1
201-Weekday-66-23SUMM-1111100,20230801,1
201-Weekday-66-23SUMM-1111100,20230802,1
201-Weekday-66-23SUMM-1111100,20230803,1
201-Weekday-66-23SUMM-1111100,20230804,1
201-Weekday-66-23SUMM-1111100,20230807,1
201-Weekday-66-23SUMM-1111100,20230808,1
201-Weekday-66-23SUMM-1111100,20230809,1
201-Weekday-66-23SUMM-1111100,20230810,1
201-Weekday-66-23SUMM-1111100,20230811,1
201-Weekday-66-23SUMM-1111100,20230814,1
201-Weekday-66-23SUMM-1111100,20230815,1
201-Weekday-66-23SUMM-1111100,20230816,1
201-Weekday-66-23SUMM-1111100,20230817,1
201-Weekday-66-23SUMM-1111100,20230818,1
201-Weekday-66-23SUMM-1111100,20230821,1
201-Weekday-66-23SUMM-1111100,20230822,1
201-Weekday-66-23SUMM-1111100,20230823,1
201-Weekday-66-23SUMM-1111100,20230824,1
201-Weekday-66-23SUMM-1111100,20230825,1
201-Weekday-66-23SUMM-1111100,20230828,1
201-Weekday-66-23SUMM-1111100,20230829,1
201-Weekday-66-23SUMM-1111100,20230830,1
201-Weekday-66-23SUMM-1111100,20230831,1
201-Weekday-66-23SUMM-1111100,20230901,1

# routes.txt
route_id,agency_id,route_short_name,route_long_name,route_desc,route_type
201,grt,iXpress Fischer-Hallman,,3,https://www.grt.ca/en/schedules-maps/schedules.aspx

# trips.txt
route_id,service_id,trip_id,trip_headsign,direction_id,block_id,shape_id,wheelchair_accessible,bikes_allowed
201,201-Weekday-66-23SUMM-1111100,11,Conestoga Station,0,340341,2010025,1,1
201,201-Weekday-66-23SUMM-1111100,12,Conestoga Station,0,340341,2010025,1,1
201,201-Weekday-66-23SUMM-1111100,13,Conestoga Station,0,340341,2010025,1,1
201,201-Weekday-66-23SUMM-1111100,14,Conestoga Station,0,340341,2010025,1,1
201,201-Weekday-66-23SUMM-1111100,15,Conestoga Station,0,340341,2010025,1,1
201,201-Weekday-66-23SUMM-1111100,2,Conestoga Station,0,340341,2010025,1,1
201,201-Weekday-66-23SUMM-1111100,3,Conestoga Station,0,340341,2010025,1,1



# stop_times.txt
trip_id,arrival_time,departure_time,stop_id,stop_sequence,pickup_type,drop_off_type
11,05:00:00,05:15:00,2351,1,0,0
11,06:00:00,06:15:00,1033,2,0,0
11,07:00:00,07:15:00,2086,3,0,0
11,08:00:00,08:15:00,2885,4,0,0
12,05:00:00,05:15:00,2351,1,0,0
12,06:00:00,06:15:00,1033,2,0,0
12,07:00:00,07:15:00,2086,3,0,0
12,08:00:00,08:15:00,2885,4,0,0
13,05:00:00,05:15:00,2351,1,0,0
13,06:00:00,06:15:00,1033,2,0,0
13,07:00:00,07:15:00,2086,3,0,0
13,08:00:00,08:15:00,2885,4,0,0
14,05:00:00,05:15:00,2351,1,0,0
14,06:00:00,06:15:00,1033,2,0,0
14,07:00:00,07:15:00,2086,3,0,0
14,08:00:00,08:15:00,2885,4,0,0
15,05:00:00,05:15:00,2351,1,0,0
15,06:00:00,06:15:00,1033,2,0,0
15,07:00:00,07:15:00,2086,3,0,0
15,08:00:00,08:15:00,2885,4,0,0
2,04:00:00,04:15:00,2351,1,0,0
2,05:00:00,05:15:00,1033,2,0,0
2,06:00:00,06:15:00,2086,3,0,0
2,07:00:00,07:15:00,2885,4,0,0
3,05:00:00,05:15:00,2351,1,0,0
3,06:00:00,06:15:00,1033,2,0,0
3,07:00:00,07:15:00,2086,3,0,0
3,08:00:00,08:15:00,2885,4,0,0
)");
}

// Test case:
// 3 Messungen pro Segment:
//      1 43.42172857221422, -80.46300152025462; 43.420610844275686, -80.4643359499176; 43.419077923156436, -80.46617924157172;
//      2 43.418882028555885, -80.46726934123926; 43.41855161265139, -80.47031974600564; 43.41788098396262, -80.4730982288067;
//      3 43.41760042248819, -80.4741388861675; 43.416960971123125, -80.47671371418541; 43.41582276438298, -80.48000727370437;
// 5 historische Trips:
//      1. 5min  delay überall
//      2. 7min  delay überall
//      3. 3min  delay überall
//      4. 10min delay überall
//      5. 0min  delay überall
// 1 direkter Vorgänger-Trip:
//      10min delay
// 1 "live" Trip
//      25min delay

auto const kVehiclePositionT11 =
    R"({
 "header": {
  "gtfsRealtimeVersion": "2.0",
  "incrementality": "FULL_DATASET",
  "timestamp": "1688371200"
 },
 "entity": [
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "11",
      "startTime": "05:15:00",
      "startDate": "20230703",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.42172857221422",
      "longitude": "-80.46300152025462"
    },
    "timestamp": "1688361900",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "112",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "11",
      "startTime": "05:15:00",
      "startDate": "20230703",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.420610844275686",
      "longitude": "-80.4643359499176"
    },
    "timestamp": "1688362980",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "113",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "11",
      "startTime": "05:15:00",
      "startDate": "20230703",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.419077923156436",
      "longitude": "-80.46617924157172"
    },
    "timestamp": "1688364000",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "114",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "11",
      "startTime": "05:15:00",
      "startDate": "20230703",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.418882028555885",
      "longitude": "-80.46726934123926"
    },
    "timestamp": "1688365500",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "115",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "11",
      "startTime": "05:15:00",
      "startDate": "20230703",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41855161265139",
      "longitude": "-80.47031974600564"
    },
    "timestamp": "1688366580",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "116",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "11",
      "startTime": "05:15:00",
      "startDate": "20230703",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41788098396262",
      "longitude": "-80.4730982288067"
    },
    "timestamp": "1688367600",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "117",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "11",
      "startTime": "05:15:00",
      "startDate": "20230703",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41760042248819",
      "longitude": "-80.4741388861675"
    },
    "timestamp": "1688369100",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "118",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "11",
      "startTime": "05:15:00",
      "startDate": "20230703",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.416960971123125",
      "longitude": "-80.47671371418541"
    },
    "timestamp": "1688370180",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "119",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "11",
      "startTime": "05:15:00",
      "startDate": "20230703",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41582276438298",
      "longitude": "-80.48000727370437"
    },
    "timestamp": "1688371200",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  }
 ]
})"s;

auto const kVehiclePositionT12 =
    R"({
 "header": {
  "gtfsRealtimeVersion": "2.0",
  "incrementality": "FULL_DATASET",
  "timestamp": "1688975880"
 },
 "entity": [
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "12",
      "startTime": "05:15:00",
      "startDate": "20230710",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.42172857221422",
      "longitude": "-80.46300152025462"
    },
    "timestamp": "1688966580",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "12",
      "startTime": "05:15:00",
      "startDate": "20230710",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.420610844275686",
      "longitude": "-80.4643359499176"
    },
    "timestamp": "1688967660",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "12",
      "startTime": "05:15:00",
      "startDate": "20230710",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.419077923156436",
      "longitude": "-80.46617924157172"
    },
    "timestamp": "1688968680",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "12",
      "startTime": "05:15:00",
      "startDate": "20230710",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.418882028555885",
      "longitude": "-80.46726934123926"
    },
    "timestamp": "1688970180",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "12",
      "startTime": "05:15:00",
      "startDate": "20230710",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41855161265139",
      "longitude": "-80.47031974600564"
    },
    "timestamp": "1688971260",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "12",
      "startTime": "05:15:00",
      "startDate": "20230710",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41788098396262",
      "longitude": "-80.4730982288067"
    },
    "timestamp": "1688972280",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "12",
      "startTime": "05:15:00",
      "startDate": "20230710",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41760042248819",
      "longitude": "-80.4741388861675"
    },
    "timestamp": "1688973780",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "12",
      "startTime": "05:15:00",
      "startDate": "20230710",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.416960971123125",
      "longitude": "-80.47671371418541"
    },
    "timestamp": "1688974860",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "12",
      "startTime": "05:15:00",
      "startDate": "20230710",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41582276438298",
      "longitude": "-80.48000727370437"
    },
    "timestamp": "1688975880",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  }
 ]
})"s;

auto const kVehiclePositionT13 =
    R"({
 "header": {
  "gtfsRealtimeVersion": "2.0",
  "incrementality": "FULL_DATASET",
  "timestamp": "1689580920"
 },
 "entity": [
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "13",
      "startTime": "05:15:00",
      "startDate": "20230717",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.42172857221422",
      "longitude": "-80.46300152025462"
    },
    "timestamp": "1689571620",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "13",
      "startTime": "05:15:00",
      "startDate": "20230717",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.420610844275686",
      "longitude": "-80.4643359499176"
    },
    "timestamp": "1689572700",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "13",
      "startTime": "05:15:00",
      "startDate": "20230717",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.419077923156436",
      "longitude": "-80.46617924157172"
    },
    "timestamp": "1689573720",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "13",
      "startTime": "05:15:00",
      "startDate": "20230717",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.418882028555885",
      "longitude": "-80.46726934123926"
    },
    "timestamp": "1689575220",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "13",
      "startTime": "05:15:00",
      "startDate": "20230717",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41855161265139",
      "longitude": "-80.47031974600564"
    },
    "timestamp": "1689576300",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "13",
      "startTime": "05:15:00",
      "startDate": "20230717",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41788098396262",
      "longitude": "-80.4730982288067"
    },
    "timestamp": "1689577320",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "13",
      "startTime": "05:15:00",
      "startDate": "20230717",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41760042248819",
      "longitude": "-80.4741388861675"
    },
    "timestamp": "1689578820",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "13",
      "startTime": "05:15:00",
      "startDate": "20230717",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.416960971123125",
      "longitude": "-80.47671371418541"
    },
    "timestamp": "1689579900",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "13",
      "startTime": "05:15:00",
      "startDate": "20230717",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41582276438298",
      "longitude": "-80.48000727370437"
    },
    "timestamp": "1689580920",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  }
 ]
})"s;

auto const kVehiclePositionT14 =
    R"({
 "header": {
  "gtfsRealtimeVersion": "2.0",
  "incrementality": "FULL_DATASET",
  "timestamp": "1690185900"
 },
 "entity": [
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "14",
      "startTime": "05:15:00",
      "startDate": "20230724",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.42172857221422",
      "longitude": "-80.46300152025462"
    },
    "timestamp": "1690176600",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "14",
      "startTime": "05:15:00",
      "startDate": "20230724",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.420610844275686",
      "longitude": "-80.4643359499176"
    },
    "timestamp": "1690177680",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "14",
      "startTime": "05:15:00",
      "startDate": "20230724",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.419077923156436",
      "longitude": "-80.46617924157172"
    },
    "timestamp": "1690178700",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "14",
      "startTime": "05:15:00",
      "startDate": "20230724",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.418882028555885",
      "longitude": "-80.46726934123926"
    },
    "timestamp": "1690180200",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "14",
      "startTime": "05:15:00",
      "startDate": "20230724",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41855161265139",
      "longitude": "-80.47031974600564"
    },
    "timestamp": "1690181280",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "14",
      "startTime": "05:15:00",
      "startDate": "20230724",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41788098396262",
      "longitude": "-80.4730982288067"
    },
    "timestamp": "1690182300",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "14",
      "startTime": "05:15:00",
      "startDate": "20230724",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41760042248819",
      "longitude": "-80.4741388861675"
    },
    "timestamp": "1690183800",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "14",
      "startTime": "05:15:00",
      "startDate": "20230724",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.416960971123125",
      "longitude": "-80.47671371418541"
    },
    "timestamp": "1690184880",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "14",
      "startTime": "05:15:00",
      "startDate": "20230724",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41582276438298",
      "longitude": "-80.48000727370437"
    },
    "timestamp": "1690185900",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  }
 ]
})"s;

auto const kVehiclePositionT15 =
    R"({
 "header": {
  "gtfsRealtimeVersion": "2.0",
  "incrementality": "FULL_DATASET",
  "timestamp": "1690790100"
 },
 "entity": [
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "15",
      "startTime": "05:15:00",
      "startDate": "20230731",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.42172857221422",
      "longitude": "-80.46300152025462"
    },
    "timestamp": "1690780800",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "15",
      "startTime": "05:15:00",
      "startDate": "20230731",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.420610844275686",
      "longitude": "-80.4643359499176"
    },
    "timestamp": "1690781880",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "15",
      "startTime": "05:15:00",
      "startDate": "20230731",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.419077923156436",
      "longitude": "-80.46617924157172"
    },
    "timestamp": "1690782900",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "15",
      "startTime": "05:15:00",
      "startDate": "20230731",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.418882028555885",
      "longitude": "-80.46726934123926"
    },
    "timestamp": "1690784400",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "15",
      "startTime": "05:15:00",
      "startDate": "20230731",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41855161265139",
      "longitude": "-80.47031974600564"
    },
    "timestamp": "1690785480",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "15",
      "startTime": "05:15:00",
      "startDate": "20230731",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41788098396262",
      "longitude": "-80.4730982288067"
    },
    "timestamp": "1690786500",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "15",
      "startTime": "05:15:00",
      "startDate": "20230731",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41760042248819",
      "longitude": "-80.4741388861675"
    },
    "timestamp": "1690788000",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "15",
      "startTime": "05:15:00",
      "startDate": "20230731",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.416960971123125",
      "longitude": "-80.47671371418541"
    },
    "timestamp": "1690789080",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "15",
      "startTime": "05:15:00",
      "startDate": "20230731",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41582276438298",
      "longitude": "-80.48000727370437"
    },
    "timestamp": "1690790100",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  }
 ]
})"s;

auto const kVehiclePositionT2 =
    R"({
 "header": {
  "gtfsRealtimeVersion": "2.0",
  "incrementality": "FULL_DATASET",
  "timestamp": "1691391900"
 },
 "entity": [
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "2",
      "startTime": "04:15:00",
      "startDate": "20230807",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.42172857221422",
      "longitude": "-80.46300152025462"
    },
    "timestamp": "1691382600",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "2",
      "startTime": "04:15:00",
      "startDate": "20230807",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.420610844275686",
      "longitude": "-80.4643359499176"
    },
    "timestamp": "1691383680",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "2",
      "startTime": "04:15:00",
      "startDate": "20230807",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.419077923156436",
      "longitude": "-80.46617924157172"
    },
    "timestamp": "1691384700",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "2",
      "startTime": "04:15:00",
      "startDate": "20230807",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.418882028555885",
      "longitude": "-80.46726934123926"
    },
    "timestamp": "1691386200",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "2",
      "startTime": "04:15:00",
      "startDate": "20230807",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41855161265139",
      "longitude": "-80.47031974600564"
    },
    "timestamp": "1691387280",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "2",
      "startTime": "04:15:00",
      "startDate": "20230807",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41788098396262",
      "longitude": "-80.4730982288067"
    },
    "timestamp": "1691388300",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "2",
      "startTime": "04:15:00",
      "startDate": "20230807",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41760042248819",
      "longitude": "-80.4741388861675"
    },
    "timestamp": "1691389800",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "2",
      "startTime": "04:15:00",
      "startDate": "20230807",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.416960971123125",
      "longitude": "-80.47671371418541"
    },
    "timestamp": "1691390880",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "2",
      "startTime": "04:15:00",
      "startDate": "20230807",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41582276438298",
      "longitude": "-80.48000727370437"
    },
    "timestamp": "1691391900",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  }
 ]
})"s;

auto const kVehiclePositionT3 =
    R"({
 "header": {
  "gtfsRealtimeVersion": "2.0",
  "incrementality": "FULL_DATASET",
  "timestamp": "1691396400"
 },
 "entity": [
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "3",
      "startTime": "05:15:00",
      "startDate": "20230807",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.42172857221422",
      "longitude": "-80.46300152025462"
    },
    "timestamp": "1691387100",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "3",
      "startTime": "05:15:00",
      "startDate": "20230807",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.420610844275686",
      "longitude": "-80.4643359499176"
    },
    "timestamp": "1691388180",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "3",
      "startTime": "05:15:00",
      "startDate": "20230807",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.419077923156436",
      "longitude": "-80.46617924157172"
    },
    "timestamp": "1691389200",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "3",
      "startTime": "05:15:00",
      "startDate": "20230807",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.418882028555885",
      "longitude": "-80.46726934123926"
    },
    "timestamp": "1691390700",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "3",
      "startTime": "05:15:00",
      "startDate": "20230807",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41855161265139",
      "longitude": "-80.47031974600564"
    },
    "timestamp": "1691391780",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "3",
      "startTime": "05:15:00",
      "startDate": "20230807",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41788098396262",
      "longitude": "-80.4730982288067"
    },
    "timestamp": "1691392800",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "3",
      "startTime": "05:15:00",
      "startDate": "20230807",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41760042248819",
      "longitude": "-80.4741388861675"
    },
    "timestamp": "1691394300",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "3",
      "startTime": "05:15:00",
      "startDate": "20230807",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.416960971123125",
      "longitude": "-80.47671371418541"
    },
    "timestamp": "1691395380",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  },
  {
    "id": "111",
    "isDeleted": false,
    "vehicle": {
    "trip": {
      "tripId": "3",
      "startTime": "05:15:00",
      "startDate": "20230807",
      "routeId": "201"
    },
    "position": {
      "latitude": "43.41582276438298",
      "longitude": "-80.48000727370437"
    },
    "timestamp": "1691396400",
    "occupancy_status": "MANY_SEATS_AVAILABLE"
    }
  }
 ]
})"s;


constexpr auto const expected_tts = R"(

)"sv;

constexpr auto const expected_dps = R"(

)"sv;

}  // namespace

TEST(rt, gtfsrt_rt_delay_calc_test) {
  std::cout << "Test rt::gtfsrt_rt_delay_calc_test" << std::endl;

  // Load static timetable.
  timetable tt;
  register_special_stations(tt);
  tt.date_range_ = {date::sys_days{2023_y / July / 3},
                    date::sys_days{2023_y / August / 12}};
  load_timetable({}, source_idx_t{0}, test_files(), tt);
  finalize(tt);

  // Create empty RT timetable.
  auto rtt = rt::create_rt_timetable(tt, date::sys_days{2023_y / August / 12});

  // Create empty mm_paged_vecvec
  using data_t = mm_paged_vecvec<coord_seq_idx_t, trip_time_data_idx_t>::data_t;
  using idx_t = mm_paged_vecvec<coord_seq_idx_t, trip_time_data_idx_t>::index_t;

  auto idx_backend = idx_t{cista::mmap{std::tmpnam(nullptr)}};
  auto data_backend_underlying = mm_vec<trip_time_data_idx_t>{cista::mmap{std::tmpnam(nullptr)}};
  auto data_backend = data_t{std::move(data_backend_underlying)};

  mm_paged_vecvec<coord_seq_idx_t, trip_time_data_idx_t> empty_mm_paged_vecvec{std::move(data_backend), std::move(idx_backend)};

  // Create empty Trip Time Data Storage
  auto tts = hist_trip_times_storage{hash_map<key, coord_seq_idx_t>{}, paged_vecvec<coord_seq_idx_t, location_idx_t>{},
    (std::move(empty_mm_paged_vecvec)), vector_map<trip_time_data_idx_t, trip_time_data>{},};

  // Create empty Delay Prediction Storage
  auto dps = delay_prediction_storage{};

  // Update.
  auto const msg11 = rt::json_to_protobuf(kVehiclePositionT11);
  auto const msg12 = rt::json_to_protobuf(kVehiclePositionT12);
  auto const msg13 = rt::json_to_protobuf(kVehiclePositionT13);
  auto const msg14 = rt::json_to_protobuf(kVehiclePositionT14);
  auto const msg15 = rt::json_to_protobuf(kVehiclePositionT15);
  auto const msg2 = rt::json_to_protobuf(kVehiclePositionT2);
  auto const msg3 = rt::json_to_protobuf(kVehiclePositionT3);

  gtfsrt_update_buf(tt, rtt, source_idx_t{0}, "", msg11, true, &dps, &tts);
  gtfsrt_update_buf(tt, rtt, source_idx_t{0}, "", msg12, true, &dps, &tts);
  gtfsrt_update_buf(tt, rtt, source_idx_t{0}, "", msg13, true, &dps, &tts);
  gtfsrt_update_buf(tt, rtt, source_idx_t{0}, "", msg14, true, &dps, &tts);
  gtfsrt_update_buf(tt, rtt, source_idx_t{0}, "", msg15, true, &dps, &tts);
  gtfsrt_update_buf(tt, rtt, source_idx_t{0}, "", msg2, true, &dps, &tts);
  gtfsrt_update_buf(tt, rtt, source_idx_t{0}, "", msg3, true, &dps, &tts);



  // Print trip.
  transit_realtime::TripDescriptor td11;
  td11.set_start_date("20230703");
  td11.set_trip_id("11");
  td11.set_start_time("05:15:00");
  auto const [r11, t11] = rt::gtfsrt_resolve_run(date::sys_days{May / 1 / 2019}, tt,
                                             &rtt, source_idx_t{0}, td11);
  transit_realtime::TripDescriptor td12;
  td12.set_start_date("20230710");
  td12.set_trip_id("12");
  td12.set_start_time("05:15:00");
  auto const [r12, t12] = rt::gtfsrt_resolve_run(date::sys_days{May / 1 / 2019}, tt,
                                             &rtt, source_idx_t{0}, td12);
  transit_realtime::TripDescriptor td13;
  td13.set_start_date("20230717");
  td13.set_trip_id("13");
  td13.set_start_time("05:15:00");
  auto const [r13, t13] = rt::gtfsrt_resolve_run(date::sys_days{May / 1 / 2019}, tt,
                                             &rtt, source_idx_t{0}, td13);
  transit_realtime::TripDescriptor td14;
  td14.set_start_date("20230724");
  td14.set_trip_id("14");
  td14.set_start_time("05:15:00");
  auto const [r14, t14] = rt::gtfsrt_resolve_run(date::sys_days{May / 1 / 2019}, tt,
                                             &rtt, source_idx_t{0}, td14);
  transit_realtime::TripDescriptor td15;
  td15.set_start_date("20230731");
  td15.set_trip_id("15");
  td15.set_start_time("05:15:00");
  auto const [r15, t15] = rt::gtfsrt_resolve_run(date::sys_days{May / 1 / 2019}, tt,
                                             &rtt, source_idx_t{0}, td15);
  transit_realtime::TripDescriptor td2;
  td2.set_start_date("20230807");
  td2.set_trip_id("2");
  td2.set_start_time("04:15:00");
  auto const [r2, t2] = rt::gtfsrt_resolve_run(date::sys_days{May / 1 / 2019}, tt,
                                             &rtt, source_idx_t{0}, td2);
  transit_realtime::TripDescriptor td3;
  td3.set_start_date("20230807");
  td3.set_trip_id("3");
  td3.set_start_time("05:15:00");
  auto const [r3, t3] = rt::gtfsrt_resolve_run(date::sys_days{May / 1 / 2019}, tt,
                                             &rtt, source_idx_t{0}, td3);
  ASSERT_TRUE(r11.valid());
  ASSERT_TRUE(r12.valid());
  ASSERT_TRUE(r13.valid());
  ASSERT_TRUE(r14.valid());
  ASSERT_TRUE(r15.valid());
  ASSERT_TRUE(r2.valid());
  ASSERT_TRUE(r3.valid());

  std::cout << "Alle Trips sind valid." << std::endl;

  std::stringstream ss_tts;
  ss_tts << tts;
  EXPECT_EQ(expected_tts, ss_tts.str());

  std::stringstream ss_dps;
  ss_dps << dps;
  EXPECT_EQ(expected_dps, ss_dps.str());




}