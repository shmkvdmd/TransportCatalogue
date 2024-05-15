#include "stat_reader.h"
#include <iostream>
#include <iomanip>
namespace tc{

void ParseAndPrintStat(const TransportCatalogue &transport_catalogue, std::string_view request,
                       std::ostream &output)
{
    std::string_view command = request.substr(0, request.find_first_of(' '));
    if (command == "Bus")
    {
        PrintBus(transport_catalogue, request, output);
    }
    if (command == "Stop"){
        PrintStop(transport_catalogue, request, output);
    }
}

void PrintBus(const TransportCatalogue &transport_catalogue, std::string_view request,
              std::ostream &output){
    std::string_view bus_name = request.substr(4);
    if (transport_catalogue.FindBusByName(bus_name) == nullptr){
        output << "Bus " << bus_name << ": not found" << std::endl;
    }
    else{
        RouteInformation route_info = transport_catalogue.GetRouteInfo(bus_name);
        output << "Bus " << route_info.bus_name << ": " << route_info.stops_on_route << " stops on route, "
                << route_info.unique_stops << " unique stops, " << std::setprecision(6) << route_info.route_length
                << " route length, " << route_info.curvature << " curvature" << std::endl;
    }            
}

void PrintStop(const TransportCatalogue &transport_catalogue, std::string_view request,
              std::ostream &output)
{
    std::string_view stop_name = request.substr(5);
    if (transport_catalogue.FindStopByName(stop_name) == nullptr){
        output << "Stop " << stop_name << ": not found" << std::endl;
    }
    else{
        std::set<std::string_view> unique_buses = transport_catalogue.GetStopInfo(stop_name);
        if (unique_buses.size() == 0){
            output << "Stop " << stop_name << ": no buses" << std::endl;
        }
        else{
            output << "Stop " << stop_name << ": buses ";
            for (const auto &bus : unique_buses){
                output << bus << " ";
            }
            output << std::endl;
        }
    }
}

}