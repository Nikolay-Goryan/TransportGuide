//
//  request.cpp
//  TransportGuide_A
//
//  Created by Николай Горян on 09.09.2020.
//  Copyright © 2020 Николай Горян. All rights reserved.
//


#include "request.h"
#include "graph.h"
#include <cmath>
#include <set>
#include <iostream>
#include <iomanip>
	
Request::Request(Nature nature) : nature_(nature) {};

ModifyingRequest::ModifyingRequest(Type type) : Request(Nature::MODIFY), type_(type) {}

InfoRequest::InfoRequest(Type type) : Request(Nature::INFO), type_(type) {}

BusLogInfoRequest::BusLogInfoRequest() : ModifyingRequest(Type::BUS_LOG_INFO) {}

StopLogInfoRequest::StopLogInfoRequest() : ModifyingRequest(Type::STOP_LOG_INFO) {}

BusInfoRequest::BusInfoRequest() : InfoRequest(Type::BUS_INFO) {}

StopInfoRequest::StopInfoRequest() : InfoRequest(Type::STOP_INFO) {}

RouteInfoRequest::RouteInfoRequest() : InfoRequest(Type::ROUTE_INFO) {}

void BusLogInfoRequest::ParseFrom(std::map<std::string, Json::Node>& parameters) {
	bus_name = parameters["name"].AsString();
	is_circular_ = parameters["is_roundtrip"].AsBool();
	for (const Json::Node& stop: parameters["stops"].AsArray()) {
		stops_.push_back(stop.AsString());
	}
}

void BusLogInfoRequest::Process(RequestManager& manager) const {
	Route route(is_circular_, stops_);
	manager.AddBus(bus_name, route);
	manager.AddBusOnStops(bus_name, stops_); 
}


std::unordered_map<std::string, int> setDistances(std::string_view& input) {
	std::unordered_map<std::string, int> distances;
	std::string distance = std::string(ReadToken(input, "m"));
	distance = std::string(DeleteSpaces(distance));
	std::string name;
	while (!distance.empty()) {
		ReadToken(input, "to");
		ReadToken(input);
		name = DeleteSpaces(ReadToken(input, ","));
		if (*prev(name.end()) == '\n') {
			name.resize(name.size() - 1);
			name = std::string(DeleteSpaces(name));
		}
		if (name != "") {
			distances[name] = std::stod(std::string(distance));
			distance = ReadToken(input, "m");
			DeleteSpaces(distance);
		}
	}
	return distances;
}

void StopLogInfoRequest::ParseFrom(std::map<std::string, Json::Node>& parameters) {
	stop_name_ = parameters["name"].AsString();
	latitude_ = parameters["latitude"].AsDouble();
	longitude_ = parameters["longitude"].AsDouble();
	
	for (const auto& [name, node]: parameters["road_distances"].AsMap()) {
		distances[name] = (int)(node.AsDouble());
	}
}

void StopLogInfoRequest::Process(RequestManager& manager) const {
	manager.AddStop(stop_name_, latitude_, longitude_);
	manager.AddBusOnStops(std::string(), std::vector<std::string>{stop_name_});
	manager.AddRoadDistances(stop_name_, move(distances));
}


void BusInfoRequest::ParseFrom(std::map<std::string, Json::Node>& parameters) {
	bus_number_ = parameters["name"].AsString();
	id_ = (long)(parameters["id"].AsDouble());
}

#define PI 3.1415926535

double haversine(double lat1, double lon1, double lat2, double lon2) {
	// distance between latitudes
	// and longitudes
	double dLat = (lat2 - lat1) * PI / 180.0;
	double dLon = (lon2 - lon1) * PI / 180.0;

	// convert to radians
	lat1 = (lat1) * PI / 180.0;
	lat2 = (lat2) * PI / 180.0;

	// apply formulae
	double a = pow(sin(dLat / 2), 2) + pow(sin(dLon / 2), 2) * cos(lat1) * cos(lat2);
	double rad = 6371;
	double c = 2 * asin(sqrt(a));
	return rad * c * 1000;
}

double generateLength(const std::vector<std::string>& stops, bool is_circular, const RequestManager& manager) {
	double length = 0.0;
	std::string from = *stops.begin();
	for (auto to = next(stops.begin()); to != stops.end(); ++to) {
		const auto& from_coord = manager.getStops().at(from);
		const auto& to_coord = manager.getStops().at(*to);
		length += haversine(from_coord.first, from_coord.second, to_coord.first, to_coord.second);
		from = *to;
	}
	if (!is_circular) {
		length *= 2;
	}
	return length;
}

size_t getUniqueStopsNumber(const std::vector<std::string>& stops) {
	std::set<std::string> unique_stops(stops.begin(), stops.end());
	return unique_stops.size();
}

int generateRoadDistance(const std::vector<std::string>& stops, bool is_circular, const RequestManager& manager) {
	int distance = 0;
	std::string from = *stops.begin();
	if (!is_circular) {
		for (auto to = next(stops.begin()); to != stops.end(); ++to) {
			distance += manager.getRoadDistances().at(from).at(*to);
			distance += manager.getRoadDistances().at(*to).at(from);
			from = *to;
		}
	} else {
		for (auto to = next(stops.begin()); to != stops.end(); ++to) {
			distance += manager.getRoadDistances().at(from).at(*to);
			from = *to;
		}
	}
	return distance;
}


std::pair<int, double> generateDistances(const Route& route, const RequestManager& manager) {
	std::string response;
	bool isCircular = route.IsCircular();
	int distance = generateRoadDistance(route.getStops(), isCircular, manager);
	double length = generateLength(route.getStops(), isCircular, manager);
	return {distance, length};
}

void BusInfoRequest::Process(const RequestManager& manager, std::ostream& out = std::cout) const {
	out << "{";
	std::optional<Route> opt_route = manager.getRouteByName(bus_number_);
	if (!opt_route) {
		out << "\"request_id\": " << id_ << ", ";
		out << "\"error_message\": \"not found\"";
	} else {
		Route route = std::move(opt_route.value());
		bool is_circular = route.IsCircular();
		auto [distance, length] = generateDistances(route, manager);
		out << "\"route_length\": " << distance << ", ";
		out << "\"request_id\": " << id_ << ", ";
		out << "\"curvature\": " << std::setprecision(6) << (double)distance / length << ", ";
		size_t stops_count = route.getStops().size();
		if (is_circular) {
			out << "\"stop_count\": " << stops_count << ", ";
		} else {
			out << "\"stop_count\": " << stops_count * 2 - 1 << ", ";
		}
		out << "\"unique_stop_count\": " << getUniqueStopsNumber(route.getStops());
	}
	out << "}";
}

void StopInfoRequest::ParseFrom(std::map<std::string, Json::Node>& parameters) {
	stop_name_ = parameters["name"].AsString();
	id_ = (long)parameters["id"].AsDouble();
}

void StopInfoRequest::Process(const RequestManager& manager, std::ostream& out = std::cout) const {
	out << "{";
	if (manager.getBussesOnStops().count(stop_name_) == 0) {
		out << "\"request_id\": " << id_ << ", ";
		out << "\"error_message\": \"not found\"";
		out << "}";
		return;
	}
	
	std::set<std::string> buses = manager.getBussesOnStops().at(stop_name_);
	
	if (buses.size() == 0) {
		out << "\"buses\": [], ";
		out << "\"request_id\": " << id_;
	} else {
		out << "\"buses\": [";
		for (const std::string& bus: buses) {
			out << "\"" << bus << "\"";
			if (bus == *prev(buses.end())) {
				out << "";
			} else {
				out << ", ";
			}
		}
		out << "], ";
		out << "\"request_id\": " << id_;
	}
	out << "}";
}

void RouteInfoRequest::ParseFrom(std::map<std::string, Json::Node>& parameters) {
	from_stop_ = parameters["from"].AsString();
	to_stop_ = parameters["to"].AsString();
	id_ = (long)parameters["id"].AsDouble();
}

void RouteInfoRequest::Process(const RequestManager& manager, std::ostream& out) const {
	out << "{";
	if (from_stop_  == to_stop_) {
		out << "\"total_time\": 0, ";
		out << "\"items\": [], ";
		out << "\"request_id\": " << id_;
		out << "}";
		return;
	}
	const auto router = manager.getRouter();
	const auto graph = manager.getGraph();
	auto from_id = manager.getIdsStops().at(from_stop_);
	auto to_id = manager.getIdsStops().at(to_stop_);
	auto route_info = router->BuildRoute(from_id, to_id);
	if (!route_info) {
		out << "\"error_message\": \"not found\", ";
		out << "\"request_id\": " << id_;
		out << "}";
		return;
	}
	auto wait_time = manager.getRoutingSettings().bus_wait_time;
	out << "\"total_time\": " << route_info->weight << ", ";
	out << "\"items\": [";
	for (size_t idx = 0; idx < route_info->edge_count; ++idx) {
		auto edge_id = router->GetRouteEdge(route_info->id, idx);
		auto edge_info = graph->GetEdge(edge_id);
		out << "{";
		out << "\"time\": " << wait_time << ", ";
		out << "\"stop_name\": " << "\"" << manager.getStopsIds().at(edge_info.from) << "\"" << ", ";
		out << "\"type\": \"Wait\"";
		out << "}, ";
		out << "{";
		out << "\"time\": " << edge_info.weight - wait_time << ", ";
		out << "\"bus\": " << "\"" << manager.getBusOnEdge().at(edge_id) << "\"" << ", ";
		out << "\"span_count\": " << manager.getSpanCounts().at(edge_id) << ", ";
		out << "\"type\": \"Bus\"";
		if (idx == route_info->edge_count - 1) {
			out << "}";
		} else {
			out << "}, ";
		}
	}
	out << "], ";
	out << "\"request_id\": " << id_;
	out << "}";
}


RequestHolder ModifyingRequest::Create(ModifyingRequest::Type type) {
	switch (type) {
		case ModifyingRequest::Type::BUS_LOG_INFO:
			return std::make_unique<BusLogInfoRequest>();
		case ModifyingRequest::Type::STOP_LOG_INFO:
			return std::make_unique<StopLogInfoRequest>();
		default:
			return nullptr;
	}
}

RequestHolder InfoRequest::Create(InfoRequest::Type type) {
	switch (type) {
		case InfoRequest::Type::BUS_INFO:
			return std::make_unique<BusInfoRequest>();
		case InfoRequest::Type::STOP_INFO:
			return std::make_unique<StopInfoRequest>();
		case InfoRequest::Type::ROUTE_INFO:
			return std::make_unique<RouteInfoRequest>();
		default:
			return nullptr;
	}
}

