// This file contains the main() function for the Scheduling
// simulation.  Init() initializes most of the variables by
// reading from a provided file.  SchedulingAlgorithm.Run() is
// called from main() to run the simulation.  Summary-Results
// is where the summary results are written, and Summary-Processes
// is where the process scheduling summary is written.

// Created by Alexander Reeder, 2001 January 06

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.PrintStream;
import java.util.Random;
import java.util.StringTokenizer;
import java.util.Vector;

public class Scheduling {
  private static int processId = 0;
  private static int processnum = 5;
  private static int meanDev = 1000;
  private static int standardDev = 100;
  private static int runtime = 1000;
  private static Vector<Process> processes = new Vector<>();
  private static String resultsFile = "Summary-Results";

  private static void init(String file) {
    var f = new File(file);
    try (var in = new BufferedReader(new FileReader(f))) {
      for (var line = in.readLine(); line != null; line = in.readLine()) {
        if (line.startsWith("numprocess")) {
          var st = new StringTokenizer(line);
          st.nextToken();
          processnum = Common.s2i(st.nextToken());
        }
        if (line.startsWith("meandev")) {
          StringTokenizer st = new StringTokenizer(line);
          st.nextToken();
          meanDev = Common.s2i(st.nextToken());
        }
        if (line.startsWith("standdev")) {
          StringTokenizer st = new StringTokenizer(line);
          st.nextToken();
          standardDev = Common.s2i(st.nextToken());
        }
        if (line.startsWith("process")) {
          var tokens = new StringTokenizer(line);
          tokens.nextToken();
          final var ioblocking = Common.s2i(tokens.nextToken());
          final var type = Process.Type.valueOf(tokens.nextToken());
          var X = Common.R1();
          while (X == -1.0) {
            X = Common.R1();
          }
          X = X * standardDev;
          final var cputime = (int) X + meanDev;
          processes.add(new Process(++processId, cputime, ioblocking, type));
        }
        if (line.startsWith("runtime")) {
          StringTokenizer st = new StringTokenizer(line);
          st.nextToken();
          runtime = Common.s2i(st.nextToken());
        }
      }
    } catch (IOException e) {
      /* Handle exceptions */
    }
  }

  public static void main(String[] args) {
    if (args.length != 1) {
      System.out.println("Usage: 'java Scheduling <INIT FILE>'");
      System.exit(-1);
    }
    var f = new File(args[0]);
    if (!(f.exists())) {
      System.out.println("Scheduling: error, file '" + f.getName() + "' does not exist.");
      System.exit(-1);
    }
    if (!(f.canRead())) {
      System.out.println("Scheduling: error, read of " + f.getName() + " failed.");
      System.exit(-1);
    }
    System.out.println("Working...");
    init(args[0]);
    if (processes.size() < processnum) {
      for (var i = 0; processes.size() < processnum; ++i) {
        double X = Common.R1();
        while (X == -1.0) {
          X = Common.R1();
        }
        X = X * standardDev;
        final int cputime = (int) X + meanDev;
        final var type = Process.Type.values()[new Random().nextInt(Process.Type.values().length)];
        processes.add(new Process(++processId, cputime, i * 100, type));
      }
    }
    final var result = SchedulingAlgorithm.run(runtime, processes);
    try (var out = new PrintStream(new FileOutputStream(resultsFile));) {
      out.println("Scheduling Type: " + result.schedulingType);
      out.println("Scheduling Name: " + result.schedulingName);
      out.println("Simulation Run Time: " + result.totalTime);
      out.println("Mean: " + meanDev);
      out.println("Standard Deviation: " + standardDev);
      out.println("Process #\tType\tCPU Time\tIO Blocking\tCPU Completed\tCPU Blocked");
      for (var i = 0; i < processes.size(); ++i) {
        var process = processes.elementAt(i);
        out.print(Integer.toString(process.id));
        if (i < 100) {
          out.print("\t\t");
        } else {
          out.print("\t");
        }
        out.printf("%s\t", process.type);
        out.print(Integer.toString(process.cpuTime));
        if (process.cpuTime < 100) {
          out.print(" (ms)\t\t");
        } else {
          out.print(" (ms)\t");
        }
        out.print(Integer.toString(process.ioBlockIn));
        if (process.ioBlockIn < 100) {
          out.print(" (ms)\t\t");
        } else {
          out.print(" (ms)\t");
        }
        out.print(Integer.toString(process.cpuDone));
        if (process.cpuDone < 100) {
          out.print(" (ms)\t\t");
        } else {
          out.print(" (ms)\t");
        }
        out.println(process.numBlocked + " times");
      }
    } catch (IOException e) {
      /* Handle exceptions */ 
    }
    System.out.println("Completed.");
  }
}
