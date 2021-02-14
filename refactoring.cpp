//
//  refactoring.cpp
//  TransportGuide_A
//
//  Created by Николай Горян on 09.09.2020.
//  Copyright © 2020 Николай Горян. All rights reserved.
//

#include <stdio.h>

#include "refactoring.h"


std::pair<std::string_view, std::optional<std::string_view>> SplitTwoStrict(std::string_view s, std::string_view delimiter) {
  const size_t pos = s.find(delimiter);
  if (pos == s.npos) {
    return {s, std::nullopt};
  } else {
    return {s.substr(0, pos), s.substr(pos + delimiter.length())};
  }
}

std::pair<std::string_view, std::string_view> SplitTwo(std::string_view s, std::string_view delimiter) {
  const auto [lhs, rhs_opt] = SplitTwoStrict(s, delimiter);
  return {lhs, rhs_opt.value_or("")};
}

std::string_view ReadToken(std::string_view& s, std::string_view delimiter) {
  const auto [lhs, rhs] = SplitTwo(s, delimiter);
  s = rhs;
  return lhs;
}

std::string_view DeleteSpaces(std::string_view str_view) {       //tested
	size_t i = str_view.find_first_not_of(" ");
	str_view.remove_prefix(i);
	i = 0;
	for (auto it = str_view.rbegin(); it != str_view.rend(); it++) {
		if (*it == ' ') {
			i++;
		} else {
			break;
		}
	}
	str_view.remove_suffix(i);
	return str_view;
}
