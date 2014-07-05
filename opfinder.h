#pragma once
#include "opcounter.h"

typedef std::vector<TOpCounter> TOpFinderData;

struct TOpFinderPlot {
    double XAxis;
    double YAxis;

    bool operator<(const TOpFinderPlot& p) const;
};

class TOpFinder {
public:
    struct TResults {
        double OptimalThreshold;
        double Target;
        double Argument;
        double AUC;
    };

    TOpFinder(size_t actual, size_t predicted, int positive, int negative, bool fixed = true, double alpha = 0.5);
    ~TOpFinder();

    void ReadFromStream(const std::string& inputFileName = "");
    void WriteDataToFile(const std::string& fileName, const std::string& format) const;
    void WritePlotToFile(const std::string& fileName, const std::string& xAxis, const std::string& yAxis, size_t n);
    void Calculate();
    void FindOptimalThreshold(const std::string& target, const std::string& argument, double argVal = 0.95);

    const TResults& GetResults() const;

private:
    TOpFinderData Data;

    void FindNextThreshold(TOpFinderData::iterator& i, double thr) const;

    size_t ActualPosition;
    size_t PredictedPosition;
    double Alpha;
    size_t PC;
    size_t NC;
    bool FixedClass;
    int PositiveClass;
    int NegativeClass;

    TResults Results;
};