//
//  request.h
//  TransportGuide_A
//
//  Created by Николай Горян on 09.09.2020.
//  Copyright © 2020 Николай Горян. All rights reserved.
//


#pragma once

#include "manager.h"
#include "refactoring.h"
#include "json.h"

#include <string_view>
#include <memory>
#include <string>
#include <vector>
#include <iostream>


struct Request;
using RequestHolder = std::unique_ptr<Request>;

struct Request {
	enum class Nature {
		INFO,
		MODIFY,
	};
	
	Request(Nature nature);
	virtual void ParseFrom(std::map<std::string, Json::Node>& parameters) = 0;
	virtual ~Request() = default;

	const Nature nature_;
};


struct ModifyingRequest : Request {
	enum class Type {
		BUS_LOG_INFO,
		STOP_LOG_INFO,
	};
	
	ModifyingRequest(Type type);
	static RequestHolder Create(Type type);
	virtual void Process(RequestManager& manager) const = 0;
	
	Type type_;
};

struct BusLogInfoRequest : ModifyingRequest {
public:
	BusLogInfoRequest();
	void ParseFrom(std::map<std::string, Json::Node>& parameters) override;
	void Process(RequestManager& manager) const override;
	
private:
	std::string bus_name;
	std::vector<std::string> stops_;
	bool is_circular_ = true;
};

struct StopLogInfoRequest : ModifyingRequest {
public:
	StopLogInfoRequest();
	void ParseFrom(std::map<std::string, Json::Node>& parameters) override;
	void Process(RequestManager& manager) const override;
	
private:
	std::string stop_name_;
	double latitude_;
	double longitude_;
	std::unordered_map<std::string, int> distances;
};

struct InfoRequest : Request {
	enum class Type {
		BUS_INFO,
		STOP_INFO,
		ROUTE_INFO,
	};
	
	InfoRequest(Type type);
	static RequestHolder Create(Type type);
	virtual void Process(const RequestManager& manager, std::ostream& out) const = 0;
	
	Type type_;
};

struct BusInfoRequest : InfoRequest {
public:
	BusInfoRequest();
	void ParseFrom(std::map<std::string, Json::Node>& parameters) override;
	void Process(const RequestManager& manager, std::ostream& out) const override;
	
	std::string bus_number_;
	long id_;
	
};

struct StopInfoRequest : InfoRequest {
public:
	StopInfoRequest();
	void ParseFrom(std::map<std::string, Json::Node>& parameters) override;
	void Process(const RequestManager& manager, std::ostream& out) const override;
	
	std::string stop_name_;
	long id_;
};

struct RouteInfoRequest : InfoRequest {
public:
	RouteInfoRequest();
	void ParseFrom(std::map<std::string, Json::Node>& parameters) override;
	void Process(const RequestManager& manager, std::ostream& out) const override;

	std::string from_stop_;
	std::string to_stop_;
	long id_;
};

