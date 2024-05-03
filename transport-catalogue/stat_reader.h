#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace tc{

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output);

void PrintStop(const TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output);

void PrintBus(const TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output);
}