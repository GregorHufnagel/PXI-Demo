#pragma once

#include <iostream>
#include <nidcpower.h>
#include <vector>

using namespace std;

//#define first_stage 
//#define second_stage
//#define easy 
#define combi_stage

ViSession           session;
ViChar              resourceName[]  = "LCR_4190_C02_S08";
ViConstString       channelName     = "0";
volatile ViStatus   error           = 0;
ViChar              mssg[300]       = "No Error";
ViAddr              compensationData{};

#ifdef easy
std::vector<ViReal64> frequencies    = { 1e+5, 150e+3, 200e+3 };
std::vector<ViReal64> dc_bias_step   = { -2, 0, 2 , 1.0 };
std::vector<ViReal64> osc_level_step = { 0.1, 0.2, 0.3 };
std::vector<ViReal64> voltages       = { 0.1, 1.0 };
#else
std::vector<ViReal64> frequencies    = { 500e+3, 1000e+3 } ;
std::vector<ViReal64> dc_bias_step   = { -2, -1.5, -1, -0.5 , 0, 0.5 , 1, 1.5, 2};
std::vector<ViReal64> osc_level_step = { 0.5, 1, 1.5 , 7};
std::vector<ViReal64> voltages       = { 0.1, 1.0, 1.5, 0.1, 0.2 };
#endif

ViInt32 actualCount;
ViInt32 numStepsLcr;
ViInt32 numStepsSmu;
int     attribute_smu_count;

void handleError(unsigned long session, int error) {
    if (error) {
        niDCPower_error_message(session, error, mssg);
        std::cout << "Error Message: " << mssg << std::endl << std::endl;
    }
}

int init_session() {
    std::cout << "Start init session ..." << std::endl;
    error = niDCPower_InitializeWithIndependentChannels(resourceName, VI_TRUE, VI_FALSE, &session);
    handleError(session, error);

    return 0;
}

int perform_compensation() {
    std::cout << "Start compensation ..." << std::endl;
    const ViReal64 additionalFrequencies[] = { 0.5e+6, 1e+6 };
    error = niDCPower_PerformLCROpenCompensation(session, "0", 2, additionalFrequencies);
    handleError(session, error);
    std::cout << "open compensation done ..." << std::endl;
    error = niDCPower_PerformLCRShortCompensation(session, "0", 2, additionalFrequencies);
    handleError(session, error);
    std::cout << "short compensation done ..." << std::endl;
    error = niDCPower_GetLCRCompensationData(session, "0", 0, compensationData);
    handleError(session, error);
    std::cout << "get lcr compensation data ..." << std::endl << std::endl;

    return error;
}

int create_lcr(string seq_name) { 
    std::cout << "Start lcr seq configuration ..." << std::endl << std::endl;
    ViInt32 freq_count  = frequencies.size();
    ViInt32 dc_count    = dc_bias_step.size();
    ViInt32 ac_count    = osc_level_step.size();
    numStepsLcr         = freq_count * dc_count * ac_count;

    const ViInt32 attributesLCR[] = { NIDCPOWER_ATTR_INSTRUMENT_MODE , NIDCPOWER_ATTR_LCR_FREQUENCY, NIDCPOWER_ATTR_LCR_VOLTAGE_AMPLITUDE, NIDCPOWER_ATTR_LCR_DC_BIAS_VOLTAGE_LEVEL , NIDCPOWER_ATTR_LCR_IMPEDANCE_RANGE };
    int attribute_lcr_count = sizeof(attributesLCR) / sizeof(attributesLCR[0]);

    NILCRMeasurement* measurements = new NILCRMeasurement[numStepsLcr];

    //error += niDCPower_ResetDevice(session);
    error = niDCPower_ConfigureSourceModeWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE);
    handleError(session, error);
    error = niDCPower_ConfigurePowerLineFrequency(session, NIDCPOWER_VAL_50_HERTZ);
    handleError(session, error);
    error = niDCPower_SetAttributeViBoolean(session, "0", NIDCPOWER_ATTR_LCR_OPEN_COMPENSATION_ENABLED, VI_TRUE);
    handleError(session, error);
    error = niDCPower_SetAttributeViBoolean(session, "0", NIDCPOWER_ATTR_LCR_SHORT_COMPENSATION_ENABLED, VI_TRUE);
    handleError(session, error);
    error = niDCPower_SetAttributeViBoolean(session, "0", NIDCPOWER_ATTR_LCR_LOAD_COMPENSATION_ENABLED, 0);
    handleError(session, error);
    error = niDCPower_SetAttributeViBoolean(session, "0", NIDCPOWER_ATTR_LCR_SHORT_CUSTOM_CABLE_COMPENSATION_ENABLED, 0);
    handleError(session, error);
    error = niDCPower_SetAttributeViInt32(session, "0", NIDCPOWER_ATTR_LCR_STIMULUS_FUNCTION, NIDCPOWER_VAL_AC_VOLTAGE);
    handleError(session, error);
    error = niDCPower_SetAttributeViInt32(session, "0", NIDCPOWER_ATTR_LCR_DC_BIAS_SOURCE, NIDCPOWER_VAL_DC_BIAS_VOLTAGE);
    handleError(session, error);
    error = niDCPower_SetAttributeViInt32(session, "0", NIDCPOWER_ATTR_LCR_MEASUREMENT_TIME, NIDCPOWER_VAL_MEASUREMENT_TIME_SHORT);
    handleError(session, error);
    error = niDCPower_SetAttributeViInt32(session, "0", NIDCPOWER_ATTR_LCR_AUTOMATIC_LEVEL_CONTROL, NIDCPOWER_VAL_ON);
    handleError(session, error);
    error = niDCPower_SetAttributeViInt32(session, "0", NIDCPOWER_ATTR_LCR_DC_BIAS_AUTOMATIC_LEVEL_CONTROL, NIDCPOWER_VAL_ON);
    handleError(session, error);
    error = niDCPower_SetAttributeViInt32(session, "0", NIDCPOWER_ATTR_LCR_SOURCE_DELAY_MODE, NIDCPOWER_VAL_LCR_SOURCE_DELAY_MODE_AUTOMATIC);
    handleError(session, error);
    error = niDCPower_SetAttributeViInt32(session, "0", NIDCPOWER_ATTR_LCR_IMPEDANCE_RANGE_SOURCE, NIDCPOWER_VAL_LCR_IMPEDANCE_RANGE); /**/
    handleError(session, error);
    std::cout << "lcr pre cfg ... " << std::endl;
    handleError(session, error);
    ViRsrc sequence_name = const_cast<ViRsrc>(seq_name.c_str());


    error = niDCPower_CreateAdvancedSequenceWithChannels(session, channelName, sequence_name, attribute_lcr_count, attributesLCR, true);
    handleError(session, error);

    for (ViInt32 step_dc = 0; step_dc < dc_count; ++step_dc) {
        for (ViInt32 step_osc = 0; step_osc < ac_count; ++step_osc) {
            for (ViInt32 step_freq = 0; step_freq < freq_count; ++step_freq) {
                error += niDCPower_CreateAdvancedSequenceStep(session, true);
                error += niDCPower_SetAttributeViInt32(session,  "0", NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_VAL_LCR);
                error += niDCPower_SetAttributeViReal64(session, "0", NIDCPOWER_ATTR_LCR_FREQUENCY, frequencies[step_freq]);
                error += niDCPower_SetAttributeViReal64(session, "0", NIDCPOWER_ATTR_LCR_VOLTAGE_AMPLITUDE, osc_level_step[step_osc]);
                error += niDCPower_SetAttributeViReal64(session, "0", NIDCPOWER_ATTR_LCR_DC_BIAS_VOLTAGE_LEVEL, dc_bias_step[step_dc]);
                ViReal64 expected_impedanz = 0.159 / (frequencies[step_freq] * 1e-12);
                error += niDCPower_SetAttributeViReal64(session, "0", NIDCPOWER_ATTR_LCR_IMPEDANCE_RANGE, expected_impedanz);
            }
        }
    }


    std::cout << "lcr seq cfg: npoint " << numStepsLcr << std::endl;
    handleError(session, error);

    std::cout << "Start of LCR" << std::endl;
    error = niDCPower_SetAttributeViString(session, NULL, NIDCPOWER_ATTR_ACTIVE_ADVANCED_SEQUENCE, sequence_name);
    handleError(session, error);
    error = niDCPower_InitiateWithChannels(session, channelName);
    std::cout << "init lcr seq ... " << std::endl;
    handleError(session, error);

    std::cout << "Waiting for the done event" << std::endl;
    error = niDCPower_WaitForEventWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE_ENGINE_DONE_EVENT, 100);
    handleError(session, error);
    std::cout << "fetching" << std::endl;
    error = niDCPower_FetchMultipleLCR(session, channelName, 1000, numStepsLcr, measurements, &actualCount);
    handleError(session, error);

    for (ViInt32 i = 0; i < numStepsLcr; i++) {
        std::cout << "Measured Cp for step " << i << ": " << measurements[i].Cp << std::endl;
    }
    std::cout << "after lcr seq ... " << std::endl;
    handleError(session, error);
    error = niDCPower_Abort(session);
    handleError(session, error);
    delete[] measurements;

    return error;
}

int create_smu(string seq_name) {
    ViRsrc sequence_name = const_cast<ViRsrc>(seq_name.c_str());
    numStepsSmu = voltages.size();
    std::cout << "smu seq cfg: npoint " << numStepsSmu << std::endl;
    const ViInt32 attributesSMU[] = { NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_ATTR_OUTPUT_FUNCTION, NIDCPOWER_ATTR_VOLTAGE_LEVEL };
    attribute_smu_count = sizeof(attributesSMU) / sizeof(attributesSMU[0]);
    ViReal64* voltageMeasurements   = new double[numStepsSmu];
    ViReal64* currentMeasurements   = new double[numStepsSmu];
    ViBoolean* inCompliance         = new unsigned short[numStepsSmu];

    error += niDCPower_CreateAdvancedSequenceWithChannels(session, channelName, sequence_name, attribute_smu_count, attributesSMU, true);
    for (ViInt32 step = 0; step < numStepsSmu; ++step) {
        error += niDCPower_CreateAdvancedSequenceStep(session, true);
        error += niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_VAL_SMU_PS);
        error += niDCPower_ConfigureOutputFunction(session, channelName, NIDCPOWER_VAL_DC_VOLTAGE);
        error += niDCPower_ConfigureVoltageLevel(session, channelName, voltages[step]);
    }
    std::cout << "create smu seq ... " << std::endl;
    handleError(session, error);
    error = niDCPower_SetAttributeViString(session, NULL, NIDCPOWER_ATTR_ACTIVE_ADVANCED_SEQUENCE, sequence_name);
    std::cout << "NIDCPOWER_ATTR_ACTIVE_ADVANCED_SEQUENCE, SMU1" << std::endl;
    handleError(session, error);
    error = niDCPower_InitiateWithChannels(session, channelName);
    std::cout << "inti smu seq ..."  << std::endl;
    handleError(session, error);

    std::cout << "Waiting for trigger" << std::endl;
    error = niDCPower_WaitForEventWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE_ENGINE_DONE_EVENT, 100);
    if (error) {
        handleError(session, error);
    }
    else {
        error += niDCPower_FetchMultiple(session, channelName, 1000, numStepsSmu, voltageMeasurements, currentMeasurements, inCompliance, &actualCount);
        for (ViInt32 i = 0; i < numStepsSmu; i++) {
            std::cout << "Measured voltage for step " << i << ": " << voltageMeasurements[i] << std::endl;
            std::cout << "Measured current for step " << i << ": " << currentMeasurements[i] << std::endl;
            std::cout << "Measured compliance for step " << i << ": " << inCompliance[i] << std::endl << std::endl;
        }
    }

    std::cout << "after smu seq ... " << std::endl;
    handleError(session, error);
    error = niDCPower_Abort(session);

    return error;
}

int trigger_lcr_seq(string seq_name) {
    ViRsrc sequence_name = const_cast<ViRsrc>(seq_name.c_str());
    NILCRMeasurement* measurements = new NILCRMeasurement[numStepsLcr];
    error += niDCPower_SetAttributeViString(session, NULL, NIDCPOWER_ATTR_ACTIVE_ADVANCED_SEQUENCE, sequence_name);
    error += niDCPower_InitiateWithChannels(session, channelName);
    std::cout << "init lcr seq ... " << std::endl;
    handleError(session, error);
    std::cout << "Waiting for trigger" << std::endl;
    error += niDCPower_WaitForEventWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE_ENGINE_DONE_EVENT, 100);
    std::cout << "Triggered" << std::endl;
    error += niDCPower_FetchMultipleLCR(session, channelName, 1000, numStepsLcr, measurements, &actualCount);
    for (ViInt32 i = 0; i < numStepsLcr; i++) {
        std::cout << "Measured Cp for step " << i << ": " << measurements[i].Cp << std::endl;
    }
    std::cout << "after lcr seq ... " << std::endl;
    handleError(session, error);
    error += niDCPower_Abort(session);

    return error;
}

int trigger_smu_seq(string seq_name) {
    ViRsrc sequence_name = const_cast<ViRsrc>(seq_name.c_str());
    ViReal64* voltageMeasurements  = new double[numStepsSmu];
    ViReal64* currentMeasurements  = new double[numStepsSmu];
    ViBoolean* inCompliance        = new unsigned short[numStepsSmu];
    // SMU 1
    error += niDCPower_SetAttributeViString(session, NULL, NIDCPOWER_ATTR_ACTIVE_ADVANCED_SEQUENCE, sequence_name);
    error = niDCPower_InitiateWithChannels(session, channelName);
    std::cout << "inti smu seq ..." << std::endl;
    handleError(session, error);

    std::cout << "Waiting for trigger" << std::endl;
    error += niDCPower_WaitForEventWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE_ENGINE_DONE_EVENT, 100);
    handleError(session, error);

    std::cout << "Triggered" << std::endl;
    error += niDCPower_FetchMultiple(session, channelName, 1000, numStepsSmu, voltageMeasurements, currentMeasurements, inCompliance, &actualCount);
    for (ViInt32 i = 0; i < numStepsSmu; ++i) {
        std::cout << "Measured voltage for step "    << i << ": " << voltageMeasurements[i] << std::endl;
        std::cout << "Measured current for step "    << i << ": " << currentMeasurements[i] << std::endl;
        std::cout << "Measured compliance for step " << i << ": " << inCompliance[i]        << std::endl << std::endl;
    }

    std::cout << "after smu seq ... " << std::endl;
    handleError(session, error); 
    error += niDCPower_Abort(session);

    return error;
};

int LCR_AdvancedSequenceWithTriggerv33() {
    std::cerr << "Start von Sequence v33" << std::endl << std::endl;
    // Initialize variables
    ViSession session;
    ViChar resourceName[] = "PXI1Slot6";
    ViConstString channelName = "0";
    volatile ViStatus error = 0;
    ViChar mssg[300] = "No Error";
    const ViReal64 frequencies[] = { 100000, 150000, 200000 };
    const ViReal64 dc_bias_step[] = { -2, 0, 2 };
    const ViReal64 osc_level_step[] = { 0.2, 0.5, 1 };
    const ViReal64 voltages[] = { 0.1, 1.0, 1.5 };
    ViInt32 numSteps = sizeof(frequencies) / sizeof(frequencies[0]);
    const ViInt32 attributesLCR[] = { NIDCPOWER_ATTR_INSTRUMENT_MODE , NIDCPOWER_ATTR_LCR_FREQUENCY, NIDCPOWER_ATTR_LCR_VOLTAGE_AMPLITUDE, NIDCPOWER_ATTR_LCR_DC_BIAS_VOLTAGE_LEVEL };
    int attribute_lcr_count = sizeof(attributesLCR) / sizeof(attributesLCR[0]);
    const ViInt32 attributesSMU[] = { NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_ATTR_OUTPUT_FUNCTION, NIDCPOWER_ATTR_VOLTAGE_LEVEL };
    int attribute_smu_count = sizeof(attributesSMU) / sizeof(attributesSMU[0]);
    NILCRMeasurement measurements[3];
    ViReal64    voltageMeasurements[3];
    ViReal64    currentMeasurements[3];
    ViBoolean   inCompliance[3];
    ViInt32     actualCount;

    // Configure and run advanced sequences
    error += niDCPower_InitializeWithIndependentChannels(resourceName, VI_TRUE, VI_FALSE, &session);

    //    error += niDCPower_ResetDevice(session);
    error += niDCPower_ConfigureSourceModeWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE);
    error += niDCPower_ConfigurePowerLineFrequency(session, NIDCPOWER_VAL_50_HERTZ);

    // Function to cause the Sequence to Fail
    //error += niDCPower_SetAttributeViInt32(session, "0", NIDCPOWER_ATTR_LCR_IMPEDANCE_AUTO_RANGE, NIDCPOWER_VAL_AUTO_RANGE_ON);

    error += niDCPower_SetAttributeViBoolean(session, "0", NIDCPOWER_ATTR_LCR_OPEN_COMPENSATION_ENABLED, false);
    error += niDCPower_SetAttributeViBoolean(session, "0", NIDCPOWER_ATTR_LCR_SHORT_COMPENSATION_ENABLED, false);
    error += niDCPower_SetAttributeViBoolean(session, "0", NIDCPOWER_ATTR_LCR_LOAD_COMPENSATION_ENABLED, false);
    error += niDCPower_SetAttributeViInt32(session, "0", NIDCPOWER_ATTR_LCR_STIMULUS_FUNCTION, NIDCPOWER_VAL_AC_VOLTAGE);
    error += niDCPower_SetAttributeViInt32(session, "0", NIDCPOWER_ATTR_LCR_DC_BIAS_SOURCE, NIDCPOWER_VAL_DC_BIAS_VOLTAGE);
    error += niDCPower_SetAttributeViInt32(session, "0", NIDCPOWER_ATTR_LCR_MEASUREMENT_TIME, NIDCPOWER_VAL_MEASUREMENT_TIME_SHORT);
    error += niDCPower_SetAttributeViInt32(session, "0", NIDCPOWER_ATTR_LCR_AUTOMATIC_LEVEL_CONTROL, NIDCPOWER_VAL_ON);
    error += niDCPower_SetAttributeViInt32(session, "0", NIDCPOWER_ATTR_LCR_DC_BIAS_AUTOMATIC_LEVEL_CONTROL, NIDCPOWER_VAL_ON);
    error += niDCPower_SetAttributeViInt32(session, "0", NIDCPOWER_ATTR_LCR_SOURCE_DELAY_MODE, NIDCPOWER_VAL_LCR_SOURCE_DELAY_MODE_AUTOMATIC);
    std::cerr << "Error code: " << error << std::endl << std::endl;

    error += niDCPower_CreateAdvancedSequenceWithChannels(session, channelName, "LCR1", attribute_lcr_count, attributesLCR, true);
    for (ViInt32 step = 0; step < numSteps; ++step) {
        error += niDCPower_CreateAdvancedSequenceStep(session, true);
        error += niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_VAL_LCR);
        error += niDCPower_SetAttributeViReal64(session, channelName, NIDCPOWER_ATTR_LCR_FREQUENCY, frequencies[step]);
        error += niDCPower_SetAttributeViReal64(session, "0", NIDCPOWER_ATTR_LCR_VOLTAGE_AMPLITUDE, osc_level_step[step]);
        error += niDCPower_SetAttributeViReal64(session, "0", NIDCPOWER_ATTR_LCR_DC_BIAS_VOLTAGE_LEVEL, dc_bias_step[step]);
    }
    std::cerr << "Error code: " << error << std::endl << std::endl;
    error += niDCPower_CreateAdvancedSequenceWithChannels(session, channelName, "SMU1", attribute_smu_count, attributesSMU, true);
    for (ViInt32 step = 0; step < numSteps; ++step) {
        error += niDCPower_CreateAdvancedSequenceStep(session, true);
        error += niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_VAL_SMU_PS);
        error += niDCPower_ConfigureOutputFunction(session, channelName, NIDCPOWER_VAL_DC_VOLTAGE);
        error += niDCPower_ConfigureVoltageLevel(session, channelName, voltages[step]);
    }
    std::cerr << "Error code: " << error << std::endl << std::endl;
    // SMU 1
    error += niDCPower_SetAttributeViString(session, NULL, NIDCPOWER_ATTR_ACTIVE_ADVANCED_SEQUENCE, "SMU1");
    error += niDCPower_InitiateWithChannels(session, channelName);
    std::cerr << "Error code: " << error << std::endl << std::endl;
    std::cerr << "Waiting for trigger" << std::endl;
    error += niDCPower_WaitForEventWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE_ENGINE_DONE_EVENT, 100);
    std::cerr << "Triggered" << std::endl;
    error += niDCPower_FetchMultiple(session, channelName, 1000, numSteps, voltageMeasurements, currentMeasurements, inCompliance, &actualCount);
    for (ViInt32 i = 0; i < numSteps; ++i) {
        std::cerr << "Measured voltage for step " << i << ": " << voltageMeasurements[i] << std::endl;
        std::cerr << "Measured current for step " << i << ": " << currentMeasurements[i] << std::endl;
        std::cerr << "Measured compliance for step " << i << ": " << inCompliance[i] << std::endl << std::endl;
    }
    error += niDCPower_Abort(session);
    std::cerr << "Start of LCR" << std::endl << std::endl;
    std::cerr << "Error code: " << error << std::endl << std::endl;
    // LCR 1
    error += niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_VAL_LCR);
    std::cerr << "Error code: " << error << std::endl << std::endl;
    error += niDCPower_SetAttributeViString(session, NULL, NIDCPOWER_ATTR_ACTIVE_ADVANCED_SEQUENCE, "LCR1");
    std::cerr << "Error code: " << error << std::endl << std::endl;
    error += niDCPower_InitiateWithChannels(session, channelName);

    niDCPower_error_message(session, error, mssg);
    std::cerr << "Error Message: " << mssg << std::endl << std::endl;


    std::cerr << "Error code: " << error << std::endl << std::endl;
    std::cerr << "Waiting for trigger" << std::endl;
    error += niDCPower_WaitForEventWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE_ENGINE_DONE_EVENT, 100);
    std::cerr << "Triggered" << std::endl;
    error += niDCPower_FetchMultipleLCR(session, channelName, 1000, numSteps, measurements, &actualCount);
    for (ViInt32 i = 0; i < numSteps; ++i) {
        std::cerr << "Measured Cp for step " << i << ": " << measurements[i].Cp << std::endl;
    }
    error += niDCPower_Abort(session);
    std::cerr << "Error code: " << error << std::endl << std::endl;
    // SMU 1
    error += niDCPower_SetAttributeViString(session, NULL, NIDCPOWER_ATTR_ACTIVE_ADVANCED_SEQUENCE, "SMU1");
    error += niDCPower_InitiateWithChannels(session, channelName);
    std::cerr << "Waiting for trigger" << std::endl;
    error += niDCPower_WaitForEventWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE_ENGINE_DONE_EVENT, 100);
    std::cerr << "Triggered" << std::endl;
    error += niDCPower_FetchMultiple(session, channelName, 1000, numSteps, voltageMeasurements, currentMeasurements, inCompliance, &actualCount);
    for (ViInt32 i = 0; i < numSteps; ++i) {
        std::cerr << "Measured voltage for step " << i << ": " << voltageMeasurements[i] << std::endl;
        std::cerr << "Measured current for step " << i << ": " << currentMeasurements[i] << std::endl;
        std::cerr << "Measured compliance for step " << i << ": " << inCompliance[i] << std::endl << std::endl;
    }
    error += niDCPower_Abort(session);
    std::cerr << "Error code: " << error << std::endl << std::endl;
    // LCR 1
    error += niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_VAL_LCR);
    error += niDCPower_SetAttributeViString(session, NULL, NIDCPOWER_ATTR_ACTIVE_ADVANCED_SEQUENCE, "LCR1");
    error += niDCPower_InitiateWithChannels(session, channelName);
    std::cerr << "Waiting for trigger" << std::endl;
    error += niDCPower_WaitForEventWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE_ENGINE_DONE_EVENT, 100);
    std::cerr << "Triggered" << std::endl;
    error += niDCPower_FetchMultipleLCR(session, channelName, 1000, numSteps, measurements, &actualCount);
    for (ViInt32 i = 0; i < numSteps; ++i) {
        std::cerr << "Measured Cp for step " << i << ": " << measurements[i].Cp << std::endl;
    }
    error += niDCPower_Abort(session);

    error += niDCPower_close(session);
    std::cerr << "Error code: " << error << std::endl << std::endl;
    return 0;
}

int create_combined_seq(string seq_name) {
        std::cout << "Start combi seq configuration ..." << std::endl << std::endl;
        ViInt32 freq_count = frequencies.size();
        ViInt32 dc_count = dc_bias_step.size();
        ViInt32 ac_count = osc_level_step.size();
        numStepsLcr = freq_count * dc_count * ac_count;

        const ViInt32 attributesLCR[] = { NIDCPOWER_ATTR_INSTRUMENT_MODE , NIDCPOWER_ATTR_LCR_FREQUENCY, NIDCPOWER_ATTR_LCR_VOLTAGE_AMPLITUDE, NIDCPOWER_ATTR_LCR_DC_BIAS_VOLTAGE_LEVEL , NIDCPOWER_ATTR_VOLTAGE_LEVEL };
        int attribute_lcr_count = sizeof(attributesLCR) / sizeof(attributesLCR[0]);

        NILCRMeasurement* measurements = new NILCRMeasurement[numStepsLcr];

        error += niDCPower_InitializeWithIndependentChannels(resourceName, VI_TRUE, VI_FALSE, &session);
        error += niDCPower_ResetDevice(session);
        error += niDCPower_ConfigureSourceModeWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE);
        error += niDCPower_ConfigurePowerLineFrequency(session, NIDCPOWER_VAL_50_HERTZ);
        error += niDCPower_SetAttributeViBoolean(session, "0", NIDCPOWER_ATTR_LCR_OPEN_COMPENSATION_ENABLED, false);
        error += niDCPower_SetAttributeViBoolean(session, "0", NIDCPOWER_ATTR_LCR_SHORT_COMPENSATION_ENABLED, false);
        error += niDCPower_SetAttributeViBoolean(session, "0", NIDCPOWER_ATTR_LCR_LOAD_COMPENSATION_ENABLED, false);
        error += niDCPower_SetAttributeViInt32(session,   "0", NIDCPOWER_ATTR_LCR_STIMULUS_FUNCTION, NIDCPOWER_VAL_AC_VOLTAGE);
        error += niDCPower_SetAttributeViInt32(session,   "0", NIDCPOWER_ATTR_LCR_DC_BIAS_SOURCE, NIDCPOWER_VAL_DC_BIAS_VOLTAGE);
        error += niDCPower_SetAttributeViInt32(session,   "0", NIDCPOWER_ATTR_LCR_MEASUREMENT_TIME, NIDCPOWER_VAL_MEASUREMENT_TIME_SHORT);
        error += niDCPower_SetAttributeViInt32(session,   "0", NIDCPOWER_ATTR_LCR_AUTOMATIC_LEVEL_CONTROL, NIDCPOWER_VAL_ON);
        error += niDCPower_SetAttributeViInt32(session,   "0", NIDCPOWER_ATTR_LCR_DC_BIAS_AUTOMATIC_LEVEL_CONTROL, NIDCPOWER_VAL_ON);
        error += niDCPower_SetAttributeViInt32(session,   "0", NIDCPOWER_ATTR_LCR_SOURCE_DELAY_MODE, NIDCPOWER_VAL_LCR_SOURCE_DELAY_MODE_AUTOMATIC);
        std::cout << "cmobi pre cfg ... " << std::endl;
        handleError(session, error);
        ViRsrc sequence_name = const_cast<ViRsrc>(seq_name.c_str());

        numStepsSmu = voltages.size();
        ViReal64* voltageMeasurements = new double[numStepsSmu];
        ViReal64* currentMeasurements = new double[numStepsSmu];
        ViBoolean* inCompliance = new unsigned short[numStepsSmu];

        error += niDCPower_CreateAdvancedSequenceWithChannels(session, channelName, sequence_name, attribute_lcr_count, attributesLCR, true);

        for (ViInt32 step_dc = 0; step_dc < dc_count; ++step_dc) {
            for (ViInt32 step_osc = 0; step_osc < ac_count; ++step_osc) {
                for (ViInt32 step_freq = 0; step_freq < freq_count; ++step_freq) {
                    error += niDCPower_CreateAdvancedSequenceStep(session, true);
                    error += niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_VAL_LCR);
                    error += niDCPower_SetAttributeViReal64(session, channelName, NIDCPOWER_ATTR_LCR_FREQUENCY, frequencies[step_freq]);
                    error += niDCPower_SetAttributeViReal64(session, "0", NIDCPOWER_ATTR_LCR_VOLTAGE_AMPLITUDE, osc_level_step[step_osc]);
                    error += niDCPower_SetAttributeViReal64(session, "0", NIDCPOWER_ATTR_LCR_DC_BIAS_VOLTAGE_LEVEL, dc_bias_step[step_dc]);
                }
            }
        }
        for (ViInt32 step = 0; step < numStepsSmu; ++step) {
            error += niDCPower_CreateAdvancedSequenceStep(session, true);
            error += niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_VAL_SMU_PS);
            error += niDCPower_ConfigureOutputFunction(session, channelName, NIDCPOWER_VAL_DC_VOLTAGE);
            error += niDCPower_ConfigureVoltageLevel(session, channelName, voltages[step]);
        }

        std::cout << "combi seq cfg: npoint " << numStepsLcr << std::endl;
        handleError(session, error);

        std::cout << "Start of Combi" << std::endl;
        error += niDCPower_InitiateWithChannels(session, channelName);
        std::cout << "init cmobi seq ... " << std::endl;
        handleError(session, error);

        std::cout << "Waiting for the done event" << std::endl;
        error += niDCPower_WaitForEventWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE_ENGINE_DONE_EVENT, 100);
        std::cout << "fetching" << std::endl;
        error += niDCPower_FetchMultipleLCR(session, channelName, 1000, numStepsLcr, measurements, &actualCount);
        handleError(session, error);
        error += niDCPower_FetchMultiple(session, channelName, 1000, numStepsSmu, voltageMeasurements, currentMeasurements, inCompliance, &actualCount);

        for (ViInt32 i = 0; i < numStepsLcr; i++) {
            std::cout << "Measured Cp for step " << i << ": " << measurements[i].Cp << std::endl;
        }
        for (ViInt32 i = 0; i < numStepsSmu; ++i) {
            std::cout << "Measured voltage for step " << i << ": " << voltageMeasurements[i] << std::endl;
            std::cout << "Measured current for step " << i << ": " << currentMeasurements[i] << std::endl;
            std::cout << "Measured compliance for step " << i << ": " << inCompliance[i] << std::endl << std::endl;
        }
        std::cout << "after lcr seq ... " << std::endl;
        handleError(session, error);
        error += niDCPower_Abort(session);
        delete[] measurements;

        return error;
}