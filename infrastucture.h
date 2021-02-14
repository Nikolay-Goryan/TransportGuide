//
//  infrastucture.h
//  TransportGuide_A
//
//  Created by Николай Горян on 09.09.2020.
//  Copyright © 2020 Николай Горян. All rights reserved.
//

#pragma once

#include <string>
#include <vector>


class Stop {
public:
	Stop(std::string name, double latitude, double longitude);
	
	bool operator ==(const Stop & obj) const;
	
	const std::string getName() const;
	const double getLatitude() const;
	const double getLongitude() const;
	
private:
	std::string name_;
	double latitude_;
	double longitude_;
};

template<>
struct std::hash<Stop> {
	size_t operator()(const Stop & stop) const {
		return hash<std::string>()(stop.getName());
	}
};


class Route {
public:
	Route(bool is_circular, std::vector<std::string> stops);
	Route();
	
	const bool IsCircular() const {
		return is_circular_;
	}
	
	const std::vector<std::string> getStops() const {
		return stops_;
	}

	bool operator== (const Route& other) const;
	
private:
	bool is_circular_;
	std::vector<std::string> stops_;
};
