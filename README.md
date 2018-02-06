# Basic DAQ Analysis Code #

**To Use**

First, you need to change `SBNDDAQ_DATATYPES_LOCATION` in `setup.sh` to
the location of your local repository containing `sbnddaq-datatypes` and
`artdaq`. And you need to add a `build` sub-directory to this directory
which will store output from `make` (other than the executable).

Then, running `setup.sh` will setup the required libraries, and then calling
`make all` will build the executable and the associated ROOT
dictionaries. 

To run the program, call `./analysis config_file [input_files]`, where
`config_file` is a `.json` configuration file (`default_config.json`
which just sets the defaults, is a very minimal example), and
`[input_files]` is a list of space separated root files from the output
of one of the drivers from `sbnddaq-readout` (e.g. `test_driver`). 

See `Main.cc` for what can be configured in the `.json` configration
file.

**Plots**

Plotting scripts are in the `scripts` directory. As an example, 
you can make a waveform plot with the output of `analysis` by running
the script `scripts/waveform.py`. See the source for arguments setting
input/output file names and channel/entry to be plotted.

