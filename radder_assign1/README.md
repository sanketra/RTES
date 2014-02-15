
Assignment1-Scheduling periodic and aperiodic tasks, event handling.
====================================================================
To run this program:

1. Go to source directory on the terminal.
2. Type make and press enter. This creates and executable named task.
3. Run the code with the following command:
    
	sudo ./task ../test/input.txt

4. You can add different input files in the test directory and use the 
   same by changing above path.
5. To generate the trace files use:

   sudo trace-cmd record -e sched_switch ./task ../test/input.txt

6. Sample input, output and the traces files are included in test directory.

***IMP NOTE***
1. Run with sudo command.
2. In case if it is required to change the path of the keyboard device node, change the 
   constant 'KBD_PATH' defined in the task.h header file. 
3. The maximum number of task that can be created is defined by constant MAX_TASKS in 
   the task.h header file.
4. Directory test consists of sample output but all the printf in the code are commented
   in the submitted code.
