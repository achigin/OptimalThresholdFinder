#include "opfinder.h"
#include "common.h"

#include <stdlib.h>
#include <iostream>
#include <istream>
#include <fstream>
#include <memory>
#include <algorithm>
#include <stdexcept>

bool TOpFinderPlot::operator<(const TOpFinderPlot& p) const {
    if (XAxis < p.XAxis)
        return true;
    else if (XAxis == p.XAxis)
        return YAxis < p.YAxis;
    else
        return false;
}

TOpFinder::TOpFinder(size_t actual, size_t predicted, int positive, int negative, bool fixed, double alpha)
    : ActualPosition(actual)
    , PredictedPosition(predicted)
    , Alpha(alpha)
    , FixedClass(fixed)
    , PositiveClass(positive)
    , NegativeClass(negative)
{
    Data.clear();
}

void TOpFinder::ReadFromStream(const std::string& inputFileName) {
    std::string line;
    std::vector<std::string> fields;
    TOpCounter st;

    std::unique_ptr<std::istream> inputStream;
    if (!inputFileName.empty()) {
        inputStream.reset(new std::ifstream(inputFileName));
    } else {
        inputStream.reset(&std::cin);
    }

    Data.clear();
    PC = 0; NC = 0;
    size_t lineCounter = 0;
    double predicted;
    int actual;

    while (std::getline(*(inputStream.get()), line)) {
        if (line.empty())
            break;
        fields.clear();
        ++lineCounter;
        split(line, '\t', fields);
        if (PredictedPosition >= fields.size()) {
            std::cerr << "Predicted class column doesn't exist in line " << lineCounter << " : " << line << std::endl;
            continue;
        }
        if (ActualPosition >= fields.size()) {
            std::cerr << "Actual class column doesn't exist in line " << lineCounter << " : " << line << std::endl;
            continue;
        }
        try {
            predicted = atof(fields[PredictedPosition].c_str());
        } catch (...) {
            std::cerr << "Predicted class value = " << fields[PredictedPosition]
                 << " is not double. Ignoring line : " << lineCounter << std::endl;
            continue;
        }
        try {
            actual = atoi(fields[ActualPosition].c_str());
        } catch (...) {
            std::cerr << "Actual class value = " << fields[ActualPosition]
                 << " is not double. Ignoring line : " << lineCounter << std::endl;
            continue;
        }
        st.SetParameters(Alpha, predicted, actual);

        if (FixedClass) {
            if (st.Actual == PositiveClass) {
                ++PC;
                Data.push_back(st);
            }
            else if (st.Actual == NegativeClass) {
                ++NC;
                Data.push_back(st);
            }
        } else {
            if (st.Actual > PositiveClass) {
                ++PC;
                Data.push_back(st);
            } else {
                ++NC;
                Data.push_back(st);
            }
        }
    }
    if (inputFileName.empty())
        inputStream.release();
    if (Data.size()) {
        std::sort(Data.begin(), Data.end());
        if (!PC)
            std::cerr << "All found records are in set of negative classes. Results may be inaccurate!" << std::endl;
        if (!NC)
            std::cerr << "All found records are in set of positive classes. Results may be inaccurate!" << std::endl;
    }
    else
        std::cerr << "No records related to positive or negative classes have been found." << std::endl
             << "No results are going to be calculated." << std::endl;
}

void TOpFinder::Calculate() {
    size_t pc = 0;
    size_t nc = 0;
    Results.AUC = 0;
    for (size_t i = 0; i < Data.size(); ++i) {
        try {
            Data[i].Calculate(pc, PC, nc, NC);
        }
        catch (...) {
            std::cerr << "Something happened" << std::endl;
        }
        if (FixedClass) {
            if (Data[i].Actual == PositiveClass)
                ++pc;
            else
                ++nc;
        } else {
            if (Data[i].Actual > PositiveClass)
                ++pc;
            else
                ++nc;
        }
        if (i) {
            Results.AUC += (Data[i - 1].GetValue(TOpCounter::FPR) - Data[i].GetValue(TOpCounter::FPR)) *
                            (Data[i].GetValue(TOpCounter::TPR) + Data[i - 1].GetValue(TOpCounter::TPR)) / 2.0;
        }
    }
}

void TOpFinder::WriteDataToFile(const std::string& fileName, const std::string& format) const {
    std::string outString;
    std::ofstream outStream(fileName);
    for (TOpFinderData::const_iterator i = Data.begin(); i != Data.end(); ++i) {
        i->GetLine(format, outString);
        outStream << outString << std::endl;
    }
}

void TOpFinder::WritePlotToFile(const std::string& fileName, const std::string& xAxis, const std::string& yAxis, size_t n) {
    TOpCounter::FieldOffset xOffset = TOpCounter::GetFieldOffset(xAxis);
    if (xOffset == TOpCounter::InvalidOffset)
        throw std::runtime_error("Invalid plot x-axis value");
    TOpCounter::FieldOffset yOffset = TOpCounter::GetFieldOffset(yAxis);
    if (yOffset == TOpCounter::InvalidOffset)
        throw std::runtime_error("Invalid plot y-axis value");
    std::ofstream outStream(fileName);

    size_t step = (size_t)((double)Data.size() / (double)n);
    std::vector<TOpFinderPlot> plotFile(n * 2);
    size_t j = 0;
    for (size_t i = 0; ((i < Data.size()) && (j < n)); i += step, ++j) {
        plotFile[j].XAxis = Data[i].GetValue(xOffset);
        plotFile[j].YAxis = Data[i].GetValue(yOffset);
    }
    if (Data.size()) {
        double thrStep = (Data.back().GetValue(TOpCounter::Threshold) - Data.front().GetValue(TOpCounter::Threshold)) / (double)n;
        double thr = Data.front().GetValue(TOpCounter::Threshold);
        TOpFinderData::iterator it = Data.begin();
        for (size_t i = 0; (i < n) && (j < n * 2); ++i, ++j) {
            if (it != Data.end()) {
                plotFile[j].XAxis = it->GetValue(xOffset);
                plotFile[j].YAxis = it->GetValue(yOffset);
            } else {
                --j;
            }
            thr += thrStep;
            FindNextThreshold(it, thr);
        }
    }

    if (plotFile.size() > j)
        plotFile.resize(j);
    std::sort(plotFile.begin(), plotFile.end());

    for (size_t i = 0; i < plotFile.size(); ++i) {
        outStream << plotFile[i].XAxis << "\t" << plotFile[i].YAxis << std::endl;
    }
}

void TOpFinder::FindNextThreshold(TOpFinderData::iterator& i, double thr) const {
    while ((i->GetValue(TOpCounter::Threshold) < thr) && (i != Data.end())) {
        ++i;
    }
}

void TOpFinder::FindOptimalThreshold(const std::string& target, const std::string& argument, double argVal) {
    TOpCounter::FieldOffset targetOffset = TOpCounter::GetFieldOffset(target);
    if (targetOffset == TOpCounter::InvalidOffset)
        throw std::runtime_error("Invalid target function value");
    TOpCounter::FieldOffset argumentOffset;
    if (!argument.empty()) {
        argumentOffset = TOpCounter::GetFieldOffset(argument);
        if (argumentOffset == TOpCounter::InvalidOffset)
            throw std::runtime_error("Invalid argument for function value");
    }

    TOpFinderData::iterator targetVal = Data.begin();

    if (!argument.empty()) {
        targetVal = Data.end();
        for (TOpFinderData::iterator i = Data.begin(); i != Data.end(); ++i) {
            if (i->GetValue(argumentOffset) >= argVal) {
                if (targetVal == Data.end())
                    targetVal = i;
                else if (i->GetValue(targetOffset) > targetVal->GetValue(targetOffset))
                    targetVal = i;
            }
        }
        Results.Argument = targetVal->GetValue(argumentOffset);
    } else {
        for (TOpFinderData::iterator i = Data.begin(); i != Data.end(); ++i) {
            if (i->GetValue(targetOffset) > targetVal->GetValue(targetOffset))
                targetVal = i;
        }
    }

    Results.OptimalThreshold = targetVal->GetValue(TOpCounter::Threshold);
    Results.Target = targetVal->GetValue(targetOffset);
}

const TOpFinder::TResults& TOpFinder::GetResults() const {
    return Results;
}

TOpFinder::~TOpFinder() {
    Data.clear();
}
