#include "opfinder.h"

#include <iostream>
#include <string>
#include <algorithm>
#include <getopt.h>

void print_usage() {
    std::cout << "Usage:\n"
              << "\t-I, --inputfile\n\t\tTab separated file. If not specified data will be read from stdin. To finish input in stdin input empty line.\n"
              << "\t-A, --actualcol\n\t\tNumber of column in tab separated input file, where actual class is stored. Starting from 0.\n"
              << "\t-P, --predictedcol\n\t\tNumber of column in tab separated input file, where predicted class is stored. Starting from 0."
              << "\t-O, --outputfile\n\t\tFile to store calculated results\n"
              << "\t-F, --formatstring\n\t\tFormat string to specify output format. No whitespaces are allowed. \\t - :\n"
              << "\t\t%T - Threshold\n"
              << "\t\t%p - Precision\n"
              << "\t\t%d - False Discovery Rate\n"
              << "\t\t%r - True Positive Rate (Recall)\n"
              << "\t\t%t - True Negative Rate\n"
              << "\t\t%f - False Positive Rate\n"
              << "\t\t%a - Accuracy\n"
              << "\t\t%n - Negative Predictive Value\n"
              << "\t\t%F - F-measure\n"
              << "\t\t%A - Alpha\n"
              << "\t-n, --points\n\t\tNumber of points to build plot\n"
              << "\t-p, --plot\n\t\tSpecifies file to output values for plot. If option is not specified, plot is not going to be calculated.\n"
              << "\t-x, --xaxis\n\t\tSpecifies a value to be calculated as an X-axis in the plot.\n"
              << "\t-y, --yaxis\n\t\tSpecifies a value to be calculated as an Y-axis in the plot. Possible values for X and Y axis::\n"
              << "\t\tprc - Precision\n"
              << "\t\ttpr - True Positive Rate (Recall)\n"
              << "\t\tfms - F-measure\n"
              << "\t\ttnr - True Negative Rate\n"
              << "\t\tnpv - Negative Predictive Value\n"
              << "\t\tfdr - False Discovery Rate\n"
              << "\t\tfpr - False Positive Rate\n"
              << "\t\tacc - Accuracy\n"
              << "\t\tthr - Threshold\n"
              << "\t-a, --alpha\n\t\tNumber in [0;1] to specify alpha coeff in F-measure.\n"
              << "\t-t, --target\n\t\tTarget function for optimal threshold. If you set this option, you'll need to set argument.\n"
              << "\t\t\tPossible values:\n"
              << "\t\tprc - Precision\n"
              << "\t\ttpr - True Positive Rate (Recall)\n"
              << "\t\tfms - F-measure\n"
              << "\t\ttnr - True Negative Rate\n"
              << "\t\tnpv - Negative Predictive Value\n"
              << "\t\tfdr - False Discovery Rate\n"
              << "\t\tfpr - False Positive Rate\n"
              << "\t\tacc - Accuracy\n"
              << "\t-Y, --argument\n\t\tArgument for target function. Possible values are the same as for target function\n"
              << "\t-M, --argval\n\t\tSpecifies min arg value for target function. Should be in [0;1].\n"
              << "\t-q, --pc\n\t\tSpecified value is treated as positive class. Classes that are nor positive nor negative will be ignored.\n"
              << "\t-w, --nc\n\t\tSpecified value is treated as negative class. Classes that are nor positive nor negative will be ignored.\n"
              << "\t-C, --C\n\t\tSpecified value is treated as a boundary value in a set of positive and negative classes.\n"
              << "\t\tThis option overrides --pc and --nc options\n"
              << "\t\tClass > VALUE => Positive Class\n"
              << "\t\tClass <= VALUE => Negative Class\n";
}

/* default values */
static const std::string DEFAULT_FORMAT_STRING = "%T:%p:%d:%r:%t:%f:%a:%n:%F";
static const std::string DEFAULT_POINTS_COUNT = "10000";
static const std::string DEFAULT_X_AXIS = "fpr";
static const std::string DEFAULT_Y_AXIS = "tpr";
static const std::string DEFAULT_ALPHA = "0.5";
static const std::string DEFAULT_ARGUMENT_VALUE = "0.95";
static const std::string DEFAULT_POSITIVE_CLASS = "1";
static const std::string DEFAULT_NEGATIVE_CLASS = "0";

static int auc = 0;

static struct option long_options[] =
{
/* These options set a flag. */
    {"auc",             no_argument, &auc, 1},

/* These options don't set a flag.
We distinguish them by their indices. */
    {"inputfile",       required_argument, 0, 'I'},
    {"actualcol",       required_argument, 0, 'A'},
    {"predictedcol",    required_argument, 0, 'P'},
    {"outputfile",      required_argument, 0, 'O'},
    {"formatstring",    required_argument, 0, 'F'},
    {"points",          required_argument, 0, 'n'},
    {"plot",            required_argument, 0, 'p'},
    {"xaxis",           required_argument, 0, 'x'},
    {"yaxis",           required_argument, 0, 'y'},
    {"alpha",           required_argument, 0, 'a'},
    {"target",          required_argument, 0, 't'},
    {"argument",        required_argument, 0, 'Y'},
    {"argval",          required_argument, 0, 'M'},
    {"pc",              required_argument, 0, 'q'},
    {"nc",              required_argument, 0, 'w'},
    {"C",               required_argument, 0, 'C'},
    {"help",            no_argument, 0, '?'},
    {0, 0, 0, 0}
};

int main (int argc, char* argv[]) {
    int opt = 0;
    int long_index = 0;
    std::string inputFileName, actualColumn, predictedColumn, outputFileName,
           outputFormatString, pointsCount, plotFileName, plotXAxis, plotYAxis,
           alphaValue, targetFunction, argumentForFunction, argumentValue,
           positiveClass, negativeClass, classBound;

    outputFormatString = DEFAULT_FORMAT_STRING;
    pointsCount = DEFAULT_POINTS_COUNT;
    plotXAxis = DEFAULT_X_AXIS;
    plotYAxis = DEFAULT_Y_AXIS;
    alphaValue = DEFAULT_ALPHA;
    argumentValue = DEFAULT_ARGUMENT_VALUE;
    positiveClass = DEFAULT_POSITIVE_CLASS;
    negativeClass = DEFAULT_NEGATIVE_CLASS;


    while ((opt = getopt_long(argc, argv, "I:A:P:O:F:n:p:x:y:a:t:Y:M:q:w:C:?",
            long_options, &long_index )) != -1) {
        switch (opt) {
            case 'I': inputFileName = optarg; break;
            case 'A': actualColumn = optarg; break;
            case 'P': predictedColumn = optarg; break;
            case 'O': outputFileName = optarg; break;
            case 'F': outputFormatString = optarg; break;
            case 'n': pointsCount = optarg; break;
            case 'p': plotFileName = optarg; break;
            case 'x': plotXAxis = optarg; break;
            case 'y': plotYAxis = optarg; break;
            case 'a': alphaValue = optarg; break;
            case 't': targetFunction = optarg; break;
            case 'Y': argumentForFunction = optarg; break;
            case 'M': argumentValue = optarg; break;
            case 'q': positiveClass = optarg; break;
            case 'w': negativeClass = optarg; break;
            case 'C': classBound = optarg; break;
            case '?': print_usage(); return 1;
        }
    }

    //now.... second circle of parsing hell
    double alpha = atof(alphaValue.c_str());
    if ((alpha < 0) || (alpha > 1))
        throw std::runtime_error("Alpha should be in [0;1]");

    double argVal = atof(argumentValue.c_str());
    if ((argVal < 0) || (argVal > 1))
        throw std::runtime_error("Argument value is outside of [0;1].");
    int positive = atoi(positiveClass.c_str());
    int negative = atoi(negativeClass.c_str());
    bool fuzzy = !classBound.empty();
    if (fuzzy)
        positive = atoi(classBound.c_str());

    std::replace(outputFormatString.begin(), outputFormatString.end(), ':', '\t');


    TOpFinder opfinder(atoi(actualColumn.c_str()), atoi(predictedColumn.c_str()), positive, negative, !fuzzy, alpha);
    opfinder.ReadFromStream(inputFileName);
    opfinder.Calculate();

    const TOpFinder::TResults& results = opfinder.GetResults();

    if (auc)
        std::cout << "AUC = " << results.AUC << std::endl;

    if (!outputFileName.empty()) {
        opfinder.WriteDataToFile(outputFileName, outputFormatString);
    }

    if (!plotFileName.empty()) {
        opfinder.WritePlotToFile(plotFileName, plotXAxis, plotYAxis, atoi(pointsCount.c_str()));
    }

    if (!targetFunction.empty()) {
        opfinder.FindOptimalThreshold(targetFunction, argumentForFunction, argVal);
        std::cout << "Optimal threshold = " << results.OptimalThreshold << "\tTarget function = " << results.Target;
        if (argumentForFunction.empty())
            std::cout << std::endl;
        else
            std::cout << "\tArgument = " << results.Argument << std::endl;
    }
    return 0;
}
