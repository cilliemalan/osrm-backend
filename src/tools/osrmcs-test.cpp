#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "osrmcs.hpp"

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

    auto route = osrmcs_optimize(
        instance, coordinates, sizeof(coordinates) / sizeof(coordinates[0]), true, false, false);
    osrmcs_delete(route);
    osrmcs_delete_instance(instance);

    return 0;
}
