#pragma once

#include <string>
#include <iostream>
#include <fstream>

typedef std::chrono::steady_clock::time_point time_point;

class Bench
{
    public:
    Bench()=default;
    ~Bench();

    void open(std::string file, bool overwrite=false);
    void numVertices(size_t num);
    void numCubes(size_t _num);
    void numThreads(size_t num);

    time_point start();
    inline void frameTime(time_point _time)
    {
        float d = duration(_time);
        m_frame = d;
    }
    inline void updateVBOTime(time_point _time)
    {
        float d = duration(_time);
        m_updateVBO = d;
    }
    inline void startupTime(time_point _time)
    {
        float d = duration(_time);
        m_startup = d;
    }
    void record();
    void close();

    private:
    std::fstream m_file;
    size_t m_numVertices=0;
    size_t m_numCubes=0;
    size_t m_numThreads=0;
    float m_frame=0.0f;
    float m_updateVBO=0.0f;
    float m_startup=0.0f;

    float duration(time_point _time)
    {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<float,std::chrono::milliseconds::period>(end - _time).count();
        return duration;
    }
};