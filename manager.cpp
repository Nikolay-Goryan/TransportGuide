//
//  manager.cpp
//  TransportGuide_A
//
//  Created by Николай Горян on 10.09.2020.
//  Copyright © 2020 Николай Горян. All rights reserved.
//

#include <stdio.h>
#include <iostream>

#include "manager.h"



double ComputeTimeForward(std::string to,
						   RequestManager& manager,
						   std::vector<std::string>::iterator from_it) {
	const auto& road_distances = manager.getRoadDistances();
	int velocity = manager.getRoutingSettings().bus_velocity;
	int wait_time = manager.getRoutingSettings().bus_wait_time;
	double result = 0.0;
	if (to == *from_it) {
		return result;
	}
	int sum_distance = 0;
	for (auto it = next(from_it); *it != to; ++it) {
		sum_distance += road_distances.at(*from_it).at(*it);
		from_it = it;
	}
	sum_distance += road_distances.at(*from_it).at(to);
	
	result = sum_distance / 1000.0 / velocity * 60.0 + wait_time;
	
	return result;
}

double ComputeTimeBackward(std::string to,
						   RequestManager& manager,
						   std::vector<std::string>::iterator from_it) {
	const auto& road_distances = manager.getRoadDistances();
	int velocity = manager.getRoutingSettings().bus_velocity;
	int wait_time = manager.getRoutingSettings().bus_wait_time;
	double result = 0.0;
	if (to == *from_it) {
		return result;
	}
	int sum_distance = 0;
	for (auto it = prev(from_it); *it != to; --it) {
		sum_distance += road_distances.at(*from_it).at(*it);
		from_it = it;
	}
	sum_distance += road_distances.at(*from_it).at(to);
	
	result = sum_distance / 1000.0 / velocity * 60.0 + wait_time;
	
	return result;
}



void FillGraph(RequestManager& manager, Graph::DirectedWeightedGraph<double>& graph) {
	manager.FillStopsIds();
	for (const auto& bus: manager.getBusses()) {
		std::vector<std::string> stops = bus.second.getStops();
		std::string bus_name = bus.first;
		bool isCircular = bus.second.IsCircular();
		
		double time;
		size_t to_id;
		if (isCircular) {
			// fills all the edges from first station and backwards by cycle
			std::vector<std::string>::iterator from_stop_it = stops.begin();
			size_t from_id = manager.getIdsStops().at(*from_stop_it);
//			if (stops.size() < 3) {
//				return;
//			}
			for (auto it = next(from_stop_it); it != prev(stops.end()); ++it) {
				to_id = manager.getIdsStops().at(*it);
				time = ComputeTimeForward(*it, manager, from_stop_it);
				auto edge_id = graph.AddEdge(Graph::Edge<double>{from_id, to_id, time});
//				std::cout << edge_id << " : " << *from_stop_it << " : " << *it << " : " << time <<  std::endl;
				manager.AddBusOnEdge(edge_id, bus_name);
				manager.AddSpanCouns(edge_id, it -  from_stop_it);
				time = ComputeTimeForward(*from_stop_it, manager, it);
				edge_id = graph.AddEdge(Graph::Edge<double>{to_id, from_id, time});
//				std::cout << edge_id <<  " : " << *it << " : " << *from_stop_it << " : " << time <<  std::endl;
				manager.AddBusOnEdge(edge_id, bus_name);
				manager.AddSpanCouns(edge_id, stops.end() - 1 - it);
				// TODO: add information about number of stops: span_count.
			}
//			if (stops.size() == 3) {
//				return;
//			}
			// fills all the edges begining from second to each following exclude last(first)
			for (auto from = next(from_stop_it); from != prev(stops.end(), 2); ++from) {
				for (auto to = next(from); to != prev(stops.end()); ++to) {
					
					time = ComputeTimeForward(*to, manager, from);
					from_id = manager.getIdsStops().at(*from);
					to_id = manager.getIdsStops().at(*to);
					auto edge_id = graph.AddEdge(Graph::Edge<double>{from_id, to_id, time});
//					std::cout << edge_id << " : " << *from << " : " << *to << " : " << time <<  std::endl;
					manager.AddBusOnEdge(edge_id, bus_name);
					manager.AddSpanCouns(edge_id, to -  from);
				}
			}
		} else {
			size_t from_id;
			for (auto from = stops.begin(); from != prev(stops.end()); ++from) {
				for (auto to = next(from); to != stops.end(); ++to) {
					from_id = manager.getIdsStops().at(*from);
					to_id = manager.getIdsStops().at(*to);
					time = ComputeTimeForward(*to, manager, from);
					auto edge_id = graph.AddEdge(Graph::Edge<double>{from_id, to_id, time});
//					std::cout << edge_id << " : " << *from << " : " << *to << " : " << time <<  std::endl;
					manager.AddBusOnEdge(edge_id, bus_name);
					manager.AddSpanCouns(edge_id, to - from);
					time = ComputeTimeBackward(*from, manager, to);
					edge_id = graph.AddEdge(Graph::Edge<double>{to_id, from_id, time});
//					std::cout << edge_id << " : " << *to << " : " << *from << " : " << time <<  std::endl;
					manager.AddBusOnEdge(edge_id, bus_name);
					manager.AddSpanCouns(edge_id, to - from);
				}
			}
		}
	}
}


void RequestManager::CreateGraph(Graph::DirectedWeightedGraph<double>& graph) {
	graph_ = &graph;
	FillGraph(*this, *graph_);
}

void RequestManager::CreateRouter(Graph::Router<double>* router) {
	router_ = router;
}

void RequestManager::AddBusOnStops(std::string bus_name, std::vector<std::string> stops) {
	for (const std::string& stop: stops) {
		if (bus_name == "") {
			busses_on_stops_[stop];
			continue;
		} else {
			busses_on_stops_[stop].insert(bus_name);
		}
		
	}
}

void RequestManager::AddBus(std::string number, Route route) {
	busses_.insert({number, std::move(route)});
}

void RequestManager::AddStop(std::string name, double latitude, double longitude) {
	stops_.insert({name, {latitude, longitude}});
}

void RequestManager::AddRoadDistances(const std::string stop_name, std::unordered_map<std::string, int> distances) {
	for (auto distance: distances) {
		road_distances[stop_name][distance.first] = distance.second;
		if (road_distances[distance.first][stop_name] == 0) {
			road_distances[distance.first][stop_name] = distance.second;
		}
	}
}

void RequestManager::AddRoutingSettings(RoutingSettings routing_set) {
	routing_set_ = routing_set;
}

void RequestManager::AddBusOnEdge(size_t edge, std::string bus) {
	buses_on_edges[edge] = bus;
}

void RequestManager::AddSpanCouns(size_t edge, size_t count) {
	span_counts[edge] = count;
}

//void RequestManager::AddStopsIds(size_t stop_id, std::string stop_name) {
//	stops_ids_[stop_id] = stop_name;
//}

//void RequestManager::AddBusesIds(size_t bus_id, std::string bus_name) {
//	buses_ids_[bus_id] = bus_name;
//}

void RequestManager::FillStopsIds() {
	size_t i = 0;
	for (const auto& stop: stops_) {
		ids_stops_[stop.first] = i;
		stops_ids_[i] = stop.first;
		i++;
	}
}

const std::unordered_map<std::string, Route>& RequestManager::getBusses() const {
	return busses_;
}

const std::unordered_map<std::string, std::pair<double, double>>& RequestManager::getStops() const {
	return stops_;
}

const std::unordered_map<std::string, std::set<std::string>>& RequestManager::getBussesOnStops() const {
	return busses_on_stops_;
}

const std::optional<Route> RequestManager::getRouteByName(const std::string& name) const {
	if (busses_.find(name) != busses_.end()) {
		return busses_.at(name);
	}
	return std::nullopt;
}

const std::unordered_map<std::string, std::unordered_map<std::string, int>>& RequestManager::getRoadDistances() const {
	return road_distances;
}

const RoutingSettings& RequestManager::getRoutingSettings() const {
	return routing_set_;
}

const std::unordered_map<size_t, std::string>& RequestManager::getStopsIds() const {
	return stops_ids_;
}

const std::unordered_map<std::string, size_t>& RequestManager::getIdsStops() const {
	return ids_stops_;
}

const std::unordered_map<size_t, std::string>& RequestManager::getBusOnEdge() const {
	return buses_on_edges;
}

const std::unordered_map<size_t, size_t>& RequestManager::getSpanCounts() const {
	return span_counts;
}

const Graph::DirectedWeightedGraph<double>* RequestManager::getGraph() const {
	return graph_;
}

const Graph::Router<double>* RequestManager::getRouter() const {
	return router_;
}

//const std::unordered_map<size_t, std::string>& RequestManager::getBusesIds() const {
//	return buses_ids_;
//}
