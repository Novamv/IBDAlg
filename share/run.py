
#!/usr/bin/env python

import sys
import os.path
import Sniper
import argparse 
import SNiPERToPlainTree
import Geometry

def get_parser():
    parser = argparse.ArgumentParser(description="SNiPERToPlainTree module")
    parser.add_argument("--input", nargs="+", default=["sample_elecsim.root", "sample_calib.root"], help= "input list of file separated by space. Example sample_elecsim.root sample_calib.root")
    parser.add_argument("--input-list", default=None, help= "input file name")
    parser.add_argument("--input-correlations", nargs="+", help="Name of the correlation files")
    parser.add_argument("--input-correlations-list", default=None, help="Name of the correlation files")
    parser.add_argument("--output", default="sample_plain.root", help="output file name suffix")
    parser.add_argument("--loglevel", default="Info", choices=["Test", "Debug", "Info", "Warn", "Error", "Fatal"], help="Set Log level")
    parser.add_argument("--time-window", default="0.01", type = float, help="Time window of events")
    parser.add_argument("--IBD", dest="enableIBDSelection", action="store_true")
    parser.add_argument("--Calib", dest="saveCalib", action="store_true")
    parser.add_argument("--BiPo", dest="saveBiPo", action="store_true")

    return parser

if __name__ == "__main__":

    parser = get_parser()
    args = parser.parse_args()

    DATA_LOG_MAP = {
        "Test":0, "Debug":2, "Info":3, "Warn":4, "Error":5, "Fatal":6
    }

    import Sniper
    task = Sniper.TopTask("task")
    #task.asTop()
    task.setLogLevel(DATA_LOG_MAP[args.loglevel])

    import BufferMemMgr
    bufMgr = task.createSvc("BufferMemMgr")
    bufMgr.property("TimeWindow").set([0.0-args.time_window, args.time_window]) # in seconds

    # alg = task.createAlg("SNiPERToPlainTree/alg_example") 

    import SpmtElecConfigSvc 
    spmtelecconfigsvc = task.createSvc("SpmtElecConfigSvc")
    spmtelecconfigsvc.property("ConfigFile").set("/sps/juno/llabit/JUNO_J23.1.x/junosw/Examples/TestSpmtElecAlg/share/SpmtElecConfig.txt")

    pmtparamsvc = task.createSvc("PMTParamSvc")


    alg = task.createAlg("SNiPERToPlainTree/alg_example")
    alg.property("enableIBDSelection").set(args.enableIBDSelection)
    alg.property("saveCalib").set(args.saveCalib)
    alg.property("saveBiPo").set(args.saveBiPo)
    
    import RootIOSvc
    import RootIOTools
    inputs = []
    input_corr = []
    inputsvc = task.createSvc("RootInputSvc/InputSvc")
    if(args.input_list):
        # if not os.path.exists(args.input_list):
        #     sys.exit(-1)
        with open (args.input_list) as f:
            for line in f:
                newline = line.strip()
                inputs.append(newline)
    else:
        inputs.append(args.input[0])
    print(inputs)
    inputsvc.property("InputFile").set(inputs)
    
    if (args.input_correlations_list):
        with open (args.input_correlations_list) as f:
            for line in f: 
                newline = line.strip()
                input_corr.append(newline)
    else:
        input_corr.append(args.input_correlations[0])

    inputsvc.property("InputCorrelationFile").set(input_corr)
    

    import RootWriter
    rootwriter = task.createSvc("RootWriter")
    rootwriter.property("Output").set({"Data":args.output})

    import OECComJSONSvc
    oeccomjsonsvc = task.createSvc("OECComJSONSvc")

    import OECTagSvc
    oectagsvc = task.createSvc("OECTagSvc")




    task.setEvtMax(-1)
    task.show()
    task.run()
