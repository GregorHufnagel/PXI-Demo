#include "iostream"
#include <Windows.h>
#include "nidcpower.h"

using namespace std;

int LCR_AdvancedSequenceWithTriggerv3() {
    // Initialize variables
    ViSession session;
    ViChar resourceName[] = "PXI2Slot8";
    ViConstString channelName = "0";
    volatile ViStatus error = 0;
    const ViReal64 frequencies[] = { 100000, 150000, 200000 };
    const ViReal64 dc_bias_step[] = { -2, 0, 2 };
    const ViReal64 osc_level_step[] = { 0.2, 0.5, 1 };
    const ViReal64 voltages[]   = { 0.1, 1.0, 1.5 };
    ViInt32 numSteps = sizeof(frequencies) / sizeof(frequencies[0]);
    const ViInt32 attributesLCR[] = { NIDCPOWER_ATTR_INSTRUMENT_MODE , NIDCPOWER_ATTR_LCR_FREQUENCY, NIDCPOWER_ATTR_LCR_VOLTAGE_AMPLITUDE, NIDCPOWER_ATTR_LCR_DC_BIAS_VOLTAGE_LEVEL};
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
    error += niDCPower_ConfigureSourceModeWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE);
    error += niDCPower_ConfigurePowerLineFrequency(session, NIDCPOWER_VAL_50_HERTZ);
    error += niDCPower_SetAttributeViInt32(session,   "0",  NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_VAL_LCR);
    error += niDCPower_SetAttributeViInt32(session,   "0",  NIDCPOWER_ATTR_LCR_STIMULUS_FUNCTION, NIDCPOWER_VAL_AC_VOLTAGE);
    error += niDCPower_SetAttributeViInt32(session,   "0",  NIDCPOWER_ATTR_LCR_DC_BIAS_SOURCE, NIDCPOWER_VAL_DC_BIAS_VOLTAGE);
    error += niDCPower_SetAttributeViInt32(session,   "0",  NIDCPOWER_ATTR_LCR_IMPEDANCE_AUTO_RANGE, NIDCPOWER_VAL_AUTO_RANGE_ON);
    error += niDCPower_SetAttributeViInt32(session,   "0",  NIDCPOWER_ATTR_LCR_MEASUREMENT_TIME, NIDCPOWER_VAL_MEASUREMENT_TIME_SHORT);
    error += niDCPower_SetAttributeViBoolean(session, "0",  NIDCPOWER_ATTR_LCR_OPEN_COMPENSATION_ENABLED,  false);
    error += niDCPower_SetAttributeViBoolean(session, "0",  NIDCPOWER_ATTR_LCR_SHORT_COMPENSATION_ENABLED, false);
    error += niDCPower_SetAttributeViBoolean(session, "0",  NIDCPOWER_ATTR_LCR_LOAD_COMPENSATION_ENABLED,  false);
    error += niDCPower_SetAttributeViInt32(session,   "0",  NIDCPOWER_ATTR_LCR_AUTOMATIC_LEVEL_CONTROL, NIDCPOWER_VAL_ON);
    error += niDCPower_SetAttributeViInt32(session,   "0",  NIDCPOWER_ATTR_LCR_DC_BIAS_AUTOMATIC_LEVEL_CONTROL, NIDCPOWER_VAL_ON);
    error += niDCPower_SetAttributeViInt32(session,   "0",  NIDCPOWER_ATTR_LCR_SOURCE_DELAY_MODE, NIDCPOWER_VAL_LCR_SOURCE_DELAY_MODE_AUTOMATIC);


    error += niDCPower_CreateAdvancedSequenceWithChannels(session, channelName, "LCR1", attribute_lcr_count, attributesLCR, true);
    for (ViInt32 step = 0; step < numSteps; ++step) {
        error += niDCPower_CreateAdvancedSequenceStep(session, true);
        error += niDCPower_SetAttributeViInt32(session, channelName,    NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_VAL_LCR);
        error += niDCPower_SetAttributeViReal64(session, channelName,   NIDCPOWER_ATTR_LCR_FREQUENCY, frequencies[step]);
        error += niDCPower_SetAttributeViReal64(session, "0",           NIDCPOWER_ATTR_LCR_DC_BIAS_VOLTAGE_LEVEL, dc_bias_step[step]);
        error += niDCPower_SetAttributeViReal64(session, "0",           NIDCPOWER_ATTR_LCR_VOLTAGE_AMPLITUDE, osc_level_step[step]);
    }

    error += niDCPower_CreateAdvancedSequenceWithChannels(session, channelName, "SMU1", attribute_smu_count, attributesSMU, true);
    for (ViInt32 step = 0; step < numSteps; ++step) {
        error += niDCPower_CreateAdvancedSequenceStep(session, true);
        error += niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_VAL_SMU_PS);
        error += niDCPower_ConfigureOutputFunction(session, channelName, NIDCPOWER_VAL_DC_VOLTAGE);
        error += niDCPower_ConfigureVoltageLevel(session, channelName, voltages[step]);
    }

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

int main() {
    LCR_AdvancedSequenceWithTriggerv3();
	cout << "thanks" << endl;
	return 0;
}