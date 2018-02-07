import ROOT
import sys
import os
from array import array
import argparse

def main(args):
    adc_data_file = ROOT.TFile(args.input_file)

    adc_data = adc_data_file.Get("channel_data") 
    adc_data.GetEntry(args.entry)
    header_tree = adc_data_file.Get("nevis_header") 
    header_tree.GetEntry(args.entry)

    waveform = adc_data.channel_data[args.channel].waveform

    graph_title = "Event %i Channel %i Waveform" % (header_tree.header_data.event_number, args.channel)
    plot(waveform, args.output, graph_title, args)

def plot(adc_data, output_name, graph_title, args):
    n_data = len(adc_data)

    adc_data_array = array('d')
    time_array = array('d')
    for i,d in enumerate(adc_data):
        adc_data_array.append(d)
        time_array.append(float(i))
    
    canvas = ROOT.TCanvas("canvas", "Waveform Canvas", 250,100,700,500)

    graph = ROOT.TGraph(n_data, time_array, adc_data_array)
    graph.SetTitle(graph_title)
    graph.GetXaxis().SetTitle("adc number")
    graph.GetYaxis().SetTitle("adc value")
    graph.Draw()
    canvas.Update()
    if args.wait:
       raw_input("Press Enter to continue...")
    if args.save:
       canvas.SaveAs(output_name + ".pdf")
    

if __name__ == "__main__":
    buildpath = os.environ["SBNDDAQ_ANALYSIS_BUILD_PATH"]
    if not buildpath:
        print "ERROR: SBNDDAQ_ANALYSIS_BUILD_PATH not set"
        sys.exit() 
    ROOT.gROOT.ProcessLine(".L " + buildpath + "/libsbnddaq_analysis_data_dict.so")
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--input_file", default="output.root")
    parser.add_argument("-o", "--output", default="waveform")
    parser.add_argument("-c", "--channel", type=int, default=0)
    parser.add_argument("-e", "--entry", type=int, default=0)
    parser.add_argument("-w", "--wait", action="store_true")
    parser.add_argument("-s", "--save", action="store_true")
    
    main(parser.parse_args())
