#ifndef SCANPERFORMANCECONFIGWIZARD_H
#define SCANPERFORMANCECONFIGWIZARD_H

class ScanPerformanceConfigWizard
{
public:
    ScanPerformanceConfigWizard();

    int ScanDeep;
    int SynThreadCount;
    int Retry;
    int TimeOut;
};

#endif // SCANPERFORMANCECONFIGWIZARD_H
