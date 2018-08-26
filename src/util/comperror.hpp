#ifndef DEF_GRAPHWALKER_COMPERROR
#define DEF_GRAPHWALKER_COMPERROR

#include <string>
#include "api/filename.hpp"

    template <typename VertexDataType>
    void initialVertexValue(int N, std::string basefilename){
    int maxwindow = 256*1024*1024;
    int st = 0, len = 0;
        while( st < N ){
            len = N-st < maxwindow ? N-st : maxwindow;
            logstream(LOG_INFO) << " s , len : " << st << " " << len << std::endl;
            VertexDataType *vertex_value = (VertexDataType *)malloc(len*sizeof(VertexDataType));
            memset(vertex_value, 0, len*sizeof(VertexDataType));
            int f = open(filename_vertex_data(basefilename).c_str(), O_WRONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
            assert(f >= 0);
            pwritea(f, vertex_value, sizeof(VertexDataType)*len, sizeof(VertexDataType)*st);
            close(f);
            free(vertex_value);
            st += len;
        }
    }

    template <typename VertexDataType>
    void writeFile(int N, std::string basefilename){
        // compute the sum of counting
        VertexDataType *vertex_value = (VertexDataType*)malloc(sizeof(VertexDataType)*N);
        int fv = open(filename_vertex_data(basefilename).c_str(), O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
        assert(fv >= 0);
        preada(fv, vertex_value, sizeof(VertexDataType)*N, sizeof(VertexDataType)*0);
        close(fv);
        unsigned sum = 0;
        for(int i = 0; i < N; i++ ){
            sum += vertex_value[i];
        }
        logstream(LOG_INFO) << "sum : " << sum << std::endl;
        free(vertex_value);

        // compute the counting probability
        int maxwindow = 256*1024*1024;
        int st = 0, len = 0;
        while( st < N ){
            len = N-st < maxwindow ? N-st : maxwindow;
            // logstream(LOG_INFO) << " s , len : " << st << " " << len << std::endl;
            // len = min( maxwindow, N - st );
            vertex_value = (VertexDataType*)malloc(sizeof(VertexDataType)*len);
            fv = open(filename_vertex_data(basefilename).c_str(), O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
            assert(fv >= 0);
            preada(fv, vertex_value, sizeof(VertexDataType)*len, sizeof(VertexDataType)*st);
            close(fv);
            float *visit_prob = (float*)malloc(sizeof(float)*len);
            for(int i = 0; i < len; i++ )
                visit_prob[i] = vertex_value[i] * 1.0 / sum;
            int fp = open(filename_vertex_data(basefilename).c_str(), O_WRONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
            assert(fp >= 0);
            pwritea(fp, visit_prob, sizeof(float)*len, 0);//sizeof(unsigned)*st);
            close(fp);
            free(vertex_value);
            free(visit_prob);
            st += len;
        }
    }

    template <typename VertexDataType>
    void computeError(int N, std::string basefilename, int ntop, std::string app){
        writeFile<VertexDataType>(N, basefilename);
        //read the vertex value
        logstream(LOG_INFO) << "compute error.." << std::endl;
        float* visit_prob = (float*)malloc(sizeof(float)*N);
        int fv = open(filename_vertex_data(basefilename).c_str(), O_RDONLY | O_CREAT, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
        assert(fv >= 0);
        preada(fv, visit_prob, sizeof(float)*N, 0);
        close(fv);

        // read the accurate value and compute the error
        std::ifstream fin(basefilename + "_CompError/accurate_" + app + "_top100.value");
        logstream(LOG_DEBUG) << "accurate " + app + " file : " << basefilename + "_accurate_pr_top100.value" << std::endl;
        int vid ;
        float err=0, appv; //accurate pagerank value
        for(int i = 0; i < ntop; i++ ){
            fin >> vid >> appv;
            // logstream(LOG_INFO) << "vid appv vertex_value err: " << vid << " " << appv << " " << visit_prob[vid] << " " << fabs(visit_prob[vid]-appv)<< " " << fabs(visit_prob[vid]-appv)/appv << std::endl;
            err += fabs(visit_prob[vid]-appv)/appv;
        }
        free(visit_prob);
        err = err / ntop;
        logstream(LOG_DEBUG) << "Error : " << err << std::endl;

        std::ofstream errfile;
        errfile.open(basefilename + "_CompError/GraphWalker_" + app + "_top100.error", std::ofstream::app);
        errfile << err << "\n" ;
        errfile.close();
    }

#endif