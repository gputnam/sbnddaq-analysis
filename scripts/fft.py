import ROOT
from array import array
import argparse

def main(args):
    fft_data_file = ROOT.TFile(args.input_file)

    fft_data = fft_data_file.Get("channel_data") 
    fft_data.GetEntry(args.entry)
    header_tree = fft_data_file.Get("nevis_header") 
    header_tree.GetEntry(args.entry)

    real = fft_data.channel_data[args.channel].fft_real
    imag = fft_data.channel_data[args.channel].fft_imag

    graph_title = "Event %i Channel %i FFT" % (header_tree.header_data.event_number, args.channel)
    plot(real, imag, args.output, graph_title, not args.keep_baseline)

def plot(fft_real, fft_imag, output_name, graph_title, ignore_baseline):
    skip = int(ignore_baseline)
    n_data = len(fft_real) - skip

    fft_data_array = array('d')
    freq_array = array('d')
    for i,(re,im) in enumerate(zip(fft_real, fft_imag)):
        if i < skip:
            continue
        d = (re*re + im*im)**(1./2.)
        fft_data_array.append(d)
        freq_array.append(float(i+skip))

    canvas = ROOT.TCanvas("canvas", "Waveform Canvas", 250,100,700,500)

    graph = ROOT.TGraph(n_data, freq_array, fft_data_array)
    graph.SetTitle(graph_title)
    graph.GetXaxis().SetTitle("fft number")
    graph.GetYaxis().SetTitle("fft value")
    graph.Draw()

    if args.wait:
        raw_input("Press Enter to continue...")

    canvas.Update()
    canvas.SaveAs(output_name + ".pdf")
    

if __name__ == "__main__":
    ROOT.gROOT.ProcessLine(".L ../build/libsbnddaq_analysis_data_dict.so")

    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--input_file", default="output.root")
    parser.add_argument("-o", "--output", default="fft")
    parser.add_argument("-c", "--channel", type=int, default=0)
    parser.add_argument("-e", "--entry", type=int, default=0)
    parser.add_argument("-b", "--keep_baseline", action="store_true")
    parser.add_argument("-w", "--wait", action="store_true")
    
    main(parser.parse_args())
