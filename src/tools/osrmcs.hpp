#if IMPLEMENTATION

#define EXPORT extern "C" __declspec(dllexport)
#include "osrm/osrm.hpp"
typedef osrm::OSRM *instance_t;

#else // IMPLEMENTATION

#define EXPORT extern "C" __declspec(dllimport)
typedef void *instance_t;

#endif // IMPLEMENTATION

struct Coordinate
{
    double latitude;
    double longitude;
};

EXPORT instance_t osrmcs_create_instance(const char *database);
EXPORT void osrmcs_delete_instance(instance_t instance);
EXPORT unsigned int osrmcs_ver();
EXPORT const char *osrmcs_route(instance_t instance,
                                Coordinate *coordinates,
                                uint32_t num_coordinates,
                                bool steps,
                                bool overview);
EXPORT const char *osrmcs_optimize(instance_t instance,
                                   Coordinate *coordinates,
                                   uint32_t num_coordinates,
                                   bool round_trip,
                                   bool steps,
                                   bool overview);
EXPORT const char *
osrmcs_table(instance_t instance, Coordinate *coordinates, uint32_t num_coordinates);
EXPORT const char *osrmcs_optimize_advanced(instance_t instance,
                                            const char *request,
                                            unsigned int searches,
                                            unsigned int exploration_level,
                                            unsigned int threads,
                                            unsigned int timeout);
EXPORT void osrmcs_delete(const char *wut);