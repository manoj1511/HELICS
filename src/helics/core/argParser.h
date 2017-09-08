/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _ARG_PARSER_
#define _ARG_PARSER_

/**
@file common function for parsing command line arguments for core and brokers
*/

#include <boost/program_options.hpp>
#include <vector>
#include <string>

namespace helics
{
class argDescriptor
{
public:
	std::string flag_;
	std::string type_;
	std::string desc_;
	argDescriptor() = default;
	argDescriptor(std::string flag, std::string type, std::string desc)
		:flag_(std::move(flag)), type_(std::move(type)), desc_(std::move(desc))
	{}
};
using argDescriptors = std::vector<argDescriptor>;

void argumentParser(int argc, char *argv[], boost::program_options::variables_map &vm_map, const argDescriptors &additionalArgs);
}

#endif //_ARG_PARSER_
