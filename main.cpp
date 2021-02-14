//
//  main.cpp
//  TransportGuide_A
//
//  Created by Николай Горян on 01.09.2020.
//  Copyright © 2020 Николай Горян. All rights reserved.
//



#include "request.h"
#include "json.h"
#include "test_runner.h"

#include <iostream>
#include <istream>
#include <string>
#include <vector>
#include <string_view>
#include <optional>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <cmath>
#include <tuple>

using namespace std;


const unordered_map<string_view, InfoRequest::Type> STR_TO_INFO_REQUEST_TYPE = {
	{"Bus", InfoRequest::Type::BUS_INFO},
	{"Stop", InfoRequest::Type::STOP_INFO},
	{"Route", InfoRequest::Type::ROUTE_INFO},
};

const unordered_map<string_view, ModifyingRequest::Type> STR_TO_MODIFYING_REQUEST_TYPE = {
	{"Bus", ModifyingRequest::Type::BUS_LOG_INFO},
	{"Stop", ModifyingRequest::Type::STOP_LOG_INFO},
};

optional<InfoRequest::Type> ConvertInfoRequestTypeFromString(string_view type_str) {
  if (const auto it = STR_TO_INFO_REQUEST_TYPE.find(type_str);
      it != STR_TO_INFO_REQUEST_TYPE.end()) {
    return it->second;
  } else {
    return nullopt;
  }
}

optional<ModifyingRequest::Type> ConvertModifyingRequestTypeFromString(string_view type_str) {
	if (const auto it = STR_TO_MODIFYING_REQUEST_TYPE.find(type_str);
      it != STR_TO_MODIFYING_REQUEST_TYPE.end()) {
    return it->second;
  } else {
    return nullopt;
  }
}

template <typename Number>
Number ReadNumberOnLine(istream& stream) {
  Number number;
  stream >> number;
  string dummy;
  getline(stream, dummy);
  return number;
}

RequestHolder ParseRequest(bool is_modifying, std::map<std::string, Json::Node> parameters) {
	RequestHolder request;
	string_view type = parameters["type"].AsString();
	if (!is_modifying) {
		const auto request_type = ConvertInfoRequestTypeFromString(type);
		if (!request_type) {
			return nullptr;
		}
		request = InfoRequest::Create(*request_type);
	} else {
		const auto request_type = ConvertModifyingRequestTypeFromString(type);
		if (!request_type) {
			return nullptr;
		}
		request = ModifyingRequest::Create(*request_type);
	}
	if (request) {
		request->ParseFrom(parameters);
	};
	return request;
}

const std::tuple<RoutingSettings, vector<RequestHolder>, vector<RequestHolder>> ReadRequests(istream& in_stream = cin) {				// not editable
	Json::Document document = Json::Load(in_stream);
	std::vector<RequestHolder> b_requests;
	std::vector<RequestHolder> s_requests;
	
	const Json::Node& root = document.GetRoot();
	std::map<std::string, Json::Node> request_types = root.AsMap();
	std::map<std::string, Json::Node> routing_settings = request_types["routing_settings"].AsMap();
	RoutingSettings routing_set{static_cast<int>((routing_settings["bus_wait_time"].AsDouble())), static_cast<int>((routing_settings["bus_velocity"].AsDouble()))};
	std::vector<Json::Node> base_requests = request_types["base_requests"].AsArray();
	std::vector<Json::Node> stat_requests = request_types["stat_requests"].AsArray();
	
	for (const Json::Node& base_request: base_requests) {
		b_requests.push_back(ParseRequest(true, base_request.AsMap()));
	}
	
	for (const Json::Node& stat_request: stat_requests) {
		s_requests.push_back(ParseRequest(false, stat_request.AsMap()));
	}

	return std::make_tuple(routing_set, move(b_requests), move(s_requests));
}


void ProcessRequests(const vector<RequestHolder>& base_requests, const vector<RequestHolder>& stat_requests, RoutingSettings routing_set, RequestManager& manager, ostream& out = cout) {  // not editable
	manager.AddRoutingSettings(routing_set);
	out << "[";
	for (const auto& request_holder: base_requests) {
		const auto& request = static_cast<const ModifyingRequest&>(*request_holder);
		request.Process(manager);
	}
//	cout << "-----------------------------------------------" << endl;
	Graph::DirectedWeightedGraph<double> graph(manager.getStops().size());
	manager.CreateGraph(graph);
	Graph::Router<double> router(graph);
	manager.CreateRouter(&router);
//	cout << "-----------------------------------------------" << endl;
	
	for (const auto& request_holder: stat_requests) {
		const auto& request = static_cast<const InfoRequest&>(*request_holder);
		request.Process(manager, out);
		if (request_holder != (*prev(stat_requests.end()))) {
			out << ", ";
		}
	}
	
//	for (const auto& request_holder : requests) {
//		if (request_holder->nature_ == Request::Nature::INFO) {
//			const auto& request = static_cast<const InfoRequest&>(*request_holder);
//			request.Process(manager, out);
//			if (request_holder != (*prev(requests.end()))) {
//				out << ", ";
//			}
//		} else if (request_holder->nature_ == Request::Nature::MODIFY) {
//			const auto& request = static_cast<const ModifyingRequest&>(*request_holder);
//			request.Process(manager);
//		}
//	}
	out << "]";
}


int main() {
	RequestManager manager;
	const auto [routing_set, base_requests, stat_requests] = ReadRequests();
	ProcessRequests(base_requests, stat_requests, routing_set, manager, cout);
	
	return 0;
}



/*
 {
   "routing_settings": {
	 "bus_wait_time": 0,
	 "bus_velocity": 10
   },
   "base_requests": [
	 {
	   "type": "Bus",
	   "name": "777",
	   "stops": [
		 "A",
		 "B",
		 "C",
		"D",
		"B",
		"A"
	   ],
	   "is_roundtrip": true
	 },
	 {
	   "type": "Bus",
	   "name": "888",
	   "stops": [
		 "A",
		 "B",
		 "D",
		"C",
		"B",
		"A"
	   ],
	   "is_roundtrip": true
	 },
	 {
	   "type": "Stop",
	   "road_distances": {
		 "B": 1000
	   },
	   "longitude": 37.6517,
	   "name": "A",
	   "latitude": 55.574371
	 },
	 {
	   "type": "Stop",
	   "road_distances": {
		"A":1000,
		 "D": 1000,
		"C": 2000
	   },
	   "longitude": 37.645687,
	   "name": "B",
	   "latitude": 55.587655
	 },
	 {
	   "type": "Stop",
	   "road_distances": {
		"B": 2000,
		"C": 1000
	 },
	   "longitude": 37.603938,
	   "name": "D",
	   "latitude": 55.611717
	 },
 {
   "type": "Stop",
   "road_distances": {
	"B": 4000,
	"D": 2000
 },
   "longitude": 37.603938,
   "name": "C",
   "latitude": 55.611717
 }
   ],
   "stat_requests": [
	 {
	   "type": "Route",
	   "from": "B",
	   "to": "C",
	   "id": 4
	 },
	 {
	   "type": "Route",
	   "from": "C",
	   "to": "B",
	   "id": 5
	 }
   ]
 }
 */
