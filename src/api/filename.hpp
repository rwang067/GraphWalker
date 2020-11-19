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

static std::string blockname( std::string basefilename, vid_t startv ){
    std::stringstream ss;
    ss << basefilename;
    ss << "_GraphWalker/graphinfo/block";
    ss << "_" << startv;
    return ss.str();
}

static std::string walksname( std::string basefilename, bid_t p ){
    std::stringstream ss;
    ss << basefilename;
    ss << "_GraphWalker/walks/pool";
    ss << "_" << p << ".walks";
    return ss.str();
}

// static std::string filerangename(std::string basefilename, uint16_t filesize_MB){
//     std::stringstream ss;
//     ss << basefilename;
//     ss << "_GraphWalker/filesize_" << filesize_MB << "MB.filerange";
//     return ss.str();
// }

static std::string blockrangename(std::string basefilename, uint16_t blocksize_MB){
    std::stringstream ss;
    ss << basefilename;
    ss << "_GraphWalker/graphinfo/blocksize_" << blocksize_MB << "MB.blockrange";
    return ss.str();
}

static std::string nverticesname(std::string basefilename) {
    std::stringstream ss;
    ss << basefilename;
    ss << "_GraphWalker/graphinfo/N.nvertices"; 
    return ss.str();
}

/**
 * Configuration file name
 */
static std::string configname() {
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
static std::string configlocalname() {
    char * chi_root = getenv("GRAPHCHI_ROOT");
    if (chi_root != NULL) {
        return std::string(chi_root) + "/conf/graphwalker.local.cnf";
    } else {
        return "conf/graphwalker.local.cnf";
    }
}

#endif