import ROOT
from array import array
import argparse

def main(args):
    fft_data_file = ROOT.TFile(args.input_file)

    fft_data = fft_data_file.Get("adc_fft") 
    fft_data.GetEntry(args.entry)
    header_tree = fft_data_file.Get("nevis_header") 
    header_tree.GetEntry(args.entry)

    branchname = "channel_%d_adc_fft_real" % args.channel
    data = getattr(fft_data, branchname)

    graph_title = "Event %i Channel %i FFT" % (header_tree.event_number, args.channel)
    plot(data, args.output, graph_title)

def plot(fft_data, output_name, graph_title):
    n_data = len(fft_data)

    fft_data_array = array('d')
    freq_array = array('d')
    for i,d in enumerate(fft_data):
        fft_data_array.append(d)
        freq_array.append(float(i))
    
    canvas = ROOT.TCanvas("canvas", "Waveform Canvas", 250,100,700,500)

    graph = ROOT.TGraph(n_data, freq_array, fft_data_array)
    graph.SetTitle(graph_title)
    graph.GetXaxis().SetTitle("fft number")
    graph.GetYaxis().SetTitle("fft value")
    graph.Draw()

    canvas.Update()
    canvas.SaveAs(output_name + ".pdf")
    

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--input_file", default="output.root")
    parser.add_argument("-o", "--output", default="fft")
    parser.add_argument("-c", "--channel", type=int, default=0)
    parser.add_argument("-e", "--entry", type=int, default=0)
    
    main(parser.parse_args())
