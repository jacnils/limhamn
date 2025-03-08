//
// Created by Jacob Nilsson on 2025-03-07.
//

#pragma once

#include <iostream>

#define REQUIRE(expr) if (!(expr)) { std::cerr << "Requirement failed: " << #expr << std::endl; std::exit(EXIT_FAILURE); }