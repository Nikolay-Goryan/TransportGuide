//
//  refactoring.h
//  TransportGuide_A
//
//  Created by Николай Горян on 09.09.2020.
//  Copyright © 2020 Николай Горян. All rights reserved.
//

#pragma once

#include <string_view>
#include <optional>



std::pair<std::string_view, std::optional<std::string_view>> SplitTwoStrict(std::string_view s, std::string_view delimiter = " ");

std::pair<std::string_view, std::string_view> SplitTwo(std::string_view s, std::string_view delimiter = " ");

std::string_view ReadToken(std::string_view& s, std::string_view delimiter = " ");

std::string_view DeleteSpaces(std::string_view str_view);
