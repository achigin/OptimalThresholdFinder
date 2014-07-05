#pragma once

#include <string>
#include <vector>
#include <array>

class TOpCounter {
public:

    enum FieldOffset {
        Precision = 0,
        Recall,
        Fmeasure,
        TPR,
        TNR,
        NPV,
        FDR,
        FPR,
        Accuracy,
        Alpha,
        Threshold,
        FieldCount,
        InvalidOffset
    };

    TOpCounter(double alpha = 0.5, double threshold = 0.5);
    ~TOpCounter();

    void Reset();
    void Reset(double alpha, double threshold);
    void SetParameters(double alpha, double threshold, int actual);
    void Calculate(size_t positiveCountPassed, size_t positiveCountTotal,
                   size_t negativeCountPassed, size_t negativeCountTotal, bool asc = true);
    void GetLine(const std::string& format, std::string& destination) const;
    double GetValue(const FieldOffset offset) const;
    bool operator<(const TOpCounter& c) const;

    int Actual;             //Actual value for this threshold

    static FieldOffset GetFieldOffset(const std::string& str);
private:
    std::array<double, FieldCount> Values;
    static constexpr double EPS = 1e-12;
};
