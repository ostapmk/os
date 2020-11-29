// run() is called from Scheduling.main() and is where
// the scheduling algorithm written by the user resides.
// User modification should occur within the run() function.

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PrintStream;
import java.util.Comparator;
import java.util.EnumMap;
import java.util.LinkedList;
import java.util.PriorityQueue;
import java.util.Random;
import java.util.Vector;

final class SuspendedProcess {
  public final int suspendedUntil;
  public final Process process;

  public SuspendedProcess(final int suspendedUntil, final Process process) {
    this.suspendedUntil = suspendedUntil;
    this.process = process;
  }
};

public class SchedulingAlgorithm {

  public static Results run(final int runTime, final Vector<Process> input) {
    var result = new Results();
    result.schedulingType = "Preemptive";
    result.schedulingName = "Multilevel Queue";

    final String resultsFile = "Summary-Processes";

    var processes = new LinkedList<Process>();
    input.forEach((process) -> { processes.add(process); });

    var queues = new EnumMap<Process.Type, PriorityQueue<SuspendedProcess>>(Process.Type.class);
    try (var out = new PrintStream(new FileOutputStream(resultsFile))) {
      while (!(processes.isEmpty() && queues.isEmpty()) && (result.totalTime < runTime)) {
        Process process = null;
        if (!processes.isEmpty()) {
          process = processes.peek();
        }

        Process suspended = null;
        for (var pair : queues.entrySet()) {
          var current = pair.getValue().peek();
          if (current.suspendedUntil <= result.totalTime && (suspended == null || current.process.type.getValue() < suspended.type.getValue())) {
            suspended = current.process;
          }
        }

        if (process != null && suspended != null) {
          if (suspended.type.getValue() < process.type.getValue()) {
            process = suspended;
            queues.get(process.type).remove();
            if (queues.get(process.type).isEmpty()) {
              queues.remove(process.type);
            }
          } else {
            processes.remove();
          }
        } else if (process != null) {
          processes.remove();
        } else if (suspended != null) {
          process = suspended;
          queues.get(process.type).remove();
          if (queues.get(process.type).isEmpty()) {
            queues.remove(process.type);
          }
        } else {
          ++result.totalTime;
          continue;
        }

        out.printf("Process[%s]: %d registered... (%d, %d, %d)\n", process.type, process.id, process.cpuTime, process.ioBlockIn, process.cpuDone);

        final var step = Math.min(Math.min(process.ioBlockIn, runTime - result.totalTime), process.cpuTime - process.cpuDone);
        process.cpuDone += step;
        result.totalTime += step;

        if (process.cpuTime == process.cpuDone) {
          out.printf("Process[%s]: %d completed... (%d, %d, %d)\n", process.type, process.id, process.cpuTime, process.ioBlockIn, process.cpuDone);
        } else {
          ++process.numBlocked;
          final var blockFor = new Random().nextInt(100);

          out.printf("Process[%s]: %d I/O blocked... (%d, %d, %d, %d)\n", process.type, process.id, process.cpuTime, process.ioBlockIn, process.cpuDone, blockFor);

          if (queues.get(process.type) == null) {
            Comparator<SuspendedProcess> comp = (lhs, rhs) -> {return  Integer.compare(lhs.suspendedUntil, rhs.suspendedUntil);};
            queues.put(process.type, new PriorityQueue<>(comp));
          }

          // Suspend process for some time since operation is blocking
          queues.get(process.type).add(new SuspendedProcess(blockFor + result.totalTime, process));
        }
      }
    } catch (IOException e) {
      /* Handle exceptions */
    }

    return result;
  }
}
