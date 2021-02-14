//
//  manager.h
//  TransportGuide_A
//
//  Created by Николай Горян on 09.09.2020.
//  Copyright © 2020 Николай Горян. All rights reserved.
//

#pragma once

#include "infrastucture.h"
#include "router.h"

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <vector>
#include <set>

struct RoutingSettings {
	int bus_wait_time = 0;
	int bus_velocity = 0;
};

class RequestManager {
	
public:
	void AddBus(std::string number, Route route);
	void AddStop(std::string name, double latitude, double longitude);
	void AddBusOnStops(std::string bus_name, std::vector<std::string> stops);
	void AddRoadDistances(const std::string stop_name, std::unordered_map<std::string, int> distances);
	void AddRoutingSettings(RoutingSettings routing_set);
	void AddBusOnEdge(size_t edge, std::string bus);
	void AddSpanCouns(size_t edge, size_t count);
	void CreateGraph(Graph::DirectedWeightedGraph<double>& graph);
	void CreateRouter(Graph::Router<double>* router);
//	void AddStopsIds(size_t stop_id, std::string stop_name);
//	void AddBusesIds(size_t bus_id, std::string bus_name);
	
	void FillStopsIds();
//	void FillBusesIds();
	
	
	const std::unordered_map<std::string, Route>& getBusses() const;
	const std::unordered_map<std::string, std::pair<double, double>>& getStops() const;
	const std::optional<Route> getRouteByName(const std::string& name) const;
	const std::unordered_map<std::string, std::set<std::string>>& getBussesOnStops() const;
	const std::unordered_map<std::string, std::unordered_map<std::string, int>>& getRoadDistances() const;
	const RoutingSettings& getRoutingSettings() const;
	const std::unordered_map<size_t, std::string>& getStopsIds() const;
	const std::unordered_map<std::string, size_t>& getIdsStops() const;
	const std::unordered_map<size_t, std::string>& getBusOnEdge() const;
	const std::unordered_map<size_t, size_t>& getSpanCounts() const;
	const Graph::DirectedWeightedGraph<double>* getGraph() const;
	const Graph::Router<double>* getRouter() const;
//	const std::unordered_map<size_t, std::string>& getBusesIds() const;
	
	
private:
	std::unordered_map<std::string, std::pair<double, double>> stops_;
	std::unordered_map<std::string, Route> busses_;
	std::unordered_map<std::string, std::set<std::string>> busses_on_stops_;
	std::unordered_map<std::string, std::unordered_map<std::string, int>> road_distances;
	RoutingSettings routing_set_;
	std::unordered_map<size_t, std::string> stops_ids_;
//	std::unordered_map<size_t, std::string> buses_ids_;
	std::unordered_map<std::string, size_t> ids_stops_;
	std::unordered_map<size_t, std::string> buses_on_edges;
	std::unordered_map<size_t, size_t> span_counts;
	Graph::DirectedWeightedGraph<double>* graph_;
	Graph::Router<double>* router_;
};

double ComputeTimeForFreeDistance(std::string to,
						   RequestManager& manager,
								  std::vector<std::string>::iterator from_it);

void FillGraph(RequestManager& manager, Graph::DirectedWeightedGraph<double>& graph);
