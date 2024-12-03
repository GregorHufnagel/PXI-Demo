import argparse
import nidigital
import os
import sys
import time
import numpy as np

with nidigital.Session(resource_name='6570') as session:

        # define channels variables specified in the pinmap
        UARTch='UART_RX_06,UART_TX_07'
        Vext='Vext'
        All='UART_RX_06,UART_TX_07,Vext'
        
        # Load the pin map (.pinmap) created using the Digital Pattern Editor
        pin_map_filename ='C:\\Users\\8880_AN\\Documents\\Customers\\STM\\657x_UART\\DigitalPattern\\PinMap.pinmap'
        session.load_pin_map(pin_map_filename)
      
      # Load the specifications (.specs), levels (.digilevels), and timing (.digitiming) sheets created using the Digital Pattern Editor
        spec_filename ='C:\\Users\\8880_AN\\Documents\\Customers\\STM\\657x_UART\\DigitalPattern\\Specifications\\Specifications.specs'
        levels_filename ='C:\\Users\\8880_AN\\Documents\\Customers\\STM\\657x_UART\\DigitalPattern\\Levels\\LevelsDefault.digilevels'
        timing_filename ='C:\\Users\\8880_AN\\Documents\\Customers\\STM\\657x_UART\\DigitalPattern\\Timing\\TimingDefault.digitiming'
        session.load_specifications_levels_and_timing(spec_filename, levels_filename, timing_filename)

        # Apply the settings from the levels and timing sheets we just loaded to the session
        session.apply_levels_and_timing(levels_filename, timing_filename)
     
     # Loading the pattern file (.digipat) created using the Digital Pattern Editor
        pattern_filename ='C:\\Users\\8880_AN\\Documents\\Customers\\STM\\657x_UART\\DigitalPattern\\Pattern\\UART_Main.digipat'
        session.load_pattern(pattern_filename)
        pattern_filename ='C:\\Users\\8880_AN\\Documents\\Customers\\STM\\657x_UART\\DigitalPattern\\Pattern\\UART_SubCall.digipat'
        session.load_pattern(pattern_filename)

        #set channels function
        session.channels[UARTch].selected_function=nidigital.SelectedFunction.DIGITAL
        session.channels[Vext].selected_function=nidigital.SelectedFunction.PPMU

        #set ppmu for external voltage
        session.channels[Vext].ppmu_output_function=nidigital.PPMUOutputFunction.VOLTAGE
        session.channels[Vext].ppmu_current_limit_range=0.032
        session.channels[Vext].ppmu_voltage_level=5
        session.channels[Vext].ppmu_source()

        time.sleep(0.2)
 
      # configure source and capture
        Command_reset = [0x1,0x3,0xC,0x0]  # HCI_Reset command

        session.pins['UART_RX_06'].create_source_waveform_serial('uart_source',nidigital.SourceDataMapping.BROADCAST,8,nidigital.BitOrder.LSB)
        session.pins['UART_TX_07'].create_capture_waveform_serial('uart_capture',8,nidigital.BitOrder.LSB)
        session.write_source_waveform_broadcast('uart_source',Command_reset)
        session.write_sequencer_register(nidigital.SequencerRegister.REGISTER0,4) # 4 bytes of the command
        session.write_sequencer_register(nidigital.SequencerRegister.REGISTER1,7) # 7 bytes of the response

      #burst pattern and capture
        session.burst_pattern('Uart_Main',False,False,1)
        Response=session.fetch_capture_waveform('uart_capture',7,1)
        memory_view= Response[0]
        replyL=np.frombuffer(memory_view,dtype=np.uint)
        replyHex=np.vectorize(hex)(replyL)
      
        print("Command Response",replyHex)

      #set Vext to 0V
        session.channels[Vext].ppmu_voltage_level=0
        session.channels[Vext].ppmu_source()
      
      #disconnect and release
        session.channels[All].selected_function=nidigital.SelectedFunction.DISCONNECT
        

     