# Export decompiled functions to a file
from ghidra.app.decompiler import DecompInterface
from ghidra.util.task import ConsoleTaskMonitor
import os

output_dir = "/Users/inoki/Builds/srcs/Rosetta2"
program = currentProgram
program_name = program.getName()

# Create output file
output_file = os.path.join(output_dir, f"{program_name}_decomp.c")

decomp_interface = DecompInterface()
decomp_interface.openProgram(program)

monitor = ConsoleTaskMonitor()

with open(output_file, "w") as f:
    f.write(f"// Decompiled code for {program_name}\n")
    f.write(f"// Language: {program.getLanguageID()}\n\n")

    functions = program.getFunctionManager().getFunctions(True)
    for func in functions:
        result = decomp_interface.decompileFunction(func, 0, monitor)
        if result.decompileCompleted():
            decompiled_code = result.getDecompiledFunction().getC()
            f.write(decompiled_code)
            f.write("\n\n")

print(f"Decompiled code written to {output_file}")
decomp_interface.dispose()
