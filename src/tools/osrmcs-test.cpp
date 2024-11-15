#include <stdint.h>

struct Coordinate
{
    double latitude;
    double longitude;
};

struct RouteLeg
{
    Coordinate start;
    Coordinate end;
    double distance;
    double duration;
    Coordinate *coordinates;
    uint32_t num_coordinates;
};

struct Route
{
    double distance;
    double duration;
    RouteLeg *legs;
    uint32_t n_legs;
    const char *message;
};

#define IMPORT extern "C" __declspec(dllimport)
IMPORT const void *osrmcs_create_instance(const char *database);
IMPORT void osrmcs_delete_instance(const void *instance);
IMPORT unsigned int osrmcs_ver();
IMPORT const Route *
osrmcs_route(const void *instance, Coordinate *coordinates, uint32_t num_coordinates);
IMPORT const Route *osrmcs_optimize(const void *instance,
                                    Coordinate *coordinates,
                                    uint32_t num_coordinates,
                                    bool round_trip);
IMPORT void osrmcs_delete_route(const Route *route);

int main(int argc, const char **argv)
{
    auto instance = osrmcs_create_instance("C:\\osm\\africa\\africa-latest.osrm");

    Coordinate coordinates[] = {
        {11.348876953125002, 52.62972886718355},
        {8.525390625000002, 49.403824657885124},
        {10.464477539062502, 51.34433866059924},
        {11.980590820312502, 51.37863823622007},
    };

    auto route = osrmcs_route(instance, coordinates, sizeof(coordinates) / sizeof(coordinates[0]));
    osrmcs_delete_route(route);
    osrmcs_delete_instance(instance);

    return 0;
}
