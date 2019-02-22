#!/usr/bin/python
import os
import sys
import subprocess

import numpy as np
import numpy.ma as ma
from scipy import stats

import math

if "DISPLAY" not in os.environ:
    import matplotlib as mpl
    mpl.use('Agg')

stream = False
showFig = True
log = False

import matplotlib.pyplot as plt

from time import localtime, strftime

from decimal import Decimal

def getDefines(options):

    defines = ""

    for opt,val in options.iteritems():

        if opt == 'skeleton':
            defines += " -D"+val.upper()
            continue

        if opt == 'problem':
            defines += " -D"+val.upper()
            continue

        if val:
            defines += " -D"+opt.upper()+"="+val.upper()
        else:
            defines += " -D"+opt.upper()

    return defines

def execCommand(command):

    output = ""
    process = subprocess.Popen(command.split(), stdout=subprocess.PIPE)
    for line in iter(process.stdout.readline, ''):  # replace '' with b'' for Python 3
        if stream:
            sys.stdout.write(line)
        output += line

    # output = subprocess.check_output(command.split(), stderr=subprocess.PIPE)

    return output

def getCollatzSteps(seed):

    s = 0
    n = seed

    while(n!=1):
        s += 1
        if n % 2:
            n = 3 * n + 1
        else:
            n /= 2

    return s

def plotBenchResult(output,figureName):

    header = False

    readingRun = False

    results = {}

    runID = 0

    results["runs"] = []


    for line in output.split("\n"):

        if not line:
            continue

        if "#" in line:
            header = not header
            continue


        if header:

            key, value = filter(None,line.split("\t"))

            if "Number of Runs" in key:
                results["#RUNS"] = value
                continue

            if "Hardware Concurrency" in key:
                results["HWTHREADS"] = value
                continue

            if "Max Threads" in key:
                results["MTHREADS"] = value
                continue

            if "Items per Run" in key:
                results["#ITEMS"] = value
                continue

            if "Balanced" in key:
                results["BALANCED"] = value
                continue

            if "Collatz" in key:
                results["Collatz"] = value
                continue

        if "Benchmarking" in line:
            results["Skeleton"] = line.split(" ")[1]
            continue


        if line[0] == '-':
            if readingRun:
                runID += 1
                run = ([runID,thread_num,gain],[seqRun,seqAVG,skelRun,skelAVG])
                results["runs"].append( run )
            else:
                readingRun = True

            seqAVG = None
            seqRun = []

            skelAVG = None
            skelRun = []
            continue

        if readingRun:
            if "Sequential Run" in line:
                seqRun.append( filter(None, line.split("\t"))[1] )
                continue
            if "Sequential took" in line:
                seqAVG = filter(None, line.split("\t"))[1]
                continue
            if "Skeleton Run" in line:
                skelRun.append(filter(None, line.split("\t"))[1])
                continue
            if "Skeleton took" in line:
                skelAVG = filter(None, line.split("\t"))[1]
                continue
            if "Gain" in line:
                gain = filter(None, line.split("\t"))[1]
                continue
            if "Number of threads" in line:
                thread_num = filter(None, line.split("\t"))[1]
                continue

    if readingRun:
        runID += 1
        run = ([runID,thread_num,gain], [seqRun, seqAVG, skelRun, skelAVG])
        results["runs"].append( run )


        threads = []
        skeletonRunTimes = []
        skeletonAVGTime = []
        skeletonSpeedup = []
        sequentialRunTimes = [ float(x) for x in results["runs"][0][1][0] ]
        sequentialAVGTime = float(results["runs"][0][1][1])

        for run in results["runs"]:
            threads.append( int(run[0][1]) )
            skeletonRunTimes.append( [float(x) for x in run[1][2]] )
            skeletonAVGTime.append( float(run[1][3]) )
            skeletonSpeedup.append( float(run[0][2]) )


        seqError = stats.sem( sequentialRunTimes )
        skelError = [ stats.sem(x) for x in skeletonRunTimes ]

        x=range(1,len(threads)+1)

        ax1 = plt.gca()
        ax2 = ax1.twinx()

        ax1Color = "#b35900"
        ax1.bar(x,skeletonSpeedup, 0.15, color=ax1Color, zorder=3 )
        ax1.tick_params(axis='y', labelcolor=ax1Color)

        ax2.errorbar([1],sequentialAVGTime, yerr=seqError, marker="o", label="Sequential", ecolor="red", barsabove=True, color='lime')
        p2 = ax2.errorbar(x,skeletonAVGTime, yerr=skelError, marker="o", label="Skeleton", barsabove=True, ecolor="red" )
        ax2Color = p2[0].get_color()
        ax2.tick_params(axis='y', labelcolor=ax2Color)



        plt.xticks(x,[str(x) for x in threads])


        cores = None
        sockets = None
        for l in os.popen('lscpu').readlines():
            if "Core(s) per socket:" in l:
                cores = l.split(":")[1]
                continue
            if "Socket(s)" in l:
                sockets = l.split(":")[1]
                continue
        text = "Hardware Concurrency: "+results["HWTHREADS"]
        if cores and sockets:
            text += " ( Cores: "+cores.strip()+" , Sockets: "+sockets.strip()+" )"
        text += "\nNumber of Runs for AVG: "+results["#RUNS"]
        text += "\nNumber of Elements per Run: "+('%.E' % Decimal(results["#ITEMS"]))
        text += "\nCollatz seed: "+results["Collatz"] +" ( " + str(getCollatzSteps(int(results["Collatz"]))) + " steps )"
        props = dict(boxstyle='round', facecolor='white', alpha=0.5)



        plt.text(0.6, 0.09, text, fontsize=9, transform=plt.gcf().transFigure, bbox=props)
        plt.subplots_adjust(bottom=0.3)

        plt.legend(bbox_to_anchor=(1.01,0.015), loc="upper left", fontsize=9)
        plt.title(results["Skeleton"].title() + " Skeleton Benchmark " + ("(Balanced)" if results["BALANCED"]=="YES" else "(Unbalanced)" ) )
        plt.xlabel("Threads")
        ax1.set_ylabel("Speedup", color=ax1Color, fontsize=10)
        ax2.set_ylabel("AVG Execution Time (sec)", color=ax2Color,fontsize=10)
        ax1.set_xlabel("Threads", fontsize=10)
        ax1.grid(zorder=0)
        plt.margins(0.1)
        fg = plt.gcf()
        fg.savefig(figureName+'.pdf',bbox_inches='tight')
        if showFig:
            plt.show()


def plotSummedAreaTiming(output, figureName):

    readingData = False

    results = {}

    data = None

    sizeOfSumAreaTable = None

    for line in output.split("\n"):

        if not line:
            continue

        if "#" in line:

            if readingData:

                if numOfSimpSamples not in data:
                    data[numOfSimpSamples] = [[],[],[],[],[],[]]

                data[numOfSimpSamples][0].append(skelTime)
                data[numOfSimpSamples][1].append(seqTime)
                data[numOfSimpSamples][2].append(seqNonParTime)
                data[numOfSimpSamples][3].append(seqGain)
                data[numOfSimpSamples][4].append(pthreadGain)
                data[numOfSimpSamples][5].append(pthreadTime)

                if sizeOfSumAreaTable not in SATSize:
                    SATSize.append( sizeOfSumAreaTable )



            else:
                readingData = True
                data = {}
                SATSize = []
            continue



        if "Problem" in line:
            results["Problem"] = line.split(" ")[1]
            continue

        if "Summed-Area problem with" in line:
            continue

        line = filter(None,line.split("\t"))
        if len(line) == 2 :
            key, value = line
        else:
            continue

        if "Skeleton Time:" in key:
            skelTime = float(value)
            continue

        if "Sequential Time:" in key:
            seqTime = float(value)
            continue

        if "PThread Time:" in key:
            pthreadTime = float(value)
            continue

        if "Seq. Non-Parallel Time:" in key:
            seqNonParTime = float(value)
            continue

        if "Number of simpson samples" in key:
            numOfSimpSamples = int(value)
            continue

        if "Number of Threads" in key:
            results["NTHREADS"] = value
            continue

        if "Hardware Concurrency" in key:
            results["HWTHREADS"] = value
            continue

        if "Number of Runs" in key:
            numOfRuns = value
            continue

        if "Size of summed-area table" in key:
            sizeOfSumAreaTable = int(value)
            continue

        if "Seq Gain" in key:
            seqGain = float(value)

        if "PThread Gain" in key:
            pthreadGain = float(value)

        continue

    if readingData:

        if numOfSimpSamples not in data:
            data[numOfSimpSamples] = [[],[],[],[],[],[]]

        data[numOfSimpSamples][0].append(skelTime)
        data[numOfSimpSamples][1].append(seqTime)
        data[numOfSimpSamples][2].append(seqNonParTime)
        data[numOfSimpSamples][3].append(seqGain)
        data[numOfSimpSamples][4].append(pthreadGain)
        data[numOfSimpSamples][5].append(pthreadTime)



        if sizeOfSumAreaTable not in SATSize:
            SATSize.append( sizeOfSumAreaTable )

        ax = plt.gca()

        x=np.arange(1,len(SATSize)+1)

        pointsGrpNum = len( data )
        clusterBarWidth = 0.5
        barWidth = clusterBarWidth/pointsGrpNum
        shft = -barWidth*(pointsGrpNum-1)/2.0

        ax.grid(zorder=0)

        for SimpsonSamples,v in sorted(data.iteritems()):

            SimpsonSamples = '%.E' % Decimal(SimpsonSamples)
            seqGain = np.array(v[3])
            pthreadGain = np.array(v[4])
            pthreadOverSeqGain = np.array(v[1]) / np.array(v[5])

            amdahlBestTime = np.array(v[2]) + (np.array(v[1])-np.array(v[2])) / float(results["NTHREADS"])
            bestPossibleSpeedUp = np.array(v[1]) / amdahlBestTime

            mask1 = ma.where(seqGain>=pthreadOverSeqGain)
            mask2 = ma.where(pthreadOverSeqGain>=seqGain)

            b = None

            if mask1[0].size:
                tempSeqGain = np.zeros(seqGain.size)
                tempSeqGain[mask1] = seqGain[mask1]
                if mask2[0].size:
                    b2 = ax.bar( x+shft, tempSeqGain, barWidth, zorder=3 )
                else:
                    b2 = ax.bar( x+shft, tempSeqGain, barWidth, label=str(SimpsonSamples)+" Simpson Subintervals (Skel. Speedup)", zorder=3 )

            b = ax.bar( x+shft, pthreadOverSeqGain, barWidth, label=str(SimpsonSamples)+" Simpson Subintervals (Pthr. Speedup)", zorder=3 )

            if mask2[0].size:
                tempSeqGain = np.zeros(seqGain.size)
                tempSeqGain[mask2] = seqGain[mask2]
                if mask1[0].size:
                    b = ax.bar( x+shft, tempSeqGain, barWidth, zorder=3, label=str(SimpsonSamples)+" Simpson Subintervals (Skel. Speedup)", color=b2.patches[0].get_facecolor() )
                else:
                    b = ax.bar( x+shft, tempSeqGain, barWidth, label=str(SimpsonSamples)+" Simpson Subintervals (Skel. Speedup)", zorder=3 )

            shft += barWidth

            x_start = np.array([plt.getp(item, 'x') for item in b])
            x_end   = x_start+[plt.getp(item, 'width') for item in b]
            plt.hlines(bestPossibleSpeedUp, x_start, x_end)
        ax.legend()

        xtcks = [('%.E' % Decimal(sas)) for sas in SATSize]
        plt.xticks(x,xtcks)
        cores = None
        sockets = None
        for l in os.popen('lscpu').readlines():
            if "Core(s) per socket:" in l:
                cores = l.split(":")[1]
                continue
            if "Socket(s)" in l:
                sockets = l.split(":")[1]
                continue

        text = "Number of Threads: "+results["NTHREADS"]
        text += "\nHardware Concurrency: "+results["HWTHREADS"]
        if cores and sockets:
            text += " ( Cores: "+cores.strip()+" , Sockets: "+sockets.strip()+" )"
        text += "\nNumber of Runs for AVG: "+numOfRuns
        props = dict(boxstyle='round', facecolor='white', alpha=0.5)



        plt.text(0.91, 0.3, text, fontsize=9, transform=plt.gcf().transFigure, bbox=props)
        plt.subplots_adjust(bottom=0.3)

        plt.legend(bbox_to_anchor=(1,1), loc="upper left", fontsize=9)
        plt.title("Cumulated Function Problem Benchmark")
        plt.xlabel("Size of Summed-Area Table")
        plt.ylabel("Speedup" )
        plt.margins(0.1)
        fg = plt.gcf()
        fg.savefig(figureName+'.svg',bbox_inches='tight')
        fg.savefig(figureName+'.pdf',bbox_inches='tight')
        if showFig:
            plt.show()


def plotKMeansData(output, figureName):

    header = False

    readingData = False

    results = {}

    results["initial"] = []


    for line in output.split("\n"):

        if not line:
            continue

        if "#" in line:
            header = not header
            continue


        if header:

            key, value = filter(None,line.split("\t"))

            if "K-Means problem with" in key:
                continue

            if "Number of 2-D Points" in key:
                results["#POINTS"] = value
                continue

            if "Number of Threads" in key:
                results["NTHREADS"] = value
                continue

            if "Hardware Concurrency" in key:
                results["HWTHREADS"] = value
                continue

            if "Number of Clusters" in key:
                results["#CLUSTERS"] = value
                continue

            if "Collatz" in key:
                results["Collatz"] = value
                continue

        if "Problem" in line:
            results["Problem"] = line.split(" ")[1]
            continue


        if line[0] == '-':
            readingData = True

            clusters = {}

            continue

        if readingData:
            point,cluster = line.replace("(","").replace(")","").split("->")

            p_x,p_y = map(float,point.split(','))
            c_x,c_y = map(float,cluster.split(','))

            if (c_x,c_y) not in clusters:
                clusters[(c_x, c_y)] = []

            clusters[(c_x, c_y)].append((p_x, p_y))

            continue

    if readingData:
        results["results"] = clusters

        import matplotlib.cm as cm

        if "DISPLAY" not in os.environ:
            colors = cm.rainbow(np.linspace(0, 1, len(results["results"])))

            for count, (c,p) in enumerate(results["results"].iteritems(), 1):
                points = p
                plt.scatter( [x[0] for x in points],[x[1] for x in points], c=colors[count-1], s=5 )
        else:
            for c,p in results["results"].iteritems():
                points = p
                plt.scatter( [x[0] for x in points],[x[1] for x in points], s=5 )

        for c,p in results["results"].iteritems():
            plt.scatter(c[0],c[1], c='black', marker='x', s=50)

        plt.margins(0.1)

        fg = plt.gcf()
        fg.savefig(figureName+'.pdf',bbox_inches='tight')
        if showFig:
            plt.show()


def plotKMeansTiming(output, figureName):

    readingData = False

    results = {}

    data = None

    numOfClusters = None

    for line in output.split("\n"):

        if not line:
            continue

        if "#" in line:

            if readingData:

                if numOfPoints not in data:
                    data[numOfPoints] = [[],[],[],[],[],[]]

                data[numOfPoints][0].append(skelTime)
                data[numOfPoints][1].append(seqTime)
                data[numOfPoints][2].append(seqNonParTime)
                data[numOfPoints][3].append(seqGain)
                data[numOfPoints][4].append(pthreadGain)
                data[numOfPoints][5].append(pthreadTime)

                if numOfClusters not in clusters:
                    clusters.append( numOfClusters )



            else:
                readingData = True
                data = {}
                clusters = []
            continue


        if "Problem" in line:
            results["Problem"] = line.split(" ")[1]
            continue

        if "K-Means problem with" in line:
            continue

        line = filter(None,line.split("\t"))
        if len(line) == 2 :
            key, value = line
        else:
            continue

        if "Skeleton Time:" in key:
            skelTime = float(value)
            continue

        if "Sequential Time:" in key:
            seqTime = float(value)
            continue

        if "PThread Time:" in key:
            pthreadTime = float(value)
            continue

        if "Seq. Non-Parallel Time:" in key:
            seqNonParTime = float(value)
            continue

        if "Number of 2-D Points" in key:
            numOfPoints = int(value)
            continue

        if "Number of Threads" in key:
            results["NTHREADS"] = value
            continue

        if "Hardware Concurrency" in key:
            results["HWTHREADS"] = value
            continue

        if "Number of Runs" in key:
            numOfRuns = value
            continue

        if "Number of Clusters" in key:
            numOfClusters = int(value)
            continue

        if "Seq Gain" in key:
            seqGain = float(value)

        if "PThread Gain" in key:
            pthreadGain = float(value)

        continue

    if readingData:

        if numOfPoints not in data:
            data[numOfPoints] = [[],[],[],[],[],[]]

        data[numOfPoints][0].append(skelTime)
        data[numOfPoints][1].append(seqTime)
        data[numOfPoints][2].append(seqNonParTime)
        data[numOfPoints][3].append(seqGain)
        data[numOfPoints][4].append(pthreadGain)
        data[numOfPoints][5].append(pthreadTime)



        if numOfClusters not in clusters:
            clusters.append( numOfClusters )

        ax = plt.gca()

        x=np.array(clusters)

        pointsGrpNum = len( data )
        clusterBarWidth = 0.5
        barWidth = clusterBarWidth/pointsGrpNum
        shft = -barWidth*(pointsGrpNum-1)/2.0

        ax.grid(zorder=0)

        for points,v in sorted(data.iteritems()):

            points = '%.E' % Decimal(points)
            seqGain = np.array(v[3])
            pthreadGain = np.array(v[4])
            pthreadOverSeqGain = np.array(v[1]) / np.array(v[5])

            amdahlBestTime = np.array(v[2]) + (np.array(v[1])-np.array(v[2])) / float(results["NTHREADS"])
            bestPossibleSpeedUp = np.array(v[1]) / amdahlBestTime

            mask1 = ma.where(seqGain>=pthreadOverSeqGain)
            mask2 = ma.where(pthreadOverSeqGain>=seqGain)

            b = None

            if mask1[0].size:
                tempSeqGain = np.zeros(seqGain.size)
                tempSeqGain[mask1] = seqGain[mask1]
                b2 = ax.bar( x+shft, tempSeqGain, barWidth, label=str(points)+" Points (Skel. Speedup)", zorder=3 )

            b = ax.bar( x+shft, pthreadOverSeqGain, barWidth, label=str(points)+" Points (Pthr. Speedup)", zorder=3 )

            if mask2[0].size:
                tempSeqGain = np.zeros(seqGain.size)
                tempSeqGain[mask2] = seqGain[mask2]
                if mask1[0].size:
                    b = ax.bar( x+shft, tempSeqGain, barWidth, zorder=3, label=str(points)+" Points (Skel. Speedup)", color=b2.patches[0].get_facecolor() )
                else:
                    b = ax.bar( x+shft, tempSeqGain, barWidth, label=str(points)+" Points (Skel. Speedup)", zorder=3 )

            shft += barWidth

            x_start = np.array([plt.getp(item, 'x') for item in b])
            x_end   = x_start+[plt.getp(item, 'width') for item in b]
            plt.hlines(bestPossibleSpeedUp, x_start, x_end)
        ax.legend()

        plt.xticks(clusters)
        cores = None
        sockets = None
        for l in os.popen('lscpu').readlines():
            if "Core(s) per socket:" in l:
                cores = l.split(":")[1]
                continue
            if "Socket(s)" in l:
                sockets = l.split(":")[1]
                continue

        text = "Number of Threads: "+results["NTHREADS"]
        text += "\nHardware Concurrency: "+results["HWTHREADS"]
        if cores and sockets:
            text += " ( Cores: "+cores.strip()+" , Sockets: "+sockets.strip()+" )"
        text += "\nNumber of Runs for AVG: "+numOfRuns
        props = dict(boxstyle='round', facecolor='white', alpha=0.5)



        plt.text(0.91, 0.3, text, fontsize=9, transform=plt.gcf().transFigure, bbox=props)
        plt.subplots_adjust(bottom=0.3)

        plt.legend(bbox_to_anchor=(1,1), loc="upper left", fontsize=9)
        plt.title(results["Problem"].title() + " Problem Benchmark")
        plt.xlabel("Number of Clusters")
        plt.ylabel("Speedup" )
        plt.margins(0.1)
        fg = plt.gcf()
        fg.savefig(figureName+'.svg',bbox_inches='tight')
        fg.savefig(figureName+'.pdf',bbox_inches='tight')
        if showFig:
            plt.show()

def testBench(options):

    name = options['skeleton'].title()+"_"+strftime("%d-%m_%H:%M", localtime())
    figureName = "figure"+name

    for opt in sys.argv[2:]:

        if "figure=" in opt:
            name = options['skeleton'].title()+"_"+opt.split("=")[1]
            figureName = "figure"+name
            continue

        if "=" in opt:
            opt,value = opt.split('=')

            options[opt] = value
            continue
        else:
            options[opt] = None
            continue

    cppFiles = "test"+options['skeleton']+".cpp miscFun.cpp"
    if os.path.exists("ThreadPool.cpp"):
        cppFiles += " ThreadPool.cpp Task.cpp"

    defines = getDefines(options)


    outputFile = options['skeleton'].lower() + ".test"
    objectFile = " -o " + outputFile
    libOption = " -std=c++11 -lpthread "

    bashCommand = "g++ main.cpp " + cppFiles + libOption + defines + objectFile


    print(bashCommand)


    if( execCommand(bashCommand) == False ):
        exit(-1)

    execOutputFile = "./" + outputFile

    output = execCommand(execOutputFile)

    if output:

        if log:
            logName="log"+name
            f = open(logName+".txt", "w")
            f.write(output)

        plotBenchResult(output,figureName)




def testProbl(options):

    name = options['problem'].title()+"_"+strftime("%d-%m_%H:%M", localtime())
    figureName = "figure"+name

    for opt in sys.argv[2:]:

        if "figure=" in opt:
            name = options['problem'].title()+"_"+opt.split("=")[1]
            figureName = "figure"+name
            continue

        if "=" in opt:
            opt,value = opt.split('=')

            options[opt] = value
            continue
        else:
            options[opt] = None
            continue

    cppFiles = "miscFun.cpp problem"+options['problem']+".cpp"
    if os.path.exists("ThreadPool.cpp"):
        cppFiles += " ThreadPool.cpp"

    defines = getDefines(options)

    outputFile = options['problem'].lower() + ".test"
    objectFile = " -o " + outputFile
    libOption = " -std=c++11 -lpthread"

    bashCommand = "g++ main.cpp " + cppFiles + libOption + defines + objectFile


    print(bashCommand)

    if( execCommand(bashCommand) == None ):
        exit(-1)

    execOutputFile = "./" + outputFile

    output = execCommand(execOutputFile)

    if output:

        if log:
            logName="log"+name
            f = open(logName+".txt", "w")
            f.write(output)

        if "timing" in options:
            if options['problem'] in ['Kmeans','Kmeans_Mr']:
                plotKMeansTiming(output, figureName)
            if options['problem'] in ['SummedArea']:
                plotSummedAreaTiming(output,figureName)

        else:
            plotKMeansData(output, figureName)








def main():

    skeletons = ["Map", "Reduce", "Mapreduce", "Scan", "Stencil", "Pipeline"]
    problems = ["Kmeans","Kmeans_Mr", "SummedArea"]

    options = {}


    if "stream" in sys.argv:
        global stream
        stream = True
        sys.argv.remove('stream')

    if "noshow" in sys.argv:
        global showFig
        showFig = False
        sys.argv.remove('noshow')

    if "log" in sys.argv:
        global log
        log = True
        sys.argv.remove('log')

    if "plot" == sys.argv[1]:
        logFile = sys.argv[3]
        logData = open(logFile,"r").read()

        figureName = logFile.replace("log","figure")
        figureName = figureName.replace(".txt","")

        if "bench" == sys.argv[2]:
            plotBenchResult(logData,figureName)
        if sys.argv[2] in ["kmeans","kmeans_mr"]:
            plotKMeansTiming(logData,figureName)
        if sys.argv[2] in ["SummedArea"]:
            plotSummedAreaTiming(logData,figureName)

        return


    if sys.argv[1].title() in skeletons:
        options['skeleton'] = sys.argv[1].title()

        if "Mapreduce" in options['skeleton']:
            options['skeleton'] = options['skeleton'][:3]+options['skeleton'][3].upper()+options['skeleton'][4:]

        testBench(options)

        return


    options['problem'] = sys.argv[1].title()

    if "Kmeans_Mr" in options['problem']:
        options['problem'] = options['problem'][:-1] + options['problem'][-1].upper()

    if "Summedarea" in options['problem']:
        options['problem'] = "SummedArea"

    testProbl(options)

    return





if __name__ == "__main__":
    main()






