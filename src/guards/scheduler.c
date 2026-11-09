
//turn-taking mechanism, not a thread: programmers register their request
//here and check "is it my turn?" (fifo = arrival order, edf = earliest
//burnout deadline, deterministic tie-break). while it is not their turn
//they sleep on a cond, woken when a dongle is released or a request arrives
