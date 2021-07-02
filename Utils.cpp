#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include "Utils.h"
#include "./ext_libs/Eigen/Sparse"

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#include <sys/resource.h>

#if defined(__APPLE__) && defined(__MACH__)
#include <mach/mach.h>

#elif (defined(_AIX) || defined(__TOS__AIX__)) || (defined(__sun__) || defined(__sun) || defined(sun) && (defined(__SVR4) || defined(__svr4__)))
#include <fcntl.h>
#include <procfs.h>

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
#include <stdio.h>

#endif

#else
#error "Cannot define getPeakRSS( ) or getCurrentRSS( ) for an unknown OS."
#endif

using namespace std;

void Utils::usage() {
    cerr << "pagerank [-mp metapath] [-n nodes_dir] [-r relations_dir] [-co constraints_file] " << endl
         << " -m metapath" << endl
         << "    metapth to be used" << endl
         << " -indir nodes_dir" << endl
         << "    directory with node data files" << endl
         << " -idir relations_file" << endl
         << "    directory with relations files" << endl
         << " -c constraints_file" << endl
         << "    file with node constraints" << endl
         << " -algo algorithm to be used" << endl
         << "    available algorithms: { 'Seq', 'DynP' }" << endl
         << " -ad adaptive matrix multiplication (disk based)" << endl
         << "    values : {0, 1}" << endl
         << " -mem total memory to be used in case of adaptive matrix mulitplication" << endl
         << "    total memory in MB" << endl
         << " -out output directory" << endl
         << "    if not given results are printed on stdout" << endl;
}

int Utils::checkArg(int i, int max) {
    if (i == max) {
        usage();
        exit(1);
    }
    return i + 1;
}

int Utils::get_column_idx(string filename, string fieldName) {

    ifstream infile(filename);

    if (infile.good()) {

        string line, columnName;

        getline(infile, line);
        std::istringstream iss(line);

        int i = 0;
        while (getline(iss, columnName, '\t')) {

            if (columnName == fieldName) {
                infile.close();
                return i;
            }
            i++;
        }
    }
    infile.close();
    return -1;
}

int Utils::get_max_column_value(string filename, int column_idx) {
    ifstream infile;
    infile.open(filename);

    int max_value = -1;

    if (infile.good()) {
        string line;

        while (getline(infile, line)) {

            int i = 0;
            std::istringstream iss(line);
            while (getline(iss, line, '\t')) {
                if (i == column_idx) {
                    int cur_value = strtol(line.c_str(), NULL, 10);
                    if (cur_value > max_value) {
                        max_value = cur_value;
                    }
                }
                i++;

            }
        }
    }

    infile.close();
    return max_value;
}

void Utils::print(Eigen::SparseMatrix<int> *matrix_) {
    for (int k=0; k < (*matrix_).outerSize(); ++k)
    {
        for (Eigen::SparseMatrix<int>::InnerIterator it(*matrix_, k); it; ++it)
        {
            std::cout << it.row() << "\t" << it.col() << "\t" << it.value() << endl;
        }
    }
    std::cout << endl;
}

void Utils::debug_msg(string msg) {
    #ifdef DEBUG_MSG
        cout << msg << endl;
    #endif
}

vector<TransitionMatrix*> Utils::slice(vector<TransitionMatrix*> matrices, size_t start, size_t len) {
    return vector<TransitionMatrix*>(matrices.begin() + start, matrices.begin() + start + len);
}

vector<int> Utils::slice(vector<int> matrices, size_t start, size_t len) {
    return vector<int>(matrices.begin() + start, matrices.begin() + start + len);
}

void Utils::split(string line, vector<string> &tokens, char delim) {
    std::istringstream iss(line);
    std::string token;
    while(std::getline(iss, token, delim)) {
        tokens.push_back(token);
    }
}

void Utils::getMetapathAndConstraints(string query_line, string &metapath, tuple<string, string, string> &constraint) {

    vector<string> parts;

    Utils::split(query_line, parts, '\t');
    metapath = parts[0];

    vector<string> c_parts;
    Utils::split(parts[1], c_parts, '=');
    string val = c_parts[1];

    // remove double quotes from value
    val.erase(
            remove( val.begin(), val.end(), '\"' ),
            val.end()
    );

    vector<string> cc_parts;
    Utils::split(c_parts[0], cc_parts, '.');
    string obj = cc_parts[0];
    string pred = cc_parts[1];
    constraint = make_tuple(obj, pred, val);
}

void Utils::printConstraint(tuple<string, string, string> constraint) {
    cout << "\t-\t[ C: " << get<0>(constraint) << ", P: " << get<1>(constraint) << ", V: " << get<2>(constraint) << " ]";
}

/**
 * Returns the peak (maximum so far) resident set size (physical
 * memory use) measured in bytes, or zero if the value cannot be
 * determined on this OS.
 */
size_t Utils::getPeakRSS( )
{
#if defined(_WIN32)
    /* Windows -------------------------------------------------- */
    PROCESS_MEMORY_COUNTERS info;
    GetProcessMemoryInfo( GetCurrentProcess( ), &info, sizeof(info) );
    return (size_t)info.PeakWorkingSetSize;

#elif (defined(_AIX) || defined(__TOS__AIX__)) || (defined(__sun__) || defined(__sun) || defined(sun) && (defined(__SVR4) || defined(__svr4__)))
    /* AIX and Solaris ------------------------------------------ */
    struct psinfo psinfo;
    int fd = -1;
    if ( (fd = open( "/proc/self/psinfo", O_RDONLY )) == -1 )
        return (size_t)0L;      /* Can't open? */
    if ( read( fd, &psinfo, sizeof(psinfo) ) != sizeof(psinfo) )
    {
        close( fd );
        return (size_t)0L;      /* Can't read? */
    }
    close( fd );
    return (size_t)(psinfo.pr_rssize * 1024L);

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
    /* BSD, Linux, and OSX -------------------------------------- */
    struct rusage rusage;
    getrusage( RUSAGE_SELF, &rusage );
#if defined(__APPLE__) && defined(__MACH__)
    return (size_t)rusage.ru_maxrss;
#else
    return (size_t)(rusage.ru_maxrss * 1024L);
#endif

#else
    /* Unknown OS ----------------------------------------------- */
    return (size_t)0L;          /* Unsupported. */
#endif
}

/**
 * Returns the current resident set size (physical memory use) measured
 * in bytes, or zero if the value cannot be determined on this OS.
 */
long double Utils::getCurrentRSS( )
{
#if defined(_WIN32)
    /* Windows -------------------------------------------------- */
    PROCESS_MEMORY_COUNTERS info;
    GetProcessMemoryInfo( GetCurrentProcess( ), &info, sizeof(info) );
    return (size_t)info.WorkingSetSize / (1024.0 * 1024.0);

#elif defined(__APPLE__) && defined(__MACH__)
    /* OSX ------------------------------------------------------ */
    struct mach_task_basic_info info;
    mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
    if ( task_info( mach_task_self( ), MACH_TASK_BASIC_INFO,
        (task_info_t)&info, &infoCount ) != KERN_SUCCESS )
        return (size_t)0L;      /* Can't access? */
    return (size_t)info.resident_size / (1024.0 * 1024.0);

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
    /* Linux ---------------------------------------------------- */
    long rss = 0L;
    FILE* fp = NULL;
    if ( (fp = fopen( "/proc/self/statm", "r" )) == NULL )
        return 0L;      /* Can't open? */
    if ( fscanf( fp, "%*s%ld", &rss ) != 1 )
    {
        fclose( fp );
        return 0L;      /* Can't read? */
    }
    fclose( fp );
    return (size_t)rss * (size_t)sysconf( _SC_PAGESIZE)  / (1024.0 * 1024.0);

#else
    /* AIX, BSD, Solaris, and Unknown OS ------------------------ */
    return 0L;          /* Unsupported. */
#endif
}
