#define IMPLEMENTATION 1
#include "osrmcs.hpp"

#include <stdint.h>
#include <vector>

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
#include "util/json_renderer.hpp"

#include "routing/osrm_routed_wrapper.h"
#include "structures/vroom/input/input.h"
#include "structures/vroom/job.h"
#include "structures/vroom/vehicle.h"
#include "utils/exception.h"
#include "utils/helpers.h"
#include "utils/input_parser.h"
#include "utils/output_json.h"
#include "utils/version.h"

#include "../include/rapidjson/include/rapidjson/document.h"
#include "../include/rapidjson/include/rapidjson/stringbuffer.h"
#include "../include/rapidjson/include/rapidjson/writer.h"

EXPORT instance_t osrmcs_create_instance(const char *database)
{
    try
    {
        osrm::EngineConfig config;
        config.storage_config = {database};
        config.use_shared_memory = false;
        // config.use_mmap = false;
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

EXPORT void osrmcs_delete_instance(instance_t instance) { delete instance; }

EXPORT unsigned int osrmcs_ver() { return 1; }

EXPORT const char *osrmcs_route(instance_t instance,
                                Coordinate *coordinates,
                                uint32_t num_coordinates,
                                bool steps,
                                bool overview)
{
    try
    {
        osrm::RouteParameters params;
        params.geometries = osrm::RouteParameters::GeometriesType::GeoJSON;
        params.overview = overview ? osrm::RouteParameters::OverviewType::Full
                                   : osrm::RouteParameters::OverviewType::False;
        params.steps = steps;
        params.generate_hints = false;
        for (uint32_t i = 0; i < num_coordinates; i++)
        {
            params.coordinates.push_back({osrm::util::FloatLongitude{coordinates[i].longitude},
                                          osrm::util::FloatLatitude{coordinates[i].latitude}});
        }
        osrm::engine::api::ResultT result = osrm::json::Object();
        const auto status = instance->Route(params, result);
        auto &json_result = std::get<osrm::json::Object>(result);
        std::string responsedata;
        osrm::util::json::render(responsedata, json_result);
        return _strdup(responsedata.c_str());
    }
    catch (std::exception &ex)
    {
        return _strdup(ex.what());
    }
    catch (...)
    {
        return _strdup("An unknown error occurred");
    }
}

EXPORT const char *osrmcs_optimize(instance_t instance,
                                   Coordinate *coordinates,
                                   uint32_t num_coordinates,
                                   bool round_trip,
                                   bool steps,
                                   bool overview)
{
    try
    {
        osrm::TripParameters params;
        params.geometries = osrm::RouteParameters::GeometriesType::GeoJSON;
        params.overview = overview ? osrm::RouteParameters::OverviewType::Full
                                   : osrm::RouteParameters::OverviewType::False;
        params.steps = steps;
        params.generate_hints = false;
        if (round_trip)
        {
            params.roundtrip = true;
            params.source = osrm::TripParameters::SourceType::First;
            params.destination = osrm::TripParameters::DestinationType::Last;
        }
        else
        {
            params.roundtrip = false;
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
        std::string responsedata;
        osrm::util::json::render(responsedata, json_result);
        return _strdup(responsedata.c_str());
    }
    catch (std::exception &ex)
    {
        return _strdup(ex.what());
    }
    catch (...)
    {
        return _strdup("An unknown error occurred");
    }
}

EXPORT const char *
osrmcs_table(instance_t instance, Coordinate *coordinates, uint32_t num_coordinates)
{
    try
    {
        osrm::engine::api::TableParameters params;
        params.generate_hints = false;
        params.annotations = osrm::engine::api::TableParameters::AnnotationsType::Distance |
                             osrm::engine::api::TableParameters::AnnotationsType::Duration;

        for (uint32_t i = 0; i < num_coordinates; i++)
        {
            params.coordinates.push_back({osrm::util::FloatLongitude{coordinates[i].longitude},
                                          osrm::util::FloatLatitude{coordinates[i].latitude}});
        }

        osrm::engine::api::ResultT result = osrm::json::Object();
        const auto status = instance->Table(params, result);
        auto &json_result = std::get<osrm::json::Object>(result);
        std::string responsedata;
        osrm::util::json::render(responsedata, json_result);
        return _strdup(responsedata.c_str());
    }
    catch (std::exception &ex)
    {
        return _strdup(ex.what());
    }
    catch (...)
    {
        return _strdup("An unknown error occurred");
    }
}

EXPORT const char *osrmcs_optimize_advanced(instance_t instance, const char *request)
{
    if (!request || !instance)
    {
        return nullptr;
    }

    try
    {

        vroom::io::Servers servers;
        vroom::Server osrm{reinterpret_cast<void *>(instance)};
        servers[vroom::DEFAULT_PROFILE] = osrm;
        vroom::Input problem{servers, vroom::ROUTER::LIBOSRM, true};
        vroom::io::parse(problem, request, false);

        const vroom::Solution sol = problem.solve(vroom::DEFAULT_EXPLORATION_LEVEL,
                                                  vroom::DEFAULT_EXPLORATION_LEVEL,
                                                  vroom::DEFAULT_THREADS_NUMBER);

        auto json = vroom::io::to_json(sol, problem.report_distances());

        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> r_writer(s);
        json.Accept(r_writer);
        return _strdup(s.GetString());
    }
    catch (std::exception& ex)
    {
        return _strdup(ex.what());
    }
    catch (...)
    {
        return _strdup("An unknown error occurred");
    }
}

EXPORT void osrmcs_delete_route(const char *route)
{
    if (!route)
    {
        return;
    }

    free(const_cast<char *>(route));
}

EXPORT void osrmcs_delete(const char *wut)
{
    if (!wut)
    {
        return;
    }

    free(const_cast<char *>(wut));
}