#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define IMPLEMENTATION 0
#include "osrmcs.hpp"

const char vroomreq[] = R"END(
{
  "vehicles": [
    {
      "id": 1,
      "start": [27.801654, -25.648911],
      "end": [27.801654, -25.648911],
      "capacity": [
        4
      ],
      "skills": [
        1,
        14
      ],
      "time_window": [
        1600416000,
        1600430400
      ]
    },
    {
      "id": 2,
      "start": [27.801654, -25.648911],
      "end": [27.801654, -25.648911],
      "capacity": [
        4
      ],
      "skills": [
        2,
        14
      ],
      "time_window": [
        1600416000,
        1600430400
      ],
      "breaks": [
        {
          "id": 2,
          "service": 300,
          "time_windows": [
            [
              1600423200,
              1600425000
            ]
          ]
        }
      ]
    }
  ],
  "jobs": [
    {
      "id": 1,
      "service": 300,
      "delivery": [
        1
      ],
      "location": [27.782810, -25.638839],
      "skills": [
        1
      ],
      "time_windows": [
        [
          1600419600,
          1600423200
        ]
      ]
    },
    {
      "id": 2,
      "service": 300,
      "pickup": [
        1
      ],
      "location": [27.782450, -25.631061],
      "skills": [
        1
      ]
    },
    {
      "id": 5,
      "service": 300,
      "delivery": [
        1
      ],
      "location": [27.794629, -25.622724],
      "skills": [
        14
      ]
    },
    {
      "id": 6,
      "service": 300,
      "delivery": [
        1
      ],
      "location": [27.783958, -25.638316],
      "skills": [
        14
      ]
    }
  ],
  "shipments": [
    {
      "amount": [
        1
      ],
      "skills": [
        2
      ],
      "pickup": {
        "id": 4,
        "service": 300,
        "location": [27.779071, -25.641201]
      },
      "delivery": {
        "id": 3,
        "service": 300,
        "location": [27.793146, -25.719918]
      }
    }
  ]
}
)END";

int main(int argc, const char **argv)
{
    (void)argc;
    (void)argv;
    auto instance = osrmcs_create_instance("C:\\osm\\africa-latest.osrm");

    if (!instance)
    {
        fprintf(stderr, "The instance could not be created.\n");
        return 1;
    }

    Coordinate coordinates[] = {
        {11.348876953125002, 52.62972886718355},
        {8.525390625000002, 49.403824657885124},
        {10.464477539062502, 51.34433866059924},
        {11.980590820312502, 51.37863823622007},
    };

    osrmcs_optimize_advanced(instance, vroomreq);

    //auto route = osrmcs_optimize(instance, coordinates, sizeof(coordinates) / sizeof(coordinates[0]), true, false, false);
    //osrmcs_delete(route);
    osrmcs_delete_instance(instance);

    return 0;
}
