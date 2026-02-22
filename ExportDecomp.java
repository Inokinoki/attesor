// Export decompiled functions to a file
import ghidra.app.decompiler.DecompInterface;
import ghidra.util.task.ConsoleTaskMonitor;
import ghidra.program.model.listing.*;
import java.io.*;

import ghidra.app.script.GhidraScript;

public class ExportDecomp extends GhidraScript {
    @Override
    public void run() throws Exception {
        Program program = currentProgram;
        String programName = program.getName();

        String outputDir = "/Users/inoki/Builds/srcs/Rosetta2";
        File outputFile = new File(outputDir, programName + "_decomp.c");

        DecompInterface decompInterface = new DecompInterface();
        decompInterface.openProgram(program);

        ConsoleTaskMonitor monitor = new ConsoleTaskMonitor();

        try (PrintWriter writer = new PrintWriter(new FileWriter(outputFile))) {
            writer.println("// Decompiled code for " + programName);
            writer.println("// Language: " + program.getLanguageID());
            writer.println();

            for (Function func : program.getFunctionManager().getFunctions(true)) {
                var result = decompInterface.decompileFunction(func, 0, monitor);
                if (result.decompileCompleted()) {
                    writer.println(result.getDecompiledFunction().getC());
                    writer.println();
                }
            }

            println("Decompiled code written to " + outputFile.getAbsolutePath());
        } catch (IOException e) {
            e.printStackTrace();
        }

        decompInterface.dispose();
    }
}
