#ifndef EVK_EXAMPLES_BENCH_H_
#define EVK_EXAMPLES_BENCH_H_

#include <string>
#include <iostream>
#include <fstream>

typedef std::chrono::steady_clock::time_point time_point;

class Bench
{
    public:
    Bench()=default;
    ~Bench()
    {
        record();
        if (m_file.is_open()) m_file.close();
    }

    void open(std::string file)
    {
        std::ios_base::openmode mode = std::fstream::out;
        m_file.open(file, mode);
        m_file<<"numThreads,";
        m_file<<"frame,";
        m_file<<"startup\n";
    }

    void numThreads(size_t numThreads)
    {
        m_numThreads = numThreads;
    }

    time_point start()
    {
        return std::chrono::high_resolution_clock::now();
    }

    inline void frameTime(time_point _time)
    {
        float d = duration(_time);
        m_frame = d;
    }

    inline void startupTime(time_point _time)
    {
        float d = duration(_time);
        m_startup = d;
    }

    void record()
    {
        m_file<<m_numThreads<<",";
        m_file<<m_frame<<",";
        m_file<<m_startup;
        m_file<<"\n";
    }

    void close()
    {
        m_file.close();
    }

    private:
    std::fstream m_file;
    size_t m_numThreads=1;
    float m_frame=0.0f;
    float m_startup=0.0f;

    float duration(time_point _time)
    {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration =
            std::chrono::duration<float,std::chrono::milliseconds::period>(
                end - _time).count();
        return duration;
    }
};

#endif