//
//  infrastructure.cpp
//  TransportGuide_A
//
//  Created by Николай Горян on 09.09.2020.
//  Copyright © 2020 Николай Горян. All rights reserved.
//

#include <stdio.h>
#include "infrastucture.h"


Stop::Stop(std::string name, double latitude, double longitude) :
	name_(name),
	latitude_(latitude),
	longitude_(longitude)
	{}
	
bool Stop::operator ==(const Stop & obj) const {
	if (name_ == obj.name_ && latitude_ == obj.latitude_ && longitude_ == obj.longitude_)
		return true;
	else
		return false;
}
	
const std::string Stop::getName() const {
	return name_;
}

const double Stop::getLatitude() const {
	return latitude_;
}
const double Stop::getLongitude() const {
	return longitude_;
}


Route::Route(bool is_circular, std::vector<std::string> stops)
: is_circular_(is_circular), stops_(std::move(stops)) {}

Route::Route() = default;


bool Route::operator== (const Route& other) const {
	if (is_circular_ == other.is_circular_ && stops_ == other.stops_) {
		return true;
	}
	return false;
}
