#include "osrm/coordinate.hpp"
#include "osrm/engine_config.hpp"
#include "osrm/json_container.hpp"
#include "osrm/match_parameters.hpp"
#include "osrm/nearest_parameters.hpp"
#include "osrm/osrm.hpp"
#include "osrm/route_parameters.hpp"
#include "osrm/status.hpp"
#include "osrm/table_parameters.hpp"
#include "osrm/trip_parameters.hpp"

#include <stdint.h>
#include <vector>

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

#define EXPORT extern "C" __declspec(dllexport)
EXPORT const osrm::OSRM *osrmcs_create_instance(const char *database);
EXPORT void osrmcs_delete_instance(const osrm::OSRM *instance);
EXPORT unsigned int osrmcs_ver();
EXPORT const Route *
osrmcs_route(const osrm::OSRM *instance, Coordinate *coordinates, uint32_t num_coordinates);
EXPORT const Route *osrmcs_optimize(const osrm::OSRM *instance,
                                    Coordinate *coordinates,
                                    uint32_t num_coordinates,
                                    bool round_trip);
EXPORT void osrmcs_delete_route(const Route *route);

static void populate_response(osrm::util::json::Object &json_result, Route *response);

EXPORT const osrm::OSRM *osrmcs_create_instance(const char *database)
{
    try
    {
        osrm::EngineConfig config;
        config.storage_config = {database};
        config.use_shared_memory = false;
        //config.use_mmap = false;
        config.algorithm = osrm::EngineConfig::Algorithm::MLD;

        const auto instance = new osrm::OSRM{config};
        return instance;
    }
    catch (std::exception &ex)
    {
        return nullptr;
    }
    catch (...)
    {
        return nullptr;
    }
}

EXPORT void osrmcs_delete_instance(const osrm::OSRM *instance) { delete instance; }

EXPORT unsigned int osrmcs_ver() { return 1; }

EXPORT const Route *
osrmcs_route(const osrm::OSRM *instance, Coordinate *coordinates, uint32_t num_coordinates)
{
    Route *response = new Route();
    try
    {
        osrm::RouteParameters params;
        params.geometries = osrm::RouteParameters::GeometriesType::GeoJSON;
        params.overview = osrm::RouteParameters::OverviewType::Full;
        for (uint32_t i = 0; i < num_coordinates; i++)
        {
            params.coordinates.push_back({osrm::util::FloatLongitude{coordinates[i].longitude},
                                          osrm::util::FloatLatitude{coordinates[i].latitude}});
        }
        osrm::engine::api::ResultT result = osrm::json::Object();
        const auto status = instance->Route(params, result);
        auto &json_result = std::get<osrm::json::Object>(result);

        if (status == osrm::Status::Error)
        {
            const auto message = std::get<osrm::json::String>(json_result.values["message"]).value;
            if (message.size())
            {
                response->message = _strdup(message.c_str());
            }
            return response;
        }

        populate_response(json_result, response);
    }
    catch (std::exception &ex)
    {
        response->message = _strdup(ex.what());
    }
    catch (...)
    {
        response->message = _strdup("An unknown error occurred");
    }

    return response;
}

EXPORT const Route *osrmcs_optimize(const osrm::OSRM *instance,
                                    Coordinate *coordinates,
                                    uint32_t num_coordinates,
                                    bool round_trip)
{
    Route *response = new Route();
    try
    {
        osrm::TripParameters params;
        params.geometries = osrm::RouteParameters::GeometriesType::GeoJSON;
        params.overview = osrm::RouteParameters::OverviewType::Full;
        if (round_trip)
        {
            params.roundtrip = false;
            params.source = osrm::TripParameters::SourceType::First;
            params.destination = osrm::TripParameters::DestinationType::Last;
        }
        else
        {
            params.roundtrip = true;
            params.source = osrm::TripParameters::SourceType::First;
            params.destination = osrm::TripParameters::DestinationType::Any;
        }

        for (uint32_t i = 0; i < num_coordinates; i++)
        {
            params.coordinates.push_back({osrm::util::FloatLongitude{coordinates[i].longitude},
                                          osrm::util::FloatLatitude{coordinates[i].latitude}});
        }
        osrm::engine::api::ResultT result = osrm::json::Object();
        const auto status = instance->Trip(params, result);
        auto &json_result = std::get<osrm::json::Object>(result);

        if (status == osrm::Status::Error)
        {
            const auto message = std::get<osrm::json::String>(json_result.values["message"]).value;
            if (message.size())
            {
                response->message = _strdup(message.c_str());
            }
            return response;
        }

        populate_response(json_result, response);
    }
    catch (std::exception &ex)
    {
        response->message = _strdup(ex.what());
    }
    catch (...)
    {
        response->message = _strdup("An unknown error occurred");
    }

    return response;
}

EXPORT void osrmcs_delete_route(const Route *route)
{
    if (!route)
    {
        return;
    }

    for (uint32_t a = 0; route->legs && a < route->n_legs; a++)
    {
        if (!route->legs[a].coordinates)
        {
            continue;
        }

        delete[] route->legs[a].coordinates;
    }

    delete[] route->legs;
    free(const_cast<char *>(route->message));
    delete route;
}

static void populate_response(osrm::util::json::Object &json_result, Route *response)
{
    // check that there actually is a route
    auto &routes = std::get<osrm::json::Array>(json_result.values["routes"]);
    if (routes.values.size() == 0)
    {
        response->message = _strdup("No route found");
        return;
    }
    auto &route = std::get<osrm::json::Object>(routes.values.at(0));

    // record all waypoints
    std::vector<Coordinate> wpts;
    auto &waypoints = std::get<osrm::json::Array>(json_result.values["waypoints"]);
    for (auto &pwaypoint : waypoints.values)
    {
        auto &waypoint = std::get<osrm::json::Object>(pwaypoint);
        auto &location = std::get<osrm::json::Array>(waypoint.values["location"]);
        auto lon = std::get<osrm::json::Number>(location.values.at(0)).value;
        auto lat = std::get<osrm::json::Number>(location.values.at(1)).value;
        wpts.push_back({lat, lon});
    }

    // convert geometry
    std::vector<Coordinate> gmtry;
    std::vector<size_t> wpt_indices;
    auto &geometry = std::get<osrm::json::Object>(route.values["geometry"]);
    auto &coordinates = std::get<osrm::json::Array>(geometry.values["coordinates"]);
    for (auto &pcoord : coordinates.values)
    {
        auto &coord = std::get<osrm::json::Array>(pcoord);
        auto lon = std::get<osrm::json::Number>(coord.values.at(0)).value;
        auto lat = std::get<osrm::json::Number>(coord.values.at(1)).value;
        Coordinate crd{lat, lon};
        if (crd.latitude == wpts[wpt_indices.size()].latitude &&
            crd.longitude == wpts[wpt_indices.size()].longitude)
        {
            wpt_indices.push_back(gmtry.size());
        }
        gmtry.push_back(crd);
    }

    // record legs
    response->distance = std::get<osrm::json::Number>(route.values["distance"]).value;
    response->duration = std::get<osrm::json::Number>(route.values["duration"]).value;
    auto &legs = get<osrm::json::Array>(route.values["legs"]);
    size_t nlegs = response->n_legs;
    response->n_legs = (uint32_t)nlegs;
    response->legs = new RouteLeg[nlegs];
    memset(response->legs, 0, sizeof(*response->legs) * nlegs);
    for (size_t i = 0; i < nlegs; i++)
    {
        auto &leg = std::get<osrm::json::Object>(legs.values.at(i));
        response->legs[i].distance = std::get<osrm::json::Number>(leg.values["distance"]).value;
        response->legs[i].duration = std::get<osrm::json::Number>(leg.values["duration"]).value;

        if (i + 1 < wpt_indices.size())
        {
            auto wpt_start = wpt_indices[i];
            auto wpt_end = wpt_indices[i + 1];
            auto ncoords = wpt_end - wpt_start + 1;
            response->legs[i].num_coordinates = (uint32_t)ncoords;
            response->legs[i].coordinates = new Coordinate[ncoords];
            for (size_t j = 0; j < ncoords; j++)
            {
                response->legs[i].coordinates[j] = gmtry[wpt_start + j];
            }

            response->legs[i].start = gmtry[wpt_start];
            response->legs[i].start = gmtry[wpt_end];
        }
    }
}