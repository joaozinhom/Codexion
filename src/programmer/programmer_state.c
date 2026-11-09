//one writer per piece of state: each programmer updates his own variables
//here, under his own mutex, so observers (the watchdog, the turn check)
//always read a consistent value