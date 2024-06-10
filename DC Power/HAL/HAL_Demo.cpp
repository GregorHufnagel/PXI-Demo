// HAL_Demo.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include "nidcpower.h"
#include <nidcpower.h>
#include "hal.h"
#include "niSMU.cpp"  // Include the implementation for NI SMU
#include "otherSMU.cpp" // Include the implementation for Keysight SMU


// Define the function prototype of the function in the DLL

int HAL_Demo() {
    typedef int (*DLLFunction)(int);
    // Create an instance of the HAL
    HAL* instrument = new NISmu(); // Change this to OtherSmu if needed

    // Initialize the instrument
    if (!instrument->initialize("PXI1Slot3")) {
        std::cerr << "Failed to initialize instrument" << std::endl;
        delete instrument;
        return 1;
    }

    // Measure voltage
    // double voltage = instrument->measureVoltage();
    // std::cout << "Measured voltage: " << voltage << " Volts" << std::endl;

    // Measure LCR
    //instrument->measureCapacitance();

    // Advanced sequence
    // instrument->voltageSequence();

    // Close the instrument
    if (!instrument->close()) {
        std::cerr << "Failed to close instrument" << std::endl;
        delete instrument;
        return 1;
    }

    // Clean up
    delete instrument;

    return 0;
}





int SinglePoint() {
    // Initialize the NI-DCPower session
    ViSession session;
    ViStatus status;

    ViRsrc name = const_cast<ViRsrc>("PXI1Slot3");

    // Open a session to the NI-DCPower device
    status = niDCPower_init(name, VI_FALSE, VI_FALSE, &session); 
    if (status != VI_SUCCESS) {
        std::cerr << "Failed to open session. Error code: " << status << std::endl;
        return 1;
    }
 
    // Configure the voltage measurement
    status = niDCPower_ConfigureVoltageLevel(session, "0", 5.0);
    if (status != VI_SUCCESS) {
        std::cerr << "Failed to configure voltage level. Error code: " << status << std::endl;
        niDCPower_close(session);
        return 1;
    }

    // Measure voltage
    ViReal64 voltage;
    status = niDCPower_Measure(session, "0", 0, &voltage);
    if (status != VI_SUCCESS) {
        std::cerr << "Failed to measure voltage. Error code: " << status << std::endl;
        niDCPower_close(session);
        return 1;
    }

    std::cout << "Measured voltage: " << voltage << " Volts" << std::endl;

    // Close the session
    status = niDCPower_close(session);
    if (status != VI_SUCCESS) {
        std::cerr << "Failed to close session. Error code: " << status << std::endl;
        return 1;
    }

    return 0;
}





int AdvancedSequence() {

    // Initialize variables
    ViSession session;
    ViChar resourceName[] = "PXI1Slot3";
    ViConstString channelName = "0";
    ViStatus status;


    // Initialize NI DC Power session
    status = niDCPower_InitializeWithIndependentChannels(resourceName, VI_FALSE, VI_FALSE, &session);
    if (status < VI_SUCCESS) {
        std::cerr << "Failed to initialize NI DC Power session." << std::endl;
        return status;
    }


    // Configure source mode
    status = niDCPower_ConfigureSourceModeWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE);
    // status = niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_SOURCE_MODE, NIDCPOWER_VAL_SEQUENCE);
    if (status < VI_SUCCESS) {
        std::cerr << "Failed to configure source mode" << std::endl;
        return status;
    }


    // Create a sequence of voltage levels and durations
    const ViReal64 voltageLevels[] = { -10.0, 0.0, 10.0 };
    // Modify as needed
    // const ViReal64 durations[] = { 1.0, 2.0, 1.0 };
    // Modify as needed
    const ViInt32 numSteps = sizeof(voltageLevels) / sizeof(voltageLevels[0]);
    const ViInt32 attributes[] = { NIDCPOWER_ATTR_OUTPUT_FUNCTION, NIDCPOWER_ATTR_VOLTAGE_LEVEL };


    // Configure the sequence
    status = niDCPower_CreateAdvancedSequenceWithChannels(session, channelName, "MySequence", 2, attributes, true);
    if (status < VI_SUCCESS) {
        std::cerr << "Failed to create advanced sequence." << std::endl;
        niDCPower_close(session);
        return status;
    }

    // Add steps to the sequence
    //for (ViInt32 step = 0; step < numSteps; ++step) {
    status = niDCPower_CreateAdvancedSequenceStep(session, true);
    if (status < VI_SUCCESS) {
        std::cerr << "Failed to add step to advanced sequence." << std::endl;
        niDCPower_close(session);
        return status;
    }
    //}



    // Configure the output function to use the sequence
    status = niDCPower_ConfigureOutputFunction(session, channelName, NIDCPOWER_VAL_DC_VOLTAGE);
    // status = niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_OUTPUT_FUNCTION, NIDCPOWER_VAL_DC_VOLTAGE);
    if (status < VI_SUCCESS) {
        std::cerr << "Failed to configure output function." << std::endl;
        niDCPower_close(session);
        return status;
    }



    // Configure voltage level
    status = niDCPower_ConfigureVoltageLevel(session, channelName, 0);
    if (status < VI_SUCCESS) {
        std::cerr << "Failed to configure voltage level." << std::endl;
        niDCPower_close(session);
        return status;
    }




    // Enable the output
    status = niDCPower_InitiateWithChannels(session, channelName);
    if (status < VI_SUCCESS) {
        std::cerr << "Failed to initiate output." << std::endl;
        niDCPower_close(session);
        return status;
    }

    // Wait for the sequence to complete
    status = niDCPower_WaitForEventWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE_ENGINE_DONE_EVENT, 100);
    if (status < VI_SUCCESS) {
        std::cerr << "Failed to check if sequence is done." << std::endl;
        niDCPower_close(session);
        return status;
    }

    // Disable the output
    status = niDCPower_Abort(session);
    if (status < VI_SUCCESS) {
        std::cerr << "Failed to abort output." << std::endl;
        niDCPower_close(session);
        return status;
    }

    // Close the session
    status = niDCPower_close(session);
    if (status < VI_SUCCESS) {
        std::cerr << "Failed to close NI DC Power session." << std::endl;
        return status;
    }
    return 0;
}





int SMU_AdvancedSequence() {

    // Initialize variables
    ViSession session;
    ViChar resourceName[] = "PXI1Slot4";
    ViConstString channelName = "0";
    ViStatus status;
    const ViReal64 voltageLevels[] = { -1.0, 0.1, 1.0 };
    const ViInt32 numSteps = sizeof(voltageLevels) / sizeof(voltageLevels[0]);
    const ViInt32 attributes[] = { NIDCPOWER_ATTR_OUTPUT_FUNCTION, NIDCPOWER_ATTR_VOLTAGE_LEVEL };
    ViReal64 currents[3];
    ViReal64 voltages[3];
    ViBoolean compliances[3];
    ViInt32 actualcount[3];

    // Configure and run advanced sequence
    niDCPower_InitializeWithIndependentChannels(resourceName, VI_FALSE, VI_FALSE, &session);
    niDCPower_ConfigureSourceModeWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE);
    niDCPower_CreateAdvancedSequenceWithChannels(session, channelName, "MySequence", 2, attributes, true);
    for (ViInt32 step = 0; step < numSteps; ++step) {
        niDCPower_CreateAdvancedSequenceStep(session, true);
        //niDCPower_ConfigureApertureTime(session, channelName, 0.0001, NIDCPOWER_VAL_SECONDS);
        niDCPower_ConfigureOutputFunction(session, channelName, NIDCPOWER_VAL_DC_VOLTAGE);
        niDCPower_ConfigureVoltageLevel(session, channelName, voltageLevels[step]);
    }
    niDCPower_InitiateWithChannels(session, channelName);
    niDCPower_WaitForEventWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE_ENGINE_DONE_EVENT, 100);
    // niDCPower_Measure(session, channelName, NIDCPOWER_VAL_MEASURE_CURRENT, &current);
    niDCPower_FetchMultiple(session, channelName, 100, 3, voltages, currents, compliances, actualcount);
    std::cerr << voltages[0] << "   " << voltages[1] << "   " << voltages[2] << std::endl;
    std::cerr << currents[0] << "   " << currents[1] << "   " << currents[2]  << std::endl;
    niDCPower_Abort(session);
    niDCPower_close(session);
    return 0;
}





int LCR_AdvancedSequence() {

    // Initialize variables
    ViSession session;
    ViChar resourceName[] = "PXI1Slot4";
    ViConstString channelName = "0";
    ViStatus status;
    const ViReal64 frequencies[] = { 100000, 300000, 500000 };
    ViInt32 numSteps = sizeof(frequencies) / sizeof(frequencies[0]);
    const ViInt32 attributes[] = { NIDCPOWER_ATTR_LCR_FREQUENCY };
    NILCRMeasurement measurements;

    // Configure and run advanced sequence
    niDCPower_InitializeWithIndependentChannels(resourceName, VI_FALSE, VI_FALSE, &session);
    niDCPower_ConfigureSourceModeWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE);
    niDCPower_ConfigurePowerLineFrequency(session, NIDCPOWER_VAL_50_HERTZ);
    niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_VAL_LCR);
    niDCPower_CreateAdvancedSequenceWithChannels(session, channelName, "MySequence", 2, attributes, true);
    for (ViInt32 step = 0; step < numSteps; ++step) {
        niDCPower_CreateAdvancedSequenceStep(session, true);
        niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_LCR_FREQUENCY, frequencies[step]);
    }
    niDCPower_InitiateWithChannels(session, channelName);
    niDCPower_WaitForEventWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE_ENGINE_DONE_EVENT, 100);
    niDCPower_FetchMultipleLCR(session, channelName, 1000, numSteps, &measurements, &numSteps);
    std::cerr << measurements.Cp << std::endl;
    niDCPower_Abort(session);
    niDCPower_close(session);
    return 0;
}





int Triggered() {
    // Initialize variables
    ViSession session;
    ViChar resourceName[] = "PXI1Slot3";
    ViConstString channelName = "0";
    ViStatus status;
    const ViReal64 voltageLevels[] = { -10.0, 0.0, 10.0 };
    const ViInt32 numSteps = sizeof(voltageLevels) / sizeof(voltageLevels[0]);
    const ViInt32 attributes[] = { NIDCPOWER_ATTR_OUTPUT_FUNCTION, NIDCPOWER_ATTR_VOLTAGE_LEVEL };
    ViReal64 currents[3];
    ViReal64 voltages[3];
    ViBoolean compliances[3];
    ViInt32 actualcount[3];

    // Configure and run advanced sequence
    niDCPower_InitializeWithIndependentChannels(resourceName, VI_FALSE, VI_FALSE, &session);
    niDCPower_ConfigureSourceModeWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE);

    // Triggers
    //niDCPower_ConfigureDigitalEdgeStartTrigger(session, "PXI1Slot6/PXI_Trig2", NIDCPOWER_VAL_RISING);
    niDCPower_ConfigureDigitalEdgeSequenceAdvanceTrigger(session, "PXI1Slot3/PXI_Trig2", NIDCPOWER_VAL_RISING);
    //niDCPower_SetAttributeViInt32(session,channelName, NIDCPOWER_ATTR_SEQUENCE_ADVANCE_TRIGGER_TYPE, NIDCPOWER_VAL_SOFTWARE_EDGE);

    niDCPower_CreateAdvancedSequenceWithChannels(session, channelName, "MySequence", 2, attributes, true);
    for (ViInt32 step = 0; step < numSteps; ++step) {
        niDCPower_CreateAdvancedSequenceStep(session, true);
        //niDCPower_ConfigureApertureTime(session, channelName, 0.0001, NIDCPOWER_VAL_SECONDS);
        niDCPower_ConfigureOutputFunction(session, channelName, NIDCPOWER_VAL_DC_VOLTAGE);
        niDCPower_ConfigureVoltageLevel(session, channelName, voltageLevels[step]);
    }

    niDCPower_InitiateWithChannels(session, channelName);
    niDCPower_WaitForEventWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE_ENGINE_DONE_EVENT, 100);
    // niDCPower_Measure(session, channelName, NIDCPOWER_VAL_MEASURE_CURRENT, &current);
    niDCPower_FetchMultiple(session, channelName, 100, 3, voltages, currents, compliances, actualcount);
    std::cerr << currents[0] << "   " << currents[1] << "   " << currents[2] << std::endl;
    niDCPower_Abort(session);
    niDCPower_close(session);
    return 0;
}





int LCR_AdvancedSequenceWithTriggerBackup() {
    // Initialize variables
    ViSession session;
    ViChar resourceName[] = "PXI1Slot4";
    ViConstString channelName = "0";
    ViStatus error = -50;
    const ViReal64 frequencies[] = { 100000, 150000, 200000 };
    ViInt32 numSteps = sizeof(frequencies) / sizeof(frequencies[0]);
    const ViInt32 attributes[] = { NIDCPOWER_ATTR_LCR_FREQUENCY };
    NILCRMeasurement measurements[3];

    // Configure and run advanced sequence with trigger
    error = niDCPower_InitializeWithIndependentChannels(resourceName, VI_FALSE, VI_FALSE, &session);
	std::cerr << error << std::endl;
    error = niDCPower_ConfigureSourceModeWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE);
    error = niDCPower_ConfigurePowerLineFrequency(session, NIDCPOWER_VAL_50_HERTZ);
    error = niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_VAL_LCR);
    // niDCPower_ConfigureDigitalEdgeSequenceAdvanceTrigger(session, "PXI1Slot4/PXI_Trig2", NIDCPOWER_VAL_RISING);
    error = niDCPower_CreateAdvancedSequenceWithChannels(session, channelName, "MySequence", 1, attributes, true);
    for (ViInt32 step = 0; step < numSteps; ++step) {
        error = niDCPower_CreateAdvancedSequenceStep(session, true);
        error = niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_LCR_FREQUENCY, frequencies[step]);
    }
    error = niDCPower_InitiateWithChannels(session, channelName);
	std::cerr << "Waiting for trigger" << std::endl;
    error = niDCPower_WaitForEventWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE_ENGINE_DONE_EVENT, 100);
    std::cerr << "Triggered" << std::endl;
    error = niDCPower_FetchMultipleLCR(session, channelName, 1000, numSteps, measurements, &numSteps);
	for (ViInt32 i = 0; i < numSteps; ++i) {
        std::cerr << "Measured Cp for step " << i << ": " << measurements[i].Cp << std::endl;
    }
    error = niDCPower_Abort(session);
    error = niDCPower_close(session);
    return 0;
}






int LCR_AdvancedSequenceWithTrigger() {
    // Initialize variables
    ViSession session;
    ViChar resourceName[] = "PXI1Slot4";
    ViConstString channelName = "0";
    volatile ViStatus error = -50;
    const ViReal64 frequencies[] = { 100000, 150000, 200000 };
    const ViReal64 voltages[] = { -1.0, 0.1, 2.0 };
    ViInt32 numSteps = sizeof(frequencies) / sizeof(frequencies[0]);
    const ViInt32 attributesLCR[] = { NIDCPOWER_ATTR_LCR_FREQUENCY };
    const ViInt32 attributesSMU[] = { NIDCPOWER_ATTR_OUTPUT_FUNCTION, NIDCPOWER_VAL_DC_VOLTAGE };
    NILCRMeasurement measurements[3];
    ViReal64 voltageMeasurements[3];
    ViReal64 currentMeasurements[3];
    ViBoolean inCompliance[3];
    ViInt32 actualCount;

    // Configure and run advanced sequences with trigger
    error = niDCPower_InitializeWithIndependentChannels(resourceName, VI_FALSE, VI_FALSE, &session);
    std::cerr << error << std::endl;
    error = niDCPower_ConfigureSourceModeWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE);
    error = niDCPower_ConfigurePowerLineFrequency(session, NIDCPOWER_VAL_50_HERTZ);
    //error = niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_VAL_LCR);
    // niDCPower_ConfigureDigitalEdgeSequenceAdvanceTrigger(session, "PXI1Slot4/PXI_Trig2", NIDCPOWER_VAL_RISING);
    error = niDCPower_CreateAdvancedSequenceWithChannels(session, channelName, "LCR1", 1, attributesLCR, true);
    for (ViInt32 step = 0; step < numSteps; ++step) {
        /*
        err = niDCPower_CreateAdvancedSequenceStep(session, true);
        double freq_step = test_points[step][frequency] * 1e3;
        niDCPower_SetAttributeViInt32(session, "0", NIDCPOWER_ATTR_LCR_FREQUENCY, freq_step);
        double dc_bias_step = test_points[step][dc_bias];
        niDCPower_SetAttributeViInt32(session, "0", NIDCPOWER_ATTR_LCR_DC_BIAS_VOLTAGE_LEVEL, dc_bias_step);
        double osc_level_step = test_points[step][osc_level];
        niDCPower_SetAttributeViInt32(session, "0", NIDCPOWER_ATTR_LCR_VOLTAGE_AMPLITUDE, osc_level_step);
        */
        error = niDCPower_CreateAdvancedSequenceStep(session, true);
        error = niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_LCR_FREQUENCY, frequencies[step]);
    }
    error = niDCPower_CreateAdvancedSequenceWithChannels(session, channelName, "LCR2", 1, attributesLCR, true);
    for (ViInt32 step = 0; step < numSteps; ++step) {
        error = niDCPower_CreateAdvancedSequenceStep(session, true);
        error = niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_LCR_FREQUENCY, frequencies[step]);
    }
    error = niDCPower_CreateAdvancedSequenceWithChannels(session, channelName, "SMU1", 2, attributesSMU, true);
    for (ViInt32 step = 0; step < numSteps; ++step) {
        /*
        error = niDCPower_CreateAdvancedSequenceStep(session, true);
        error = niDCPower_ConfigureOutputFunction(session, "0", NIDCPOWER_VAL_DC_VOLTAGE);
        double aperture_time_step = test_points[step][aperture] * 1e-3;
        error = niDCPower_ConfigureApertureTime(session, "0", aperture_time_step, NIDCPOWER_VAL_SECONDS);
        double voltage_step = test_points[step][voltage];
        error += niDCPower_ConfigureVoltageLevel(session, "0", voltage_step);
        error += niDCPower_ConfigureCurrentLimit(session, channel, NIDCPOWER_VAL_CURRENT_REGULATE, test_points[step][current_limit]);
        error += niDCPower_ConfigureCurrentLimitRange(session, channel, test_points[step][current_range]);
        */


        error = niDCPower_CreateAdvancedSequenceStep(session, true);
        error = niDCPower_ConfigureOutputFunction(session, channelName, NIDCPOWER_VAL_DC_VOLTAGE);
		//error = niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_OUTPUT_FUNCTION, NIDCPOWER_VAL_DC_VOLTAGE);
        //error = niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_VOLTAGE_LEVEL, voltages[step]);
        error = niDCPower_ConfigureVoltageLevel(session, channelName, voltages[step]);

    }

    // SMU 1
    //error = niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_VAL_SMU_PS);
    error = niDCPower_SetAttributeViString(session, NULL, NIDCPOWER_ATTR_ACTIVE_ADVANCED_SEQUENCE, "SMU1");

    error = niDCPower_InitiateWithChannels(session, channelName);
    std::cerr << "Waiting for trigger" << std::endl;
    error = niDCPower_WaitForEventWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE_ENGINE_DONE_EVENT, 100);
    std::cerr << "Triggered" << std::endl;
    error = niDCPower_FetchMultiple(session, channelName, 1000, numSteps, voltageMeasurements, currentMeasurements, inCompliance, &actualCount);
    for (ViInt32 i = 0; i < numSteps; ++i) {
        std::cerr << "Measured voltage for step " << i << ": " << voltageMeasurements[i] << std::endl;
        std::cerr << "Measured current for step " << i << ": " << currentMeasurements[i] << std::endl;
        std::cerr << "Measured compliance for step " << i << ": " << inCompliance[i] << std::endl << std::endl;
    }
    error = niDCPower_Abort(session);

    // LCR 1
    error = niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_VAL_LCR);
    error = niDCPower_SetAttributeViString(session, NULL, NIDCPOWER_ATTR_ACTIVE_ADVANCED_SEQUENCE, "LCR1");
    error = niDCPower_InitiateWithChannels(session, channelName);
    std::cerr << "Waiting for trigger" << std::endl;
    error = niDCPower_WaitForEventWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE_ENGINE_DONE_EVENT, 100);
    std::cerr << "Triggered" << std::endl;
    error = niDCPower_FetchMultipleLCR(session, channelName, 1000, numSteps, measurements, &actualCount);
    for (ViInt32 i = 0; i < numSteps; ++i) {
        std::cerr << "Measured Cp for step " << i << ": " << measurements[i].Cp << std::endl;
    }
    error = niDCPower_Abort(session);

	// LCR 2
    error = niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_VAL_LCR);
    error = niDCPower_SetAttributeViString(session, NULL, NIDCPOWER_ATTR_ACTIVE_ADVANCED_SEQUENCE, "LCR2");
    error = niDCPower_InitiateWithChannels(session, channelName);
    std::cerr << "Waiting for trigger" << std::endl;
    error = niDCPower_WaitForEventWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE_ENGINE_DONE_EVENT, 100);
    std::cerr << "Triggered" << std::endl;
    error = niDCPower_FetchMultipleLCR(session, channelName, 1000, numSteps, measurements, &actualCount);
    for (ViInt32 i = 0; i < numSteps; ++i) {
        std::cerr << "Measured Cp for step " << i << ": " << measurements[i].Cp << std::endl;
    }
    error = niDCPower_Abort(session);




    error = niDCPower_close(session);
    return 0;
}





























int LCR_AdvancedSequenceWithTriggerv3() {
    // Initialize variables
    ViSession session;
    ViChar resourceName[] = "PXI1Slot4";
    ViConstString channelName = "0";
    volatile ViStatus error = 0;
    const ViReal64 frequencies[] = { 100000, 150000, 200000 };
    const ViReal64 voltages[] = { 0.1, 1.0, 1.5 };
    ViInt32 numSteps = sizeof(frequencies) / sizeof(frequencies[0]);
    const ViInt32 attributesLCR[] = { NIDCPOWER_ATTR_INSTRUMENT_MODE};
    const ViInt32 attributesSMU[] = { NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_ATTR_OUTPUT_FUNCTION, NIDCPOWER_ATTR_VOLTAGE_LEVEL };
    NILCRMeasurement measurements[3];
    ViReal64 voltageMeasurements[3];
    ViReal64 currentMeasurements[3];
    ViBoolean inCompliance[3];
    ViInt32 actualCount;

    // Configure and run advanced sequences
    error += niDCPower_InitializeWithIndependentChannels(resourceName, VI_FALSE, VI_FALSE, &session);
    std::cerr << error << std::endl;
    error += niDCPower_ConfigureSourceModeWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE);
    error += niDCPower_ConfigurePowerLineFrequency(session, NIDCPOWER_VAL_50_HERTZ);


    //const ViInt32 attributes[] = { NIDCPOWER_ATTR_OUTPUT_FUNCTION, NIDCPOWER_ATTR_VOLTAGE_LEVEL };
    //error += niDCPower_CreateAdvancedSequenceWithChannels(session, channelName, "MySequence", 2, attributes, true);
    

    error += niDCPower_CreateAdvancedSequenceWithChannels(session, channelName, "LCR1", 1, attributesLCR, true);
    for (ViInt32 step = 0; step < numSteps; ++step) {
        error += niDCPower_CreateAdvancedSequenceStep(session, true);
        error += niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_VAL_LCR);
        //error += niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_LCR_FREQUENCY, frequencies[step]);
    }

    error += niDCPower_CreateAdvancedSequenceWithChannels(session, channelName, "LCR2", 1, attributesLCR, true);
    for (ViInt32 step = 0; step < numSteps; ++step) {
        error += niDCPower_CreateAdvancedSequenceStep(session, true);
        error += niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_VAL_LCR);
        //error += niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_LCR_FREQUENCY, frequencies[step]);
    }

    error += niDCPower_CreateAdvancedSequenceWithChannels(session, channelName, "SMU1", 3, attributesSMU, true);
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

    // LCR 2
    error += niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_VAL_LCR);
    error += niDCPower_SetAttributeViString(session, NULL, NIDCPOWER_ATTR_ACTIVE_ADVANCED_SEQUENCE, "LCR2");
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
	std::cerr << "Error code: " << error << std::endl;
    return 0;
}



































    int createSequences(ViSession session, ViConstString channelName, ViInt32 numSteps, ViInt32 attributesLCR[], ViInt32 attributesSMU[], ViReal64 voltages[]) {

        volatile ViStatus error = 0;

        error += niDCPower_CreateAdvancedSequenceWithChannels(session, channelName, "LCR1", 1, attributesLCR, true);
        for (ViInt32 step = 0; step < numSteps; ++step) {
            error += niDCPower_CreateAdvancedSequenceStep(session, true);
            error += niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_VAL_LCR);
            //error += niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_LCR_FREQUENCY, frequencies[step]);
        }

        error += niDCPower_CreateAdvancedSequenceWithChannels(session, channelName, "LCR2", 1, attributesLCR, true);
        for (ViInt32 step = 0; step < numSteps; ++step) {
            error += niDCPower_CreateAdvancedSequenceStep(session, true);
            error += niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_VAL_LCR);
            //error += niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_LCR_FREQUENCY, frequencies[step]);
        }

        error += niDCPower_CreateAdvancedSequenceWithChannels(session, channelName, "SMU1", 3, attributesSMU, true);
        for (ViInt32 step = 0; step < numSteps; ++step) {
            error += niDCPower_CreateAdvancedSequenceStep(session, true);
            error += niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_VAL_SMU_PS);
            error += niDCPower_ConfigureOutputFunction(session, channelName, NIDCPOWER_VAL_DC_VOLTAGE);
            error += niDCPower_ConfigureVoltageLevel(session, channelName, voltages[step]);

        }

        return 0;
    }

    int triggerSMU(ViSession session, ViConstString channelName, ViInt32 numSteps, ViReal64 voltageMeasurements[3], ViReal64 currentMeasurements[3], ViBoolean inCompliance[3], ViInt32 actualCount) {
        
        volatile ViStatus error = 0;

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
        return 0;
        }
    
    int triggerLCR(ViSession session, ViConstString channelName, ViInt32 numSteps, NILCRMeasurement measurements[3], ViInt32 actualCount, ViChar SequenceName[]) {

        volatile ViStatus error = 0;

        error += niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_VAL_LCR);
        error += niDCPower_SetAttributeViString(session, NULL, NIDCPOWER_ATTR_ACTIVE_ADVANCED_SEQUENCE, SequenceName);
        error += niDCPower_InitiateWithChannels(session, channelName);
        std::cerr << "Waiting for trigger" << std::endl;
        error += niDCPower_WaitForEventWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE_ENGINE_DONE_EVENT, 100);
        std::cerr << "Triggered" << std::endl;
        error += niDCPower_FetchMultipleLCR(session, channelName, 1000, numSteps, measurements, &actualCount);
        for (ViInt32 i = 0; i < numSteps; ++i) {
            std::cerr << "Measured Cp for step " << i << ": " << measurements[i].Cp << std::endl;
        }
        error += niDCPower_Abort(session);
        return 0;
    }

    int runMain() {
        // Initialize variables
        ViSession session;
        ViChar resourceName[] = "PXI1Slot4";
        ViConstString channelName = "0";
        volatile ViStatus error = 0;
        const ViReal64 frequencies[] = { 100000, 150000, 200000 };
        ViReal64 voltages[] = { 0.1, 1.0, 1.5 };
        ViInt32 numSteps = sizeof(frequencies) / sizeof(frequencies[0]);
        ViInt32 attributesLCR[] = { NIDCPOWER_ATTR_INSTRUMENT_MODE };
        ViInt32 attributesSMU[] = { NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_ATTR_OUTPUT_FUNCTION, NIDCPOWER_ATTR_VOLTAGE_LEVEL };
        NILCRMeasurement measurements[3];
        ViReal64 voltageMeasurements[3];
        ViReal64 currentMeasurements[3];
        ViBoolean inCompliance[3];
        ViInt32 actualCount = 0;

        // Configure and run advanced sequences
        error += niDCPower_InitializeWithIndependentChannels(resourceName, VI_FALSE, VI_FALSE, &session);
        std::cerr << error << std::endl;
        error += niDCPower_ConfigureSourceModeWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE);
        error += niDCPower_ConfigurePowerLineFrequency(session, NIDCPOWER_VAL_50_HERTZ);


		createSequences(session, channelName, numSteps, attributesLCR, attributesSMU, voltages);

		triggerSMU(session, channelName, numSteps, voltageMeasurements, currentMeasurements, inCompliance, actualCount);



        error += niDCPower_close(session);
        std::cerr << "Error code: " << error << std::endl;
        return 0;
    }



// pseudo code 
/*
configureSequence*()


TriggerSequence* (seqName) {
    -activate seq
        - run seq
}

main() {
    configureSequence();
    TriggerSequence();
    TriggerSequence();
    TriggerSequence();
    TriggerSequence();
}
*/






































// ToDo:
// 2 different LCR sequences with 3 parameters - Frequency, AC level and DC bias
// 2 different SMU sequences each with 3 parameters - Voltage
 





/*
#include <iostream>
#include <Windows.h>
#include "nidcpower.h"
#include <nidcpower.h>
#include "hal.h"
#include "niSMU.cpp"  // Include the implementation for NI SMU
#include "otherSMU.cpp" // Include the implementation for Keysight SMU

int LCR_AdvancedSequenceWithTriggerv2() {
    // Initialize variables
    ViSession session;
    ViChar resourceName[] = "PXI1Slot4";
    ViConstString channelName = "0";
    ViStatus status;
    const ViReal64 frequencies[] = { 100000, 150000, 200000 };
    ViInt32 numSteps = sizeof(frequencies) / sizeof(frequencies[0]);
    const ViInt32 attributes[] = { NIDCPOWER_ATTR_LCR_FREQUENCY };
    NILCRMeasurement measurements[3];

    // Initialize the NI-DCPower session
    status = niDCPower_init(resourceName, VI_FALSE, VI_FALSE, &session);
    if (status != VI_SUCCESS) {
        std::cerr << "Failed to open session. Error code: " << status << std::endl;
        return 1;
    }

    // Set active channel
    status = niDCPower_SetAttributeViString(session, NULL, NIDCPOWER_ATTR_ACTIVE_CHANNEL, channelName);
    if (status != VI_SUCCESS) {
        std::cerr << "Failed to set active channel. Error code: " << status << std::endl;
        niDCPower_close(session);
        return 1;
    }

    // Set instrument mode to LCR
    status = niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_INSTRUMENT_MODE, NIDCPOWER_VAL_LCR);
    if (status != VI_SUCCESS) {
        std::cerr << "Failed to set instrument mode. Error code: " << status << std::endl;
        niDCPower_close(session);
        return 1;
    }

    // Set LCR Stimulus function to AC Voltage
    status = niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_LCR_STIMULUS_FUNCTION, NIDCPOWER_VAL_AC_VOLTAGE);
    if (status != VI_SUCCESS) {
        std::cerr << "Failed to set LCR stimulus function. Error code: " << status << std::endl;
        niDCPower_close(session);
        return 1;
    }

    
    // Set values for Cable Length, LCR Voltage Amplitude, LCR Measurement Time
    status = niDCPower_SetAttributeViReal64(session, channelName, NIDCPOWER_ATTR_LCR_CABLE_LENGTH, 1.0);
    if (status != VI_SUCCESS) {
        std::cerr << "Failed to set cable length. Error code: " << status << std::endl;
        niDCPower_close(session);
        return 1;
    }
    

    status = niDCPower_SetAttributeViReal64(session, channelName, NIDCPOWER_ATTR_LCR_VOLTAGE_AMPLITUDE, 1.0);
    if (status != VI_SUCCESS) {
        std::cerr << "Failed to set LCR voltage amplitude. Error code: " << status << std::endl;
        niDCPower_close(session);
        return 1;
    }

    status = niDCPower_SetAttributeViReal64(session, channelName, NIDCPOWER_ATTR_LCR_MEASUREMENT_TIME, 1.0);
    if (status != VI_SUCCESS) {
        std::cerr << "Failed to set LCR measurement time. Error code: " << status << std::endl;
        niDCPower_close(session);
        return 1;
    }

    // Set LCR DC Bias Source, LCR DC Bias Voltage Level, LCR Custom Measurement Time, LCR Impedance Range
    status = niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_LCR_DC_BIAS_SOURCE, NIDCPOWER_VAL_INTERNAL);
    if (status != VI_SUCCESS) {
        std::cerr << "Failed to set LCR DC bias source. Error code: " << status << std::endl;
        niDCPower_close(session);
        return 1;
    }

    status = niDCPower_SetAttributeViReal64(session, channelName, NIDCPOWER_ATTR_LCR_DC_BIAS_VOLTAGE_LEVEL, 0.0);
    if (status != VI_SUCCESS) {
        std::cerr << "Failed to set LCR DC bias voltage level. Error code: " << status << std::endl;
        niDCPower_close(session);
        return 1;
    }

    status = niDCPower_SetAttributeViReal64(session, channelName, NIDCPOWER_ATTR_LCR_CUSTOM_MEASUREMENT_TIME, 0.0);
    if (status != VI_SUCCESS) {
        std::cerr << "Failed to set LCR custom measurement time. Error code: " << status << std::endl;
        niDCPower_close(session);
        return 1;
    }

    
    status = niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_LCR_IMPEDANCE_RANGE, NIDCPOWER_VAL_AUTO_RANGE);
    if (status != VI_SUCCESS) {
        std::cerr << "Failed to set LCR impedance range. Error code: " << status << std::endl;
        niDCPower_close(session);
        return 1;
    }
    

    // Create advanced sequence with channels 1 attribute to be changed - the frequency
    status = niDCPower_CreateAdvancedSequenceWithChannels(session, channelName, "MySequence", 1, attributes, true);
    if (status != VI_SUCCESS) {
        std::cerr << "Failed to create advanced sequence. Error code: " << status << std::endl;
        niDCPower_close(session);
        return 1;
    }

    // Create 3 sequence steps in a loop giving LCR frequency as a parameter to be changed
    for (ViInt32 step = 0; step < numSteps; ++step) {
        status = niDCPower_CreateAdvancedSequenceStep(session, true);
        if (status != VI_SUCCESS) {
            std::cerr << "Failed to create sequence step. Error code: " << status << std::endl;
            niDCPower_close(session);
            return 1;
        }
        status = niDCPower_SetAttributeViInt32(session, channelName, NIDCPOWER_ATTR_LCR_FREQUENCY, frequencies[step]);
        if (status != VI_SUCCESS) {
            std::cerr << "Failed to set LCR frequency. Error code: " << status << std::endl;
            niDCPower_close(session);
            return 1;
        }
    }

    // Initiate the sequence
    status = niDCPower_InitiateWithChannels(session, channelName);
    if (status != VI_SUCCESS) {
        std::cerr << "Failed to initiate sequence. Error code: " << status << std::endl;
        niDCPower_close(session);
        return 1;
    }

    // Wait for the Sequence Engine done event
    status = niDCPower_WaitForEventWithChannels(session, channelName, NIDCPOWER_VAL_SEQUENCE_ENGINE_DONE_EVENT, 100);
    if (status != VI_SUCCESS) {
        std::cerr << "Failed to wait for sequence engine done event. Error code: " << status << std::endl;
        niDCPower_close(session);
        return 1;
    }

    // Use Fetch Multiple LCR to get all the values from all 3 steps
    status = niDCPower_FetchMultipleLCR(session, channelName, 1000, numSteps, measurements, &numSteps);
    if (status != VI_SUCCESS) {
        std::cerr << "Failed to fetch multiple LCR measurements. Error code: " << status << std::endl;
        niDCPower_close(session);
        return 1;
    }

    // Print fetched values for all 3 steps
    for (ViInt32 i = 0; i < numSteps; ++i) {
        std::cout << "Measured Cp for step " << i << ": " << measurements[i].Cp << std::endl;
    }

    // Close the session
    status = niDCPower_close(session);
    if (status != VI_SUCCESS) {
        std::cerr << "Failed to close session. Error code: " << status << std::endl;
        return 1;
    }

    return 0;
}
*/



int main() {
    //HAL_Demo();
    //SinglePoint();
    //AdvancedSequence();
    //SMU_AdvancedSequence();
    //LCR_AdvancedSequence();
    //Triggered();
    //LCR_AdvancedSequenceWithTriggerBackup();
    ////LCR_AdvancedSequenceWithTrigger();
    //LCR_AdvancedSequenceWithTriggerv2();
    LCR_AdvancedSequenceWithTriggerv3();
    //runMain();
}
