#include "opcounter.h"
#include <sstream>

TOpCounter::TOpCounter(double alpha, double threshold)
{
    Reset(alpha, threshold);
}

TOpCounter::~TOpCounter() {
}

void TOpCounter::Reset() {
    Values.fill(0);
}

void TOpCounter::Reset(double alpha, double threshold) {
    Reset();
    Values[Alpha] = alpha;
    Values[Threshold] = threshold;
}

void TOpCounter::SetParameters(double alpha, double threshold, int actual) {
    Values[Alpha] = alpha;
    Values[Threshold] = threshold;
    Actual = actual;
}

void TOpCounter::Calculate(size_t positiveCountPassed, size_t positiveCountTotal,
        size_t negativeCountPassed, size_t negativeCountTotal, bool asc)  {
    size_t tpc, fpc, tnc, fnc;
    if (asc) {
        tpc = positiveCountTotal - positiveCountPassed;
        fpc = negativeCountTotal - negativeCountPassed;
        tnc = negativeCountPassed;
        fnc = positiveCountPassed;
    } else {
        tpc = positiveCountPassed;
        fpc = negativeCountPassed;
        tnc = negativeCountTotal - negativeCountPassed;
        fnc = positiveCountTotal - positiveCountPassed;
    }
    Values[Precision] = (double)tpc / (double)(tpc + fpc + EPS);
    Values[FDR] = (double)fpc / (double)(tpc + fpc + EPS);
    Values[Recall] = (double)tpc / (double)(tpc + fnc + EPS);
    Values[TPR] = Values[Recall];
    Values[TNR] = (double)tnc / (double)(tnc + fpc + EPS);
    Values[FPR] = (double)fpc / (double)(tnc + fpc + EPS);
    Values[Accuracy] = (double)(tpc + tnc) / (double)(tpc + tnc + fpc + fnc + EPS);
    Values[NPV] = (double)tnc / (double)(tnc + fnc + EPS);
    Values[Fmeasure] = 1 / (Values[Alpha] / Values[Precision] + (1.0 - Values[Alpha]) / Values[Recall]);
}

/*
    %T - Threshold
    %p - Precision
    %d - False Discovery Rate
    %r - True Positive Rate or Recall
    %t - True Negative Rate
    %f - False Positive Rate
    %a - Accuracy
    %n - Negative Predictive Value
    %F - F-measure
    %A - Alpha
*/
TOpCounter::FieldOffset TOpCounter::GetFieldOffset(const std::string& str) {
    FieldOffset offset;
    if ((str == "prc") || (str == "p"))
        offset = Precision;
    else if ((str == "tpr") || (str == "r"))
        offset = TPR;
    else if ((str == "fms") || (str == "F"))
        offset = Fmeasure;
    else if ((str == "tnr") || (str == "t"))
        offset = TNR;
    else if ((str == "npv") || (str == "n"))
        offset = NPV;
    else if ((str == "fdr") || (str == "d"))
        offset = FDR;
    else if ((str == "fpr") || (str == "f"))
        offset = FPR;
    else if ((str == "acc") || (str == "a"))
        offset = Accuracy;
    else if ((str == "thr") || (str == "T"))
        offset = Threshold;
    else if (str == "A")
        offset = Alpha;
    else
        offset = InvalidOffset;
    return offset;
}

void TOpCounter::GetLine(const std::string& format, std::string& destination) const {
    std::stringstream ss;
    FieldOffset offset;

    for (size_t i = 0; i < format.length(); ++i) {
        if (format[i] != '%')
            ss << format[i];
        else {
            ++i;
            offset = GetFieldOffset(std::string(1, format[i]));
            if (offset != InvalidOffset)
                ss << Values[offset];
            else
                ss << '%' << format[i];
        }
    }

    destination = ss.str();
}

double TOpCounter::GetValue(const FieldOffset offset) const {
    if (offset < Values.size())
        return Values[offset];
    else
        return EPS;
}

bool TOpCounter::operator<(const TOpCounter& c) const {
    return Values[Threshold] < c.Values[Threshold];
}
