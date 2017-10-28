import numpy as np

load_avg = 3 # assumed
nice = np.array([0, 1, 2])
recent_cpu = np.array([0, 0, 0])
ticks = 0
running = None

letters = ['A', 'B', 'C']

for i in range(0, 10):
    priority = 63 - (recent_cpu / 4) - (nice * 2)
    # print(i, 'priority after %d ticks' % (i * 4), priority)
    priority = priority.astype(np.int32)
    thread_to_run = np.argmax(priority)
    ## shady stuff
    if i == 3:
        thread_to_run = 1
    if i == 7:
        thread_to_run = 1
    ## 
    # print(i, 'thread to run:', thread_to_run)
    recent_cpu[thread_to_run] += 4
    recent_cpu = ((2 * load_avg) / (2 * load_avg + 1)) * recent_cpu + nice
    print(i*4, recent_cpu.astype(np.int32), priority, letters[thread_to_run])
