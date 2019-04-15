#ifndef GRAPHWALKER_FILENAMES_DEF
#define GRAPHWALKER_FILENAMES_DEF

#include <fstream>
#include <fcntl.h>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#include "api/datatype.hpp"
#include "logger/logger.hpp"

static std::string intervalname( std::string basefilename, sid_t p ){
    std::stringstream ss;
    ss << basefilename;
    ss << "_GraphWalker/graphinfo/interval";
    ss << "_" << p;
    return ss.str();
}

static std::string walksname( std::string basefilename, sid_t p ){
    std::stringstream ss;
    ss << basefilename;
    ss << "_GraphWalker/walks/pool";
    ss << "_" << p << ".walks";
    return ss.str();
}

static std::string filename_intervals(std::string basefilename, unsigned long long shardsize_kb){
    std::stringstream ss;
    ss << basefilename;
    ss << "_GraphWalker/shardsize_" << shardsize_kb << "KB.intervals";
    return ss.str();
}

static std::string filename_nvertices(std::string basefilename) {
    std::stringstream ss;
    ss << basefilename;
    ss << "_GraphWalker/N.nvertices"; 
    return ss.str();
}

/**
 * Configuration file name
 */
static std::string filename_config() {
    char * chi_root = getenv("GRAPHCHI_ROOT");
    if (chi_root != NULL) {
        return std::string(chi_root) + "/conf/graphchi.cnf";
    } else {
        return "conf/graphwalker.cnf";
    }
}

/**
 * Configuration file name - local version which can
 * override the version in the version control.
 */
static std::string filename_config_local() {
    char * chi_root = getenv("GRAPHCHI_ROOT");
    if (chi_root != NULL) {
        return std::string(chi_root) + "/conf/graphwalker.local.cnf";
    } else {
        return "conf/graphwalker.local.cnf";
    }
}

#endif